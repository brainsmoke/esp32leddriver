
# IKEA OBEGRÄNSAD driver code

* Desolder the MCU (U1) on the bottom panel (looking at the silkscreen text, the panels are mounted upside down).
  It's the only chip that's only present on one of the boards.
* (Only) desolder the data-out to data-in wires connecting the panels (the potting compound can be easily removed with some isopropanol)
* All signals to the panel need to be boosted to 5V (for example using a 74HCT245)
* Leave the VCC/GND/CLK/Latch/Not-output-enable wires intact
* Connect the data-in pins to the esp32leddriver board (or another board with an stm32f0xx running this firmware)
* Add ground wires next to the data wires for signal integrity
* The attached USB cable is data-only, but can be replaced
* The side-button is accessible via a pad on R5 on the bottom panel, it is pulled to ground when pressed.
* There is an alternative firmware for the STM32F042 which reads frames from USB-serial, see: [usb_proto/firmware/obegraensad](https://github.com/brainsmoke/usb_proto/tree/main/firmware/obegraensad)

<img src="/img/obegraensad_mcu_button_pin.jpg" width="640" alt="close-up of the removed MCU footprint & pad to connect to the button">


## Pinout (panel, data-in side):

Left-to-right, silkscreen orientation:

* VCC
* Latch
* Clock
* Data in
* Not Output Enable
* Ground

<img src="/img/obegraensad_panel_pinout.jpg" width="640" alt="photo of pinout on the panels">

## Pinout (driver board):

Output, boosted to 5V signalling:

* PA0: Data in, top panel
* PA1: Data in, 2nd panel
* PA2: Data in, 3rd panel
* PA3: Data in, bottom panel
* PA4: Clock (wired to bottom panel)
* PA5: Latch (wired to bottom panel)
* PA6: Not Output Enable (wired to bottom panel)

Input:

* PA10: UART RX, 1MBaud (In my case, written to by one the esp32's UARTs)

The pinout on the esp32leddriver board (v4.0, with integrated level shifters):

<img src="/img/obegraensad_driver_wiring.jpg" width="640" alt="photo of pinout on the esp32leddriver board (v4.0)">

## Protocol to the STM32 driver:

Signal: UART at 1MBaud

Sending a frame: `( [16 bit brightness]*256 [ FF FF FF F0 ] )*`

Brightness must be little endian integers in the (inclusive) range `[0 .. 0xFF00]`

One brightness value per pixel. Pixels are addressed left to right, top to bottom.

`[ FF FF FF F0 ]` is an end of frame marker and allows the protocol to synchronize
in the event of an uneven number of bytes being written to the serial port

## Picture of wiring setup

<img src="/img/obegraensad_bottom_overview.jpg" width="640" alt="photo giving an overview of the wiring situation">

## Animations software on the ESP32

My driver board contains an ESP32 which serves animations over a UART.
GPIO13 (uart-tx) of the ESP32 is connected to PA10 on the STM32 (uart-rx)

The ESP32 runs the animation software from [this repo](../../esp32).
instead of `tools/flash.sh`, run `tools/flash.sh obegraensad` to program the right configuration for this
project.

## Button

The button on the side connects to ground when pressed, and is accessable via a pad on the bottom panel.
I've connected it to GPIO2 of the ESP32 on my driver board.

## Other IKEA OBEGRÄNSAD hacks

[ph1h/ikea-led-obegraensad](https://github.com/ph1p/ikea-led-obegraensad)

