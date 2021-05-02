
#include "default_route.h"


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

