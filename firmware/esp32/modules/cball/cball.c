
#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include "py/binary.h"

#include "esp_system.h"

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

STATIC mp_obj_t cball_framebuffer_gamma(mp_obj_t buf_obj, mp_obj_t gamma_map_obj, mp_obj_t cut_off_obj)
{
	/* args:
	 * -     buf              [n_leds*3]    uint16,
	 * -     gamma_map        [256]         uint16,
	 * -     cut_off                        int,
	 */

	uint16_t *buf;
	size_t buf_len = cball_get_uint16_array(buf_obj, &buf, MP_BUFFER_WRITE,
	                 "buf needs to be a uarray.array('H',...)");

	uint16_t *gamma_map;
	size_t gamma_map_len = cball_get_uint16_array(gamma_map_obj, &gamma_map, MP_BUFFER_READ,
	                       "gamma_map needs to be a uarray.array('H',...)");

	uint32_t cut_off = mp_obj_get_int(cut_off_obj);

	if ( gamma_map_len != 256 )
		mp_raise_ValueError("gamma_map array needs to have 256 elements");

	size_t i;

	for (i=0; i<buf_len; i++)
	{
		uint32_t in = buf[i];
		uint32_t gamma_ix = (in-(in>>8))>>8;
		uint32_t inter = (in-gamma_ix-(gamma_ix<<8));
		uint32_t out = gamma_map[gamma_ix];

		if (inter != 0) /* gamma_ix == 255, iff in == 0xffff, inter == 0 */
			out += ( (gamma_map[gamma_ix+1] - out ) * inter * 0xff + 0x100 )>>16;

		uint32_t low = out&0xff;

		if ( low < cut_off )
		{
			out &=~ 0xff;
			if ( (low<<1) > cut_off )
				out += cut_off;
		}
		else
		{
			low = 0x100-low;
			if ( low < cut_off )
			{
				out &=~ 0xff;
				out += 0x100;
				if ( (low<<1) > cut_off )
					out -= cut_off;
			}
		}

		buf[i] = htole16(out);
	}

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(cball_framebuffer_gamma_obj, cball_framebuffer_gamma);

STATIC mp_obj_t cball_framebuffer_floatto16(mp_obj_t dest_obj, mp_obj_t src_obj)
{
	/* args:
	 * -     dest             [n_leds*3]    uint16,
	 * -     src              [n_leds*3]    float,
	 */

	uint16_t *dest;
	size_t dest_len = cball_get_uint16_array(dest_obj, &dest, MP_BUFFER_WRITE,
	                  "dest needs to be a uarray.array('H',...)");

	float *src;
	size_t src_len  = cball_get_float_array(src_obj, &src, MP_BUFFER_READ,
	                  "src needs to be a uarray.array('f',...)");

	if ( src_len > dest_len )
		mp_raise_ValueError("dest needs to be at least as big as src");

	size_t i;

	for (i=0; i<src_len; i++)
	{
		int h;
		float v;

		v = src[i] * UINT16_MAX;

		                         h = (int)v;
		     if (v < 0.f)        h = 0;
		else if (v > UINT16_MAX) h = UINT16_MAX;

		dest[i] = h;
	}

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(cball_framebuffer_floatto16_obj, cball_framebuffer_floatto16);

STATIC mp_obj_t cball_framebuffer_8to16(mp_obj_t dest_obj, mp_obj_t src_obj)
{
	/* args:
	 * -     dest             [n_leds*3]    uint16,
	 * -     src              [n_leds*3]    uint8,
	 */

	uint16_t *dest;
	size_t dest_len = cball_get_uint16_array(dest_obj, &dest, MP_BUFFER_WRITE,
	                  "dest needs to be a uarray.array('H',...)");

	uint8_t *src;
	size_t src_len  = cball_get_bytearray(src_obj, &src, MP_BUFFER_READ,
	                  "src needs to be a bytearray");

	if ( src_len > dest_len )
		mp_raise_ValueError("dest needs to be at least as big as src");

	size_t i;

	for (i=0; i<src_len; i++)
		dest[i] = src[i] + (src[i]<<8);

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(cball_framebuffer_8to16_obj, cball_framebuffer_8to16);

static int cball_remap_rgb(uint16_t dest[], const uint16_t src[], const uint16_t *remap, size_t led_count, const int led_order[3])
{

	size_t i, r_ix=led_order[0], g_ix=led_order[1], b_ix=led_order[2];
	const uint16_t *p_src = src;

	if ( (r_ix|g_ix|b_ix) != 3 || (r_ix+g_ix+b_ix) != 3 )
		return 0;

	for (i=0; i<led_count; i++)
	{
		int index;

		if (remap)
		{
			index = remap[i];
			if ( !(index < led_count) )
				return 0;
		}
		else
			index = i;

		index *= 3;
		dest[index+r_ix] = htole16(*p_src++);
		dest[index+g_ix] = htole16(*p_src++);
		dest[index+b_ix] = htole16(*p_src++);
	}

	return 1;
}


static int cball_remap_rgbw(uint16_t dest[], const uint16_t src[], const uint16_t *remap, size_t led_count, const int led_order[4])
{

	size_t i, r_ix=led_order[0], g_ix=led_order[1], b_ix=led_order[2], w_ix=led_order[3];
	const uint16_t *p_src = src;

	if ( (r_ix|g_ix|b_ix|w_ix) != 3 ||
	     (r_ix+g_ix+b_ix+w_ix) != 6 ||
	     (r_ix+1)*(g_ix+1)*(b_ix+1)*(w_ix+1) != 24 )
		return 0;

	for (i=0; i<led_count; i++)
	{
		int index;
		if (remap)
		{
			index = remap[i];
			if ( !(index < led_count) )
				return 0;
		}
		else
			index = i;

		index *= 4;
		int r, g, b, w;
		r = *p_src++;
		g = *p_src++;
		b = *p_src++;

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

STATIC mp_obj_t cball_framebuffer_remap(size_t n_args, const mp_obj_t *args)
{
	/* args:
	 * -     dest             [>=n_leds*3]  uint16,
	 * -     src              [n_leds*3]    uint16,
	 * -     remap            [n_leds]      uint16,
	 * -     led_order                      string, [ "RGB" | "GRB" | ... | "RGBW" | "GRBW" | ... ]
	 */

	uint16_t *dest;
	size_t dest_len = cball_get_uint16_array(args[0], &dest, MP_BUFFER_WRITE,
	                  "dest needs to be a uarray.array('H',...)");

	uint16_t *src;
	size_t src_len  = cball_get_uint16_array(args[1], &src, MP_BUFFER_READ,
	                  "dest needs to be a uarray.array('H',...)");

	uint16_t *remap = NULL;
	size_t led_count;

	if ( args[2] == mp_const_none )
		led_count = src_len / 3;
	else
		led_count = cball_get_uint16_array(args[2], &remap, MP_BUFFER_READ,
	                "remap needs to be None or a uarray.array('H',...)");

	if ( led_count*3 != src_len )
		mp_raise_ValueError("size of src needs to be a multiple of 3 and optional remap array needs to be 3x size of remap array");

	int led_order[4];
	int n_components = cball_get_led_order(led_order, args[3]);

	if ( led_count*n_components > dest_len )
		mp_raise_ValueError("dest array too small");

	if ( n_components == 4 )
	{
		if ( !cball_remap_rgbw(dest, src, remap, led_count, led_order) )
			mp_raise_ValueError("remap index out of range");
	}
	else
	{
		if ( !cball_remap_rgb(dest, src, remap, led_count, led_order) )
			mp_raise_ValueError("remap index out of range");
	}

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cball_framebuffer_remap_obj, 4, 4, cball_framebuffer_remap);

const float log_prime[] =
{
// [ math.log(i) for i in [ 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53,
//                        59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113,
//                        127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181,
//                        191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251 ] ]

0.6931471805599453, 1.0986122886681098, 1.6094379124341003, 1.9459101490553132, 2.3978952727983707,
2.5649493574615367, 2.833213344056216, 2.9444389791664403, 3.1354942159291497, 3.367295829986474,
3.4339872044851463, 3.6109179126442243, 3.713572066704308, 3.7612001156935624, 3.8501476017100584,
3.970291913552122, 4.07753744390572, 4.110873864173311, 4.204692619390966, 4.2626798770413155,
4.290459441148391, 4.3694478524670215, 4.418840607796598, 4.48863636973214, 4.574710978503383,
4.61512051684126, 4.634728988229636, 4.672828834461906, 4.6913478822291435, 4.727387818712341,
4.844187086458591, 4.875197323201151, 4.919980925828125, 4.9344739331306915, 5.003946305945459,
5.017279836814924, 5.056245805348308, 5.093750200806762, 5.117993812416755, 5.153291594497779,
5.187385805840755, 5.198497031265826, 5.25227342804663, 5.262690188904886, 5.2832037287379885,
5.293304824724492, 5.351858133476067, 5.407171771460119, 5.424950017481403, 5.43372200355424,
5.4510384535657, 5.476463551931511, 5.484796933490655, 5.5254529391317835
};

STATIC mp_obj_t cball_calc_gamma_map(mp_obj_t gamma_map_obj, mp_obj_t gamma_obj, mp_obj_t max_obj)
{
	/* args:
	 * -     gamma_map        [256]         uint16,
	 * -     gamma                          float,
	 * -     max                            int,
	 */

	int prime_ix = 0;
	#define NEXT_PRIME_POW(x) ( expf(x*log_prime[prime_ix++]) )

	uint16_t *gamma_map;
	size_t gamma_map_len = cball_get_uint16_array(gamma_map_obj, &gamma_map, MP_BUFFER_WRITE,
	                       "gamma_map needs to be a uarray.array('H',...)");

	if (gamma_map_len != 256)
		mp_raise_ValueError("gamma_map needs to have 256 elements");

	float gamma = mp_obj_get_float(gamma_obj);

	int max = mp_obj_get_int(max_obj);
	if (max < 0 || max > 0xffff)
		mp_raise_ValueError("max needs to be in the range [0, 65535]");

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
		if (g_int < 0)
			g_int = 0;
		else if (g_int > max)
			g_int = max;

		gamma_map[i] = (uint16_t)g_int;
	}
	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(cball_calc_gamma_map_obj, cball_calc_gamma_map);

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


/* dest = 255.* ( a/255. * b/255. ) */
STATIC mp_obj_t cball_bytearray_interval_multiply(mp_obj_t dest, mp_obj_t a_in, mp_obj_t b_in)
{
	/* args:
	 * -     out           [n]  uint8,
	 * -     in1           [n]  uint8,
	 * -     in2           [n]  uint8,
	 *
	 */

	mp_buffer_info_t dest_info;
	mp_get_buffer_raise(dest, &dest_info, MP_BUFFER_WRITE);
	mp_buffer_info_t src_a_info;
	mp_get_buffer_raise(a_in, &src_a_info, MP_BUFFER_READ);
	mp_buffer_info_t src_b_info;
	mp_get_buffer_raise(b_in, &src_b_info, MP_BUFFER_READ);

	if ( (dest_info.len != src_a_info.len) || (src_a_info.len != src_b_info.len) )
		mp_raise_ValueError("array sizes dont match");

	uint8_t *d = dest_info.buf, *a = src_a_info.buf, *b = src_b_info.buf;

	size_t i;
	for (i=0; i<dest_info.len; i++)
		d[i] = ( a[i]*b[i]*65794 ) >> 24;

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(cball_bytearray_interval_multiply_obj, cball_bytearray_interval_multiply);

STATIC mp_obj_t cball_bytearray_add_clamp(mp_obj_t dest, mp_obj_t a_in, mp_obj_t b_in)
{
	/* args:
	 * -     out           [n]  uint8,
	 * -     in1           [n]  uint8,
	 * -     in2           [n]  uint8,
	 *
	 */

	mp_buffer_info_t dest_info;
	mp_get_buffer_raise(dest, &dest_info, MP_BUFFER_WRITE);
	mp_buffer_info_t src_a_info;
	mp_get_buffer_raise(a_in, &src_a_info, MP_BUFFER_READ);
	mp_buffer_info_t src_b_info;
	mp_get_buffer_raise(b_in, &src_b_info, MP_BUFFER_READ);

	if ( (dest_info.len != src_a_info.len) || (src_a_info.len != src_b_info.len) )
		mp_raise_ValueError("array sizes dont match");

	uint8_t *d = dest_info.buf, *a = src_a_info.buf, *b = src_b_info.buf;

	size_t i;
	for (i=0; i<dest_info.len; i++)
	{
		int v = a[i]+b[i];
		if ( v > 255 ) v = 255;
		d[i] = v;
	}

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(cball_bytearray_add_clamp_obj, cball_bytearray_add_clamp);

STATIC mp_obj_t cball_bytearray_sub_clamp(mp_obj_t dest, mp_obj_t a_in, mp_obj_t b_in)
{
	/* args:
	 * -     out           [n]  uint8,
	 * -     in1           [n]  uint8,
	 * -     in2           [n]  uint8,
	 *
	 */

	mp_buffer_info_t dest_info;
	mp_get_buffer_raise(dest, &dest_info, MP_BUFFER_WRITE);
	mp_buffer_info_t src_a_info;
	mp_get_buffer_raise(a_in, &src_a_info, MP_BUFFER_READ);
	mp_buffer_info_t src_b_info;
	mp_get_buffer_raise(b_in, &src_b_info, MP_BUFFER_READ);

	if ( (dest_info.len != src_a_info.len) || (src_a_info.len != src_b_info.len) )
		mp_raise_ValueError("array sizes dont match");

	uint8_t *d = dest_info.buf, *a = src_a_info.buf, *b = src_b_info.buf;

	size_t i;
	for (i=0; i<dest_info.len; i++)
	{
		int v = a[i]-b[i];
		if ( v < 0 ) v = 0;
		d[i] = v;
	}

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(cball_bytearray_sub_clamp_obj, cball_bytearray_sub_clamp);

STATIC mp_obj_t cball_bytearray_max(mp_obj_t dest, mp_obj_t a_in, mp_obj_t b_in)
{
	/* args:
	 * -     out           [n]  uint8,
	 * -     in1           [n]  uint8,
	 * -     in2           [n]  uint8,
	 *
	 */

	mp_buffer_info_t dest_info;
	mp_get_buffer_raise(dest, &dest_info, MP_BUFFER_WRITE);
	mp_buffer_info_t src_a_info;
	mp_get_buffer_raise(a_in, &src_a_info, MP_BUFFER_READ);
	mp_buffer_info_t src_b_info;
	mp_get_buffer_raise(b_in, &src_b_info, MP_BUFFER_READ);

	if ( (dest_info.len != src_a_info.len) || (src_a_info.len != src_b_info.len) )
		mp_raise_ValueError("array sizes dont match");

	uint8_t *d = dest_info.buf, *a = src_a_info.buf, *b = src_b_info.buf;

	size_t i;
	for (i=0; i<dest_info.len; i++)
	{
		if (a[i] > b[i])
			d[i] = a[i];
		else
			d[i] = b[i];
	}

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(cball_bytearray_max_obj, cball_bytearray_max);

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

static const float hsi_table[256] =
{
/*

table = [ '{: f},'.format( math.cos(math.pi*2*     i /768) /
                           math.cos(math.pi*2*(128-i)/768) ) for i in range(256) ]
for i in range(0,256,8):
    print ( '\t' + ' '.join( table[i:i+8] ) )
 *
 */
	 2.000000,  1.972055,  1.944876,  1.918429,  1.892682,  1.867604,  1.843165,  1.819338,
	 1.796099,  1.773421,  1.751283,  1.729662,  1.708537,  1.687889,  1.667700,  1.647951,
	 1.628626,  1.609709,  1.591184,  1.573038,  1.555255,  1.537824,  1.520731,  1.503964,
	 1.487513,  1.471365,  1.455512,  1.439942,  1.424646,  1.409615,  1.394840,  1.380313,
	 1.366025,  1.351970,  1.338139,  1.324525,  1.311121,  1.297922,  1.284920,  1.272109,
	 1.259484,  1.247039,  1.234768,  1.222666,  1.210729,  1.198950,  1.187327,  1.175853,
	 1.164525,  1.153338,  1.142288,  1.131372,  1.120585,  1.109924,  1.099385,  1.088965,
	 1.078660,  1.068467,  1.058383,  1.048405,  1.038529,  1.028754,  1.019076,  1.009492,
	 1.000000,  0.990597,  0.981281,  0.972050,  0.962900,  0.953830,  0.944838,  0.935921,
	 0.927076,  0.918303,  0.909600,  0.900963,  0.892391,  0.883883,  0.875436,  0.867049,
	 0.858719,  0.850447,  0.842228,  0.834063,  0.825949,  0.817885,  0.809869,  0.801900,
	 0.793976,  0.786096,  0.778259,  0.770462,  0.762706,  0.754988,  0.747307,  0.739662,
	 0.732051,  0.724473,  0.716928,  0.709414,  0.701929,  0.694473,  0.687044,  0.679641,
	 0.672263,  0.664909,  0.657579,  0.650270,  0.642981,  0.635713,  0.628463,  0.621230,
	 0.614014,  0.606814,  0.599628,  0.592456,  0.585296,  0.578148,  0.571010,  0.563882,
	 0.556762,  0.549650,  0.542545,  0.535446,  0.528351,  0.521260,  0.514172,  0.507085,
	 0.500000,  0.492915,  0.485828,  0.478740,  0.471649,  0.464554,  0.457455,  0.450350,
	 0.443238,  0.436118,  0.428990,  0.421852,  0.414704,  0.407544,  0.400372,  0.393186,
	 0.385986,  0.378770,  0.371537,  0.364287,  0.357019,  0.349730,  0.342421,  0.335091,
	 0.327737,  0.320359,  0.312956,  0.305527,  0.298071,  0.290586,  0.283072,  0.275527,
	 0.267949,  0.260338,  0.252693,  0.245012,  0.237294,  0.229538,  0.221741,  0.213904,
	 0.206024,  0.198100,  0.190131,  0.182115,  0.174051,  0.165937,  0.157772,  0.149553,
	 0.141281,  0.132951,  0.124564,  0.116117,  0.107609,  0.099037,  0.090400,  0.081697,
	 0.072924,  0.064079,  0.055162,  0.046170,  0.037100,  0.027950,  0.018719,  0.009403,
	 0.000000, -0.009492, -0.019076, -0.028754, -0.038529, -0.048405, -0.058383, -0.068467,
	-0.078660, -0.088965, -0.099385, -0.109924, -0.120585, -0.131372, -0.142288, -0.153338,
	-0.164525, -0.175853, -0.187327, -0.198950, -0.210729, -0.222666, -0.234768, -0.247039,
	-0.259484, -0.272109, -0.284920, -0.297922, -0.311121, -0.324525, -0.338139, -0.351970,
	-0.366025, -0.380313, -0.394840, -0.409615, -0.424646, -0.439942, -0.455512, -0.471365,
	-0.487513, -0.503964, -0.520731, -0.537824, -0.555255, -0.573038, -0.591184, -0.609709,
	-0.628626, -0.647951, -0.667700, -0.687889, -0.708537, -0.729662, -0.751283, -0.773421,
	-0.796099, -0.819338, -0.843165, -0.867604, -0.892682, -0.918429, -0.944876, -0.972055,

};

static void HSItoRGB(float h, float s, float i, uint8_t *rgb)
{
	int h_int = (int)( h * 768.f ) % 768;
	if (h_int < 0) h_int += 768;

	if      (s < 0.f) s = 0.f;
	else if (s > 1.f) s = 1.f;

	i *= 85.f;
	if      (i <  0.f) i =  0.f;
	else if (i > 85.f) i = 85.f;

	int hsi_wave = hsi_table[h_int & 0xff];

	uint8_t color[5];

	color[0] = (uint8_t)(i * (1.f + s *     hsi_wave ) );
	color[1] = (uint8_t)(i * (1.f + s *(1.f-hsi_wave)) );
	color[2] = (uint8_t)(i * (1.f - s                ) );
	color[3] = color[0];
	color[4] = color[1];

	int ix=0;
	if (h_int >= 512)
		ix=2;
	else if (h_int >= 256)
		ix=1;

	memcpy(rgb, &color[ix], 3);
}

static mp_obj_t cball_HSItoRGB(mp_obj_t h, mp_obj_t s, mp_obj_t i)
{
	uint8_t rgb[3];
	HSItoRGB( (float)mp_obj_get_float(h), (float)mp_obj_get_float(s), (float)mp_obj_get_float(i), rgb );
	return mp_obj_new_bytes(rgb, 3);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(cball_HSItoRGB_obj, cball_HSItoRGB);

static mp_obj_t cball_wave_lut(mp_obj_t buf)
{
	/* args:
	 * -     dest        [n*4]  uint8,
	 */

	uint8_t *dest;
	size_t dest_len = cball_get_bytearray(buf, &dest, MP_BUFFER_WRITE,
	                  "dest needs to be a bytearray");

	if (dest_len & 0x3)
		mp_raise_ValueError("dest array size must be a multiple of 4");

	int n = dest_len/4;
	int h = dest_len/2;

	dest[0] = 0;
	dest[n] = 127;
	dest[h] = 255;
	dest[h+n] = 127;

	float fac = 2.f*(float)M_PI/(float)dest_len;
	int i;
	for (i=1; i<n; i++)
	{
		int cosine = (int)floorf( (1+cosf( (float)i * fac )) *127.5 );
		dest[dest_len-i] = dest[i] = 255-cosine;
		dest[h-i] = dest[h+i] = cosine;
	}

	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(cball_wave_lut_obj, cball_wave_lut);

static mp_obj_t cball_wave_for_gradient_lut(mp_obj_t buf)
{
	/* args:
	 * -     dest        [n*4]  uint8,
	 */

	uint8_t *dest;
	size_t dest_len = cball_get_bytearray(buf, &dest, MP_BUFFER_WRITE,
	                  "dest needs to be a bytearray");

	if (dest_len & 0x3)
		mp_raise_ValueError("dest array size must be a multiple of 4");

	int n = dest_len/4;
	int h = dest_len/2;

	dest[0] = 0;
	dest[n] = 64;
	dest[h] = 255;
	dest[h+n] = 64;

	float fac = 2.f*(float)M_PI/(float)dest_len;
	int i;
	for (i=1; i<n; i++)
	{
		float cosine = cosf( (float)i * fac );
		uint32_t tmp = (uint32_t)( (1.f-cosine)*32703.94f );
		dest[dest_len-i] = dest[i] = (tmp*tmp)>>24;
		tmp = (uint32_t)((1.f+cosine)*32703.94f);
		dest[h-i] = dest[h+i] = (tmp*tmp)>>24;
	}

	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(cball_wave_for_gradient_lut_obj, cball_wave_for_gradient_lut);

static const uint8_t colordrift_lut[1025] =
{
	0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x0a, 0x0a, 0x0a, 0x0a,
	0x0a, 0x0a, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0d,
	0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x11, 0x11, 0x11, 0x11, 0x11, 0x12, 0x12, 0x12, 0x12, 0x12,
	0x13, 0x13, 0x13, 0x13, 0x13, 0x14, 0x14, 0x14, 0x14, 0x14, 0x15, 0x15, 0x15, 0x15, 0x16, 0x16,
	0x16, 0x16, 0x16, 0x17, 0x17, 0x17, 0x17, 0x18, 0x18, 0x18, 0x18, 0x18, 0x19, 0x19, 0x19, 0x19,
	0x1a, 0x1a, 0x1a, 0x1a, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1c, 0x1c, 0x1c, 0x1c, 0x1d, 0x1d, 0x1d,
	0x1d, 0x1e, 0x1e, 0x1e, 0x1e, 0x1f, 0x1f, 0x1f, 0x1f, 0x20, 0x20, 0x20, 0x20, 0x21, 0x21, 0x21,
	0x22, 0x22, 0x22, 0x22, 0x23, 0x23, 0x23, 0x23, 0x24, 0x24, 0x24, 0x24, 0x25, 0x25, 0x25, 0x26,
	0x26, 0x26, 0x26, 0x27, 0x27, 0x27, 0x28, 0x28, 0x28, 0x28, 0x29, 0x29, 0x29, 0x2a, 0x2a, 0x2a,
	0x2a, 0x2b, 0x2b, 0x2b, 0x2c, 0x2c, 0x2c, 0x2c, 0x2d, 0x2d, 0x2d, 0x2e, 0x2e, 0x2e, 0x2f, 0x2f,
	0x2f, 0x2f, 0x30, 0x30, 0x30, 0x31, 0x31, 0x31, 0x32, 0x32, 0x32, 0x32, 0x33, 0x33, 0x33, 0x34,
	0x34, 0x34, 0x35, 0x35, 0x35, 0x36, 0x36, 0x36, 0x37, 0x37, 0x37, 0x38, 0x38, 0x38, 0x39, 0x39,
	0x39, 0x39, 0x3a, 0x3a, 0x3a, 0x3b, 0x3b, 0x3b, 0x3c, 0x3c, 0x3c, 0x3d, 0x3d, 0x3d, 0x3e, 0x3e,
	0x3e, 0x3f, 0x3f, 0x3f, 0x40, 0x40, 0x40, 0x41, 0x41, 0x41, 0x42, 0x42, 0x43, 0x43, 0x43, 0x44,
	0x44, 0x44, 0x45, 0x45, 0x45, 0x46, 0x46, 0x46, 0x47, 0x47, 0x47, 0x48, 0x48, 0x48, 0x49, 0x49,
	0x49, 0x4a, 0x4a, 0x4b, 0x4b, 0x4b, 0x4c, 0x4c, 0x4c, 0x4d, 0x4d, 0x4d, 0x4e, 0x4e, 0x4e, 0x4f,
	0x4f, 0x50, 0x50, 0x50, 0x51, 0x51, 0x51, 0x52, 0x52, 0x52, 0x53, 0x53, 0x54, 0x54, 0x54, 0x55,
	0x55, 0x55, 0x56, 0x56, 0x57, 0x57, 0x57, 0x58, 0x58, 0x58, 0x59, 0x59, 0x59, 0x5a, 0x5a, 0x5b,
	0x5b, 0x5b, 0x5c, 0x5c, 0x5c, 0x5d, 0x5d, 0x5e, 0x5e, 0x5e, 0x5f, 0x5f, 0x60, 0x60, 0x60, 0x61,
	0x61, 0x61, 0x62, 0x62, 0x63, 0x63, 0x63, 0x64, 0x64, 0x64, 0x65, 0x65, 0x66, 0x66, 0x66, 0x67,
	0x67, 0x68, 0x68, 0x68, 0x69, 0x69, 0x69, 0x6a, 0x6a, 0x6b, 0x6b, 0x6b, 0x6c, 0x6c, 0x6d, 0x6d,
	0x6d, 0x6e, 0x6e, 0x6e, 0x6f, 0x6f, 0x70, 0x70, 0x70, 0x71, 0x71, 0x72, 0x72, 0x72, 0x73, 0x73,
	0x74, 0x74, 0x74, 0x75, 0x75, 0x75, 0x76, 0x76, 0x77, 0x77, 0x77, 0x78, 0x78, 0x79, 0x79, 0x79,
	0x7a, 0x7a, 0x7b, 0x7b, 0x7b, 0x7c, 0x7c, 0x7c, 0x7d, 0x7d, 0x7e, 0x7e, 0x7e, 0x7f, 0x7f, 0x80,
	0x80, 0x80, 0x81, 0x81, 0x82, 0x82, 0x82, 0x83, 0x83, 0x84, 0x84, 0x84, 0x85, 0x85, 0x85, 0x86,
	0x86, 0x87, 0x87, 0x87, 0x88, 0x88, 0x89, 0x89, 0x89, 0x8a, 0x8a, 0x8b, 0x8b, 0x8b, 0x8c, 0x8c,
	0x8c, 0x8d, 0x8d, 0x8e, 0x8e, 0x8e, 0x8f, 0x8f, 0x90, 0x90, 0x90, 0x91, 0x91, 0x92, 0x92, 0x92,
	0x93, 0x93, 0x93, 0x94, 0x94, 0x95, 0x95, 0x95, 0x96, 0x96, 0x97, 0x97, 0x97, 0x98, 0x98, 0x98,
	0x99, 0x99, 0x9a, 0x9a, 0x9a, 0x9b, 0x9b, 0x9c, 0x9c, 0x9c, 0x9d, 0x9d, 0x9d, 0x9e, 0x9e, 0x9f,
	0x9f, 0x9f, 0xa0, 0xa0, 0xa0, 0xa1, 0xa1, 0xa2, 0xa2, 0xa2, 0xa3, 0xa3, 0xa4, 0xa4, 0xa4, 0xa5,
	0xa5, 0xa5, 0xa6, 0xa6, 0xa7, 0xa7, 0xa7, 0xa8, 0xa8, 0xa8, 0xa9, 0xa9, 0xa9, 0xaa, 0xaa, 0xab,
	0xab, 0xab, 0xac, 0xac, 0xac, 0xad, 0xad, 0xae, 0xae, 0xae, 0xaf, 0xaf, 0xaf, 0xb0, 0xb0, 0xb0,
	0xb1, 0xb1, 0xb2, 0xb2, 0xb2, 0xb3, 0xb3, 0xb3, 0xb4, 0xb4, 0xb4, 0xb5, 0xb5, 0xb5, 0xb6, 0xb6,
	0xb7, 0xb7, 0xb7, 0xb8, 0xb8, 0xb8, 0xb9, 0xb9, 0xb9, 0xba, 0xba, 0xba, 0xbb, 0xbb, 0xbb, 0xbc,
	0xbc, 0xbc, 0xbd, 0xbd, 0xbd, 0xbe, 0xbe, 0xbf, 0xbf, 0xbf, 0xc0, 0xc0, 0xc0, 0xc1, 0xc1, 0xc1,
	0xc2, 0xc2, 0xc2, 0xc3, 0xc3, 0xc3, 0xc4, 0xc4, 0xc4, 0xc5, 0xc5, 0xc5, 0xc6, 0xc6, 0xc6, 0xc7,
	0xc7, 0xc7, 0xc7, 0xc8, 0xc8, 0xc8, 0xc9, 0xc9, 0xc9, 0xca, 0xca, 0xca, 0xcb, 0xcb, 0xcb, 0xcc,
	0xcc, 0xcc, 0xcd, 0xcd, 0xcd, 0xce, 0xce, 0xce, 0xce, 0xcf, 0xcf, 0xcf, 0xd0, 0xd0, 0xd0, 0xd1,
	0xd1, 0xd1, 0xd1, 0xd2, 0xd2, 0xd2, 0xd3, 0xd3, 0xd3, 0xd4, 0xd4, 0xd4, 0xd4, 0xd5, 0xd5, 0xd5,
	0xd6, 0xd6, 0xd6, 0xd6, 0xd7, 0xd7, 0xd7, 0xd8, 0xd8, 0xd8, 0xd8, 0xd9, 0xd9, 0xd9, 0xda, 0xda,
	0xda, 0xda, 0xdb, 0xdb, 0xdb, 0xdc, 0xdc, 0xdc, 0xdc, 0xdd, 0xdd, 0xdd, 0xdd, 0xde, 0xde, 0xde,
	0xde, 0xdf, 0xdf, 0xdf, 0xe0, 0xe0, 0xe0, 0xe0, 0xe1, 0xe1, 0xe1, 0xe1, 0xe2, 0xe2, 0xe2, 0xe2,
	0xe3, 0xe3, 0xe3, 0xe3, 0xe4, 0xe4, 0xe4, 0xe4, 0xe5, 0xe5, 0xe5, 0xe5, 0xe5, 0xe6, 0xe6, 0xe6,
	0xe6, 0xe7, 0xe7, 0xe7, 0xe7, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe9, 0xe9, 0xe9, 0xe9, 0xea, 0xea,
	0xea, 0xea, 0xea, 0xeb, 0xeb, 0xeb, 0xeb, 0xec, 0xec, 0xec, 0xec, 0xec, 0xed, 0xed, 0xed, 0xed,
	0xed, 0xee, 0xee, 0xee, 0xee, 0xee, 0xef, 0xef, 0xef, 0xef, 0xef, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
	0xf0, 0xf1, 0xf1, 0xf1, 0xf1, 0xf1, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf3, 0xf3, 0xf3, 0xf3,
	0xf3, 0xf3, 0xf4, 0xf4, 0xf4, 0xf4, 0xf4, 0xf4, 0xf5, 0xf5, 0xf5, 0xf5, 0xf5, 0xf5, 0xf5, 0xf6,
	0xf6, 0xf6, 0xf6, 0xf6, 0xf6, 0xf7, 0xf7, 0xf7, 0xf7, 0xf7, 0xf7, 0xf7, 0xf8, 0xf8, 0xf8, 0xf8,
	0xf8, 0xf8, 0xf8, 0xf8, 0xf9, 0xf9, 0xf9, 0xf9, 0xf9, 0xf9, 0xf9, 0xf9, 0xfa, 0xfa, 0xfa, 0xfa,
	0xfa, 0xfa, 0xfa, 0xfa, 0xfa, 0xfb, 0xfb, 0xfb, 0xfb, 0xfb, 0xfb, 0xfb, 0xfb, 0xfb, 0xfb, 0xfc,
	0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd,
	0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe,
	0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff,
};


extern const mp_obj_type_t colordrift_type;
typedef struct
{
    mp_obj_base_t base;

	int n_colors;
	uint32_t phase_dt, phase_shift, phase_div, phase, period;
	uint8_t *colors;


} colordrift_obj_t;

static mp_obj_t cball_colordrift_make_new(const mp_obj_type_t *type,
                                          size_t n_args,
                                          size_t n_kw, const mp_obj_t *all_args)
{
    enum { ARG_period, ARG_n_colors, ARG_initial_phase };
	static const mp_arg_t allowed_args[] =
	{
		{ MP_QSTR_period,         MP_ARG_INT , { .u_int = 128} },
		{ MP_QSTR_n_colors,       MP_ARG_INT , { .u_int = 3  } },
		{ MP_QSTR_initial_phase,  MP_ARG_INT , { .u_int = 0  } },
	};
	mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
	mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

	if (args[ARG_period].u_int <= 0 || args[ARG_period].u_int >= 262144)
		mp_raise_ValueError(MP_ERROR_TEXT("expected: 0 < period < 262144"));

	if (args[ARG_n_colors].u_int <= 0 || args[ARG_n_colors].u_int >= 5)
		mp_raise_ValueError(MP_ERROR_TEXT("expected: 0 < n_colors < 5"));

	if (args[ARG_initial_phase].u_int < 0 || args[ARG_initial_phase].u_int >= args[ARG_period].u_int)
		mp_raise_ValueError(MP_ERROR_TEXT("expected: 0 <= initial_phase < period"));

	colordrift_obj_t *self = m_new_obj(colordrift_obj_t);

	self->base        = (mp_obj_base_t) { &colordrift_type };

	self->n_colors    = args[ARG_n_colors].u_int;
	self->phase_dt    =                          self->n_colors * 2048;
	self->phase_shift = args[ARG_period].u_int                  * 2048;
	self->phase_div   = args[ARG_period].u_int * self->n_colors       ;
	self->period      = args[ARG_period].u_int * self->n_colors * 2048;
	self->phase       = ( self->phase_dt * args[ARG_initial_phase].u_int ) % self->period;

	self->colors = m_new(uint8_t, 3*self->n_colors);
	memset(self->colors, 0, 3*self->n_colors);

	return MP_OBJ_FROM_PTR(self);
}

static void get_random_color(uint8_t *rgb)
{
	uint32_t random = esp_random();
	float h = (float)(random&0xffff)/65535.f;
	float s = 1.f;
	float i = (float)((random&0xffff)+0x10000)/131071.f;
	HSItoRGB(h, s, i, rgb);
}

static void colordrift_next_color(colordrift_obj_t *self, uint8_t *rgb)
{
	int r=0,g=0,b=0;
	int i;
	uint32_t p = self->phase, offset, w;
	for (i=0; i<self->n_colors; i++)
	{
		if (p < self->phase_dt)
			get_random_color(&self->colors[i*3]);

		offset = p / self->phase_div;
		w = colordrift_lut[offset < 1025 ? offset : 2048-offset];

		r += self->colors[i*3  ] * w;
		g += self->colors[i*3+1] * w;
		b += self->colors[i*3+2] * w;

		p += self->phase_shift;
		if (p >= self->period)
			p -= self->period;
	}
	self->phase += self->phase_dt;
	if (self->phase >= self->period)
		self->phase -= self->period;

	rgb[0] = (r > 0xff00) ? 0xff : (r>>8);
	rgb[1] = (g > 0xff00) ? 0xff : (g>>8);
	rgb[2] = (b > 0xff00) ? 0xff : (b>>8);
}

static mp_obj_t cball_colordrift_next_color(mp_obj_t self_in)
{
	colordrift_obj_t *self = MP_OBJ_TO_PTR(self_in);
	uint8_t color[3];
	colordrift_next_color(self, color);
	return mp_obj_new_bytes(color, 3);
}

static MP_DEFINE_CONST_FUN_OBJ_1(cball_colordrift_next_color_obj, cball_colordrift_next_color);

static mp_obj_t cball_colordrift_next_color_into(mp_obj_t self_in, mp_obj_t buf_in, mp_obj_t offset_in)
{
	uint8_t *buf;
    int buf_len = cball_get_bytearray(buf_in, &buf, MP_BUFFER_READ, "buf needs to be a bytearray");

	int offset = mp_obj_get_int(offset_in);
	if ( (offset < 0) || (buf_len-3 < offset) )
		mp_raise_ValueError(MP_ERROR_TEXT("wrong offset"));

	colordrift_obj_t *self = MP_OBJ_TO_PTR(self_in);
	colordrift_next_color(self, &buf[offset]);
	return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_3(cball_colordrift_next_color_into_obj, cball_colordrift_next_color_into);

static void cball_colordrift_print(const mp_print_t *print,
                                   mp_obj_t self_in,
                                   mp_print_kind_t kind)
{
    mp_printf(print, "<cball.ColorDrift XXX TODO Description XXX>");
}


static const mp_rom_map_elem_t cball_colordrift_locals_dict_table[] =
{
    { MP_ROM_QSTR(MP_QSTR_next_color),      MP_ROM_PTR(&cball_colordrift_next_color_obj)        },
    { MP_ROM_QSTR(MP_QSTR_next_color_into), MP_ROM_PTR(&cball_colordrift_next_color_into_obj)   },
};

static MP_DEFINE_CONST_DICT(cball_colordrift_locals_dict, cball_colordrift_locals_dict_table);


const mp_obj_type_t colordrift_type =
{
    { &mp_type_type },
    .name = MP_QSTR_ColorDrift,
    .print = cball_colordrift_print,
    .make_new = cball_colordrift_make_new,
    .locals_dict = (mp_obj_dict_t *)&cball_colordrift_locals_dict,
};


STATIC const mp_rom_map_elem_t cball_module_globals_table[] =
{
	{ MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_cball) },

	{ MP_ROM_QSTR(MP_QSTR_framebuffer_floatto16), MP_ROM_PTR(&cball_framebuffer_floatto16_obj) },
	{ MP_ROM_QSTR(MP_QSTR_framebuffer_8to16), MP_ROM_PTR(&cball_framebuffer_8to16_obj) },
	{ MP_ROM_QSTR(MP_QSTR_framebuffer_remap), MP_ROM_PTR(&cball_framebuffer_remap_obj) },
	{ MP_ROM_QSTR(MP_QSTR_framebuffer_gamma), MP_ROM_PTR(&cball_framebuffer_gamma_obj) },

	{ MP_ROM_QSTR(MP_QSTR_calc_gamma_map), MP_ROM_PTR(&cball_calc_gamma_map_obj) },
	{ MP_ROM_QSTR(MP_QSTR_apply_palette), MP_ROM_PTR(&cball_apply_palette_obj) },
	{ MP_ROM_QSTR(MP_QSTR_latt_long_map), MP_ROM_PTR(&cball_latt_long_map_obj) },
	{ MP_ROM_QSTR(MP_QSTR_bytearray_memset), MP_ROM_PTR(&cball_bytearray_memset_obj) },
	{ MP_ROM_QSTR(MP_QSTR_bytearray_memcpy), MP_ROM_PTR(&cball_bytearray_memcpy_obj) },
	{ MP_ROM_QSTR(MP_QSTR_bytearray_blend), MP_ROM_PTR(&cball_bytearray_blend_obj) },
	{ MP_ROM_QSTR(MP_QSTR_bytearray_interval_multiply), MP_ROM_PTR(&cball_bytearray_interval_multiply_obj) },
	{ MP_ROM_QSTR(MP_QSTR_bytearray_add_clamp), MP_ROM_PTR(&cball_bytearray_add_clamp_obj) },
	{ MP_ROM_QSTR(MP_QSTR_bytearray_sub_clamp), MP_ROM_PTR(&cball_bytearray_sub_clamp_obj) },
	{ MP_ROM_QSTR(MP_QSTR_bytearray_max), MP_ROM_PTR(&cball_bytearray_max_obj) },

	{ MP_ROM_QSTR(MP_QSTR_shader), MP_ROM_PTR(&cball_shader_obj) },
	{ MP_ROM_QSTR(MP_QSTR_gradient), MP_ROM_PTR(&cball_gradient_obj) },
	{ MP_ROM_QSTR(MP_QSTR_wobble), MP_ROM_PTR(&cball_wobble_obj) },
	{ MP_ROM_QSTR(MP_QSTR_ca_update), MP_ROM_PTR(&cball_ca_update_obj) },
	{ MP_ROM_QSTR(MP_QSTR_orbit_update), MP_ROM_PTR(&cball_orbit_update_obj) },
	{ MP_ROM_QSTR(MP_QSTR_lorenz_update), MP_ROM_PTR(&cball_lorenz_update_obj) },

	{ MP_ROM_QSTR(MP_QSTR_HSItoRGB), MP_ROM_PTR(&cball_HSItoRGB_obj) },
	{ MP_ROM_QSTR(MP_QSTR_wave_lut), MP_ROM_PTR(&cball_wave_lut_obj) },
	{ MP_ROM_QSTR(MP_QSTR_wave_for_gradient_lut), MP_ROM_PTR(&cball_wave_for_gradient_lut_obj) },
	{ MP_ROM_QSTR(MP_QSTR_ColorDrift), MP_ROM_PTR(&colordrift_type) },
};

STATIC MP_DEFINE_CONST_DICT(cball_module_globals, cball_module_globals_table);

const mp_obj_module_t cball_user_cmodule =
{
	.base = { &mp_type_module },
	.globals = (mp_obj_dict_t*)&cball_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_cball, cball_user_cmodule, MODULE_CBALL_ENABLED);

