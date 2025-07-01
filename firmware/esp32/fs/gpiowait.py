
import machine, uasyncio, utime

class PinEvent:

    def _irq(self):
        self.flag.set()

    def __init__(self, number, trigger=machine.Pin.IRQ_FALLING | machine.Pin.IRQ_RISING, pull=None):
        self.state = bytearray(1)
        self.pin = machine.Pin(number, machine.Pin.IN, pull)
        self.flag = uasyncio.ThreadSafeFlag()
        self.pin.irq(handler=lambda _: self.flag.set(), trigger=trigger)

    def wait(self):
        self.flag.clear()
        uasyncio.run(self.flag.wait())

def button(action, number, on_state=0, pull=None, debounce=20):

    state = 1-on_state
    t_prev = utime.ticks_ms()-21
    pin = machine.Pin(number, machine.Pin.IN, pull)

    def button_irq(pin):
        nonlocal state, t_prev
        t = utime.ticks_ms()
        if utime.ticks_diff(t, t_prev) > debounce:
            new_state = pin()
            if state != new_state:
                state = new_state
                t_prev = t
                if state == on_state:
                    action()

    pin.irq(handler=button_irq, trigger=machine.Pin.IRQ_FALLING | machine.Pin.IRQ_RISING)
