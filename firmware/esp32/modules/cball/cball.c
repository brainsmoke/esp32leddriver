
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
	 * -     framebuffer_out  [pixels*3]  uint16,
	 * -     framebuffer_in   [pixels]    uint8,
	 * -     palette          [n*3]       uint16,
	 */

	uint16_t *dest;
	size_t dest_len = cball_get_uint16_array(buf_dest, &dest, MP_BUFFER_WRITE,
	                                         "dest needs to be a uarray.array('H',...)");

	if (dest_len%3 != 0)
		mp_raise_ValueError("dest array size must be a multiple of 3");

	uint8_t *src;
	size_t src_len  = cball_get_bytearray(buf_src, &src, MP_BUFFER_READ,
	                  "src needs to be a bytearray");

	uint16_t *table;
	size_t table_len  = cball_get_uint16_array(palette, &table, MP_BUFFER_READ,
	                    "palette needs to be a uarray.array('H',...)");

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
	 * -     framebuffer_out  [n_leds*3]  uint16,
	 * -     framebuffer_in   [w*h*3]     uint16,
	 * -     voronoi_map      [w*h]       uin16,
	 * -     latt_weight      [h]         float32,
	 * -     total_weight     [n_leds]    float32,
	 * -     tmp_buf          [n_leds*3]  float32,
	 */

	uint16_t *dest;
	size_t dest_len = cball_get_uint16_array(args[0], &dest, MP_BUFFER_WRITE,
	                                         "dest needs to be a uarray.array('H',...)");

	if (dest_len%3 != 0)
		mp_raise_ValueError("dest array size must be a multiple of 3");

	uint16_t *src;
	size_t src_len  = cball_get_uint16_array(args[1], &src, MP_BUFFER_READ,
	                  "src needs to be a uarray.array('H',...)");

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
		if      (v < 0)      v=0;
		else if (v > 0xffff) v=0xffff;
		dest[i*3]=v;
		v = (int)(w*tmp[i*3+1]);
		if      (v < 0)      v=0;
		else if (v > 0xffff) v=0xffff;
		dest[i*3+1]=v;
		v = (int)(w*tmp[i*3+2]);
		if      (v < 0)      v=0;
		else if (v > 0xffff) v=0xffff;
		dest[i*3+2]=v;
	}

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cball_latt_long_map_obj, 6, 6, cball_latt_long_map);

STATIC mp_obj_t cball_array_copy(mp_obj_t out, mp_obj_t in)
{
	/* args:
	 * -     out [...]  uint16,
	 * -     in  [...]  uint16,
	 *
	 * memcpy(out, in, max(out_len, in_len)*2)
	 *
	 */

	uint16_t *dest;
	size_t dest_len = cball_get_uint16_array(out, &dest, MP_BUFFER_WRITE,
	                                         "dest needs to be a uarray.array('H',...)");

	uint16_t *src;
	size_t src_len = cball_get_uint16_array(in, &src, MP_BUFFER_WRITE,
	                                        "dest needs to be a uarray.array('H',...)");

	size_t len = dest_len;
	if (len > src_len)
		len = src_len;

	memcpy(dest, src, len*2);

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(cball_array_copy_obj, cball_array_copy);


STATIC mp_obj_t cball_array_set(mp_obj_t array, mp_obj_t c)
{
	/* args:
	 * -     array [...]  uint16,
	 * -     c            int,
	 */

	uint16_t *dest;
	size_t dest_len = cball_get_uint16_array(array, &dest, MP_BUFFER_WRITE,
	                                         "dest needs to be a uarray.array('H',...)");

	size_t i;
	for (i=0; i<dest_len; i++)
		dest[i] = mp_obj_get_int(c);

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(cball_array_set_obj, cball_array_set);

STATIC mp_obj_t cball_array_blend(size_t n_args, const mp_obj_t *args)
{
	/* args:
	 * -     out           [n]  uint16,
	 * -     in1           [n]  uint16,
	 * -     in2           [n]  uint16,
	 * -     interpolation      float
	 *
	 */

	int mulb = (int)(65536.f *mp_obj_get_float(args[3]));

	if (mulb < 0)
		mulb = 0;
	else if (mulb > 65536)
		mulb = 65536;

	int mula = 65536-mulb;

	uint16_t *dest;
	size_t dest_len = cball_get_uint16_array(args[0], &dest, MP_BUFFER_WRITE,
	                                         "dest needs to be a uarray.array('H',...)");

	uint16_t *src_a;
	size_t src_a_len = cball_get_uint16_array(args[1], &src_a, MP_BUFFER_WRITE,
	                                          "src a needs to be a uarray.array('H',...)");

	uint16_t *src_b;
	size_t src_b_len = cball_get_uint16_array(args[2], &src_b, MP_BUFFER_WRITE,
	                                          "src b needs to be a uarray.array('H',...)");

	if ( (dest_len != src_a_len) || (src_a_len != src_b_len) )
		mp_raise_ValueError("array sizes dont match");

	size_t i;
	for (i=0; i<dest_len; i++)
		dest[i] = ( mula*src_a[i] + mulb*src_b[i] ) >> 16;

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cball_array_blend_obj, 4, 4, cball_array_blend);


/* dest = 65535.* ( a/65535. * b/65535. ) */
STATIC mp_obj_t cball_array_interval_multiply(mp_obj_t out, mp_obj_t a_in, mp_obj_t b_in)
{
	/* args:
	 * -     out           [n]  uint16,
	 * -     in1           [n]  uint16,
	 * -     in2           [n]  uint16,
	 *
	 */

	uint16_t *dest;
	size_t dest_len = cball_get_uint16_array(out, &dest, MP_BUFFER_WRITE,
	                                         "dest needs to be a uarray.array('H',...)");

	uint16_t *src_a;
	size_t src_a_len = cball_get_uint16_array(a_in, &src_a, MP_BUFFER_WRITE,
	                                          "src a needs to be a uarray.array('H',...)");

	uint16_t *src_b;
	size_t src_b_len = cball_get_uint16_array(b_in, &src_b, MP_BUFFER_WRITE,
	                                          "src b needs to be a uarray.array('H',...)");

	if ( (dest_len != src_a_len) || (src_a_len != src_b_len) )
		mp_raise_ValueError("array sizes dont match");

	size_t i;
	uint32_t mul;
	for (i=0; i<dest_len; i++)
	{
		mul = src_a[i]*src_b[i];
		dest[i] = ( mul + (mul>>16) + 1 ) >> 16;
	}

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(cball_array_interval_multiply_obj, cball_array_interval_multiply);

STATIC mp_obj_t cball_array_add_clamp(mp_obj_t out, mp_obj_t a_in, mp_obj_t b_in)
{
	/* args:
	 * -     out           [n]  uint16,
	 * -     in1           [n]  uint16,
	 * -     in2           [n]  uint16,
	 *
	 */

	uint16_t *dest;
	size_t dest_len = cball_get_uint16_array(out, &dest, MP_BUFFER_WRITE,
	                                         "dest needs to be a uarray.array('H',...)");

	uint16_t *src_a;
	size_t src_a_len = cball_get_uint16_array(a_in, &src_a, MP_BUFFER_WRITE,
	                                          "src a needs to be a uarray.array('H',...)");

	uint16_t *src_b;
	size_t src_b_len = cball_get_uint16_array(b_in, &src_b, MP_BUFFER_WRITE,
	                                          "src b needs to be a uarray.array('H',...)");

	if ( (dest_len != src_a_len) || (src_a_len != src_b_len) )
		mp_raise_ValueError("array sizes dont match");

	size_t i;
	for (i=0; i<dest_len; i++)
	{
		int v = src_a[i]+src_b[i];
		if ( v > 65535 ) v = 65535;
		dest[i] = v;
	}

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(cball_array_add_clamp_obj, cball_array_add_clamp);

STATIC mp_obj_t cball_array_sub_clamp(mp_obj_t out, mp_obj_t a_in, mp_obj_t b_in)
{
	/* args:
	 * -     out           [n]  uint16,
	 * -     in1           [n]  uint16,
	 * -     in2           [n]  uint16,
	 *
	 */

	uint16_t *dest;
	size_t dest_len = cball_get_uint16_array(out, &dest, MP_BUFFER_WRITE,
	                                         "dest needs to be a uarray.array('H',...)");

	uint16_t *src_a;
	size_t src_a_len = cball_get_uint16_array(a_in, &src_a, MP_BUFFER_WRITE,
	                                          "src a needs to be a uarray.array('H',...)");

	uint16_t *src_b;
	size_t src_b_len = cball_get_uint16_array(b_in, &src_b, MP_BUFFER_WRITE,
	                                          "src b needs to be a uarray.array('H',...)");

	if ( (dest_len != src_a_len) || (src_a_len != src_b_len) )
		mp_raise_ValueError("array sizes dont match");

	size_t i;
	for (i=0; i<dest_len; i++)
	{
		int v = src_a[i]+src_b[i];
		if ( v < 0 ) v = 0;
		dest[i] = v;
	}

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(cball_array_sub_clamp_obj, cball_array_sub_clamp);

STATIC mp_obj_t cball_array_max(mp_obj_t out, mp_obj_t a_in, mp_obj_t b_in)
{
	/* args:
	 * -     out           [n]  uint16,
	 * -     in1           [n]  uint16,
	 * -     in2           [n]  uint16,
	 *
	 */

	uint16_t *dest;
	size_t dest_len = cball_get_uint16_array(out, &dest, MP_BUFFER_WRITE,
	                                         "dest needs to be a uarray.array('H',...)");

	uint16_t *src_a;
	size_t src_a_len = cball_get_uint16_array(a_in, &src_a, MP_BUFFER_WRITE,
	                                          "src a needs to be a uarray.array('H',...)");

	uint16_t *src_b;
	size_t src_b_len = cball_get_uint16_array(b_in, &src_b, MP_BUFFER_WRITE,
	                                          "src b needs to be a uarray.array('H',...)");

	if ( (dest_len != src_a_len) || (src_a_len != src_b_len) )
		mp_raise_ValueError("array sizes dont match");

	size_t i;
	for (i=0; i<dest_len; i++)
	{
		if (src_a[i] > src_b[i])
			dest[i] = src_a[i];
		else
			dest[i] = src_b[i];
	}

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(cball_array_max_obj, cball_array_max);

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
	 * -     framebuffer  [n_leds*3]    float,
	 * -     led_data     [n_leds*6]    float
	 * -     lights_flat  [n_lights*6]  float
	 */

	float *fb;
	size_t fb_len  = cball_get_float_array(framebuf, &fb, MP_BUFFER_WRITE,
	                 "framebuf needs to be a uarray.array('f',...)");

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
		mp_float_t r=0.,
		           g=0.,
		           b=0.;

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
		fb[i] = r;
		fb[i+1] = g;
		fb[i+2] = b;
	}

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(cball_shader_obj, cball_shader);

STATIC mp_obj_t cball_gradient(size_t n_args, const mp_obj_t *args)
{
	/* args:
	 * -     framebuffer  [pixels*3]  uint16,
	 * -     rotations    [pixels*3]  uint16,
	 * -     rwave        [...]       uint16,
	 * -     gwave        [...]       uint16,
	 * -     bwave        [...]       uint16,
	 * -     rphi                     int,
	 * -     gphi                     int,
	 * -     bphi                     int,
	 */

	uint16_t *fb;
	size_t fb_len = cball_get_uint16_array(args[0], &fb, MP_BUFFER_WRITE,
	                "framebuffer needs to be a uarray.array('H',...)");

	uint16_t *rot;
	size_t rot_len = cball_get_uint16_array(args[1], &rot, MP_BUFFER_READ,
	                 "rotations needs to be a uarray.array('H',...)");

	if (fb_len != rot_len)
		mp_raise_ValueError("Array sizes don't match");

	uint16_t *rwave;
	size_t rwave_len = cball_get_uint16_array(args[2], &rwave, MP_BUFFER_READ,
	                   "rwave needs to be a uarray.array('H',...)");

	uint16_t *gwave;
	size_t gwave_len = cball_get_uint16_array(args[3], &gwave, MP_BUFFER_READ,
	                   "gwave needs to be a uarray.array('H',...)");

	uint16_t *bwave;
	size_t bwave_len = cball_get_uint16_array(args[4], &bwave, MP_BUFFER_READ,
	                   "bwave needs to be a uarray.array('H',...)");

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
	 * -     framebuffer  [pixels*3]  uint16,
	 * -     rotations    [pixels*3]  uint16,
	 * -     n_components             int,
	 * -     phase                    float,
	 */

	uint16_t *fb;
	size_t fb_len  = cball_get_uint16_array(args[0], &fb, MP_BUFFER_WRITE,
	                 "framebuffer needs to be a uarray.array('H',...)");

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
		int b = (int)(w*w*16384.);
		if (b<0) b=0;
		else if (b>65535) b=65535;
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

static void HSItoRGB(float h, float s, float i, uint16_t *rgb)
{
	int h_int = (int)( h * 768.f ) % 768;
	if (h_int < 0) h_int += 768;

	if      (s < 0.f) s = 0.f;
	else if (s > 1.f) s = 1.f;

	i *= 21845.f;
	if      (i <  0.f) i =  0.f;
	else if (i > 21845.f) i = 21845.f;

	int hsi_wave = hsi_table[h_int & 0xff];

	uint16_t color[5];

	color[0] = (uint16_t)(i * (1.f + s *     hsi_wave ) );
	color[1] = (uint16_t)(i * (1.f + s *(1.f-hsi_wave)) );
	color[2] = (uint16_t)(i * (1.f - s                ) );
	color[3] = color[0];
	color[4] = color[1];

	int ix=0;
	if (h_int >= 512)
		ix=2;
	else if (h_int >= 256)
		ix=1;

	memcpy(rgb, &color[ix], 3*sizeof(uint16_t));
}

static mp_obj_t cball_HSItoRGB(mp_obj_t h, mp_obj_t s, mp_obj_t i)
{
	uint16_t rgb[3];
	HSItoRGB( (float)mp_obj_get_float(h), (float)mp_obj_get_float(s), (float)mp_obj_get_float(i), rgb );
	mp_obj_tuple_t *rgb_tuple = mp_obj_new_tuple(3, NULL);
	rgb_tuple->items[0] = mp_obj_new_int(rgb[0]);
	rgb_tuple->items[1] = mp_obj_new_int(rgb[1]);
	rgb_tuple->items[2] = mp_obj_new_int(rgb[2]);
	return rgb_tuple;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(cball_HSItoRGB_obj, cball_HSItoRGB);

static mp_obj_t cball_wave_lut(mp_obj_t buf)
{
	/* args:
	 * -     dest        [n*4]  uint8,
	 */

	uint16_t *dest;
	size_t dest_len = cball_get_uint16_array(buf, &dest, MP_BUFFER_WRITE,
	                  "dest needs to be a uarray.array('H',...)");

	if (dest_len & 0x3)
		mp_raise_ValueError("dest array size must be a multiple of 4");

	int n = dest_len/4;
	int h = dest_len/2;

	dest[0] = 0;
	dest[n] = 32767;
	dest[h] = 65535;
	dest[h+n] = 32767;

	float fac = 2.f*(float)M_PI/(float)dest_len;
	int i;
	for (i=1; i<n; i++)
	{
		int cosine = (int)floorf( (1+cosf( (float)i * fac )) *32767.5 );
		dest[dest_len-i] = dest[i] = 65535-cosine;
		dest[h-i] = dest[h+i] = cosine;
	}

	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(cball_wave_lut_obj, cball_wave_lut);

static mp_obj_t cball_wave_for_gradient_lut(mp_obj_t buf)
{
	/* args:
	 * -     dest        [n*4]  uint16,
	 */

	uint16_t *dest;
	size_t dest_len = cball_get_uint16_array(buf, &dest, MP_BUFFER_WRITE,
	                  "dest needs to be a uarray.array('H',...)");

	if (dest_len & 0x3)
		mp_raise_ValueError("dest array size must be a multiple of 4");

	int n = dest_len/4;
	int h = dest_len/2;

	dest[0] = 0;
	dest[n] = 16384;
	dest[h] = 65535;
	dest[h+n] = 16384;

	float fac = 2.f*(float)M_PI/(float)dest_len;
	int i;
	for (i=1; i<n; i++)
	{
		float cosine = cosf( (float)i * fac );
		uint32_t tmp = (uint32_t)( (1.f-cosine)*32703.94f );
		dest[dest_len-i] = dest[i] = (tmp*tmp)>>16;
		tmp = (uint32_t)((1.f+cosine)*32703.94f);
		dest[h-i] = dest[h+i] = (tmp*tmp)>>16;
	}

	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(cball_wave_for_gradient_lut_obj, cball_wave_for_gradient_lut);

static const uint16_t colordrift_lut[1025] =
{
/*
 * table = [ '0x{:04x},'.format(round((2**12-.5)*(1-math.cos(math.pi*i/1024)))) for i in range(1025) ]
 * for i in range(0,1025,16):
 *     print ( '\t' + ''.join( table[i:i+16] ) )
 *
 */
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0001,0x0001,0x0001,0x0002,0x0002,0x0002,0x0003,0x0003,0x0004,0x0004,
	0x0005,0x0006,0x0006,0x0007,0x0008,0x0008,0x0009,0x000a,0x000b,0x000c,0x000d,0x000e,0x000f,0x0010,0x0011,0x0013,
	0x0014,0x0015,0x0016,0x0018,0x0019,0x001a,0x001c,0x001d,0x001f,0x0020,0x0022,0x0024,0x0025,0x0027,0x0029,0x002b,
	0x002c,0x002e,0x0030,0x0032,0x0034,0x0036,0x0038,0x003a,0x003c,0x003e,0x0041,0x0043,0x0045,0x0048,0x004a,0x004c,
	0x004f,0x0051,0x0054,0x0056,0x0059,0x005b,0x005e,0x0061,0x0064,0x0066,0x0069,0x006c,0x006f,0x0072,0x0075,0x0078,
	0x007b,0x007e,0x0081,0x0084,0x0087,0x008a,0x008e,0x0091,0x0094,0x0098,0x009b,0x009f,0x00a2,0x00a6,0x00a9,0x00ad,
	0x00b0,0x00b4,0x00b8,0x00bb,0x00bf,0x00c3,0x00c7,0x00cb,0x00cf,0x00d3,0x00d7,0x00db,0x00df,0x00e3,0x00e7,0x00eb,
	0x00ef,0x00f4,0x00f8,0x00fc,0x0101,0x0105,0x0109,0x010e,0x0112,0x0117,0x011c,0x0120,0x0125,0x0129,0x012e,0x0133,
	0x0138,0x013d,0x0141,0x0146,0x014b,0x0150,0x0155,0x015a,0x015f,0x0164,0x016a,0x016f,0x0174,0x0179,0x017f,0x0184,
	0x0189,0x018f,0x0194,0x0199,0x019f,0x01a5,0x01aa,0x01b0,0x01b5,0x01bb,0x01c1,0x01c6,0x01cc,0x01d2,0x01d8,0x01de,
	0x01e4,0x01ea,0x01f0,0x01f6,0x01fc,0x0202,0x0208,0x020e,0x0214,0x021a,0x0221,0x0227,0x022d,0x0233,0x023a,0x0240,
	0x0247,0x024d,0x0254,0x025a,0x0261,0x0267,0x026e,0x0275,0x027b,0x0282,0x0289,0x0290,0x0297,0x029d,0x02a4,0x02ab,
	0x02b2,0x02b9,0x02c0,0x02c7,0x02ce,0x02d6,0x02dd,0x02e4,0x02eb,0x02f2,0x02fa,0x0301,0x0308,0x0310,0x0317,0x031e,
	0x0326,0x032d,0x0335,0x033d,0x0344,0x034c,0x0353,0x035b,0x0363,0x036b,0x0372,0x037a,0x0382,0x038a,0x0392,0x039a,
	0x03a2,0x03aa,0x03b2,0x03ba,0x03c2,0x03ca,0x03d2,0x03da,0x03e2,0x03eb,0x03f3,0x03fb,0x0403,0x040c,0x0414,0x041d,
	0x0425,0x042d,0x0436,0x043e,0x0447,0x044f,0x0458,0x0461,0x0469,0x0472,0x047b,0x0483,0x048c,0x0495,0x049e,0x04a7,
	0x04b0,0x04b8,0x04c1,0x04ca,0x04d3,0x04dc,0x04e5,0x04ee,0x04f7,0x0501,0x050a,0x0513,0x051c,0x0525,0x052f,0x0538,
	0x0541,0x054a,0x0554,0x055d,0x0567,0x0570,0x0579,0x0583,0x058c,0x0596,0x05a0,0x05a9,0x05b3,0x05bc,0x05c6,0x05d0,
	0x05d9,0x05e3,0x05ed,0x05f7,0x0600,0x060a,0x0614,0x061e,0x0628,0x0632,0x063c,0x0646,0x0650,0x065a,0x0664,0x066e,
	0x0678,0x0682,0x068c,0x0696,0x06a0,0x06ab,0x06b5,0x06bf,0x06c9,0x06d4,0x06de,0x06e8,0x06f3,0x06fd,0x0707,0x0712,
	0x071c,0x0727,0x0731,0x073c,0x0746,0x0751,0x075b,0x0766,0x0770,0x077b,0x0786,0x0790,0x079b,0x07a6,0x07b0,0x07bb,
	0x07c6,0x07d1,0x07dc,0x07e6,0x07f1,0x07fc,0x0807,0x0812,0x081d,0x0828,0x0833,0x083e,0x0849,0x0854,0x085f,0x086a,
	0x0875,0x0880,0x088b,0x0896,0x08a1,0x08ad,0x08b8,0x08c3,0x08ce,0x08d9,0x08e5,0x08f0,0x08fb,0x0906,0x0912,0x091d,
	0x0928,0x0934,0x093f,0x094b,0x0956,0x0961,0x096d,0x0978,0x0984,0x098f,0x099b,0x09a6,0x09b2,0x09bd,0x09c9,0x09d5,
	0x09e0,0x09ec,0x09f7,0x0a03,0x0a0f,0x0a1a,0x0a26,0x0a32,0x0a3e,0x0a49,0x0a55,0x0a61,0x0a6d,0x0a78,0x0a84,0x0a90,
	0x0a9c,0x0aa8,0x0ab3,0x0abf,0x0acb,0x0ad7,0x0ae3,0x0aef,0x0afb,0x0b07,0x0b13,0x0b1f,0x0b2b,0x0b37,0x0b43,0x0b4f,
	0x0b5b,0x0b67,0x0b73,0x0b7f,0x0b8b,0x0b97,0x0ba3,0x0baf,0x0bbb,0x0bc7,0x0bd3,0x0be0,0x0bec,0x0bf8,0x0c04,0x0c10,
	0x0c1c,0x0c29,0x0c35,0x0c41,0x0c4d,0x0c59,0x0c66,0x0c72,0x0c7e,0x0c8a,0x0c97,0x0ca3,0x0caf,0x0cbc,0x0cc8,0x0cd4,
	0x0ce1,0x0ced,0x0cf9,0x0d06,0x0d12,0x0d1e,0x0d2b,0x0d37,0x0d43,0x0d50,0x0d5c,0x0d68,0x0d75,0x0d81,0x0d8e,0x0d9a,
	0x0da7,0x0db3,0x0dbf,0x0dcc,0x0dd8,0x0de5,0x0df1,0x0dfe,0x0e0a,0x0e17,0x0e23,0x0e30,0x0e3c,0x0e49,0x0e55,0x0e62,
	0x0e6e,0x0e7b,0x0e87,0x0e94,0x0ea0,0x0ead,0x0eb9,0x0ec6,0x0ed2,0x0edf,0x0eeb,0x0ef8,0x0f04,0x0f11,0x0f1d,0x0f2a,
	0x0f37,0x0f43,0x0f50,0x0f5c,0x0f69,0x0f75,0x0f82,0x0f8e,0x0f9b,0x0fa8,0x0fb4,0x0fc1,0x0fcd,0x0fda,0x0fe6,0x0ff3,
	0x0fff,0x100c,0x1019,0x1025,0x1032,0x103e,0x104b,0x1057,0x1064,0x1071,0x107d,0x108a,0x1096,0x10a3,0x10af,0x10bc,
	0x10c8,0x10d5,0x10e2,0x10ee,0x10fb,0x1107,0x1114,0x1120,0x112d,0x1139,0x1146,0x1152,0x115f,0x116b,0x1178,0x1184,
	0x1191,0x119d,0x11aa,0x11b6,0x11c3,0x11cf,0x11dc,0x11e8,0x11f5,0x1201,0x120e,0x121a,0x1227,0x1233,0x1240,0x124c,
	0x1258,0x1265,0x1271,0x127e,0x128a,0x1297,0x12a3,0x12af,0x12bc,0x12c8,0x12d4,0x12e1,0x12ed,0x12f9,0x1306,0x1312,
	0x131e,0x132b,0x1337,0x1343,0x1350,0x135c,0x1368,0x1375,0x1381,0x138d,0x1399,0x13a6,0x13b2,0x13be,0x13ca,0x13d6,
	0x13e3,0x13ef,0x13fb,0x1407,0x1413,0x141f,0x142c,0x1438,0x1444,0x1450,0x145c,0x1468,0x1474,0x1480,0x148c,0x1498,
	0x14a4,0x14b0,0x14bc,0x14c8,0x14d4,0x14e0,0x14ec,0x14f8,0x1504,0x1510,0x151c,0x1528,0x1534,0x1540,0x154c,0x1557,
	0x1563,0x156f,0x157b,0x1587,0x1592,0x159e,0x15aa,0x15b6,0x15c1,0x15cd,0x15d9,0x15e5,0x15f0,0x15fc,0x1608,0x1613,
	0x161f,0x162a,0x1636,0x1642,0x164d,0x1659,0x1664,0x1670,0x167b,0x1687,0x1692,0x169e,0x16a9,0x16b4,0x16c0,0x16cb,
	0x16d7,0x16e2,0x16ed,0x16f9,0x1704,0x170f,0x171a,0x1726,0x1731,0x173c,0x1747,0x1752,0x175e,0x1769,0x1774,0x177f,
	0x178a,0x1795,0x17a0,0x17ab,0x17b6,0x17c1,0x17cc,0x17d7,0x17e2,0x17ed,0x17f8,0x1803,0x180e,0x1819,0x1823,0x182e,
	0x1839,0x1844,0x184f,0x1859,0x1864,0x186f,0x1879,0x1884,0x188f,0x1899,0x18a4,0x18ae,0x18b9,0x18c3,0x18ce,0x18d8,
	0x18e3,0x18ed,0x18f8,0x1902,0x190c,0x1917,0x1921,0x192b,0x1936,0x1940,0x194a,0x1954,0x195f,0x1969,0x1973,0x197d,
	0x1987,0x1991,0x199b,0x19a5,0x19af,0x19b9,0x19c3,0x19cd,0x19d7,0x19e1,0x19eb,0x19f5,0x19ff,0x1a08,0x1a12,0x1a1c,
	0x1a26,0x1a2f,0x1a39,0x1a43,0x1a4c,0x1a56,0x1a5f,0x1a69,0x1a73,0x1a7c,0x1a86,0x1a8f,0x1a98,0x1aa2,0x1aab,0x1ab5,
	0x1abe,0x1ac7,0x1ad0,0x1ada,0x1ae3,0x1aec,0x1af5,0x1afe,0x1b08,0x1b11,0x1b1a,0x1b23,0x1b2c,0x1b35,0x1b3e,0x1b47,
	0x1b4f,0x1b58,0x1b61,0x1b6a,0x1b73,0x1b7c,0x1b84,0x1b8d,0x1b96,0x1b9e,0x1ba7,0x1bb0,0x1bb8,0x1bc1,0x1bc9,0x1bd2,
	0x1bda,0x1be2,0x1beb,0x1bf3,0x1bfc,0x1c04,0x1c0c,0x1c14,0x1c1d,0x1c25,0x1c2d,0x1c35,0x1c3d,0x1c45,0x1c4d,0x1c55,
	0x1c5d,0x1c65,0x1c6d,0x1c75,0x1c7d,0x1c85,0x1c8d,0x1c94,0x1c9c,0x1ca4,0x1cac,0x1cb3,0x1cbb,0x1cc2,0x1cca,0x1cd2,
	0x1cd9,0x1ce1,0x1ce8,0x1cef,0x1cf7,0x1cfe,0x1d05,0x1d0d,0x1d14,0x1d1b,0x1d22,0x1d29,0x1d31,0x1d38,0x1d3f,0x1d46,
	0x1d4d,0x1d54,0x1d5b,0x1d62,0x1d68,0x1d6f,0x1d76,0x1d7d,0x1d84,0x1d8a,0x1d91,0x1d98,0x1d9e,0x1da5,0x1dab,0x1db2,
	0x1db8,0x1dbf,0x1dc5,0x1dcc,0x1dd2,0x1dd8,0x1dde,0x1de5,0x1deb,0x1df1,0x1df7,0x1dfd,0x1e03,0x1e09,0x1e0f,0x1e15,
	0x1e1b,0x1e21,0x1e27,0x1e2d,0x1e33,0x1e39,0x1e3e,0x1e44,0x1e4a,0x1e4f,0x1e55,0x1e5a,0x1e60,0x1e66,0x1e6b,0x1e70,
	0x1e76,0x1e7b,0x1e80,0x1e86,0x1e8b,0x1e90,0x1e95,0x1e9b,0x1ea0,0x1ea5,0x1eaa,0x1eaf,0x1eb4,0x1eb9,0x1ebe,0x1ec2,
	0x1ec7,0x1ecc,0x1ed1,0x1ed6,0x1eda,0x1edf,0x1ee3,0x1ee8,0x1eed,0x1ef1,0x1ef6,0x1efa,0x1efe,0x1f03,0x1f07,0x1f0b,
	0x1f10,0x1f14,0x1f18,0x1f1c,0x1f20,0x1f24,0x1f28,0x1f2c,0x1f30,0x1f34,0x1f38,0x1f3c,0x1f40,0x1f44,0x1f47,0x1f4b,
	0x1f4f,0x1f52,0x1f56,0x1f59,0x1f5d,0x1f60,0x1f64,0x1f67,0x1f6b,0x1f6e,0x1f71,0x1f75,0x1f78,0x1f7b,0x1f7e,0x1f81,
	0x1f84,0x1f87,0x1f8a,0x1f8d,0x1f90,0x1f93,0x1f96,0x1f99,0x1f9b,0x1f9e,0x1fa1,0x1fa4,0x1fa6,0x1fa9,0x1fab,0x1fae,
	0x1fb0,0x1fb3,0x1fb5,0x1fb7,0x1fba,0x1fbc,0x1fbe,0x1fc1,0x1fc3,0x1fc5,0x1fc7,0x1fc9,0x1fcb,0x1fcd,0x1fcf,0x1fd1,
	0x1fd3,0x1fd4,0x1fd6,0x1fd8,0x1fda,0x1fdb,0x1fdd,0x1fdf,0x1fe0,0x1fe2,0x1fe3,0x1fe5,0x1fe6,0x1fe7,0x1fe9,0x1fea,
	0x1feb,0x1fec,0x1fee,0x1fef,0x1ff0,0x1ff1,0x1ff2,0x1ff3,0x1ff4,0x1ff5,0x1ff6,0x1ff7,0x1ff7,0x1ff8,0x1ff9,0x1ff9,
	0x1ffa,0x1ffb,0x1ffb,0x1ffc,0x1ffc,0x1ffd,0x1ffd,0x1ffd,0x1ffe,0x1ffe,0x1ffe,0x1fff,0x1fff,0x1fff,0x1fff,0x1fff,
	0x1fff,
};


extern const mp_obj_type_t colordrift_type;
typedef struct
{
	mp_obj_base_t base;

	int n_colors;
	uint32_t phase_dt, phase_shift, phase_div, phase, period;
	uint16_t *colors;


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

	self->colors = m_new(uint16_t, 3*self->n_colors);
	memset(self->colors, 0, sizeof(uint16_t)*3*self->n_colors);

	return MP_OBJ_FROM_PTR(self);
}

static void get_random_color(uint16_t *rgb)
{
	uint32_t random = esp_random();
	float h = (float)(random&0xffff)/65535.f;
	float s = 1.f;
	float i = (float)(((random>>16)&0xffff)+0x10000)/131071.f;
	HSItoRGB(h, s, i, rgb);
}

static void colordrift_next_color(colordrift_obj_t *self, uint16_t *rgb)
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

	rgb[0] = (r > 0x1fffe000) ? 0xffff : (r>>13);
	rgb[1] = (g > 0x1fffe000) ? 0xffff : (g>>13);
	rgb[2] = (b > 0x1fffe000) ? 0xffff : (b>>13);
}

static mp_obj_t cball_colordrift_next_color(mp_obj_t self_in)
{
	colordrift_obj_t *self = MP_OBJ_TO_PTR(self_in);
	uint16_t rgb[3];
	colordrift_next_color(self, rgb);
	mp_obj_tuple_t *rgb_tuple = mp_obj_new_tuple(3, NULL);
	rgb_tuple->items[0] = mp_obj_new_int(rgb[0]);
	rgb_tuple->items[1] = mp_obj_new_int(rgb[1]);
	rgb_tuple->items[2] = mp_obj_new_int(rgb[2]);
	return rgb_tuple;
}

static MP_DEFINE_CONST_FUN_OBJ_1(cball_colordrift_next_color_obj, cball_colordrift_next_color);

static mp_obj_t cball_colordrift_next_color_into(mp_obj_t self_in, mp_obj_t buf_in, mp_obj_t offset_in)
{
	uint16_t *buf;
	size_t buf_len = cball_get_uint16_array(buf_in, &buf, MP_BUFFER_WRITE,
	                 "buf needs to be a uarray.array('H',...)");

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
	{ MP_ROM_QSTR(MP_QSTR_array_set), MP_ROM_PTR(&cball_array_set_obj) },
	{ MP_ROM_QSTR(MP_QSTR_array_copy), MP_ROM_PTR(&cball_array_copy_obj) },
	{ MP_ROM_QSTR(MP_QSTR_array_blend), MP_ROM_PTR(&cball_array_blend_obj) },
	{ MP_ROM_QSTR(MP_QSTR_array_interval_multiply), MP_ROM_PTR(&cball_array_interval_multiply_obj) },
	{ MP_ROM_QSTR(MP_QSTR_array_add_clamp), MP_ROM_PTR(&cball_array_add_clamp_obj) },
	{ MP_ROM_QSTR(MP_QSTR_array_sub_clamp), MP_ROM_PTR(&cball_array_sub_clamp_obj) },
	{ MP_ROM_QSTR(MP_QSTR_array_max), MP_ROM_PTR(&cball_array_max_obj) },

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

