
import machine, uasyncio, time

class PinEvent:

    def _irq(self):
        self.flag.set()

    def __init__(self, number, trigger=machine.Pin.IRQ_FALLING | machine.Pin.IRQ_RISING):
        self.state = bytearray(1)
        self.pin = machine.Pin(number, machine.Pin.IN)
        self.flag = uasyncio.ThreadSafeFlag()
        self.pin.irq(handler=lambda _: self.flag.set(), trigger=trigger)

    def wait(self):
        if self.flag._flag == 1:
            uasyncio.run(self.flag.wait())

        uasyncio.run(self.flag.wait())
