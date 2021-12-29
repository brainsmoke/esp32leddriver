#!/usr/bin/env python3
#
# Copyright (c) 2020 Erik Bosman <erik@minemu.org>
#
# Permission  is  hereby  granted,  free  of  charge,  to  any  person
# obtaining  a copy  of  this  software  and  associated documentation
# files (the "Software"),  to deal in the Software without restriction,
# including  without  limitation  the  rights  to  use,  copy,  modify,
# merge, publish, distribute, sublicense, and/or sell copies of the
# Software,  and to permit persons to whom the Software is furnished to
# do so, subject to the following conditions:
#
# The  above  copyright  notice  and this  permission  notice  shall be
# included  in  all  copies  or  substantial portions  of the Software.
#
# THE SOFTWARE  IS  PROVIDED  "AS IS", WITHOUT WARRANTY  OF ANY KIND,
# EXPRESS OR IMPLIED,  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY,  FITNESS  FOR  A  PARTICULAR  PURPOSE  AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
# BE LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER LIABILITY, WHETHER IN AN
# ACTION OF CONTRACT,  TORT OR OTHERWISE,  ARISING FROM, OUT OF OR IN
# CONNECTION  WITH THE SOFTWARE  OR THE USE  OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# (http://opensource.org/licenses/mit-license.html)
#

route_a = [ (1, 0), (1, 1), (1, 2), (1, 3),
                                    (0, 3),
                                    (2, 3),
                            (0, 2),
                            (2, 2),
                    (0, 1),
                    (2, 1),
            (0, 0),
            (2, 0),
                    (1, 4),
                    (2, 4), ]

route_b = [ (1, 0), (1, 1), (1, 2), (1, 3),
            (0, 0), (1, 4), (2, 5), (0, 6),
                                    (1, 6),
                            (1, 5),

                            (0, 2),
                            (2, 2),
                    (0, 1),
                    (2, 1), ]

def msg_data(route):
    d = []
    prev = [1000]*len(route)
    for out,node in reversed(route):
        d.append( (min(prev[node],15), (int(out==0), int(out==1), int(out==2)) ) )
        prev = [ x+1 for x in prev ]
        prev[node] = 0
    return list(reversed(d))

def format_data(route):
    for count, bits in route:
        print("0x{:02x},".format( (count<<3) | (bits[0]) | (bits[1]<<1) | (bits[2]<<2) ), end='')
    print()

format_data (msg_data(route_a))
format_data (msg_data(route_b))
format_data (msg_data(route_a))
format_data (msg_data(route_b))


def msg_parse(d):
    l = [ [1,(0,0,0)] for _ in range(16) ]
    for count, bits in d:
        i=0
        while True:
            l[i][0] -= 1
            if l[i][0] == 0:
                break
            i += 1
        l[i][0] = count+1
        l[i][1] = tuple( a+b for a,b in zip(l[i][1], bits) )
        print (i, bits)

route_test = [ (1, 0), (1, 1), (1, 2), (1, 3), (1, 4), (1, 5),
                                               (0, 4),
                                       (0, 3),
                                       (2, 3),
                               (0, 2),
                       (0, 1),
                       (2, 1),
               (0, 0),
               (2, 0) ]

#route_data = msg_data(route_test)

