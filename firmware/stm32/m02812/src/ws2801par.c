#include <stdlib.h>
#include <string.h>
#include "stm32f0xx.h"

#include "ws2801par.h"
#include "util.h"

typedef struct
{
	transposed_t transpose[N_VALUES_PER_STRIP];
	uint8_t low_bytes[N_VALUES];

} frame_t;

frame_t frame_a, frame_b;

uint8_t residual[N_VALUES];
frame_t * volatile cur;
frame_t * volatile next;

#define RECV_BUF_SZ (2048)
volatile uint8_t recv_buf[RECV_BUF_SZ];

/*
# emulate original single strip order expected by software
facet_order = [
    21,22, 1,19,15,16,17,18, 2, 6, 7, 8, 9, 5, 3,
    52,53,54,50,51, 4,12,13,14,10,11, 0,23,24,20,
    34,30,44,35,36,37,38,39,40,49,45,46,47,48,41,
    59,55,56,57,58,42,26,27,28,29,25,43,31,32,33
]

led_order = [ f*5+l for f in facet_order for l in range(5) ]

byte_order = [ l*3+c for l in led_order for c in (1,0,2) ]

n_strips = 12
n_bytes_per_strip = len(byte_order)//n_strips
n_values = n_strips*n_bytes_per_strip
assert len(byte_order) == n_values

*/
static const uint16_t low_byte_index[] = 
{
/*

# byte 0 strip 0, byte 0 strip 1, ...
scattered = [ None ] * n_values

for i in range(len(byte_order)):
    scattered[i] = (byte_order[i]%n_bytes_per_strip)*n_strips + byte_order[i]//n_bytes_per_strip

for line in range(0, n_values, 15):
    print("\t" + ''.join('{:4d},'.format(n) for n in scattered[line:line+15]))
*/
	 196, 184, 208, 232, 220, 244, 268, 256, 280, 304, 292, 316, 340, 328, 352,
	 376, 364, 388, 412, 400, 424, 448, 436, 460, 484, 472, 496, 520, 508, 532,
	 192, 180, 204, 228, 216, 240, 264, 252, 276, 300, 288, 312, 336, 324, 348,
	 735, 723, 747, 771, 759, 783, 807, 795, 819, 843, 831, 855, 879, 867, 891,
	  15,   3,  27,  51,  39,  63,  87,  75,  99, 123, 111, 135, 159, 147, 171,
	 195, 183, 207, 231, 219, 243, 267, 255, 279, 303, 291, 315, 339, 327, 351,
	 375, 363, 387, 411, 399, 423, 447, 435, 459, 483, 471, 495, 519, 507, 531,
	 555, 543, 567, 591, 579, 603, 627, 615, 639, 663, 651, 675, 699, 687, 711,
	 372, 360, 384, 408, 396, 420, 444, 432, 456, 480, 468, 492, 516, 504, 528,
	 193, 181, 205, 229, 217, 241, 265, 253, 277, 301, 289, 313, 337, 325, 349,
	 373, 361, 385, 409, 397, 421, 445, 433, 457, 481, 469, 493, 517, 505, 529,
	 553, 541, 565, 589, 577, 601, 625, 613, 637, 661, 649, 673, 697, 685, 709,
	 733, 721, 745, 769, 757, 781, 805, 793, 817, 841, 829, 853, 877, 865, 889,
	  13,   1,  25,  49,  37,  61,  85,  73,  97, 121, 109, 133, 157, 145, 169,
	 552, 540, 564, 588, 576, 600, 624, 612, 636, 660, 648, 672, 696, 684, 708,
	 382, 370, 394, 418, 406, 430, 454, 442, 466, 490, 478, 502, 526, 514, 538,
	 562, 550, 574, 598, 586, 610, 634, 622, 646, 670, 658, 682, 706, 694, 718,
	 742, 730, 754, 778, 766, 790, 814, 802, 826, 850, 838, 862, 886, 874, 898,
	  22,  10,  34,  58,  46,  70,  94,  82, 106, 130, 118, 142, 166, 154, 178,
	 202, 190, 214, 238, 226, 250, 274, 262, 286, 310, 298, 322, 346, 334, 358,
	 732, 720, 744, 768, 756, 780, 804, 792, 816, 840, 828, 852, 876, 864, 888,
	 374, 362, 386, 410, 398, 422, 446, 434, 458, 482, 470, 494, 518, 506, 530,
	 554, 542, 566, 590, 578, 602, 626, 614, 638, 662, 650, 674, 698, 686, 710,
	 734, 722, 746, 770, 758, 782, 806, 794, 818, 842, 830, 854, 878, 866, 890,
	  14,   2,  26,  50,  38,  62,  86,  74,  98, 122, 110, 134, 158, 146, 170,
	 194, 182, 206, 230, 218, 242, 266, 254, 278, 302, 290, 314, 338, 326, 350,
	  12,   0,  24,  48,  36,  60,  84,  72,  96, 120, 108, 132, 156, 144, 168,
	 556, 544, 568, 592, 580, 604, 628, 616, 640, 664, 652, 676, 700, 688, 712,
	 736, 724, 748, 772, 760, 784, 808, 796, 820, 844, 832, 856, 880, 868, 892,
	  16,   4,  28,  52,  40,  64,  88,  76, 100, 124, 112, 136, 160, 148, 172,
	 738, 726, 750, 774, 762, 786, 810, 798, 822, 846, 834, 858, 882, 870, 894,
	  18,   6,  30,  54,  42,  66,  90,  78, 102, 126, 114, 138, 162, 150, 174,
	 740, 728, 752, 776, 764, 788, 812, 800, 824, 848, 836, 860, 884, 872, 896,
	  19,   7,  31,  55,  43,  67,  91,  79, 103, 127, 115, 139, 163, 151, 175,
	 199, 187, 211, 235, 223, 247, 271, 259, 283, 307, 295, 319, 343, 331, 355,
	 379, 367, 391, 415, 403, 427, 451, 439, 463, 487, 475, 499, 523, 511, 535,
	 559, 547, 571, 595, 583, 607, 631, 619, 643, 667, 655, 679, 703, 691, 715,
	 739, 727, 751, 775, 763, 787, 811, 799, 823, 847, 835, 859, 883, 871, 895,
	  20,   8,  32,  56,  44,  68,  92,  80, 104, 128, 116, 140, 164, 152, 176,
	 741, 729, 753, 777, 765, 789, 813, 801, 825, 849, 837, 861, 885, 873, 897,
	  21,   9,  33,  57,  45,  69,  93,  81, 105, 129, 117, 141, 165, 153, 177,
	 201, 189, 213, 237, 225, 249, 273, 261, 285, 309, 297, 321, 345, 333, 357,
	 381, 369, 393, 417, 405, 429, 453, 441, 465, 489, 477, 501, 525, 513, 537,
	 561, 549, 573, 597, 585, 609, 633, 621, 645, 669, 657, 681, 705, 693, 717,
	 200, 188, 212, 236, 224, 248, 272, 260, 284, 308, 296, 320, 344, 332, 356,
	 743, 731, 755, 779, 767, 791, 815, 803, 827, 851, 839, 863, 887, 875, 899,
	  23,  11,  35,  59,  47,  71,  95,  83, 107, 131, 119, 143, 167, 155, 179,
	 203, 191, 215, 239, 227, 251, 275, 263, 287, 311, 299, 323, 347, 335, 359,
	 383, 371, 395, 419, 407, 431, 455, 443, 467, 491, 479, 503, 527, 515, 539,
	 563, 551, 575, 599, 587, 611, 635, 623, 647, 671, 659, 683, 707, 695, 719,
	 380, 368, 392, 416, 404, 428, 452, 440, 464, 488, 476, 500, 524, 512, 536,
	 197, 185, 209, 233, 221, 245, 269, 257, 281, 305, 293, 317, 341, 329, 353,
	 377, 365, 389, 413, 401, 425, 449, 437, 461, 485, 473, 497, 521, 509, 533,
	 557, 545, 569, 593, 581, 605, 629, 617, 641, 665, 653, 677, 701, 689, 713,
	 737, 725, 749, 773, 761, 785, 809, 797, 821, 845, 833, 857, 881, 869, 893,
	  17,   5,  29,  53,  41,  65,  89,  77, 101, 125, 113, 137, 161, 149, 173,
	 560, 548, 572, 596, 584, 608, 632, 620, 644, 668, 656, 680, 704, 692, 716,
	 198, 186, 210, 234, 222, 246, 270, 258, 282, 306, 294, 318, 342, 330, 354,
	 378, 366, 390, 414, 402, 426, 450, 438, 462, 486, 474, 498, 522, 510, 534,
	 558, 546, 570, 594, 582, 606, 630, 618, 642, 666, 654, 678, 702, 690, 714,
};

static const uint8_t high_byte_index[] = 
{
/*
for line in range(0, n_values, 30):
    print("\t" + ''.join('{:2d},'.format(n//12) for n in scattered[line:line+30]))
*/
	16,15,17,19,18,20,22,21,23,25,24,26,28,27,29,31,30,32,34,33,35,37,36,38,40,39,41,43,42,44,
	16,15,17,19,18,20,22,21,23,25,24,26,28,27,29,61,60,62,64,63,65,67,66,68,70,69,71,73,72,74,
	 1, 0, 2, 4, 3, 5, 7, 6, 8,10, 9,11,13,12,14,16,15,17,19,18,20,22,21,23,25,24,26,28,27,29,
	31,30,32,34,33,35,37,36,38,40,39,41,43,42,44,46,45,47,49,48,50,52,51,53,55,54,56,58,57,59,
	31,30,32,34,33,35,37,36,38,40,39,41,43,42,44,16,15,17,19,18,20,22,21,23,25,24,26,28,27,29,
	31,30,32,34,33,35,37,36,38,40,39,41,43,42,44,46,45,47,49,48,50,52,51,53,55,54,56,58,57,59,
	61,60,62,64,63,65,67,66,68,70,69,71,73,72,74, 1, 0, 2, 4, 3, 5, 7, 6, 8,10, 9,11,13,12,14,
	46,45,47,49,48,50,52,51,53,55,54,56,58,57,59,31,30,32,34,33,35,37,36,38,40,39,41,43,42,44,
	46,45,47,49,48,50,52,51,53,55,54,56,58,57,59,61,60,62,64,63,65,67,66,68,70,69,71,73,72,74,
	 1, 0, 2, 4, 3, 5, 7, 6, 8,10, 9,11,13,12,14,16,15,17,19,18,20,22,21,23,25,24,26,28,27,29,
	61,60,62,64,63,65,67,66,68,70,69,71,73,72,74,31,30,32,34,33,35,37,36,38,40,39,41,43,42,44,
	46,45,47,49,48,50,52,51,53,55,54,56,58,57,59,61,60,62,64,63,65,67,66,68,70,69,71,73,72,74,
	 1, 0, 2, 4, 3, 5, 7, 6, 8,10, 9,11,13,12,14,16,15,17,19,18,20,22,21,23,25,24,26,28,27,29,
	 1, 0, 2, 4, 3, 5, 7, 6, 8,10, 9,11,13,12,14,46,45,47,49,48,50,52,51,53,55,54,56,58,57,59,
	61,60,62,64,63,65,67,66,68,70,69,71,73,72,74, 1, 0, 2, 4, 3, 5, 7, 6, 8,10, 9,11,13,12,14,
	61,60,62,64,63,65,67,66,68,70,69,71,73,72,74, 1, 0, 2, 4, 3, 5, 7, 6, 8,10, 9,11,13,12,14,
	61,60,62,64,63,65,67,66,68,70,69,71,73,72,74, 1, 0, 2, 4, 3, 5, 7, 6, 8,10, 9,11,13,12,14,
	16,15,17,19,18,20,22,21,23,25,24,26,28,27,29,31,30,32,34,33,35,37,36,38,40,39,41,43,42,44,
	46,45,47,49,48,50,52,51,53,55,54,56,58,57,59,61,60,62,64,63,65,67,66,68,70,69,71,73,72,74,
	 1, 0, 2, 4, 3, 5, 7, 6, 8,10, 9,11,13,12,14,61,60,62,64,63,65,67,66,68,70,69,71,73,72,74,
	 1, 0, 2, 4, 3, 5, 7, 6, 8,10, 9,11,13,12,14,16,15,17,19,18,20,22,21,23,25,24,26,28,27,29,
	31,30,32,34,33,35,37,36,38,40,39,41,43,42,44,46,45,47,49,48,50,52,51,53,55,54,56,58,57,59,
	16,15,17,19,18,20,22,21,23,25,24,26,28,27,29,61,60,62,64,63,65,67,66,68,70,69,71,73,72,74,
	 1, 0, 2, 4, 3, 5, 7, 6, 8,10, 9,11,13,12,14,16,15,17,19,18,20,22,21,23,25,24,26,28,27,29,
	31,30,32,34,33,35,37,36,38,40,39,41,43,42,44,46,45,47,49,48,50,52,51,53,55,54,56,58,57,59,
	31,30,32,34,33,35,37,36,38,40,39,41,43,42,44,16,15,17,19,18,20,22,21,23,25,24,26,28,27,29,
	31,30,32,34,33,35,37,36,38,40,39,41,43,42,44,46,45,47,49,48,50,52,51,53,55,54,56,58,57,59,
	61,60,62,64,63,65,67,66,68,70,69,71,73,72,74, 1, 0, 2, 4, 3, 5, 7, 6, 8,10, 9,11,13,12,14,
	46,45,47,49,48,50,52,51,53,55,54,56,58,57,59,16,15,17,19,18,20,22,21,23,25,24,26,28,27,29,
	31,30,32,34,33,35,37,36,38,40,39,41,43,42,44,46,45,47,49,48,50,52,51,53,55,54,56,58,57,59,
};

#define P0 (1<<9)
#define P1 (1<<10)
#define P2 (1<<11)
#define P3 (1<<12)
#define P4 (1<<13)
#define P5 (1<<14)
#define P6 (1<<1)
#define P7 (1<<2)
#define P8 (1<<3)
#define P9 (1<<4)
#define PA (1<<5)
#define PB (1<<6)

static const uint16_t high_byte_pin[] =
{
/*
for line in range(0, n_values, 30):
    print("\t" + ''.join('P{:X},'.format(n%12) for n in scattered[line:line+30]))
*/
	P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,
	P0,P0,P0,P0,P0,P0,P0,P0,P0,P0,P0,P0,P0,P0,P0,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,
	P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,
	P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,P3,
	P0,P0,P0,P0,P0,P0,P0,P0,P0,P0,P0,P0,P0,P0,P0,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,
	P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,
	P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,P1,
	P0,P0,P0,P0,P0,P0,P0,P0,P0,P0,P0,P0,P0,P0,P0,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,
	PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,
	PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,PA,
	P0,P0,P0,P0,P0,P0,P0,P0,P0,P0,P0,P0,P0,P0,P0,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,
	P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,
	P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,P2,
	P0,P0,P0,P0,P0,P0,P0,P0,P0,P0,P0,P0,P0,P0,P0,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,
	P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,P4,
	P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,
	P8,P8,P8,P8,P8,P8,P8,P8,P8,P8,P8,P8,P8,P8,P8,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,
	P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,
	P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,P7,
	P8,P8,P8,P8,P8,P8,P8,P8,P8,P8,P8,P8,P8,P8,P8,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,
	P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,
	P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,P9,
	P8,P8,P8,P8,P8,P8,P8,P8,P8,P8,P8,P8,P8,P8,P8,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,
	PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,
	PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,PB,
	P8,P8,P8,P8,P8,P8,P8,P8,P8,P8,P8,P8,P8,P8,P8,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,
	P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,
	P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,P5,
	P8,P8,P8,P8,P8,P8,P8,P8,P8,P8,P8,P8,P8,P8,P8,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,
	P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,P6,
};

void SysTick_Handler(void)
{
	precomp_dithering(cur->transpose, cur->low_bytes, residual, N_VALUES_PER_STRIP);
	bitbang_ws2801(cur->transpose, N_VALUES_PER_STRIP, CLK_MASK, GPIOB);
}


static void init(void)
{
	int i;
	for (i=0; i<N_VALUES; i++)
		residual[i] = i*153;
	cur = &frame_a;
	next = &frame_b;

	clock48mhz();
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN; 	// enable the clock to GPIOA
	GPIOA->ODR = 0;
	GPIOA->MODER = SWD;

	GPIOB->ODR = 0;
	GPIOB->MODER = O(0)|O(1)|O(2)|O(3)|O(4)|O(5)|O(6)|O(7)|O(8)|O(9)|O(10)|O(11)|O(12)|O(13)|O(14)|O(15);

	usart2_rx_pa3_dma5_enable(recv_buf, RECV_BUF_SZ, 48e6/2e6);
	enable_sys_tick(SYS_TICK_FRAMERATE);
}

static volatile uint8_t *recv_p=recv_buf, *recv_end=recv_buf;

static void dma_wait(void)
{
	if (recv_p == &recv_buf[RECV_BUF_SZ])
		recv_p = recv_end = &recv_buf[0];

	while(recv_p == recv_end)
	{
		recv_end = &recv_buf[RECV_BUF_SZ-DMA1_Channel5->CNDTR];
		if (recv_p > recv_end)
			recv_end = &recv_buf[RECV_BUF_SZ];
	}	
}

enum
{
	GOOD,
	GOOD_00,
	GOOD_01_FE,
	GOOD_FF,
	GOOD_FFFF,
	GOOD_FFFFFF,
	BAD,
	BAD_FF,
	BAD_FFFF,
	BAD_FFFFFF,

	STATE_COUNT,

	GOOD_RETURN,
	BAD_RETURN,
};

enum
{
	IN_00,
	IN_01_EF,
	IN_F0,
	IN_F1_FE,
	IN_FF,
};

static const uint8_t fsm[STATE_COUNT][8] =
{
	[GOOD]        = {  [IN_00] = GOOD_00, [IN_01_EF] = GOOD_01_FE, [IN_F0] = GOOD_01_FE , [IN_F1_FE] = GOOD_01_FE, [IN_FF] = GOOD_FF      , },
	[GOOD_00]     = {  [IN_00] = GOOD   , [IN_01_EF] = GOOD      , [IN_F0] = GOOD       , [IN_F1_FE] = GOOD      , [IN_FF] = GOOD         , },
	[GOOD_01_FE]  = {  [IN_00] = GOOD   , [IN_01_EF] = GOOD      , [IN_F0] = GOOD       , [IN_F1_FE] = GOOD      , [IN_FF] = BAD_FF       , },
	[GOOD_FF]     = {  [IN_00] = GOOD   , [IN_01_EF] = GOOD      , [IN_F0] = GOOD       , [IN_F1_FE] = GOOD      , [IN_FF] = GOOD_FFFF    , },
	[GOOD_FFFF]   = {  [IN_00] = BAD    , [IN_01_EF] = BAD       , [IN_F0] = BAD        , [IN_F1_FE] = BAD       , [IN_FF] = GOOD_FFFFFF  , },
	[GOOD_FFFFFF] = {  [IN_00] = BAD    , [IN_01_EF] = BAD       , [IN_F0] = GOOD_RETURN, [IN_F1_FE] = BAD       , [IN_FF] = BAD_FFFFFF   , },
	[BAD]         = {  [IN_00] = BAD    , [IN_01_EF] = BAD       , [IN_F0] = BAD        , [IN_F1_FE] = BAD       , [IN_FF] = BAD_FF       , },
	[BAD_FF]      = {  [IN_00] = BAD    , [IN_01_EF] = BAD       , [IN_F0] = BAD        , [IN_F1_FE] = BAD       , [IN_FF] = BAD_FFFF     , },
	[BAD_FFFF]    = {  [IN_00] = BAD    , [IN_01_EF] = BAD       , [IN_F0] = BAD        , [IN_F1_FE] = BAD       , [IN_FF] = BAD_FFFFFF   , },
	[BAD_FFFFFF]  = {  [IN_00] = BAD    , [IN_01_EF] = BAD       , [IN_F0] = BAD_RETURN , [IN_F1_FE] = BAD       , [IN_FF] = BAD_FFFFFF   , },

};

static int read_next_frame(void)
{
	int i, c;

	memset(next, 0, sizeof(frame_t));

	for(i=0; i<N_VALUES; i++)
	{
		if (recv_p == recv_end)
			dma_wait();

		c = *recv_p++;
		next->low_bytes[low_byte_index[i]] = c;

		if (recv_p == recv_end)
			dma_wait();

		c |= (*recv_p++)<<8;

		if (c > 0xff00)
			break;

		c >>= 8;

		transposed_t *t = &next->transpose[high_byte_index[i]];
		int pin = high_byte_pin[i];
		//write_value(t, pin, c); // faster, but there's plenty cycle budget
		if (c & 0x01)
			t->bit0 |= pin;
		if (c & 0x02)
			t->bit1 |= pin;
		if (c & 0x04)
			t->bit2 |= pin;
		if (c & 0x08)
			t->bit3 |= pin;
		if (c & 0x10)
			t->bit4 |= pin;
		if (c & 0x20)
			t->bit5 |= pin;
		if (c & 0x40)
			t->bit6 |= pin;
		if (c & 0x80)
			t->bit7 |= pin;
	}

	int s=GOOD;

	if (c == 0xffff)
		s = GOOD_FFFF;
	else if (c > 0xff00)
		s = BAD_FF;

	for(;;)
	{
		if (recv_p == recv_end)
			dma_wait();

		c = *recv_p++;

		if (c == 0)
			i = IN_00;
		else if (c < 0xf0)
			i = IN_01_EF;
		else if (c == 0xf0)
			i = IN_F0;
		else if (c == 0xff)
			i = IN_FF;
		else
			i = IN_F1_FE;

		s = fsm[s][i];

		if (s == GOOD_RETURN)
			return 1;
		else if (s == BAD_RETURN)
			return 0;
	}
}

int main(void)
{
	init();
	for(;;)
	{
		frame_t *tmp = cur;
		cur = next; /* swap is not atomic, but only the assignment of cur needs to be */
		next = tmp;
		while (! read_next_frame() );
	}
}

