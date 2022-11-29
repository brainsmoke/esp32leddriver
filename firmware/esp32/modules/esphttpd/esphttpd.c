
#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include "py/mpthread.h"

#include "mphalport.h"

#include "freertos/queue.h"

#include <sys/socket.h>

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_https_server.h"

static const char *TAG = "_esphttpd";
#define LOG_FMT(x) "%s: " x, __func__

/* message passing micropython thread <-> server thread */

enum
{
	/* (request_msg_t).type */
	REQ_NEW,
	REQ_RETURN,
	REQ_QUIT,

	/* (response_msg_t).type */
	RESP_DONE,
	RESP_SEND,
	RESP_SEND_CHUNK,
/*	RESP_SEND_FILE, */
};

typedef struct
{
	int type;
	httpd_req_t *req;
	esp_err_t err;
	int pad;

} request_msg_t;

#define REQUEST_NEW(r)      (request_msg_t) { .type = REQ_NEW,    .req = (r)    }
#define REQUEST_QUIT()      (request_msg_t) { .type = REQ_QUIT,                 }
#define REQUEST_RETURN(ret) (request_msg_t) { .type = REQ_RETURN, .err = (ret), }

typedef struct
{
	int type;
	const char *buf;
	size_t size;
	esp_err_t err;

} response_msg_t;

#define RESPONSE_DONE(error) (response_msg_t) { .type = RESP_DONE, .err = (error), }

/* _esphttpd.Request helpers
 *
 */

typedef struct
{
	int code;
	const char *desc;

} int_str_map_t;

static const int_str_map_t status_code_map[] =
{
	{ 200, "200 OK"                    },
	{ 301, "301 Moved Permanently"     },
	{ 303, "303 See Other"             },
	{ 304, "304 Not Modified"          },
	{ 400, "400 Bad Request"           },
	{ 401, "401 Unauthorized"          },
	{ 403, "403 Forbidden"             },
	{ 404, "404 File Not Found"        },
	{ 500, "500 Internal Server Error" },
};

typedef struct
{
	int num;
	const char *str;
	mp_obj_t obj;

} int_str_obj_map_t;

static const int_str_obj_map_t methods[] =
{
	{ HTTP_GET,  "GET",  MP_ROM_QSTR(MP_QSTR_GET)   },
	{ HTTP_POST, "POST", MP_ROM_QSTR(MP_QSTR_POST)  },
	{ -1,         NULL,  mp_const_none                  },
};

static const char *status_code(int num)
{
	int base = 0, size = ( sizeof(status_code_map)/sizeof(status_code_map[0]) );

	while (size > 0)
	{
		int mid = size/2;
		if (status_code_map[base+mid].code == num)
			return status_code_map[base+mid].desc;
		else if (status_code_map[base+mid].code > num)
			size = mid;
		else
		{
			base += mid + 1;
			size -= mid + 1;
		}
	}
	return NULL;
}

static int method_const(const char *name)
{
	const int_str_obj_map_t *m;

	for (m = &methods[0]; m->str; m += 1)
		if ( strcasecmp(name, m->str) == 0 )
			return m->num;

	return -1;
}

static mp_obj_t method_str(int esp_const)
{
	const int_str_obj_map_t *m;

	for (m = &methods[0]; m->str; m += 1)
		if ( m->num == esp_const )
			return m->obj;

	return mp_const_none;
}

/*  */

typedef struct
{
	mp_obj_t ctx;
	int allocated;

} context_slot_t;

static void set_session_ctx(httpd_req_t *r, mp_obj_t sess_ctx)
{
	context_slot_t *slot = r->sess_ctx;
	slot->ctx = sess_ctx;
}

static mp_obj_t get_session_ctx(httpd_req_t *r)
{
	context_slot_t *slot = r->sess_ctx;
	return slot->ctx;
}

/* _esphttpd.Server struct declared here, since it is referenced in _esphttpd.Request
 *
 */

typedef struct
{
	mp_obj_base_t base;
	httpd_handle_t handle;

	QueueHandle_t request_queue,
				  response_queue;

	int max_headers;

	context_slot_t *slots;
	int n_slots;
	mp_obj_t protocol_string;

} esphttpd_server_obj_t;

static context_slot_t *new_context_slot_array(int n_slots)
{
	context_slot_t *arr = m_new(context_slot_t, n_slots);

	for (int i=0; i<n_slots; i++)
		arr[i] = (context_slot_t)
		{
			.ctx       = mp_const_none,
			.allocated = 0,
		};

	return arr;
}

static context_slot_t *alloc_context_slot(esphttpd_server_obj_t *server)
{
	ESP_LOGD(TAG, LOG_FMT("new session ctx"));
	for (int i=0; i<server->n_slots; i++)
	{
		context_slot_t *slot = &server->slots[i];

		if (!slot->allocated)
		{
			*slot = (context_slot_t)
			{
				.ctx       = mp_const_none,
				.allocated = 1,
			};
			return slot;
		}
	}

	ESP_LOGE(TAG, LOG_FMT("too many session contexts"));
	return NULL;
}

static void free_context_slot(void *ctx)
{
	ESP_LOGD(TAG, LOG_FMT("free session ctx"));
	context_slot_t *slot = ctx;
	*slot = (context_slot_t)
	{
		.ctx       = mp_const_none,
		.allocated = 0,
	};
}

/* _esphttpd.Request
 *
 */

typedef struct
{
	mp_obj_base_t base;
	httpd_req_t *req;
	int chunk_sent;
	int all_sent;
	mp_obj_t *no_gc_objects;
	int n_alloc;
	int n_used;

	esphttpd_server_obj_t *server;

} esphttpd_request_obj_t;


static void request_prevent_gc(esphttpd_request_obj_t *req, mp_obj_t obj)
{
	if (req->n_alloc == req->n_used)
		mp_raise_ValueError(MP_ERROR_TEXT("Too many response headers"));

	req->no_gc_objects[req->n_used++] = obj;
}

static void clear_request_obj(esphttpd_request_obj_t *req)
{
	for (int i=0; i<req->n_alloc; i++)
		req->no_gc_objects[i] = mp_const_none;

	req->n_used = 0;
	req->req = NULL;
	req->chunk_sent = 0;
	req->all_sent = 0;
}

static void esphttpd_request_print(const mp_print_t *print,
                                   mp_obj_t self_in,
                                   mp_print_kind_t kind)
{
	mp_printf(print, "<_esphttpd.Request XXX TODO Description XXX>");
}

/* _esphttpd.Request.set_status(int) */
static mp_obj_t esphttpd_request_set_status(mp_obj_t self_in, mp_obj_t num)
{
	esphttpd_request_obj_t *self = MP_OBJ_TO_PTR(self_in);

	if (self->req == NULL)
		mp_raise_ValueError(MP_ERROR_TEXT("request expired"));

	const char *status = status_code(mp_obj_get_int(num));

	if (!status)
		mp_raise_ValueError(MP_ERROR_TEXT("Unknown status"));

	check_esp_err(httpd_resp_set_status(self->req, status));

	return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2( esphttpd_request_set_status_obj, esphttpd_request_set_status );

/* _esphttpd.Request.set_session_ctx(ctx) */
static mp_obj_t esphttpd_request_set_session_ctx(mp_obj_t self_in, mp_obj_t sess_ctx)
{
	esphttpd_request_obj_t *self = MP_OBJ_TO_PTR(self_in);

	if (self->req == NULL)
		mp_raise_ValueError(MP_ERROR_TEXT("request expired"));

	set_session_ctx(self->req, sess_ctx);
	return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2( esphttpd_request_set_session_ctx_obj, esphttpd_request_set_session_ctx );

/* _esphttpd.Request.get_session_ctx() */
static mp_obj_t esphttpd_request_get_session_ctx(mp_obj_t self_in)
{
	esphttpd_request_obj_t *self = MP_OBJ_TO_PTR(self_in);

	if (self->req == NULL)
		mp_raise_ValueError(MP_ERROR_TEXT("request expired"));

	return get_session_ctx(self->req);
}
MP_DEFINE_CONST_FUN_OBJ_1( esphttpd_request_get_session_ctx_obj, esphttpd_request_get_session_ctx );

/* _esphttpd.Request.recv(buf) */
static mp_obj_t esphttpd_request_recv(mp_obj_t self_in, mp_obj_t buf_out)
{
	esphttpd_request_obj_t *self = MP_OBJ_TO_PTR(self_in);

	if (self->req == NULL)
		mp_raise_ValueError(MP_ERROR_TEXT("request expired"));

	mp_buffer_info_t bufinfo;
	mp_get_buffer_raise(buf_out, &bufinfo, MP_BUFFER_WRITE);
	int n_read = httpd_req_recv(self->req, bufinfo.buf, bufinfo.len);

	if (n_read < 0)
		check_esp_err(n_read);

	return mp_obj_new_int(n_read);
}
MP_DEFINE_CONST_FUN_OBJ_2( esphttpd_request_recv_obj, esphttpd_request_recv );

static esp_err_t write_offload(esphttpd_request_obj_t *req, const char *buf, size_t len, int is_chunked)
{
	MP_THREAD_GIL_EXIT();
	response_msg_t out_msg = (response_msg_t)
	{
		.type = is_chunked ? RESP_SEND_CHUNK : RESP_SEND,
		.buf  = buf,
		.size = len,
	};
	xQueueSend(req->server->response_queue, &out_msg, portMAX_DELAY);
	request_msg_t in_msg;
	xQueueReceive(req->server->request_queue, &in_msg, portMAX_DELAY);
	if (in_msg.type != REQ_RETURN)
		ESP_LOGE(TAG, LOG_FMT("desynchonized queue, in_msg.type = %d"), in_msg.type);

	MP_THREAD_GIL_ENTER();
	return in_msg.err;
}

static void end_request(esphttpd_request_obj_t *req)
{
	if (!req->all_sent)
	{
		req->all_sent = 1;
		char c[1];
		check_esp_err( write_offload(req, &c[0], 0, req->chunk_sent ) );
	}
}

/* _esphttpd.Request.write(buf) */
static mp_obj_t esphttpd_request_write(mp_obj_t self_in, mp_obj_t buf_in)
{
	esphttpd_request_obj_t *self = MP_OBJ_TO_PTR(self_in);

	if (self->req == NULL)
		mp_raise_ValueError(MP_ERROR_TEXT("request expired"));

	if (self->all_sent)
		mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Request.write() already finished sending document"));

	self->chunk_sent = 1;

	mp_buffer_info_t bufinfo;
	mp_get_buffer_raise(buf_in, &bufinfo, MP_BUFFER_READ);
	check_esp_err( write_offload(self, bufinfo.buf, bufinfo.len, true) );

	if (bufinfo.len == 0)
		self->all_sent = 1;

	return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2( esphttpd_request_write_obj, esphttpd_request_write );

/* _esphttpd.Request.write_all(buf) */
static mp_obj_t esphttpd_request_write_all(mp_obj_t self_in, mp_obj_t buf_in)
{
	esphttpd_request_obj_t *self = MP_OBJ_TO_PTR(self_in);

	if (self->req == NULL)
		mp_raise_ValueError(MP_ERROR_TEXT("request expired"));

	if (self->all_sent)
		mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Request.write_all() already finished sending document"));

	mp_buffer_info_t bufinfo;
	mp_get_buffer_raise(buf_in, &bufinfo, MP_BUFFER_READ);

	check_esp_err( write_offload(self, bufinfo.buf, bufinfo.len, self->chunk_sent) );

	if (self->chunk_sent)
		end_request(self);

	self->all_sent = 1;

	return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2( esphttpd_request_write_all_obj, esphttpd_request_write_all );

/* _esphttpd.Request.get_header(field) */
static mp_obj_t esphttpd_request_get_header(mp_obj_t self_in, mp_obj_t field_in)
{
	esphttpd_request_obj_t *self = MP_OBJ_TO_PTR(self_in);

	if (self->req == NULL)
		mp_raise_ValueError(MP_ERROR_TEXT("request expired"));

	vstr_t vstr;
	const char *field_name = mp_obj_str_get_str(field_in);
	size_t field_len = httpd_req_get_hdr_value_len(self->req, field_name);

	vstr_init(&vstr, field_len+1);
	if (field_len != 0)
		httpd_req_get_hdr_value_str(self->req, field_name, vstr.buf, field_len+1);
	vstr.len = field_len;
	return mp_obj_new_bytes_from_vstr(&vstr);
}
MP_DEFINE_CONST_FUN_OBJ_2( esphttpd_request_get_header_obj, esphttpd_request_get_header );

/* _esphttpd.Request.get_query_string() */
static mp_obj_t esphttpd_request_get_query_string(mp_obj_t self_in)
{
	esphttpd_request_obj_t *self = MP_OBJ_TO_PTR(self_in);

	if (self->req == NULL)
		mp_raise_ValueError(MP_ERROR_TEXT("request expired"));

	vstr_t vstr;
	size_t len = httpd_req_get_url_query_len(self->req);
	vstr_init(&vstr, len+1);
	httpd_req_get_url_query_str(self->req, vstr.buf, len+1);
	vstr.len = len;
	return mp_obj_new_bytes_from_vstr(&vstr);
}
MP_DEFINE_CONST_FUN_OBJ_1( esphttpd_request_get_query_string_obj, esphttpd_request_get_query_string );

/* _esphttpd.Request.get_path() */
static mp_obj_t esphttpd_request_get_path(mp_obj_t self_in)
{
	esphttpd_request_obj_t *self = MP_OBJ_TO_PTR(self_in);

	if (self->req == NULL)
		mp_raise_ValueError(MP_ERROR_TEXT("request expired"));

	const char *path = self->req->uri;
	const char *path_end = strchr(path, '?');
	size_t len = path_end ? (path - path_end) : strlen(path);
	return mp_obj_new_bytes((const uint8_t *)path, len);
}
MP_DEFINE_CONST_FUN_OBJ_1( esphttpd_request_get_path_obj, esphttpd_request_get_path );

enum { LOCAL_PEER, REMOTE_PEER };
enum { HOST_INFO, PORT_INFO };

/* _esphttpd.Request.get_{remote,server}_{host,port}() */
static mp_obj_t esphttpd_request_get_sockinfo(mp_obj_t self_in, int peer, int type)
{
	esphttpd_request_obj_t *self = MP_OBJ_TO_PTR(self_in);

	if (self->req == NULL)
		mp_raise_ValueError(MP_ERROR_TEXT("request expired"));

	int sockfd = httpd_req_to_sockfd(self->req);

	if (sockfd < 0)
		mp_raise_msg_varg(&mp_type_OSError, MP_ERROR_TEXT("cannot retrieve socket descriptor %d"), sockfd);

	struct sockaddr_in6 addr;
	socklen_t addr_len = sizeof(addr);

	int ret = ( (peer == REMOTE_PEER) ? getpeername:getsockname)(sockfd, (struct sockaddr *)&addr, &addr_len);

	if (ret < 0)
		mp_raise_msg_varg(&mp_type_OSError, MP_ERROR_TEXT("%s() failed %d"),
		                  (peer == REMOTE_PEER) ? "getpeername":"getsockname", errno);

	if ( (addr.sin6_family != PF_INET)  &&
	     (addr.sin6_family != PF_INET6) )
		mp_raise_msg_varg(&mp_type_OSError, MP_ERROR_TEXT("unsupported socket family %d"),
		                  addr.sin6_family);

	if (type == PORT_INFO)
		return mp_obj_new_int(ntohs(addr.sin6_port));
	else
	{
		vstr_t vstr_addr;
		vstr_init_len(&vstr_addr, 128);
		memset(vstr_addr.buf, 0, 128);

		if (addr.sin6_family == PF_INET)
		{
			if (!inet_ntoa_r( ((struct sockaddr_in *)&addr)->sin_addr.s_addr,
			                   vstr_addr.buf, vstr_addr.len - 1) )
				mp_raise_msg_varg(&mp_type_OSError, MP_ERROR_TEXT("error converting address %d"), errno);
		}
		else if (addr.sin6_family == PF_INET6)
		{
			if (!inet6_ntoa_r(addr.sin6_addr, vstr_addr.buf, vstr_addr.len - 1) )
				mp_raise_msg_varg(&mp_type_OSError, MP_ERROR_TEXT("error converting v6 address %d"), errno);
		}

		vstr_addr.len = strlen(vstr_addr.buf);
		return mp_obj_new_bytes_from_vstr(&vstr_addr);
	}
}

static mp_obj_t esphttpd_request_get_remote_host(mp_obj_t self_in)
{
	return esphttpd_request_get_sockinfo(self_in, REMOTE_PEER, HOST_INFO);
}
MP_DEFINE_CONST_FUN_OBJ_1( esphttpd_request_get_remote_host_obj, esphttpd_request_get_remote_host );

static mp_obj_t esphttpd_request_get_remote_port(mp_obj_t self_in)
{
	return esphttpd_request_get_sockinfo(self_in, REMOTE_PEER, PORT_INFO);
}
MP_DEFINE_CONST_FUN_OBJ_1( esphttpd_request_get_remote_port_obj, esphttpd_request_get_remote_port );

static mp_obj_t esphttpd_request_get_server_host(mp_obj_t self_in)
{
	return esphttpd_request_get_sockinfo(self_in, LOCAL_PEER, HOST_INFO);
}
MP_DEFINE_CONST_FUN_OBJ_1( esphttpd_request_get_server_host_obj, esphttpd_request_get_server_host );

static mp_obj_t esphttpd_request_get_server_port(mp_obj_t self_in)
{
	return esphttpd_request_get_sockinfo(self_in, LOCAL_PEER, PORT_INFO);
}
MP_DEFINE_CONST_FUN_OBJ_1( esphttpd_request_get_server_port_obj, esphttpd_request_get_server_port );

static mp_obj_t esphttpd_request_get_protocol(mp_obj_t self_in)
{
	esphttpd_request_obj_t *self = MP_OBJ_TO_PTR(self_in);

	if ( (self->req == NULL) || (self->server == NULL) )
		mp_raise_ValueError(MP_ERROR_TEXT("request expired"));

	return self->server->protocol_string;
}
MP_DEFINE_CONST_FUN_OBJ_1( esphttpd_request_get_protocol_obj, esphttpd_request_get_protocol );

/* _esphttpd.Request.get_uri() */
static mp_obj_t esphttpd_request_get_uri(mp_obj_t self_in)
{
	esphttpd_request_obj_t *self = MP_OBJ_TO_PTR(self_in);

	if (self->req == NULL)
		mp_raise_ValueError(MP_ERROR_TEXT("request expired"));

	return mp_obj_new_bytes((const uint8_t *)self->req->uri, strlen(self->req->uri));
}
MP_DEFINE_CONST_FUN_OBJ_1( esphttpd_request_get_uri_obj, esphttpd_request_get_uri );

/* _esphttpd.Request.content_len() */
static mp_obj_t esphttpd_request_content_len(mp_obj_t self_in)
{
	esphttpd_request_obj_t *self = MP_OBJ_TO_PTR(self_in);

	if (self->req == NULL)
		mp_raise_ValueError(MP_ERROR_TEXT("request expired"));

	return mp_obj_new_int(self->req->content_len);
}
MP_DEFINE_CONST_FUN_OBJ_1( esphttpd_request_content_len_obj, esphttpd_request_content_len );

/* _esphttpd.Request.method() */
static mp_obj_t esphttpd_request_method(mp_obj_t self_in)
{
	esphttpd_request_obj_t *self = MP_OBJ_TO_PTR(self_in);

	if (self->req == NULL)
		mp_raise_ValueError(MP_ERROR_TEXT("request expired"));

	return method_str(self->req->method);
}
MP_DEFINE_CONST_FUN_OBJ_1( esphttpd_request_method_obj, esphttpd_request_method );

/* _esphttpd.Request.set_content_type(mime) */
static mp_obj_t esphttpd_request_set_content_type(mp_obj_t self_in, mp_obj_t type_in)
{
	esphttpd_request_obj_t *self = MP_OBJ_TO_PTR(self_in);

	if (self->req == NULL)
		mp_raise_ValueError(MP_ERROR_TEXT("request expired"));

	const char *type = mp_obj_str_get_str(type_in);
	check_esp_err(httpd_resp_set_type(self->req, type));
	request_prevent_gc(self, type_in);
	return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2( esphttpd_request_set_content_type_obj, esphttpd_request_set_content_type );

/* _esphttpd.Request.add_header(field, value) */
static mp_obj_t esphttpd_request_add_header(mp_obj_t self_in, mp_obj_t field_in, mp_obj_t value_in)
{
	esphttpd_request_obj_t *self = MP_OBJ_TO_PTR(self_in);

	if (self->req == NULL)
		mp_raise_ValueError(MP_ERROR_TEXT("request expired"));

	const char *field = mp_obj_str_get_str(field_in);
	const char *value = mp_obj_str_get_str(value_in);
	check_esp_err(httpd_resp_set_hdr(self->req, field, value));
	request_prevent_gc(self, field_in);
	request_prevent_gc(self, value_in);
	return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_3( esphttpd_request_add_header_obj, esphttpd_request_add_header );

static const mp_rom_map_elem_t esphttpd_request_locals_dict_table[] =
{
	{ MP_ROM_QSTR(MP_QSTR_set_status),       MP_ROM_PTR(&esphttpd_request_set_status_obj)        },
	{ MP_ROM_QSTR(MP_QSTR_recv),             MP_ROM_PTR(&esphttpd_request_recv_obj)              },
	{ MP_ROM_QSTR(MP_QSTR_write),            MP_ROM_PTR(&esphttpd_request_write_obj)             },
	{ MP_ROM_QSTR(MP_QSTR_write_all),        MP_ROM_PTR(&esphttpd_request_write_all_obj)         },
	{ MP_ROM_QSTR(MP_QSTR_content_len),      MP_ROM_PTR(&esphttpd_request_content_len_obj)       },
	{ MP_ROM_QSTR(MP_QSTR_method),           MP_ROM_PTR(&esphttpd_request_method_obj)            },
	{ MP_ROM_QSTR(MP_QSTR_get_header),       MP_ROM_PTR(&esphttpd_request_get_header_obj)        },
	{ MP_ROM_QSTR(MP_QSTR_get_query_string), MP_ROM_PTR(&esphttpd_request_get_query_string_obj)  },
	{ MP_ROM_QSTR(MP_QSTR_get_uri),          MP_ROM_PTR(&esphttpd_request_get_uri_obj)           },
	{ MP_ROM_QSTR(MP_QSTR_get_path),         MP_ROM_PTR(&esphttpd_request_get_path_obj)          },
	{ MP_ROM_QSTR(MP_QSTR_get_remote_host),  MP_ROM_PTR(&esphttpd_request_get_remote_host_obj)   },
	{ MP_ROM_QSTR(MP_QSTR_get_remote_port),  MP_ROM_PTR(&esphttpd_request_get_remote_port_obj)   },
	{ MP_ROM_QSTR(MP_QSTR_get_server_host),  MP_ROM_PTR(&esphttpd_request_get_server_host_obj)   },
	{ MP_ROM_QSTR(MP_QSTR_get_server_port),  MP_ROM_PTR(&esphttpd_request_get_server_port_obj)   },
	{ MP_ROM_QSTR(MP_QSTR_get_protocol),     MP_ROM_PTR(&esphttpd_request_get_protocol_obj)      },
	{ MP_ROM_QSTR(MP_QSTR_set_content_type), MP_ROM_PTR(&esphttpd_request_set_content_type_obj)  },
	{ MP_ROM_QSTR(MP_QSTR_add_header),       MP_ROM_PTR(&esphttpd_request_add_header_obj)        },
	{ MP_ROM_QSTR(MP_QSTR_set_session_ctx),  MP_ROM_PTR(&esphttpd_request_set_session_ctx_obj)   },
	{ MP_ROM_QSTR(MP_QSTR_get_session_ctx),  MP_ROM_PTR(&esphttpd_request_get_session_ctx_obj)   },
};

static MP_DEFINE_CONST_DICT(esphttpd_request_locals_dict, esphttpd_request_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
	esphttpd_request_type,
	MP_QSTR_Request,
	MP_TYPE_FLAG_NONE,
	print, esphttpd_request_print,
	locals_dict, &esphttpd_request_locals_dict
	);

static void init_request_obj(esphttpd_request_obj_t *req, int n_alloc,
                             esphttpd_server_obj_t *server)
{
	*req = (esphttpd_request_obj_t)
	{
		.base          = { &esphttpd_request_type },
		.chunk_sent    = 0,
		.all_sent      = 0,
		.no_gc_objects = m_new(mp_obj_t, n_alloc),
		.n_alloc       = n_alloc,
		.server        = server,
	};

	clear_request_obj(req);
}

/* Server object
 *
 */


static void esphttpd_server_print(const mp_print_t *print,
                                  mp_obj_t self_in,
                                  mp_print_kind_t kind)
{
	mp_printf(print, "<_esphttpd.Server XXX TODO Description XXX>");
}


/* FreeRTOS queue ops
 *
 */

static void alloc_queues(esphttpd_server_obj_t *self)
{
	self->request_queue = xQueueCreate( 1, sizeof( request_msg_t ) );
	if (!self->request_queue)
		mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Cannot allocate FreeRTOS queue"));

	self->response_queue = xQueueCreate( 1, sizeof( response_msg_t ) );
	if (!self->response_queue)
	{
		vQueueDelete(self->request_queue);
		self->request_queue = NULL;
		mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Cannot allocate FreeRTOS queue"));
	}
}

static void free_queues(esphttpd_server_obj_t *self)
{
	if (self->request_queue)
	{
		vQueueDelete(self->request_queue);
		self->request_queue = NULL;
	}
	if (self->response_queue)
	{
		vQueueDelete(self->response_queue);
		self->response_queue = NULL;
	}
}

static httpd_req_t *wait_for_request(esphttpd_server_obj_t *self)
{
	MP_THREAD_GIL_EXIT();
	request_msg_t msg;
	xQueueReceive(self->request_queue, &msg, portMAX_DELAY);
	MP_THREAD_GIL_ENTER();

	if (msg.type == REQ_QUIT)
		return NULL;

	if (msg.type != REQ_NEW)
	{
		ESP_LOGE(TAG, LOG_FMT("desynchonized queue, msg.type = %d"), msg.type);
		return NULL;
	}

	return msg.req;
}

static void handler_done(esphttpd_server_obj_t *self, esp_err_t err)
{
	MP_THREAD_GIL_EXIT();
	response_msg_t msg = RESPONSE_DONE(err);
	xQueueSend(self->response_queue, &msg, portMAX_DELAY);
	MP_THREAD_GIL_ENTER();
}

static esphttpd_server_obj_t *get_server_obj(httpd_req_t *r)
{
	return (esphttpd_server_obj_t *)httpd_get_global_user_ctx(r->handle);
}


/* runs in http server thread */
static esp_err_t handler_wrapper(httpd_req_t *r)
{
	esphttpd_server_obj_t *self = get_server_obj(r);
	request_msg_t out_msg = REQUEST_NEW(r);
	xQueueSend(self->request_queue, &out_msg, portMAX_DELAY);

	for (;;)
	{
		response_msg_t in_msg;
		xQueueReceive(self->response_queue, &in_msg, portMAX_DELAY);
		switch (in_msg.type)
		{
			case RESP_DONE:
				return in_msg.err;

			case RESP_SEND:
				out_msg = REQUEST_RETURN( httpd_resp_send(r, in_msg.buf, in_msg.size) );
				break;
			case RESP_SEND_CHUNK:
				out_msg = REQUEST_RETURN( httpd_resp_send_chunk(r, in_msg.buf, in_msg.size) );
				break;
			default:
				ESP_LOGE(TAG, LOG_FMT("unimplemented message in_msg.type = %d"), in_msg.type);
				out_msg = REQUEST_RETURN( ESP_FAIL );
		}

		xQueueSend(self->request_queue, &out_msg, portMAX_DELAY);
	}
}

/* runs in http server thread */
static void global_user_ctx_free(void *ctx)
{
	esphttpd_server_obj_t *self = (esphttpd_server_obj_t *)ctx;
	request_msg_t req_msg = REQUEST_QUIT();
	ESP_LOGD(__func__, "sending msg");
	xQueueSend(self->request_queue, &req_msg, portMAX_DELAY);
	ESP_LOGD(__func__, "sent");
	response_msg_t resp_msg;
	xQueueReceive(self->response_queue, &resp_msg, portMAX_DELAY);
	ESP_LOGD(__func__, "got reply");
	free_queues(self);
	self->handle = NULL;
}

/*
 * register request handlers
 */

static mp_obj_t esphttpd_server_register(size_t n_args, const mp_obj_t *args)
{
	esphttpd_server_obj_t *self = MP_OBJ_TO_PTR(args[0]);

	if (self->handle == NULL)
		 mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("HTTP Server not running"));

	const char *uri = mp_obj_str_get_str(args[1]);
	int method = method_const(mp_obj_str_get_str(args[2]));
	mp_obj_t handler = args[3];

	if (method == -1)
		mp_raise_ValueError(MP_ERROR_TEXT("Unknown method"));

	if (!mp_obj_is_callable(handler))
		mp_raise_ValueError(MP_ERROR_TEXT("handler must be callable"));

	httpd_uri_t uri_handler =
	{
		.uri      = uri,
		.method   = method,
		.handler  = handler_wrapper,
		.user_ctx = (void *)handler,
	};

	check_esp_err(httpd_register_uri_handler(self->handle, &uri_handler));

	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(esphttpd_server_register_obj, 4, 4, esphttpd_server_register);

static mp_obj_t esphttpd_server_unregister(size_t n_args, const mp_obj_t *args)
{
	esphttpd_server_obj_t *self = MP_OBJ_TO_PTR(args[0]);

	if (self->handle == NULL)
		 mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("HTTP Server not running"));

	const char *uri = mp_obj_str_get_str(args[1]);
	esp_err_t err;

	if (n_args > 2)
	{
		int method = method_const(mp_obj_str_get_str(args[2]));
		if (method == -1)
			mp_raise_ValueError(MP_ERROR_TEXT("Unknown method"));

		err = httpd_unregister_uri_handler(self->handle, uri, method);
	}
	else
		err = httpd_unregister_uri(self->handle, uri);

	check_esp_err(err);

	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(esphttpd_server_unregister_obj, 2, 3, esphttpd_server_unregister);


/*
 *
 */

static void init_httpd_config(esphttpd_server_obj_t *self, httpd_config_t *config)
{
	config->global_user_ctx = self;                         /* used by handler to get queue handles */
	config->global_user_ctx_free_fn = global_user_ctx_free; /* only called when global_user_ctx is non-NULL */
	config->uri_match_fn = httpd_uri_match_wildcard;
	config->max_uri_handlers = 64;
	config->max_resp_headers = 16;

	config->core_id = 1-MP_TASK_COREID;

	self->max_headers = config->max_resp_headers;

	/* gc managed user controlled session contexts */
	self->n_slots = config->max_open_sockets;
	self->slots = new_context_slot_array(self->n_slots);
}

static mp_obj_t esphttpd_server_start(size_t n_args, const mp_obj_t *args)
{
	esphttpd_server_obj_t *self = MP_OBJ_TO_PTR(args[0]);

	mp_obj_t key_in = n_args >= 2 ? args[1] : mp_const_none;
	mp_obj_t cert_in = n_args >= 3 ? args[2] : mp_const_none;

	size_t key_len, cert_len;
	const char unsigned *key = NULL, *cert = NULL;

	if (key_in != mp_const_none)
		key  = (const unsigned char *)mp_obj_str_get_data(key_in, &key_len);
	if (cert_in != mp_const_none)
		cert = (const unsigned char *)mp_obj_str_get_data(cert_in, &cert_len);

	if (self->handle != NULL)
		 mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("HTTP Server already running"));

	alloc_queues(self);

	esp_err_t err;

	if (key && cert)
	{
		httpd_ssl_config_t config = (httpd_ssl_config_t)HTTPD_SSL_CONFIG_DEFAULT();
		init_httpd_config(self, &config.httpd);
		config.prvtkey_pem = key;
		config.prvtkey_len = key_len;
		config.cacert_pem = cert;
		config.cacert_len = cert_len;
		self->protocol_string = MP_ROM_QSTR(MP_QSTR_https);
		err = httpd_ssl_start(&self->handle, &config);
	}
	else
	{
		httpd_config_t config = (httpd_config_t)HTTPD_DEFAULT_CONFIG();
		init_httpd_config(self, &config);
		self->protocol_string = MP_ROM_QSTR(MP_QSTR_http);
		err = httpd_start(&self->handle, &config);
	}

	if (err != ESP_OK)
	{
		free_queues(self);
		self->handle = NULL;
	}

	check_esp_err(err);
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(esphttpd_server_start_obj, 1, 3, esphttpd_server_start);


static mp_obj_t esphttpd_server_event_loop(mp_obj_t self_in)
{
	esphttpd_server_obj_t *self = MP_OBJ_TO_PTR(self_in);
	if (self->handle == NULL)
		 mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("HTTP Server not running"));


	esphttpd_request_obj_t *req_obj = m_new_obj(esphttpd_request_obj_t);

	/* gc 'retainer' pointers for headers {field,value} + status & mime */
	init_request_obj(req_obj, self->max_headers*2 + 2, self);

	/* print & ignore exceptions, outside of the inner loop to speed things up */
	for (;;)
	{
		nlr_buf_t nlr;
		if (nlr_push(&nlr) == 0)
		{
			httpd_req_t *req;

			while ( (req = wait_for_request(self) ) )
			{
				esp_err_t ret = ESP_OK;

				if (!req->ignore_sess_ctx_changes)
				{
					req->sess_ctx = alloc_context_slot(self);
					req->free_ctx = free_context_slot;
					req->ignore_sess_ctx_changes = true;
				}

				req_obj->req = req;

				mp_obj_t callback = (mp_obj_t)req->user_ctx,
				         request  = MP_OBJ_FROM_PTR(req_obj);

				mp_obj_t handler_ret = mp_call_function_1( callback, request );

				if (handler_ret == mp_const_false)
					ret = ESP_FAIL;

				end_request(req_obj);
				clear_request_obj(req_obj);
				handler_done(self, ret);
			}

			nlr_pop();
			break;
		}
		else
		{
			mp_obj_print_exception(&mp_plat_print, MP_OBJ_FROM_PTR(nlr.ret_val));
			clear_request_obj(req_obj);
			handler_done(self, ESP_FAIL);
		}
	}

	handler_done(self, -1); /* sync with stop/deinit */
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(esphttpd_server_event_loop_obj, esphttpd_server_event_loop);

static mp_obj_t esphttpd_server_deinit(mp_obj_t self_in)
{
	esphttpd_server_obj_t *self = MP_OBJ_TO_PTR(self_in);

	if (self->handle != NULL)
	{
		MP_THREAD_GIL_EXIT();
		ESP_LOGD(__func__, "httpd_stop()");
		esp_err_t err = httpd_stop(self->handle);
		ESP_LOGD(__func__, "httpd_stop() done");
		MP_THREAD_GIL_ENTER();
		check_esp_err(err);
	}

	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(esphttpd_server_deinit_obj, esphttpd_server_deinit);

static mp_obj_t esphttpd_server_stop(mp_obj_t self_in)
{
	esphttpd_server_obj_t *self = MP_OBJ_TO_PTR(self_in);

	if (self->handle == NULL)
		 mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("HTTP Server not running"));

	MP_THREAD_GIL_EXIT();
	esp_err_t err = httpd_stop(self->handle);
	MP_THREAD_GIL_ENTER();
	check_esp_err(err);

	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(esphttpd_server_stop_obj, esphttpd_server_stop);

static const mp_rom_map_elem_t esphttpd_server_locals_dict_table[] =
{
	{ MP_ROM_QSTR(MP_QSTR_register),   MP_ROM_PTR(&esphttpd_server_register_obj)   },
	{ MP_ROM_QSTR(MP_QSTR_unregister), MP_ROM_PTR(&esphttpd_server_unregister_obj) },
	{ MP_ROM_QSTR(MP_QSTR_start),      MP_ROM_PTR(&esphttpd_server_start_obj)      },
	{ MP_ROM_QSTR(MP_QSTR_stop),       MP_ROM_PTR(&esphttpd_server_stop_obj)       },
	{ MP_ROM_QSTR(MP_QSTR_event_loop), MP_ROM_PTR(&esphttpd_server_event_loop_obj) },
	{ MP_ROM_QSTR(MP_QSTR___del__),    MP_ROM_PTR(&esphttpd_server_deinit_obj)     },
};

static MP_DEFINE_CONST_DICT(esphttpd_server_locals_dict, esphttpd_server_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
	esphttpd_server_type,
	MP_QSTR_server,
	MP_TYPE_FLAG_NONE,
	print, esphttpd_server_print,
	locals_dict, &esphttpd_server_locals_dict
	);

static mp_obj_t esphttpd__bufcpy(mp_obj_t dest, mp_obj_t ix, mp_obj_t src)
{
	mp_buffer_info_t destinfo;
	mp_get_buffer_raise(dest, &destinfo, MP_BUFFER_WRITE);

	mp_buffer_info_t srcinfo;
	mp_get_buffer_raise(src, &srcinfo, MP_BUFFER_READ);

	uint8_t *dest_buf = destinfo.buf,
	        *src_buf  = srcinfo.buf;

	size_t index = mp_obj_get_int(ix);

	if ( (index             > destinfo.len) ||
	     (srcinfo.len       > destinfo.len) ||
	     (index+srcinfo.len > destinfo.len) )
	    return MP_OBJ_NEW_SMALL_INT(-1);

	memcpy(&dest_buf[index], src_buf, srcinfo.len);

	return mp_obj_new_int(index+srcinfo.len);
}

static MP_DEFINE_CONST_FUN_OBJ_3(esphttpd__bufcpy_obj, esphttpd__bufcpy);

static mp_obj_t esphttpd_http_server(void)
{
	esphttpd_server_obj_t *self = m_new_obj_with_finaliser(esphttpd_server_obj_t);
	*self = (esphttpd_server_obj_t)
	{
		.base            = { &esphttpd_server_type },
		.handle          = NULL,
		.request_queue   = NULL,
		.response_queue  = NULL,
		.protocol_string = mp_const_none,
	};
	return MP_OBJ_FROM_PTR(self);
}

static MP_DEFINE_CONST_FUN_OBJ_0(esphttpd_http_server_obj, esphttpd_http_server);

static const mp_rom_map_elem_t esphttpd_module_globals_table[] =
{
	{ MP_ROM_QSTR(MP_QSTR___name__),    MP_ROM_QSTR(MP_QSTR__esphttpd)        },
	{ MP_ROM_QSTR(MP_QSTR__bufcpy),     MP_ROM_PTR(&esphttpd__bufcpy_obj)     },
	{ MP_ROM_QSTR(MP_QSTR_http_server), MP_ROM_PTR(&esphttpd_http_server_obj) },
};

static MP_DEFINE_CONST_DICT(esphttpd_module_globals, esphttpd_module_globals_table);

const mp_obj_module_t esphttpd_user_cmodule =
{
	.base = { &mp_type_module },
	.globals = (mp_obj_dict_t*)&esphttpd_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR__esphttpd, esphttpd_user_cmodule);

