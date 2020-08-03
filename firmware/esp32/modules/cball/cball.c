
#include <math.h>
#include <string.h>

#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"

STATIC mp_obj_t cball_framebuf8to16(mp_obj_t in, mp_obj_t out, mp_obj_t table)
{
	mp_buffer_info_t src_info;
	mp_get_buffer_raise(in, &src_info, MP_BUFFER_READ);
	size_t src_len = src_info.len;
	uint8_t *src = src_info.buf;

	if (src_len%3 != 0)
		mp_raise_ValueError("source array must be a multiple of 3");

	mp_buffer_info_t dest_info;
	mp_get_buffer_raise(out, &dest_info, MP_BUFFER_WRITE);
	if (dest_info.len < src_len*2)
		mp_raise_ValueError("destination array too small");

	uint16_t *dest = dest_info.buf;

	mp_buffer_info_t lookup_info;
	mp_get_buffer_raise(table, &lookup_info, MP_BUFFER_READ);
	if (lookup_info.len != 512)
		mp_raise_ValueError("destination array too small");

	uint16_t *lookup = lookup_info.buf;

	size_t i;
	for (i=0; i<src_len; i+=3)
	{
		dest[i  ] = lookup[src[i+1]];
		dest[i+1] = lookup[src[i  ]];
		dest[i+2] = lookup[src[i+2]];
	}

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(cball_framebuf8to16_obj, cball_framebuf8to16);

STATIC mp_obj_t cball_bytearray_memset(mp_obj_t array, mp_obj_t c)
{
	mp_buffer_info_t buf_info;
	mp_get_buffer_raise(array, &buf_info, MP_BUFFER_WRITE);
	memset(buf_info.buf, mp_obj_get_int(c), buf_info.len);
	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(cball_bytearray_memset_obj, cball_bytearray_memset);

STATIC mp_obj_t cball_bytearray_blend(size_t n_args, const mp_obj_t *args)
{
	int mb = (int)(256.f *mp_obj_get_float(args[3]));

	if (mb < 0)
		mb = 0;
	else if (mb > 256)
		mb = 256;

	int ma = 256-mb;

	mp_buffer_info_t dest_info;
	mp_get_buffer_raise(args[0], &dest_info, MP_BUFFER_WRITE);
	mp_buffer_info_t src_a_info;
	mp_get_buffer_raise(args[1], &src_a_info, MP_BUFFER_READ);
	mp_buffer_info_t src_b_info;
	mp_get_buffer_raise(args[2], &src_b_info, MP_BUFFER_READ);

	if ( (dest_info.len != src_a_info.len) || (src_a_info.len != src_b_info.len) )
		mp_raise_ValueError("array sizes dont match");

	uint8_t *d = dest_info.buf, *a = src_a_info.buf, *b = src_b_info.buf;

	size_t i;
	for (i=0; i<dest_info.len; i++)
		d[i] = ( ma*a[i] + mb*b[i] ) >> 8;

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cball_bytearray_blend_obj, 4, 4, cball_bytearray_blend);

STATIC mp_obj_t cball_orbit_update_flat(mp_obj_t object_list, mp_obj_t obj_gmdt2, mp_obj_t count)
{

	mp_buffer_info_t objects_info;
	mp_get_buffer_raise(object_list, &objects_info, MP_BUFFER_WRITE);
	if (objects_info.typecode != 'f')
		mp_raise_ValueError("object_list needs to be a uarray.array('f',...)");

	size_t object_data_len=objects_info.len/sizeof(float);
	float *object_data = objects_info.buf;

	if (object_data_len % 6 != 0)
		mp_raise_ValueError("length of object_list not a multiple of 6");

	float gmdt2 = (float)mp_obj_get_float(obj_gmdt2);
	size_t n = mp_obj_get_int(count);

	size_t i,j;
	for (i=0; i<object_data_len; i+=6)
	{
		float px = object_data[i  ],
		      py = object_data[i+1],
		      pz = object_data[i+2],
		      vx = object_data[i+3],
		      vy = object_data[i+4],
		      vz = object_data[i+5];

		for (j=0; j<n; j++)
		{
			float d2 = px*px+py*py+pz*pz;
			float d3 = d2*sqrtf(d2);
			float  a = gmdt2/d3;
			vx += px*a;
			vy += py*a;
			vz += pz*a;
			px += vx;
			py += vy;
			pz += vz;
		}
	    object_data[i  ] = px;
		object_data[i+1] = py;
		object_data[i+2] = pz;
		object_data[i+3] = vx;
		object_data[i+4] = vy;
		object_data[i+5] = vz;
	}

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(cball_orbit_update_flat_obj, cball_orbit_update_flat);

STATIC mp_obj_t cball_lorenz_update_flat(mp_obj_t attractors, mp_obj_t delta_time, mp_obj_t iterations)
{
	mp_buffer_info_t attractors_info;
	mp_get_buffer_raise(attractors, &attractors_info, MP_BUFFER_WRITE);
	if (attractors_info.typecode != 'f')
		mp_raise_ValueError("leds_flat needs to be a uarray.array('f',...)");

	size_t attractors_data_len=attractors_info.len/sizeof(float);
	float *attractors_data = attractors_info.buf;

	float dt = (float)mp_obj_get_float(delta_time);
	int    n = mp_obj_get_int(iterations);

	size_t i,j;
	for (i=0; i<attractors_data_len; i+=6)
	{
		float x     = attractors_data[i  ],
		      y     = attractors_data[i+1],
		      z     = attractors_data[i+2],
              sigma = attractors_data[i+3],
              rho   = attractors_data[i+4],
              beta  = attractors_data[i+5];

		for (j=0; j<n; j++)
		{
			x += sigma*(y-x)*dt;
			y += (x*(rho-z)-y)*dt;
			z += (x*y-beta*z)*dt;
		}

	    attractors_data[i  ] = x;
		attractors_data[i+1] = y;
		attractors_data[i+2] = z;
	}

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(cball_lorenz_update_flat_obj, cball_lorenz_update_flat);

STATIC mp_obj_t cball_shader(size_t n_args, const mp_obj_t *args)
/*                           framebuffer, positions, normals, [ (pos, color),* ] */
{
	mp_buffer_info_t fb;
	mp_get_buffer_raise(args[0], &fb, MP_BUFFER_WRITE);
	size_t fb_len = fb.len;
	uint8_t *fb_buf = fb.buf;

	size_t pos_len;
	mp_obj_t *pos_list;
	mp_obj_get_array(args[1], &pos_len, &pos_list);

	mp_obj_t *normal_list;
	mp_obj_get_array_fixed_n(args[2], pos_len, &normal_list);

	if (pos_len*3 != fb_len)
		mp_raise_ValueError("Array sizes don't match");

	size_t lights_len;
	mp_obj_t *lights_list;
	mp_obj_get_array(args[3], &lights_len, &lights_list);

	size_t i, j;
	for (i=0; i<pos_len; i++)
	{
		mp_obj_t *vec;

		mp_float_t r=(mp_float_t)fb_buf[i*3],
		           g=(mp_float_t)fb_buf[i*3+1],
		           b=(mp_float_t)fb_buf[i*3+2];

		mp_obj_get_array_fixed_n(pos_list[i], 3, &vec);
		mp_float_t ledx = mp_obj_get_float(vec[0]),
		           ledy = mp_obj_get_float(vec[1]),
		           ledz = mp_obj_get_float(vec[2]);

		mp_obj_get_array_fixed_n(normal_list[i], 3, &vec);
		mp_float_t nx = mp_obj_get_float(vec[0]),
		           ny = mp_obj_get_float(vec[1]),
		           nz = mp_obj_get_float(vec[2]);

		for (j=0; j<lights_len; j++)
		{
			mp_obj_t *light;
			mp_obj_get_array_fixed_n(lights_list[j], 2, &light);
			mp_obj_get_array_fixed_n(light[0], 3, &vec);

			mp_float_t px = mp_obj_get_float(vec[0])-ledx,
			           py = mp_obj_get_float(vec[1])-ledy,
			           pz = mp_obj_get_float(vec[2])-ledz;

			mp_float_t sprod = px*nx+py*ny+pz*nz;
			if (sprod <= 0.f)
				continue;
			mp_float_t d2 = px*px+py*py+pz*pz;
			if (d2 < 1e-25f)
				continue;
			mp_float_t angle = sprod/sqrtf(d2);
			mp_float_t factor = angle/d2;

			mp_obj_get_array_fixed_n(light[1], 3, &vec);
			r += factor * mp_obj_get_float(vec[0]);
			g += factor * mp_obj_get_float(vec[1]);
			b += factor * mp_obj_get_float(vec[2]);
		}
		if (r>255.f) r = 255.f;
		if (g>255.f) g = 255.f;
		if (b>255.f) b = 255.f;
		fb_buf[i*3] = r;
		fb_buf[i*3+1] = g;
		fb_buf[i*3+2] = b;
	}

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cball_shader_obj, 4, 4, cball_shader);

STATIC mp_obj_t cball_shader_flat(mp_obj_t framebuf, mp_obj_t leds_flat, mp_obj_t lights_flat)
{
	mp_buffer_info_t fb_info;
	mp_get_buffer_raise(framebuf, &fb_info, MP_BUFFER_WRITE);
	size_t fb_len = fb_info.len;
	uint8_t *fb_buf = fb_info.buf;

	mp_buffer_info_t leds_info;
	mp_get_buffer_raise(leds_flat, &leds_info, MP_BUFFER_READ);
	if (leds_info.typecode != 'f')
		mp_raise_ValueError("leds_flat needs to be a uarray.array('f',...)");

	size_t led_data_len = leds_info.len / sizeof(float);
	float *led_data = leds_info.buf;

	if (led_data_len != fb_len*2)
		mp_raise_ValueError("Array sizes don't match");

	mp_buffer_info_t lights_info;
	mp_get_buffer_raise(lights_flat, &lights_info, MP_BUFFER_READ);
	if (lights_info.typecode != 'f')
		mp_raise_ValueError("lights_flat needs to be a uarray.array('f',...)");

	size_t lights_data_len = lights_info.len / sizeof(float);
	float *lights_data = lights_info.buf;

	if (lights_data_len % 6 != 0)
		mp_raise_ValueError("length of lights_flat not a multiple of 6");

	size_t i,j;
	for (i=0; i<fb_len; i+=3)
	{
		mp_float_t r=(float)fb_buf[i],
		           g=(float)fb_buf[i+1],
		           b=(float)fb_buf[i+2];

		mp_float_t ledx = led_data[i*2],
		           ledy = led_data[i*2+1],
		           ledz = led_data[i*2+2],
		           nx   = led_data[i*2+3],
		           ny   = led_data[i*2+4],
		           nz   = led_data[i*2+5];

		for (j=0; j<lights_data_len; j+=6)
		{
			float px = lights_data[j  ]-ledx,
			      py = lights_data[j+1]-ledy,
			      pz = lights_data[j+2]-ledz;

			float sprod = px*nx+py*ny+pz*nz;
			if (sprod <= 0.)
				continue;
			float d2 = px*px+py*py+pz*pz;
			if (d2 < 1e-25f)
				continue;
			float angle = sprod/sqrtf(d2);
			float factor = angle/d2;

			r += factor * lights_data[j+3];
			g += factor * lights_data[j+4];
			b += factor * lights_data[j+5];
		}
		if (r>255.f) r = 255.f;
		if (g>255.f) g = 255.f;
		if (b>255.f) b = 255.f;
		fb_buf[i] = (uint8_t)r;
		fb_buf[i+1] = (uint8_t)g;
		fb_buf[i+2] = (uint8_t)b;
	}

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(cball_shader_flat_obj, cball_shader_flat);

STATIC const mp_rom_map_elem_t cball_module_globals_table[] =
{
	{ MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_cball) },
	{ MP_ROM_QSTR(MP_QSTR_bytearray_memset), MP_ROM_PTR(&cball_bytearray_memset_obj) },
	{ MP_ROM_QSTR(MP_QSTR_bytearray_blend), MP_ROM_PTR(&cball_bytearray_blend_obj) },
	{ MP_ROM_QSTR(MP_QSTR_shader), MP_ROM_PTR(&cball_shader_obj) },
	{ MP_ROM_QSTR(MP_QSTR_shader_flat), MP_ROM_PTR(&cball_shader_flat_obj) },
	{ MP_ROM_QSTR(MP_QSTR_orbit_update_flat), MP_ROM_PTR(&cball_orbit_update_flat_obj) },
	{ MP_ROM_QSTR(MP_QSTR_lorenz_update_flat), MP_ROM_PTR(&cball_lorenz_update_flat_obj) },
	{ MP_ROM_QSTR(MP_QSTR_framebuf8to16), MP_ROM_PTR(&cball_framebuf8to16_obj) },
};

STATIC MP_DEFINE_CONST_DICT(cball_module_globals, cball_module_globals_table);

const mp_obj_module_t cball_user_cmodule =
{
	.base = { &mp_type_module },
	.globals = (mp_obj_dict_t*)&cball_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_cball, cball_user_cmodule, MODULE_CBALL_ENABLED);

