
#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include "py/binary.h"

#include <math.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <endian.h>

static size_t cball_get_buffer(const mp_obj_t buf_obj, int typecode, size_t elem_size, void **buf, mp_uint_t flags, const char *error_msg)
{
	mp_buffer_info_t info;
	mp_get_buffer_raise(buf_obj, &info, flags);

	if (info.typecode != typecode)
		mp_raise_ValueError(error_msg);

	*buf = info.buf;
	return info.len/elem_size;
}

static size_t cball_get_float_array(const mp_obj_t buf_obj, float **buf, mp_uint_t flags, const char *error_msg)
{
	return cball_get_buffer(buf_obj, 'f', sizeof(float), (void **)buf, flags, error_msg);
}

static size_t cball_get_uint16_array(const mp_obj_t buf_obj, uint16_t **buf, mp_uint_t flags, const char *error_msg)
{
	return cball_get_buffer(buf_obj, 'H', sizeof(uint16_t), (void **)buf, flags, error_msg);
}

static size_t cball_get_bytearray(const mp_obj_t buf_obj, uint8_t **buf, mp_uint_t flags, const char *error_msg)
{
	return cball_get_buffer(buf_obj, BYTEARRAY_TYPECODE, sizeof(uint8_t), (void **)buf, flags, error_msg);
}

static int cball_get_led_order(int led_order[4], mp_obj_t order_str)
{
	size_t len;
	const char *str = mp_obj_str_get_data(order_str, &len);
	int i;

	led_order[0] = led_order[1] = led_order[2] = led_order[3] = -1;

	if ( len == 3 || len == 4 )
		for (i=0; i<len; i++)
			switch ( str[i]&~0x20 )
			{
				case 'R':
					led_order[0] = i;
					break;
				case 'G':
					led_order[1] = i;
					break;
				case 'B':
					led_order[2] = i;
					break;
				case 'W':
					led_order[3] = i;
					break;
				default:
					break;
			}

	if ( led_order[0] == -1 ||
	     led_order[1] == -1 ||
	     led_order[2] == -1 ||
	     ( len == 4 && led_order[3] == -1 ) )
		mp_raise_ValueError("bad led_order");

	return len;
}

static int cball_rgb_8to16(uint16_t dest[], const uint8_t src[], size_t led_count, const int led_order[3], const uint16_t gamma_map[256])
{

	size_t i, r_ix=led_order[0], g_ix=led_order[1], b_ix=led_order[2];
	const uint8_t *p_src = src;
	uint16_t *p_dest = dest;

	if ( (r_ix|g_ix|b_ix) != 3 || (r_ix+g_ix+b_ix) != 3 )
		return 0;

	for (i=0; i<led_count; i++)
	{
		p_dest[r_ix] = htole16(gamma_map[*p_src++]);
		p_dest[g_ix] = htole16(gamma_map[*p_src++]);
		p_dest[b_ix] = htole16(gamma_map[*p_src++]);
		p_dest += 3;
	}

	return 1;
}

static int cball_rgbw_8to16(uint16_t dest[], const uint8_t src[], size_t led_count, const int led_order[4], const uint16_t gamma_map[256])
{

	size_t i, r_ix=led_order[0], g_ix=led_order[1], b_ix=led_order[2], w_ix=led_order[3];
	const uint8_t *p_src = src;
	uint16_t *p_dest = dest;

	if ( (r_ix|g_ix|b_ix|w_ix) != 3 ||
	     (r_ix+g_ix+b_ix+w_ix) != 6 ||
	     (r_ix+1)*(g_ix+1)*(b_ix+1)*(w_ix+1) != 24 )
		return 0;

	for (i=0; i<led_count; i++)
	{
		int r, g, b, w;
		r = gamma_map[*p_src++];
		g = gamma_map[*p_src++];
		b = gamma_map[*p_src++];

		w = r;
		if (g < w)
			w = g;
		if (b < w)
			w = b;

		r -= w;
		g -= w;
		b -= w;

		p_dest[r_ix] = htole16(r);
		p_dest[g_ix] = htole16(g);
		p_dest[b_ix] = htole16(b);
		p_dest[w_ix] = htole16(w);
		p_dest += 4;
	}

	return 1;
}


STATIC mp_obj_t cball_fillbuffer_gamma(size_t n_args, const mp_obj_t *args)
{
	/* args:
	 * -     dest             [n_leds*3+2]  uint16,
	 * -     src              [n_leds*3]    uint8,
	 * -     led_order                      string, [ "RGB" | "GRB" | ... | "RGBW" | "GRBW" | ... ]
	 * -     gamma_map        [256]         uint16,
	 */

	uint16_t *dest;
	size_t dest_len = cball_get_uint16_array(args[0], &dest, MP_BUFFER_WRITE,
	                  "dest needs to be a uarray.array('H',...)");

	uint8_t *src;
	size_t src_len  = cball_get_bytearray(args[1], &src, MP_BUFFER_READ,
	                  "src needs to be a bytearray");

	if ( src_len % 3 != 0 )
		mp_raise_ValueError("size of src needs to be a multiple of 3");

	size_t led_count = src_len/3;

	int led_order[4];
	int n_components = cball_get_led_order(led_order, args[2]);

	if ( led_count*n_components+2 > dest_len )
		mp_raise_ValueError("dest array too small");

	uint16_t *gamma_map;
	size_t gamma_map_len = cball_get_uint16_array(args[3], &gamma_map, MP_BUFFER_READ,
	                       "gamma_map needs to be a uarray.array('H',...)");

	if ( gamma_map_len != 256 )
		mp_raise_ValueError("gamma_map array needs to have 256 elements");

	if ( n_components == 4 )
	{
		if ( !cball_rgbw_8to16(dest, src, led_count, led_order, gamma_map) )
			mp_raise_ValueError("remap index out of range");
	}
	else
	{
		if ( !cball_rgb_8to16(dest, src, led_count, led_order, gamma_map) )
			mp_raise_ValueError("remap index out of range");
	}

	dest[led_count*n_components  ] = htole16(0xffff);
	dest[led_count*n_components+1] = htole16(0xf0ff);

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cball_fillbuffer_gamma_obj, 4, 4, cball_fillbuffer_gamma);

static int cball_rgb_floatto16(uint16_t dest[], const float src[], size_t led_count, const int led_order[3], int max)
{
	if (max > UINT16_MAX)
		max = UINT16_MAX;

	size_t i, r_ix=led_order[0], g_ix=led_order[1], b_ix=led_order[2];
	const float *p_src = src;
	uint16_t *p_dest = dest;
	float f_max = (float)max;

	if ( (r_ix|g_ix|b_ix) != 3 || (r_ix+g_ix+b_ix) != 3 )
		return 0;

	for (i=0; i<led_count; i++)
	{
		int h;
		float v;

		v = *p_src++ * f_max;

		                    h = (int)v;
		     if (v < 0.f)   h = 0;
		else if (v > f_max) h = max;

		p_dest[r_ix] = htole16(h);

		v = *p_src++ * f_max;

		                    h = (int)v;
		     if (v < 0.f)   h = 0;
		else if (v > f_max) h = max;

		p_dest[g_ix] = htole16(h);

		v = *p_src++ * f_max;

		                    h = (int)v;
		     if (v < 0.f)   h = 0;
		else if (v > f_max) h = max;

		p_dest[b_ix] = htole16(h);

		p_dest += 3;
	}

	return 1;
}

static int cball_rgbw_floatto16(uint16_t dest[], const float src[], size_t led_count, const int led_order[4], int max)
{
	if (max > UINT16_MAX)
		max = UINT16_MAX;

	size_t i, r_ix=led_order[0], g_ix=led_order[1], b_ix=led_order[2], w_ix=led_order[3];
	const float *p_src = src;
	uint16_t *p_dest = dest;
	float f_max = (float)max;

	if ( (r_ix|g_ix|b_ix|w_ix) == 3 &&
	     (r_ix+g_ix+b_ix+w_ix) == 6 &&
	     (r_ix+1)*(g_ix+1)*(b_ix+1)*(w_ix+1) == 24 )
		return 0;

	for (i=0; i<led_count; i++)
	{
		float v;

		int r, g, b, w;

		v = *p_src++ * f_max;

		                    r = (int)v;
		     if (v < 0.f)   r = 0;
		else if (v > f_max) r = max;

		v = *p_src++ * f_max;

		                    g = (int)v;
		     if (v < 0.f)   g = 0;
		else if (v > f_max) g = max;

		v = *p_src++ * f_max;

		                    b = (int)v;
		     if (v < 0.f)   b = 0;
		else if (v > f_max) b = max;

		w = r;
		if (g < w)
			w = g;
		if (b < w)
			w = b;

		r -= w;
		g -= w;
		b -= w;

		p_dest[r_ix] = htole16(r);
		p_dest[g_ix] = htole16(g);
		p_dest[b_ix] = htole16(b);
		p_dest[w_ix] = htole16(w);
		p_dest += 4;
	}

	return 1;
}

STATIC mp_obj_t cball_fillbuffer_float(size_t n_args, const mp_obj_t *args)
{
	/* args:
	 * -     dest             [n_leds*3+2]  uint16,
	 * -     src              [n_leds*3]    float,
	 * -     led_order                      string, [ "RGB" | "GRB" | ... | "RGBW" | "GRBW" | ... ]
	 * -     max                            float,
	 */

	uint16_t *dest;
	size_t dest_len = cball_get_uint16_array(args[0], &dest, MP_BUFFER_WRITE,
	                  "dest needs to be a uarray.array('H',...)");

	float *src;
	size_t src_len  = cball_get_float_array(args[1], &src, MP_BUFFER_READ,
	                  "src needs to be a uarray.array('f',...)");

	if ( src_len % 3 != 0 )
		mp_raise_ValueError("size of src needs to be a multiple of 3");

	size_t led_count = src_len/3;

	int led_order[4];
	int n_components = cball_get_led_order(led_order, args[2]);

	if ( led_count*n_components+2 > dest_len )
		mp_raise_ValueError("dest array too small");

	int max = mp_obj_get_int(args[3]);

	if ( n_components == 4 )
	{
		if ( !cball_rgbw_floatto16(dest, src, led_count, led_order, max) )
			mp_raise_ValueError("remap index out of range");
	}
	else
	{
		if ( !cball_rgb_floatto16(dest, src, led_count, led_order, max) )
			mp_raise_ValueError("remap index out of range");
	}

	dest[led_count*n_components  ] = htole16(0xffff);
	dest[led_count*n_components+1] = htole16(0xf0ff);

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cball_fillbuffer_float_obj, 4, 4, cball_fillbuffer_float);


static int cball_remap_rgb_8to16(uint16_t dest[], const uint8_t src[], const uint16_t remap[], size_t led_count, const int led_order[3], const uint16_t gamma_map[256])
{

	size_t i, r_ix=led_order[0], g_ix=led_order[1], b_ix=led_order[2];
	const uint8_t *p_src = src;

	if ( (r_ix|g_ix|b_ix) != 3 || (r_ix+g_ix+b_ix) != 3 )
		return 0;

	for (i=0; i<led_count; i++)
	{
		int index = remap[i];

		if ( !(index < led_count) )
			return 0;

		index *= 3;
		dest[index+r_ix] = htole16(gamma_map[*p_src++]);
		dest[index+g_ix] = htole16(gamma_map[*p_src++]);
		dest[index+b_ix] = htole16(gamma_map[*p_src++]);
	}

	return 1;
}


static int cball_remap_rgbw_8to16(uint16_t dest[], const uint8_t src[], const uint16_t remap[], size_t led_count, const int led_order[4], const uint16_t gamma_map[256])
{

	size_t i, r_ix=led_order[0], g_ix=led_order[1], b_ix=led_order[2], w_ix=led_order[3];
	const uint8_t *p_src = src;

	if ( (r_ix|g_ix|b_ix|w_ix) != 3 ||
	     (r_ix+g_ix+b_ix+w_ix) != 6 ||
	     (r_ix+1)*(g_ix+1)*(b_ix+1)*(w_ix+1) != 24 )
		return 0;

	for (i=0; i<led_count; i++)
	{
		int index = remap[i];

		if ( !(index < led_count) )
			return 0;

		index *= 4;
		int r, g, b, w;
		r = gamma_map[*p_src++];
		g = gamma_map[*p_src++];
		b = gamma_map[*p_src++];

		w = r;
		if (g < w)
			w = g;
		if (b < w)
			w = b;

		r -= w;
		g -= w;
		b -= w;

		dest[index+r_ix] = htole16(r);
		dest[index+g_ix] = htole16(g);
		dest[index+b_ix] = htole16(b);
		dest[index+w_ix] = htole16(w);
	}

	return 1;
}

STATIC mp_obj_t cball_fillbuffer_remap_gamma(size_t n_args, const mp_obj_t *args)
{
	/* args:
	 * -     dest             [n_leds*3+2]  uint16,
	 * -     src              [n_leds*3]    uint8,
	 * -     remap            [n_leds]      uint16,
	 * -     led_order                      string, [ "RGB" | "GRB" | ... | "RGBW" | "GRBW" | ... ]
	 * -     gamma_map        [256]         uint16,
	 */

	uint16_t *dest;
	size_t dest_len = cball_get_uint16_array(args[0], &dest, MP_BUFFER_WRITE,
	                  "dest needs to be a uarray.array('H',...)");

	uint8_t *src;
	size_t src_len  = cball_get_bytearray(args[1], &src, MP_BUFFER_READ,
	                  "src needs to be a bytearray");

	uint16_t *remap;
	size_t led_count = cball_get_uint16_array(args[2], &remap, MP_BUFFER_READ,
	                   "remap needs to be a uarray.array('H',...)");

	if ( led_count != 3*src_len )
		mp_raise_ValueError("size of src needs to be 3x size of remap array");

	int led_order[4];
	int n_components = cball_get_led_order(led_order, args[3]);

	if ( led_count*n_components+2 > dest_len )
		mp_raise_ValueError("dest array too small");

	uint16_t *gamma_map;
	size_t gamma_map_len = cball_get_uint16_array(args[4], &gamma_map, MP_BUFFER_READ,
	                       "gamma_map needs to be a uarray.array('H',...)");

	if ( gamma_map_len != 256 )
		mp_raise_ValueError("gamma_map array needs to have 256 elements");

	if ( n_components == 4 )
	{
		if ( !cball_remap_rgbw_8to16(dest, src, remap, led_count, led_order, gamma_map) )
			mp_raise_ValueError("remap index out of range");
	}
	else
	{
		if ( !cball_remap_rgb_8to16(dest, src, remap, led_count, led_order, gamma_map) )
			mp_raise_ValueError("remap index out of range");
	}

	dest[led_count*n_components  ] = htole16(0xffff);
	dest[led_count*n_components+1] = htole16(0xf0ff);

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cball_fillbuffer_remap_gamma_obj, 5, 5, cball_fillbuffer_remap_gamma);

static int cball_remap_rgb_floatto16(uint16_t dest[], const float src[], const uint16_t remap[], size_t led_count, const int led_order[3], int max)
{
	if (max > UINT16_MAX)
		max = UINT16_MAX;

	size_t i, r_ix = led_order[0], g_ix = led_order[1], b_ix=led_order[2];
	const float *p_src = src;
	float f_max = (float)max;

	if ( (r_ix|g_ix|b_ix) != 3 || (r_ix+g_ix+b_ix) != 3 )
		return 0;

	for (i=0; i<led_count; i++)
	{
		int index = remap[i], h;
		float v;

		if ( !(index < led_count) )
			return 0;

		index *= 3;

		v = *p_src++ * f_max;

		                    h = (int)v;
		     if (v < 0.f)   h = 0;
		else if (v > f_max) h = max;

		dest[index+r_ix] = htole16(h);

		v = *p_src++ * f_max;

		                    h = (int)v;
		     if (v < 0.f)   h = 0;
		else if (v > f_max) h = max;

		dest[index+g_ix] = htole16(h);

		v = *p_src++ * f_max;

		                    h = (int)v;
		     if (v < 0.f)   h = 0;
		else if (v > f_max) h = max;

		dest[index+b_ix] = htole16(h);
	}

	return 1;
}


static int cball_remap_rgbw_floatto16(uint16_t dest[], const float src[], const uint16_t remap[], size_t led_count, const int led_order[4], int max)
{
	if (max > UINT16_MAX)
		max = UINT16_MAX;

	size_t i, r_ix = led_order[0], g_ix = led_order[1], b_ix=led_order[2], w_ix=led_order[3];
	const float *p_src = src;
	float f_max = (float)max;

	if ( (r_ix|g_ix|b_ix|w_ix) == 3 &&
	     (r_ix+g_ix+b_ix+w_ix) == 6 &&
	     (r_ix+1)*(g_ix+1)*(b_ix+1)*(w_ix+1) == 24 )
		return 0;

	for (i=0; i<led_count; i++)
	{
		int index = remap[i];
		float v;

		if ( !(index < led_count) )
			return 0;

		index *= 4;

		int r, g, b, w;

		v = *p_src++ * f_max;

		                    r = (int)v;
		     if (v < 0.f)   r = 0;
		else if (v > f_max) r = max;

		v = *p_src++ * f_max;

		                    g = (int)v;
		     if (v < 0.f)   g = 0;
		else if (v > f_max) g = max;

		v = *p_src++ * f_max;

		                    b = (int)v;
		     if (v < 0.f)   b = 0;
		else if (v > f_max) b = max;

		w = r;
		if (g < w)
			w = g;
		if (b < w)
			w = b;

		r -= w;
		g -= w;
		b -= w;

		dest[index+r_ix] = htole16(r);
		dest[index+g_ix] = htole16(g);
		dest[index+b_ix] = htole16(b);
		dest[index+w_ix] = htole16(w);
	}

	return 1;
}

STATIC mp_obj_t cball_fillbuffer_remap_float(size_t n_args, const mp_obj_t *args)
{
	/* args:
	 * -     dest             [n_leds*3+2]  uint16,
	 * -     src              [n_leds*3]    float,
	 * -     remap            [n_leds]      uint16,
	 * -     led_order                      string, [ "RGB" | "GRB" | ... | "RGBW" | "GRBW" | ... ]
	 * -     max                            int,
	 */

	uint16_t *dest;
	size_t dest_len = cball_get_uint16_array(args[0], &dest, MP_BUFFER_WRITE,
	                  "dest needs to be a uarray.array('H',...)");

	float *src;
	size_t src_len  = cball_get_float_array(args[1], &src, MP_BUFFER_READ,
	                  "src needs to be a uarray.array('f',...)");

	uint16_t *remap;
	size_t led_count = cball_get_uint16_array(args[2], &remap, MP_BUFFER_READ,
	                   "remap needs to be a uarray.array('H',...)");

	if ( led_count != 3*src_len )
		mp_raise_ValueError("size of src needs to be 3x size of remap array");

	int led_order[4];
	int n_components = cball_get_led_order(led_order, args[3]);

	if ( led_count*n_components+2 > dest_len )
		mp_raise_ValueError("dest array too small");

	int max = mp_obj_get_int(args[4]);

	if ( n_components == 4 )
	{
		if ( !cball_remap_rgbw_floatto16(dest, src, remap, led_count, led_order, max) )
			mp_raise_ValueError("remap index out of range");
	}
	else
	{
		if ( !cball_remap_rgb_floatto16(dest, src, remap, led_count, led_order, max) )
			mp_raise_ValueError("remap index out of range");
	}

	dest[led_count*n_components  ] = htole16(0xffff);
	dest[led_count*n_components+1] = htole16(0xf0ff);

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cball_fillbuffer_remap_float_obj, 5, 5, cball_fillbuffer_remap_float);

STATIC mp_obj_t cball_bytearray_memcpy(mp_obj_t out, mp_obj_t in)
{
	/* args:
	 * -     out [...]  uint8,
	 * -     in  [...]  uint8,
	 *
	 * memcpy(out, in, max(out_len, in_len))
	 *
	 */

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

const float log_prime[] =
{
// [ math.log(i) for i in [ 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251 ] ]
0.6931471805599453, 1.0986122886681098, 1.6094379124341003, 1.9459101490553132, 2.3978952727983707, 2.5649493574615367, 2.833213344056216, 2.9444389791664403, 3.1354942159291497, 3.367295829986474, 3.4339872044851463, 3.6109179126442243, 3.713572066704308, 3.7612001156935624, 3.8501476017100584, 3.970291913552122, 4.07753744390572, 4.110873864173311, 4.204692619390966, 4.2626798770413155, 4.290459441148391, 4.3694478524670215, 4.418840607796598, 4.48863636973214, 4.574710978503383, 4.61512051684126, 4.634728988229636, 4.672828834461906, 4.6913478822291435, 4.727387818712341, 4.844187086458591, 4.875197323201151, 4.919980925828125, 4.9344739331306915, 5.003946305945459, 5.017279836814924, 5.056245805348308, 5.093750200806762, 5.117993812416755, 5.153291594497779, 5.187385805840755, 5.198497031265826, 5.25227342804663, 5.262690188904886, 5.2832037287379885, 5.293304824724492, 5.351858133476067, 5.407171771460119, 5.424950017481403, 5.43372200355424, 5.4510384535657, 5.476463551931511, 5.484796933490655, 5.5254529391317835
};

STATIC mp_obj_t cball_calc_gamma_map_sieve(size_t n_args, const mp_obj_t *args)
{
	/* args:
	 * -     gamma_map        [256]         uint16,
	 * -     gamma                          float,
	 * -     max                            int,
	 * -     cut_off                        int,
	 */

	int prime_ix = 0;
	#define NEXT_PRIME_POW(x) ( expf(x*log_prime[prime_ix++]) )

	uint16_t *gamma_map;
	size_t gamma_map_len = cball_get_uint16_array(args[0], &gamma_map, MP_BUFFER_WRITE,
	                       "gamma_map needs to be a uarray.array('H',...)");

	if (gamma_map_len != 256)
		mp_raise_ValueError("gamma_map needs to have 256 elements");

	float gamma = mp_obj_get_float(args[1]);

	int max = mp_obj_get_int(args[2]);
	if (max < 0 || max > 0xffff)
		mp_raise_ValueError("max needs to be in the range [0, 65535]");

	int cut_off = mp_obj_get_int(args[3]);
	if (cut_off < 0 || cut_off > 127)
		mp_raise_ValueError("max needs to be in the range [0, 127]");

	int i, j, k, l;

	static float *fgamma_map = NULL;
	if (fgamma_map == NULL)
		fgamma_map = calloc(256, sizeof(float));

	for (i=0; i<256; i++)
		fgamma_map[i] = 0.f;

	fgamma_map[1]   = 1.f;
	fgamma_map[2]   = NEXT_PRIME_POW( gamma ); /* = powf(  2.f, gamma ); */
	fgamma_map[3]   = NEXT_PRIME_POW( gamma ); /* = powf(  3.f, gamma ); */
	fgamma_map[5]   = NEXT_PRIME_POW( gamma ); /* = powf(  5.f, gamma ); */
	fgamma_map[7]   = NEXT_PRIME_POW( gamma ); /* = powf(  7.f, gamma ); */
	fgamma_map[11]  = NEXT_PRIME_POW( gamma ); /* = powf( 11.f, gamma ); */

	for (i=1; i<127; i<<=1)
	{
		if (i!=1)
			fgamma_map[i<<1] = fgamma_map[i]*fgamma_map[2];

		for (j=i; j<85; j*=3)
		{
			if (j!=1)
				fgamma_map[j*3] = fgamma_map[j]*fgamma_map[3];

			for (k=j; k<51; k*=5)
			{
				if (k!=1)
					fgamma_map[k*5] = fgamma_map[k]*fgamma_map[5];

				for (l=k; l<37; l*=7)
				{
					if (l!=1)
					{
						fgamma_map[l*7] = fgamma_map[l]*fgamma_map[7];
						if (l < 24)
							fgamma_map[l*11] = fgamma_map[l]*fgamma_map[11];
					}
				}
			}
		}
	}

	fgamma_map[121] = fgamma_map[11]*fgamma_map[11];
	fgamma_map[242] = fgamma_map[121]*fgamma_map[2];

	for (i=13; i<256; i+=2)
		if (fgamma_map[i] == 0.f)
		{
			fgamma_map[i] = NEXT_PRIME_POW( gamma ); /* = powf( (float)i, gamma ); */
			for (j=1; j*i<=255; j++)
				if (fgamma_map[j] != 0.f)
					fgamma_map[j*i] = fgamma_map[j] * fgamma_map[i];
		}

#undef NEXT_PRIME_POW
	float factor = (float)max / fgamma_map[255];

	for (i=0; i<256; i++)
	{
		int g_int = fgamma_map[i] * factor;
		if (g_int < (cut_off>>1))
			g_int = 0;
		else
		{
			int lo = g_int & 0xff;
			int hi = g_int & 0xff00;
			if (lo < cut_off)
			{
				g_int = hi;
				if (lo >= (cut_off>>1) )
					g_int += cut_off;
			}
			else if (lo > 0x100-cut_off)
			{
				g_int = hi + 0x100;
				if (lo <= 0x100-(cut_off>>1) )
					g_int -= cut_off;
			}

			if (g_int > max)
				g_int = max;

		}
		gamma_map[i] = (uint16_t)g_int;
	}
	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cball_calc_gamma_map_sieve_obj, 4, 4, cball_calc_gamma_map_sieve);

static const float dividers[11] =
{
	-42.f,
	1.f,
	1.f/2.f,
	1.f/3.f,
	1.f/4.f,
	1.f/5.f,
	1.f/6.f,
	1.f/7.f,
	1.f/8.f,
	1.f/9.f,
	1.f/10.f,
};

static const struct
{
	uint8_t index;
	uint8_t jump;
} interpol_table[62] =
{
        { 12, 2, }, { 16, 2, }, { 18, 2, }, { 22, 2, }, { 25, 2, },
        { 28, 2, }, { 30, 2, }, { 33, 2, }, { 36, 4, }, { 40, 2, },
        { 42, 2, }, { 45, 3, }, { 50, 4, }, { 56, 4, }, { 60, 3, },
        { 64, 2, }, { 66, 4, }, { 70, 2, }, { 72, 3, }, { 75, 2, },
        { 77, 3, }, { 81, 3, }, { 84, 4, }, { 88, 2, }, { 90, 6, },
        { 96, 2, }, { 100, 5, }, { 105, 3, }, { 108, 2, }, { 110, 2, },
        { 112, 8, }, { 121, 4, }, { 126, 2, }, { 128, 4, }, { 132, 3, },
        { 135, 5, }, { 140, 4, }, { 144, 3, }, { 147, 3, }, { 150, 4, },
        { 154, 6, }, { 160, 2, }, { 162, 3, }, { 165, 3, }, { 168, 7, },
        { 176, 4, }, { 180, 9, }, { 189, 3, }, { 192, 4, }, { 196, 2, },
        { 198, 2, }, { 200, 10, }, { 210, 6, }, { 216, 4, }, { 220, 4, },
        { 225, 6, }, { 231, 9, }, { 240, 2, }, { 243, 2, }, { 245, 5, },
        { 250, 2, }, { 252, 3, },
};

STATIC mp_obj_t cball_calc_gamma_map_fast(size_t n_args, const mp_obj_t *args)
{
	/* args:
	 * -     gamma_map        [256]         uint16,
	 * -     gamma                          float,
	 * -     max                            int,
	 * -     cut_off                        int,
	 */

	uint16_t *gamma_map;
	size_t gamma_map_len = cball_get_uint16_array(args[0], &gamma_map, MP_BUFFER_WRITE,
	                       "gamma_map needs to be a uarray.array('H',...)");

	if (gamma_map_len != 256)
		mp_raise_ValueError("gamma_map needs to have 256 elements");

	float gamma = mp_obj_get_float(args[1]);

	int max = mp_obj_get_int(args[2]);
	if (max < 0 || max > 0xffff)
		mp_raise_ValueError("max needs to be in the range [0, 65535]");

	int cut_off = mp_obj_get_int(args[3]);
	if (cut_off < 0 || cut_off > 127)
		mp_raise_ValueError("max needs to be in the range [0, 127]");

	int i, j, k, l;

	static float *fgamma_map = NULL;
	if (fgamma_map == NULL)
		fgamma_map = calloc(256, sizeof(float));

	for (i=0; i<256; i++)
		fgamma_map[i] = 0.f;


	fgamma_map[1]   = 1.f;
	fgamma_map[2]   = expf(0.6931471805599453f * gamma ); /* expf(log(2)*gamma) */
	fgamma_map[3]   = expf(1.0986122886681098f * gamma );
	fgamma_map[5]   = expf(1.6094379124341003f * gamma );
	fgamma_map[7]   = expf(1.9459101490553132f * gamma );
	fgamma_map[11]  = expf(2.3978952727983707f * gamma );
	fgamma_map[255] = expf(5.5412635451584260f * gamma );

	for (i=1; i<127; i<<=1)
	{
		if (i!=1)
			fgamma_map[i<<1] = fgamma_map[i]*fgamma_map[2];

		for (j=i; j<85; j*=3)
		{
			if (j!=1)
				fgamma_map[j*3] = fgamma_map[j]*fgamma_map[3];

			for (k=j; k<51; k*=5)
			{
				if (k!=1)
					fgamma_map[k*5] = fgamma_map[k]*fgamma_map[5];

				for (l=k; l<37; l*=7)
				{
					if (l!=1)
					{
						fgamma_map[l*7] = fgamma_map[l]*fgamma_map[7];
						if (l < 24)
							fgamma_map[l*11] = fgamma_map[l]*fgamma_map[11];
					}
				}
			}
		}
	}

	fgamma_map[121] = fgamma_map[11]*fgamma_map[11];
	fgamma_map[242] = fgamma_map[121]*fgamma_map[2];

	for (i=0; i<62; i++)
	{
		int last = interpol_table[i].index;
		int num  = interpol_table[i].jump;
		float div = dividers[num];
		for (j=1; j<num; j++)
			fgamma_map[last+j] = ( (float)(num-j)*fgamma_map[last] +
			                       (float)(    j)*fgamma_map[last+num] )  *  div;
	}

	float factor = (float)max / fgamma_map[255];

	for (i=0; i<256; i++)
	{
		int g_int = fgamma_map[i] * factor;
		if (g_int < (cut_off>>1))
			g_int = 0;
		else
		{
			int lo = g_int & 0xff;
			int hi = g_int & 0xff00;
			if (lo < cut_off)
			{
				g_int = hi;
				if (lo >= (cut_off>>1) )
					g_int += cut_off;
			}
			else if (lo > 0x100-cut_off)
			{
				g_int = hi + 0x100;
				if (lo <= 0x100-(cut_off>>1) )
					g_int -= cut_off;
			}

			if (g_int > max)
				g_int = max;

		}
		gamma_map[i] = (uint16_t)g_int;
	}
	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cball_calc_gamma_map_fast_obj, 4, 4, cball_calc_gamma_map_fast);



STATIC mp_obj_t cball_calc_gamma_map(size_t n_args, const mp_obj_t *args)
{
	/* args:
	 * -     gamma_map        [256]         uint16,
	 * -     gamma                          float,
	 * -     max                            int,
	 * -     cut_off                        int,
	 */

	uint16_t *gamma_map;
	size_t gamma_map_len = cball_get_uint16_array(args[0], &gamma_map, MP_BUFFER_WRITE,
	                       "gamma_map needs to be a uarray.array('H',...)");

	if (gamma_map_len != 256)
		mp_raise_ValueError("gamma_map needs to have 256 elements");

	float gamma = mp_obj_get_float(args[1]);

	int max = mp_obj_get_int(args[2]);
	if (max < 0 || max > 0xffff)
		mp_raise_ValueError("max needs to be in the range [0, 65535]");

	int cut_off = mp_obj_get_int(args[3]);
	if (cut_off < 0 || cut_off > 127)
		mp_raise_ValueError("max needs to be in the range [0, 127]");

	int i;

	float factor = (float)max / powf( 255.f, gamma );
	for (i=0; i<256; i++)
	{
		float g = powf( (float)i, gamma ) * factor;
		int g_int = (int)g;
		if (g_int < (cut_off>>1))
			g_int = 0;
		else
		{
			int lo = g_int & 0xff;
			int hi = g_int & 0xff00;
			if (lo < cut_off)
			{
				g_int = hi;
				if (lo >= (cut_off>>1))
					g_int += cut_off;
			}
			else if (lo > 0x100-cut_off)
			{
				g_int = hi + 0x100;
				if (lo <= 0x100-(cut_off>>1) )
					g_int -= cut_off;
			}

			if (g_int > max)
				g_int = max;

		}
		gamma_map[i] = (uint16_t)g_int;
	}
	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cball_calc_gamma_map_obj, 4, 4, cball_calc_gamma_map);

STATIC mp_obj_t cball_apply_palette(mp_obj_t buf_dest, mp_obj_t buf_src, mp_obj_t palette)
{
	/* args:
	 * -     framebuffer_out  [pixels*3]  uint8,
	 * -     framebuffer_in   [pixels]    uint8,
	 * -     palette          [n*3]       uint8,
	 */

	uint8_t *dest;
	size_t dest_len  = cball_get_bytearray(buf_dest, &dest, MP_BUFFER_WRITE,
	                   "dest needs to be a bytearray");

	if (dest_len%3 != 0)
		mp_raise_ValueError("dest array size must be a multiple of 3");

	uint8_t *src;
	size_t src_len  = cball_get_bytearray(buf_src, &src, MP_BUFFER_READ,
	                  "src needs to be a bytearray");

	uint8_t *table;
	size_t table_len  = cball_get_bytearray(palette, &table, MP_BUFFER_READ,
	                    "palette needs to be a bytearray");

	if (table_len%3 != 0)
		mp_raise_ValueError("palette array must be a multiple of 3");

	if (dest_len < src_len*3)
		src_len = dest_len/3;

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

	uint8_t *dest;
	size_t dest_len  = cball_get_bytearray(args[0], &dest, MP_BUFFER_WRITE,
	                   "dest needs to be a bytearray");

	if (dest_len%3 != 0)
		mp_raise_ValueError("dest array size must be a multiple of 3");

	uint8_t *src;
	size_t src_len  = cball_get_bytearray(args[1], &src, MP_BUFFER_READ,
	                  "src needs to be a bytearray");

	if (src_len%3 != 0)
		mp_raise_ValueError("src array must be a multiple of 3");

	uint16_t *voronoi_map;
	size_t voronoi_map_len = cball_get_uint16_array(args[2], &voronoi_map, MP_BUFFER_READ,
	                         "voronoi_map needs to be a uarray.array('H',...)");

	if (src_len != voronoi_map_len*3)
		mp_raise_ValueError("voronoi and source array sizes don't match");

	float *latt_weight;
	size_t latt_weight_len  = cball_get_float_array(args[3], &latt_weight, MP_BUFFER_READ,
	                          "latt_weight needs to be a uarray.array('f',...)");

	if ( voronoi_map_len % latt_weight_len != 0 )
		mp_raise_ValueError("wrong height");

	size_t h = latt_weight_len;
	size_t w = voronoi_map_len/latt_weight_len;

	float *total_weight;
	size_t total_weight_len  = cball_get_float_array(args[4], &total_weight, MP_BUFFER_READ,
	                          "total_weight needs to be a uarray.array('f',...)");

	if (total_weight_len*3 != dest_len)
		mp_raise_ValueError("dest and weight array sizes don't match");

	float *tmp;
	size_t tmp_len  = cball_get_float_array(args[5], &tmp, MP_BUFFER_WRITE,
	                  "tmp_buf needs to be a uarray.array('f',...)");

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
	/* args:
	 * -     array [...]  uint8,
	 * -     c            int,
	 */

	mp_buffer_info_t buf_info;
	mp_get_buffer_raise(array, &buf_info, MP_BUFFER_WRITE);
	memset(buf_info.buf, mp_obj_get_int(c), buf_info.len);
	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(cball_bytearray_memset_obj, cball_bytearray_memset);

STATIC mp_obj_t cball_bytearray_blend(size_t n_args, const mp_obj_t *args)
{
	/* args:
	 * -     out           [n]  uint8,
	 * -     in1           [n]  uint8,
	 * -     in2           [n]  uint8,
	 * -     interpolation      float
	 *
	 */

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
	/* args:
	 * -     objects     [n_objects*6]    float
	 * -     gmdt2                        float
	 * -     n_iterations                 int
	 */

	float *object_data;
	size_t object_data_len = cball_get_float_array(object_list, &object_data, MP_BUFFER_WRITE,
	                                              "object_list needs to be a uarray.array('f',...)");

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
	/* args:
	 * -     attractors  [n_attractors*6]    float
	 * -     dt                              float
	 * -     n_iterations                    int
	 */

	float *data_flat;
	size_t data_flat_len = cball_get_float_array(attractors, &data_flat, MP_BUFFER_WRITE,
	                                             "attractors needs to be a uarray.array('f',...)");

	if (data_flat_len % 6 != 0)
		mp_raise_ValueError("length of attractors not a multiple of 6");

	float dt = (float)mp_obj_get_float(delta_time);
	int    n = mp_obj_get_int(iterations);

	size_t i,j;
	for (i=0; i<data_flat_len; i+=6)
	{
		float x     = data_flat[i  ],
		      y     = data_flat[i+1],
		      z     = data_flat[i+2],
		      sigma = data_flat[i+3],
		      rho   = data_flat[i+4],
		      beta  = data_flat[i+5];

		for (j=0; j<n; j++)
		{
			x += sigma*(y-x)*dt;
			y += (x*(rho-z)-y)*dt;
			z += (x*y-beta*z)*dt;
		}

		data_flat[i  ] = x;
		data_flat[i+1] = y;
		data_flat[i+2] = z;
	}

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(cball_lorenz_update_obj, cball_lorenz_update);

STATIC mp_obj_t cball_shader(mp_obj_t framebuf, mp_obj_t leds_flat, mp_obj_t lights_flat)
{
	/* args:
	 * -     framebuffer  [n_leds*3]    uint8,
	 * -     led_data     [n_leds*6]    float
	 * -     lights_flat  [n_lights*6]  float
	 */

	uint8_t *fb;
	size_t fb_len = cball_get_bytearray(framebuf, &fb, MP_BUFFER_WRITE,
	                "framebuffer needs to be a bytearray");

	float *led_data;
	size_t led_data_len = cball_get_float_array(leds_flat, &led_data, MP_BUFFER_READ,
	                      "led_data needs to be a uarray.array('f',...)");

	if ( led_data_len%3 != 0 || led_data_len != fb_len*2)
		mp_raise_ValueError("Incorrect array sizes");

	float *lights_data;
	size_t lights_data_len = cball_get_float_array(lights_flat, &lights_data, MP_BUFFER_READ,
	                         "lights_flat needs to be a uarray.array('f',...)");

	if (lights_data_len % 6 != 0)
		mp_raise_ValueError("length of lights_flat not a multiple of 6");

	size_t i,j;
	for (i=0; i<fb_len; i+=3)
	{
		mp_float_t r=(float)fb[i],
		           g=(float)fb[i+1],
		           b=(float)fb[i+2];

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
		fb[i] = (uint8_t)r;
		fb[i+1] = (uint8_t)g;
		fb[i+2] = (uint8_t)b;
	}

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(cball_shader_obj, cball_shader);

STATIC mp_obj_t cball_gradient(size_t n_args, const mp_obj_t *args)
{
	/* args:
	 * -     framebuffer  [pixels*3]  uint8,
	 * -     rotations    [pixels*3]  uint16,
	 * -     rwave        [...]       uint16,
	 * -     gwave        [...]       uint16,
	 * -     bwave        [...]       uint16,
	 * -     rphi                     int,
	 * -     gphi                     int,
	 * -     bphi                     int,
	 */

	uint8_t *fb;
	size_t fb_len = cball_get_bytearray(args[0], &fb, MP_BUFFER_WRITE,
	                "framebuffer needs to be a bytearray");

	uint16_t *rot;
	size_t rot_len = cball_get_uint16_array(args[1], &rot, MP_BUFFER_READ,
	                 "rotations needs to be a uarray.array('H',...)");

	if (fb_len != rot_len)
		mp_raise_ValueError("Array sizes don't match");

	uint8_t *rwave;
	size_t rwave_len = cball_get_bytearray(args[2], &rwave, MP_BUFFER_READ,
	                   "rwave needs to be a bytearray");

	uint8_t *gwave;
	size_t gwave_len = cball_get_bytearray(args[3], &gwave, MP_BUFFER_READ,
	                   "gwave needs to be a bytearray");

	uint8_t *bwave;
	size_t bwave_len = cball_get_bytearray(args[4], &bwave, MP_BUFFER_READ,
	                   "bwave needs to be a bytearray");

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
	/* args:
	 * -     framebuffer  [pixels*3]  uint8,
	 * -     rotations    [pixels*3]  uint16,
	 * -     n_components             int,
	 * -     phase                    float,
	 */

	uint8_t *fb;
	size_t fb_len  = cball_get_bytearray(args[0], &fb, MP_BUFFER_WRITE,
	                 "framebuffer needs to be a bytearray");

	uint16_t *rot;
	size_t rot_len = cball_get_uint16_array(args[1], &rot, MP_BUFFER_READ,
	                 "rotations needs to be a uarray.array('H',...)");

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
	{ MP_ROM_QSTR(MP_QSTR_fillbuffer_gamma), MP_ROM_PTR(&cball_fillbuffer_gamma_obj) },
	{ MP_ROM_QSTR(MP_QSTR_fillbuffer_float), MP_ROM_PTR(&cball_fillbuffer_float_obj) },
	{ MP_ROM_QSTR(MP_QSTR_fillbuffer_remap_gamma), MP_ROM_PTR(&cball_fillbuffer_remap_gamma_obj) },
	{ MP_ROM_QSTR(MP_QSTR_fillbuffer_remap_float), MP_ROM_PTR(&cball_fillbuffer_remap_float_obj) },
	{ MP_ROM_QSTR(MP_QSTR_calc_gamma_map), MP_ROM_PTR(&cball_calc_gamma_map_obj) },
	{ MP_ROM_QSTR(MP_QSTR_calc_gamma_map_sieve), MP_ROM_PTR(&cball_calc_gamma_map_sieve_obj) },
	{ MP_ROM_QSTR(MP_QSTR_calc_gamma_map_fast), MP_ROM_PTR(&cball_calc_gamma_map_fast_obj) },
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

