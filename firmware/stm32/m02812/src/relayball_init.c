#include <stdlib.h>
#include "stm32f0xx.h"

#include "relayball.h"
#include "util.h"
#include "fsm.h"

uint16_t frame_a[N_VALUES];
uint16_t frame_b[N_VALUES];
uint8_t table[ROUTING_TABLE_SIZE_PADDED];
uint16_t *cur;
uint8_t *routing_table;
uint16_t pulse_width8x;

volatile uint8_t recv_buf[RECV_BUF_SZ];

void init(void)
{
	int i;
	for (i=0; i<N_VALUES; i++)
		remainders[i] = i*153;

	clock48mhz();
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN; 	// enable the clock to GPIOA
	GPIOA->ODR = 0;
	GPIOA->MODER = SWD|O(0)|O(1)|O(2)|O(3)|O(4)|O(5)|O(6)|O(7);

	GPIOB->ODR = 1<<1;
	GPIOB->MODER = O(1);
	usart1_rx_pa10_dma3_enable(recv_buf, RECV_BUF_SZ, 48e6/1e6);

	routing_table = table;
	for (i=0; i<ROUTING_TABLE_SIZE; i++)
		table[i] = default_route[i];
}

/*
 *
 * route_a = [ (1, 0), (1, 1), (1, 2), (1, 3),
 *                                     (0, 3),
 *                                     (2, 3),
 *                             (0, 2),
 *                             (2, 2),
 *             (2, 0),
 *                     (1, 4),
 *                     (2, 4),
 *             (0, 0),
 *
 *                     (0, 1),
 *                     (2, 1) ]
 *
 *
 * route_b = [ (1, 0), (1, 1), (1, 2), (1, 3),
 *             (0, 0), (1, 4), (2, 5), (0, 6),
 *                                     (1, 6),
 *                             (1, 5),
 *
 *                             (0, 2),
 *                             (2, 2),
 *                     (0, 1),
 *                     (2, 1) ]
 *
 * def msg_data(route):
 *     d = []
 *     prev = [1000]*len(route)
 *     for out,node in reversed(route):
 *         d.append( (min(prev[node],15), (int(out==0), int(out==1), int(out==2)) ) )
 *         prev = [ x+1 for x in prev ]
 *         prev[node] = 0
 *     return list(reversed(d))
 *
 * def format_data(route):
 *     for count, bits in route:
 *         print("0x{:02x},".format( (count<<3) | (bits[0]) | (bits[1]<<1) | (bits[2]<<2) ), end='')
 *     print()
 *
 * format_data (msg_data(route_a))
 * format_data (msg_data(route_b))
 * format_data (msg_data(route_b))
 * format_data (msg_data(route_a))
 *
 */

const uint8_t default_route[ROUTING_TABLE_SIZE] =
{
0x3a,0x52,0x1a,0x02,0x01,0x7c,0x01,0x7c,0x14,0x02,0x7c,0x79,0x01,0x7c,
0x1a,0x52,0x3a,0x7a,0x79,0x7a,0x14,0x01,0x7a,0x7a,0x01,0x7c,0x01,0x7c,
0x1a,0x52,0x3a,0x7a,0x79,0x7a,0x14,0x01,0x7a,0x7a,0x01,0x7c,0x01,0x7c,
0x3a,0x52,0x1a,0x02,0x01,0x7c,0x01,0x7c,0x14,0x02,0x7c,0x79,0x01,0x7c,
};

