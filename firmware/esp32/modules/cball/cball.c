
#include <math.h>
#include <string.h>
#include <stdlib.h>

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
		mp_raise_ValueError("lookup array needs to be 256 * 2 bytes");

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

STATIC mp_obj_t cball_bytearray_memcpy(mp_obj_t out, mp_obj_t in)
{
	mp_buffer_info_t dest_info;
	mp_get_buffer_raise(out, &dest_info, MP_BUFFER_WRITE);
	mp_buffer_info_t src_info;
	mp_get_buffer_raise(in, &src_info, MP_BUFFER_READ);

	size_t len = dest_info.len;
	if (len > src_info.len)
		len = src_info.len;

	memcpy(dest_info.buf, src_info.buf, len);

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(cball_bytearray_memcpy_obj, cball_bytearray_memcpy);

STATIC mp_obj_t cball_apply_palette(mp_obj_t buf_out, mp_obj_t buf_in, mp_obj_t palette)
{
	/* args:
	 * -     framebuffer_out  [pixels*3]  uint8,
	 * -     framebuffer_in   [pixels]    uint8,
	 * -     palette          [n*3]       uint8,
	 */

	mp_buffer_info_t buf_out_info;
	mp_get_buffer_raise(buf_out, &buf_out_info, MP_BUFFER_WRITE);
	size_t dest_len = buf_out_info.len;
	uint8_t *dest = buf_out_info.buf;

	if (dest_len%3 != 0)
		mp_raise_ValueError("dest array must be a multiple of 3");

	mp_buffer_info_t buf_in_info;
	mp_get_buffer_raise(buf_in, &buf_in_info, MP_BUFFER_READ);
	size_t src_len = buf_in_info.len;
	uint8_t *src = buf_in_info.buf;

	if (dest_len < src_len*3)
		src_len = dest_len/3;

	mp_buffer_info_t table_info;
	mp_get_buffer_raise(palette, &table_info, MP_BUFFER_READ);
	size_t table_len = table_info.len;
	uint8_t *table = table_info.buf;

	if (table_len%3 != 0)
		mp_raise_ValueError("palette array must be a multiple of 3");

	size_t i;
	int r,g,b,c;
	for (i=0; i<src_len; i++)
	{
		c=src[i]*3;
		if ( c < table_len )
		{
			r = table[c  ];
			g = table[c+1];
			b = table[c+2];
		}
		else
		{
			r = g = b = 0;
		}
		dest[i*3  ] = r;
		dest[i*3+1] = g;
		dest[i*3+2] = b;
	}

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(cball_apply_palette_obj, cball_apply_palette);

STATIC mp_obj_t cball_latt_long_map(size_t n_args, const mp_obj_t *args)
{
	/* args:
	 * -     framebuffer_out  [n_leds*3]  uint8,
	 * -     framebuffer_in   [w*h*3]     uint8,
	 * -     voronoi_map      [w*h]       uin16,
	 * -     latt_weight      [h]         float32,
	 * -     total_weight     [n_leds]    float32,
	 * -     tmp_buf          [n_leds*3]  float32,
	 */
	mp_buffer_info_t buf_out_info;
	mp_get_buffer_raise(args[0], &buf_out_info, MP_BUFFER_WRITE);
	size_t dest_len = buf_out_info.len;
	uint8_t *dest = buf_out_info.buf;

	if (dest_len%3 != 0)
		mp_raise_ValueError("dest array must be a multiple of 3");

	mp_buffer_info_t buf_in_info;
	mp_get_buffer_raise(args[1], &buf_in_info, MP_BUFFER_READ);
	size_t src_len = buf_in_info.len;
	uint8_t *src = buf_in_info.buf;

	if (src_len%3 != 0)
		mp_raise_ValueError("src array must be a multiple of 3");

	mp_buffer_info_t voronoi_map_info;
	mp_get_buffer_raise(args[2], &voronoi_map_info, MP_BUFFER_READ);
	size_t voronoi_map_len = voronoi_map_info.len/sizeof(uint16_t);
	uint16_t *voronoi_map = voronoi_map_info.buf;

	if (voronoi_map_info.typecode != 'H')
		mp_raise_ValueError("voronoi_map needs to be a uarray.array('H',...)");

	if (src_len != voronoi_map_len*3)
		mp_raise_ValueError("voronoi and source array sizes don't match");

	mp_buffer_info_t latt_weight_info;
	mp_get_buffer_raise(args[3], &latt_weight_info, MP_BUFFER_READ);
	size_t latt_weight_len = latt_weight_info.len/sizeof(float);
	float *latt_weight = latt_weight_info.buf;

	if (latt_weight_info.typecode != 'f')
		mp_raise_ValueError("latt_weight needs to be a uarray.array('f',...)");

	if ( voronoi_map_len % latt_weight_len != 0 )
		mp_raise_ValueError("wrong height");

	size_t h = latt_weight_len;
	size_t w = voronoi_map_len/latt_weight_len;

	mp_buffer_info_t total_weight_info;
	mp_get_buffer_raise(args[4], &total_weight_info, MP_BUFFER_READ);
	size_t total_weight_len = total_weight_info.len/sizeof(float);
	float *total_weight = total_weight_info.buf;

	if (total_weight_info.typecode != 'f')
		mp_raise_ValueError("total_weight needs to be a uarray.array('f',...)");

	if (total_weight_len*3 != dest_len)
		mp_raise_ValueError("dest and weight array sizes don't match");

	mp_buffer_info_t tmp_buf_info;
	mp_get_buffer_raise(args[5], &tmp_buf_info, MP_BUFFER_WRITE);
	size_t tmp_len = tmp_buf_info.len/sizeof(float);
	float *tmp = tmp_buf_info.buf;

	if (tmp_buf_info.typecode != 'f')
		mp_raise_ValueError("tmp_buf needs to be a uarray.array('f',...)");

	if (dest_len > tmp_len)
		mp_raise_ValueError("tmp_buf too small");

	size_t x, y, i;
	for (i=0; i<tmp_len; i++)
		tmp[i]=0.f;

	for (y=0; y<h; y++)
		for (x=0; x<w; x++)
		{
			int map = voronoi_map[y*w+x]*3;
			if (map < tmp_len)
			{
				tmp[map  ] += (float)src[(y*w+x)*3  ] * latt_weight[y];
				tmp[map+1] += (float)src[(y*w+x)*3+1] * latt_weight[y];
				tmp[map+2] += (float)src[(y*w+x)*3+2] * latt_weight[y];
			}
		}

	for (i=0; i<total_weight_len; i++)
	{
		float w = total_weight[i];
		int v = (int)(w*tmp[i*3]);
		if      (v < 0)   v=0;
		else if (v > 255) v=255;
		dest[i*3]=v;
		v = (int)(w*tmp[i*3+1]);
		if      (v < 0)   v=0;
		else if (v > 255) v=255;
		dest[i*3+1]=v;
		v = (int)(w*tmp[i*3+2]);
		if      (v < 0)   v=0;
		else if (v > 255) v=255;
		dest[i*3+2]=v;
	}

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cball_latt_long_map_obj, 6, 6, cball_latt_long_map);

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
	int mulb = (int)(256.f *mp_obj_get_float(args[3]));

	if (mulb < 0)
		mulb = 0;
	else if (mulb > 256)
		mulb = 256;

	int mula = 256-mulb;

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
		d[i] = ( mula*a[i] + mulb*b[i] ) >> 8;

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cball_bytearray_blend_obj, 4, 4, cball_bytearray_blend);

STATIC mp_obj_t cball_ca_update(size_t n_args, const mp_obj_t *args)
{
	/* args:
	 * -     cells    [w*h]  uint8,
	 * -     w               int,
	 * -     h               int,
	 * -     cap_map  [511]  uint8,
	 */
	mp_buffer_info_t cells_info;
	mp_get_buffer_raise(args[0], &cells_info, MP_BUFFER_WRITE);
	uint8_t *cells = cells_info.buf;

	size_t w = mp_obj_get_int(args[1]);
	size_t h = mp_obj_get_int(args[2]);
	size_t n_cells = cells_info.len;

	if ( (w < 2) || (h < 2) || (w*h != n_cells) || (n_cells/h != w) )
		mp_raise_ValueError("width / height & cell buffer don't match");

	mp_buffer_info_t ca_map_info;
	mp_get_buffer_raise(args[3], &ca_map_info, MP_BUFFER_READ);
	uint8_t *ca_map = ca_map_info.buf;
	size_t ca_map_max = ca_map_info.len-1;

	int sum;
	size_t row, x;
	for (row=w; row<n_cells; row+=w)
	{
		sum = cells[row+w-1] + cells[row] + cells[row+1];
		if (sum > ca_map_max)
			sum = ca_map_max;
		cells[row-w] = ca_map[sum];

		for (x=1; x<w-1; x++)
		{
			sum = cells[row+x-1] + cells[row+x] + cells[row+x+1];
			if (sum > ca_map_max)
				sum = ca_map_max;
			cells[row-w+x] = ca_map[sum];
		}

		sum = cells[row+w-2] + cells[row+w-1] + cells[row];
		if (sum > ca_map_max)
			sum = ca_map_max;
		cells[row-1] = ca_map[sum];
	}

	static int r=0, rmax=0;
	for (x=0; x<w; x++)
	{
		if (rmax == 0)
		{
			r=rand();
			rmax=RAND_MAX;
		}
		cells[(h-1)*w+x] = (r&1) * 56;
		r >>= 1;
		rmax >>= 1;
	}

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cball_ca_update_obj, 4, 4, cball_ca_update);

STATIC mp_obj_t cball_orbit_update(mp_obj_t object_list, mp_obj_t obj_gmdt2, mp_obj_t count)
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

STATIC MP_DEFINE_CONST_FUN_OBJ_3(cball_orbit_update_obj, cball_orbit_update);

STATIC mp_obj_t cball_lorenz_update(mp_obj_t attractors, mp_obj_t delta_time, mp_obj_t iterations)
{
	mp_buffer_info_t attractors_info;
	mp_get_buffer_raise(attractors, &attractors_info, MP_BUFFER_WRITE);
	if (attractors_info.typecode != 'f')
		mp_raise_ValueError("attractors needs to be a uarray.array('f',...)");

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

STATIC MP_DEFINE_CONST_FUN_OBJ_3(cball_lorenz_update_obj, cball_lorenz_update);

STATIC mp_obj_t cball_shader(mp_obj_t framebuf, mp_obj_t leds_flat, mp_obj_t lights_flat)
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

STATIC MP_DEFINE_CONST_FUN_OBJ_3(cball_shader_obj, cball_shader);

STATIC mp_obj_t cball_gradient(size_t n_args, const mp_obj_t *args)
{
	mp_buffer_info_t fb_info;
	mp_get_buffer_raise(args[0], &fb_info, MP_BUFFER_WRITE);
	size_t fb_len = fb_info.len;
	uint8_t *fb = fb_info.buf;

	mp_buffer_info_t rotations_info;
	mp_get_buffer_raise(args[1], &rotations_info, MP_BUFFER_READ);
	if (rotations_info.typecode != 'H')
		mp_raise_ValueError("rotations needs to be a uarray.array('H',...)");

	size_t rot_len = rotations_info.len / sizeof(uint16_t);
	uint16_t *rot = rotations_info.buf;

	if (fb_len != rot_len)
		mp_raise_ValueError("Array sizes don't match");

	mp_buffer_info_t wave_info;
	mp_get_buffer_raise(args[2], &wave_info, MP_BUFFER_READ);
	size_t rwave_len = wave_info.len;
	uint8_t *rwave = wave_info.buf;
	mp_get_buffer_raise(args[3], &wave_info, MP_BUFFER_READ);
	size_t gwave_len = wave_info.len;
	uint8_t *gwave = wave_info.buf;
	mp_get_buffer_raise(args[4], &wave_info, MP_BUFFER_READ);
	size_t bwave_len = wave_info.len;
	uint8_t *bwave = wave_info.buf;

	int phi_r = (rwave_len+(mp_obj_get_int(args[5])%rwave_len))%rwave_len,
	    phi_g = (gwave_len+(mp_obj_get_int(args[6])%gwave_len))%gwave_len,
	    phi_b = (bwave_len+(mp_obj_get_int(args[7])%bwave_len))%bwave_len;

	int i;
	for (i=0; i<fb_len; i+=3)
	{
		fb[i  ] = rwave[ (rwave_len + rot[i  ] + phi_r) % rwave_len ];
		fb[i+1] = gwave[ (gwave_len + rot[i+1] + phi_g) % gwave_len ];
		fb[i+2] = bwave[ (bwave_len + rot[i+2] + phi_b) % bwave_len ];
	}

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cball_gradient_obj, 8, 8, cball_gradient);

/* needed for the wobble animation, couldn't really mould it into a generic computation */
STATIC mp_obj_t cball_wobble(size_t n_args, const mp_obj_t *args)
{
	mp_buffer_info_t fb_info;
	mp_get_buffer_raise(args[0], &fb_info, MP_BUFFER_WRITE);
	size_t fb_len = fb_info.len;
	uint8_t *fb = fb_info.buf;

	mp_buffer_info_t rotations_info;
	mp_get_buffer_raise(args[1], &rotations_info, MP_BUFFER_READ);
	if (rotations_info.typecode != 'H')
		mp_raise_ValueError("rotations needs to be a uarray.array('H',...)");

	size_t rot_len = rotations_info.len / sizeof(uint16_t);
	uint16_t *rot = rotations_info.buf;

	if (fb_len != rot_len)
		mp_raise_ValueError("Array sizes don't match");

	int component = mp_obj_get_int(args[2]);
	if (component < 0 || component >= 3)
		mp_raise_ValueError("component needs to be 0, 1 or 2");

	float phi  = mp_obj_get_float(args[3]);
	float tide = sinf((float)M_PI*2.f*phi);
	phi = phi*24.f*M_PI*2.f;

	int i;
	for (i=component; i<fb_len; i+=3)
	{
		float w = 1.f+cosf( phi+(float)rot[i]*((float)M_PI*2.f/1024.f) )*tide;
        int b = (int)(w*w*64.);
		if (b<0) b=0;
		else if (b>255) b=255;
		fb[i] = b;
	}

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cball_wobble_obj, 4, 4, cball_wobble);


STATIC const mp_rom_map_elem_t cball_module_globals_table[] =
{
	{ MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_cball) },
	{ MP_ROM_QSTR(MP_QSTR_framebuf8to16), MP_ROM_PTR(&cball_framebuf8to16_obj) },
	{ MP_ROM_QSTR(MP_QSTR_apply_palette), MP_ROM_PTR(&cball_apply_palette_obj) },
	{ MP_ROM_QSTR(MP_QSTR_latt_long_map), MP_ROM_PTR(&cball_latt_long_map_obj) },
	{ MP_ROM_QSTR(MP_QSTR_bytearray_memset), MP_ROM_PTR(&cball_bytearray_memset_obj) },
	{ MP_ROM_QSTR(MP_QSTR_bytearray_memcpy), MP_ROM_PTR(&cball_bytearray_memcpy_obj) },
	{ MP_ROM_QSTR(MP_QSTR_bytearray_blend), MP_ROM_PTR(&cball_bytearray_blend_obj) },
	{ MP_ROM_QSTR(MP_QSTR_shader), MP_ROM_PTR(&cball_shader_obj) },
	{ MP_ROM_QSTR(MP_QSTR_gradient), MP_ROM_PTR(&cball_gradient_obj) },
	{ MP_ROM_QSTR(MP_QSTR_wobble), MP_ROM_PTR(&cball_wobble_obj) },
	{ MP_ROM_QSTR(MP_QSTR_ca_update), MP_ROM_PTR(&cball_ca_update_obj) },
	{ MP_ROM_QSTR(MP_QSTR_orbit_update), MP_ROM_PTR(&cball_orbit_update_obj) },
	{ MP_ROM_QSTR(MP_QSTR_lorenz_update), MP_ROM_PTR(&cball_lorenz_update_obj) },
};

STATIC MP_DEFINE_CONST_DICT(cball_module_globals, cball_module_globals_table);

const mp_obj_module_t cball_user_cmodule =
{
	.base = { &mp_type_module },
	.globals = (mp_obj_dict_t*)&cball_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_cball, cball_user_cmodule, MODULE_CBALL_ENABLED);

