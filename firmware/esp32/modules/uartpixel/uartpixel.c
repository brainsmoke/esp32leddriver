
/*
 * send frames over uart at precise intervals regardless of garbage collection
 * using a buffer of multiple frames and a timer interrupt.
 *
 * machine.UART & machine.Timer almost provide the required functionality,
 * but the Timer ISR merely schedules the callback, which means it won't be
 * run before garbage collection has completed.
 *
 * Usage:
 *
 * queue = _uartpixel.FrameQueue( uart=1, timer=3, baudrate=1000000, rx=35, tx=13, fps=60., framesize=1444, framecount=10 )
 *
 * queue.push( buf ) # will raise an exception if buf is bigger than framesize
 * ...
 *
 * del queue
 *
 */

#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include "mphalport.h"

#include "driver/timer.h"
#include "driver/uart.h"

#include "freertos/queue.h"
#include "freertos/semphr.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "esp_log.h"
#include "esp_task.h"

static const char *TAG = "_uartpixel";
#define LOG_FMT(x) "%s: " x, __func__

#define TIMER_INTR_SEL TIMER_INTR_LEVEL
#define TIMER_DIVIDER  8

// TIMER_BASE_CLK is normally 80MHz. TIMER_DIVIDER ought to divide this exactly
#define TIMER_SCALE    (TIMER_BASE_CLK / TIMER_DIVIDER)

#define TIMER_FLAGS    ( ESP_INTR_FLAG_IRAM )


typedef struct
{
	mp_obj_base_t base;

	int uart_num;

	int timer_group;
	int timer_index;

	int frame_size;
	int frame_count;
	uint8_t *buf;

	int ix;
	int quit;

	QueueHandle_t queue;
	SemaphoreHandle_t mutex;
	TaskHandle_t task;

} frame_queue_obj_t;

typedef struct
{
	uint8_t *buf;
	int size;

} write_op_t;

static void timer_task(void *self_ptr)
{
	frame_queue_obj_t *self = self_ptr;

	uint32_t event = 0;
	write_op_t op;

	uint32_t error_timeout = pdMS_TO_TICKS( 1000 );
	for (;;)
	{
		xTaskNotifyWait(0, -1, &event, error_timeout);
		if (self->quit)
			break;

		if ( xQueueReceive(self->queue, &op, portMAX_DELAY) )
			uart_write_bytes(self->uart_num, (const char *)op.buf, op.size);
	}

	xQueueReceive(self->queue, &op, 0);
	self->task = NULL;
	vTaskDelete(NULL);
}

typedef struct queue_ll_s queue_ll_t;
struct queue_ll_s
{
	mp_obj_t obj;

	queue_ll_t *next;
};
static queue_ll_t *global_q = NULL;

static void add_frame_queue(mp_obj_t obj)
{
	queue_ll_t *q = malloc(sizeof(queue_ll_t));

	*q = (queue_ll_t) { .obj = obj, .next = global_q };

	global_q = q;
}

static void delete_frame_queue(mp_obj_t obj)
{
	queue_ll_t **qq = &global_q;

	while (*qq)
	{
		if ( (*qq)->obj == obj )
		{
			queue_ll_t *q_old = *qq;
			*qq = q_old->next;
			free( q_old );
			break;
		}
		qq = &(*qq)->next;
	}
}

static void start_task(frame_queue_obj_t *self)
{
	xTaskCreate(timer_task, "uartpixel", 2048, self, ESP_TASK_PRIO_MIN+2, &self->task);
//	xTaskCreatePinnedToCore(timer_task, "uartpixel", 2048, self, ESP_TASK_PRIO_MIN+2, &self->task, MP_TASK_COREID);
}

static mp_obj_t uartpixel_frame_queue_deinit(mp_obj_t self_in)
{
	delete_frame_queue(self_in);
	frame_queue_obj_t *self = MP_OBJ_TO_PTR(self_in);

	if (self->task)
	{
		self->quit = 1;
		timer_pause(self->timer_group, self->timer_index);
		timer_isr_callback_remove(self->timer_group, self->timer_index);
		xTaskNotify(self->task, 1, eSetBits);

		while (self->task)
			taskYIELD();

		vQueueDelete(self->queue);
	    uart_driver_delete(self->uart_num);
	}
	return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_1(uartpixel_frame_queue_deinit_obj, uartpixel_frame_queue_deinit);

bool IRAM_ATTR uartpixel_frame_queue_isr(void *task_ptr)
{
	TaskHandle_t task = (TaskHandle_t)task_ptr;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xTaskNotifyFromISR(task, 0, eNoAction, &xHigherPriorityTaskWoken);
	return xHigherPriorityTaskWoken == pdTRUE;
}

extern const mp_obj_type_t frame_queue_type;

static mp_obj_t uartpixel_frame_queue_make_new(const mp_obj_type_t *type,
                                               size_t n_args,
                                               size_t n_kw, const mp_obj_t *all_args)
{
	enum { ARG_uart, ARG_baudrate, ARG_timer, ARG_rx, ARG_tx, ARG_fps, ARG_framesize, ARG_framecount };
	static const mp_arg_t allowed_args[] =
	{
		{ MP_QSTR_uart,                        MP_ARG_INT | MP_ARG_REQUIRED },
		{ MP_QSTR_baudrate,                    MP_ARG_INT | MP_ARG_REQUIRED },
		{ MP_QSTR_timer,                       MP_ARG_INT | MP_ARG_REQUIRED },
		{ MP_QSTR_rx,         MP_ARG_KW_ONLY | MP_ARG_INT | MP_ARG_REQUIRED },
		{ MP_QSTR_tx,         MP_ARG_KW_ONLY | MP_ARG_INT | MP_ARG_REQUIRED },
		{ MP_QSTR_fps,        MP_ARG_KW_ONLY | MP_ARG_OBJ | MP_ARG_REQUIRED },
		{ MP_QSTR_framesize,  MP_ARG_KW_ONLY | MP_ARG_INT | MP_ARG_REQUIRED },
		{ MP_QSTR_framecount, MP_ARG_KW_ONLY | MP_ARG_INT | MP_ARG_REQUIRED },
	};

	mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
	mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

	int frame_count = args[ARG_framecount].u_int+1; /* one extra frame to allow for concurrent filling/emptying of a buffer before locking */
	int queue_count = args[ARG_framecount].u_int-1; /* one less so that we block on the last frame */
	int frame_size = args[ARG_framesize].u_int;
	uint64_t period = (uint64_t)( (float)TIMER_SCALE / mp_obj_get_float(args[ARG_fps].u_obj) );

	if ( args[ARG_timer].u_int < 0 || args[ARG_timer].u_int > 3 )
		mp_raise_ValueError(MP_ERROR_TEXT("Bad timer"));

	if ( frame_size <= 0 || frame_count <= 1 || frame_count >= INT_MAX/frame_size )
		mp_raise_ValueError(MP_ERROR_TEXT("Bad buffer sizes"));

	frame_queue_obj_t *self = m_new_obj_with_finaliser(frame_queue_obj_t);

	*self = (frame_queue_obj_t)
	{
		.base        = { &frame_queue_type },
		.uart_num    = args[ARG_uart].u_int,
		.timer_group = args[ARG_timer].u_int & 1,
		.timer_index = (args[ARG_timer].u_int>>1) & 1,
		.frame_size  = frame_size,
		.frame_count = frame_count,
		.buf   = m_new( uint8_t, frame_size*frame_count ),
		.queue = xQueueCreate( queue_count , sizeof( write_op_t ) ),
		.mutex = xSemaphoreCreateMutex(),
		.ix    = 0,
		.quit  = 0,
	};

	if (!self->queue)
		mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Cannot allocate FreeRTOS queue"));

	if (!self->mutex)
		mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Cannot allocate FreeRTOS mutex"));


	uart_config_t uart_cfg =
	{
		.baud_rate = args[ARG_baudrate].u_int,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.rx_flow_ctrl_thresh = 0,
	};

	uart_driver_delete(self->uart_num);
	check_esp_err( uart_param_config(self->uart_num, &uart_cfg) );
	check_esp_err( uart_driver_install(self->uart_num, 256, frame_size + 256, 0, NULL, 0) );
	check_esp_err( uart_set_pin(self->uart_num, args[ARG_tx].u_int, args[ARG_rx].u_int, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) );

	timer_config_t timer_cfg =
	{
		.alarm_en    = TIMER_ALARM_EN,
		.counter_dir = TIMER_COUNT_UP,
		.divider     = TIMER_DIVIDER,
		.intr_type   = TIMER_INTR_LEVEL,
		.counter_en  = TIMER_PAUSE,
		.auto_reload = 1,
	};

	start_task(self);

	check_esp_err(timer_init(self->timer_group, self->timer_index, &timer_cfg));
	check_esp_err(timer_set_counter_value(self->timer_group, self->timer_index, 0x00000000));
	check_esp_err(timer_set_alarm_value(self->timer_group, self->timer_index, period));
	check_esp_err(timer_isr_callback_add(self->timer_group, self->timer_index, uartpixel_frame_queue_isr, self->task, TIMER_FLAGS) );

	check_esp_err(timer_start(self->timer_group, self->timer_index));

	add_frame_queue(MP_OBJ_FROM_PTR(self));
	return MP_OBJ_FROM_PTR(self);
}

static uint8_t *new_framebuf(frame_queue_obj_t *self)
{
	uint8_t *buf = &self->buf[self->ix * self->frame_size];
	self->ix++;
	self->ix %= self->frame_count;
	return buf;
}

static mp_obj_t uartpixel_frame_queue_push(mp_obj_t self_in, mp_obj_t buf)
{
	frame_queue_obj_t *self = MP_OBJ_TO_PTR(self_in);

	if (self->quit)
		mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("frame queue stopped"));

	mp_buffer_info_t bufinfo;
	mp_get_buffer_raise(buf, &bufinfo, MP_BUFFER_READ);

	if (bufinfo.len > self->frame_size)
		mp_raise_ValueError(MP_ERROR_TEXT("frame too big"));

	MP_THREAD_GIL_EXIT();
	xSemaphoreTake( self->mutex, portMAX_DELAY );

	write_op_t op =
	{
		.buf = new_framebuf(self),
		.size = bufinfo.len,
	};

	memcpy(op.buf, bufinfo.buf, bufinfo.len);

	xQueueSend(self->queue, &op, portMAX_DELAY);
	xSemaphoreGive( self->mutex );
	MP_THREAD_GIL_ENTER();

	return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_2(uartpixel_frame_queue_push_obj, uartpixel_frame_queue_push);

static void uartpixel_frame_queue_print(const mp_print_t *print,
                                        mp_obj_t self_in,
                                        mp_print_kind_t kind)
{
	mp_printf(print, "<_uartpixel.FrameQueue XXX TODO Description XXX>");
}


static const mp_rom_map_elem_t uartpixel_frame_queue_locals_dict_table[] =
{
	{ MP_ROM_QSTR(MP_QSTR_push),    MP_ROM_PTR(&uartpixel_frame_queue_push_obj)   },
	{ MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&uartpixel_frame_queue_deinit_obj) },
};

static MP_DEFINE_CONST_DICT(uartpixel_frame_queue_locals_dict, uartpixel_frame_queue_locals_dict_table);


const mp_obj_type_t frame_queue_type =
{
	{ &mp_type_type },
	.name = MP_QSTR_FrameQueue,
	.print = uartpixel_frame_queue_print,
	.make_new = uartpixel_frame_queue_make_new,
	.locals_dict = (mp_obj_dict_t *)&uartpixel_frame_queue_locals_dict,
};

/* Add to main task on reset
 *
 */
void uartpixel_deinit(void)
{
	while (global_q)
		uartpixel_frame_queue_deinit(global_q->obj);
}

static const mp_rom_map_elem_t uartpixel_module_globals_table[] =
{
	{ MP_ROM_QSTR(MP_QSTR___name__),    MP_ROM_QSTR(MP_QSTR__uartpixel) },
//	{ MP_ROM_QSTR(MP_QSTR___init__),    MP_ROM_PTR(&uartpixel_init_obj) },
	{ MP_ROM_QSTR(MP_QSTR_FrameQueue),  MP_ROM_PTR(&frame_queue_type)   },
};

static MP_DEFINE_CONST_DICT(uartpixel_module_globals, uartpixel_module_globals_table);

const mp_obj_module_t uartpixel_cmodule =
{
	.base = { &mp_type_module },
	.globals = (mp_obj_dict_t*)&uartpixel_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR__uartpixel, uartpixel_cmodule, MODULE_UARTPIXEL_ENABLED);

