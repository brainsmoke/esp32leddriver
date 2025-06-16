
# IKEA OBEGRÄNSAD driver code

* Desolder the MCU on the bottom panel
* (Only) desolder the data-out to data-in wires connecting the panels
* Leave the VCC/GND/CLK/Latch/Not-output-enable wires intact
* Connect the data-in pins to the esp32leddriver board (or another board with an stm32f0xx running this firmware)
* Add ground wires next to the data wires for signal integrity
* Signals need to be boosted to 5V (for example using a 74HCT245)

## Pinout:

* PA0: Data in, top panel
* PA1: Data in, 2nd panel
* PA2: Data in, 3rd panel
* PA3: Data in, bottom panel
* PA4: Clock (wired to bottom panel)
* PA5: Latch (wired to bottom panel)
* PA6: Not Output Enable (wired to bottom panel)
* PA10: UART RX, 1MBaud (In my case, written to by one the esp32's UARTs)

## Protocol:

Data rate: 1MBaud

Sending a frame: `( [16 bit brightness]*256 [ FF FF FF F0 ] )*`

Brightness must be little endian integers in the (inclusive) range `[0 .. 0xFF00]`
 
`[ FF FF FF F0 ]` is an end of frame marker and allows the protocol to synchronize
in the event of an uneven number of bytes being written to the serial port

## Button

The button on the side connects to ground when pressed, and is accessable via a pad on the bottom panel.
I've connected it to GPIO2 of the ESP32 on my driver board.

