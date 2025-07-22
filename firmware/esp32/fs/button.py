
import machine, utime, uasyncio

def button(number, on_down=None, on_up=None, on_state=0, pull=None, debounce=20):

    state = 1-on_state
    t_prev = utime.ticks_ms()-21
    pin = machine.Pin(number, machine.Pin.IN, pull)

    def button_irq(pin):
        nonlocal state, t_prev
        t = utime.ticks_ms()
        if 0 >= utime.ticks_diff(t, t_prev) > debounce:
            new_state = pin()
            if state != new_state:
                state = new_state
                t_prev = t
                if on_down and state == on_state:
                    on_down()
                if on_up and state != on_state:
                    on_up()

    pin.irq(handler=button_irq, trigger=machine.Pin.IRQ_FALLING | machine.Pin.IRQ_RISING)

    return pin


def button_timeout_wait(number, timeout=1000, on_state=0, pull=None):

    flag_up = uasyncio.ThreadSafeFlag()
    flag_down = uasyncio.ThreadSafeFlag()

    def down():
        flag_up.clear()
        flag_down.set()

    def up():
        flag_down.clear()
        flag_up.set()

    pin = button(number, on_down=down, on_up=up, on_state=on_state, pull=pull)

    async def do_timeout():
        try:
            while True:
                await flag_down.wait()
                await uasyncio.wait_for_ms(flag_up.wait(), timeout)
        except uasyncio.TimeoutError:
            pass

    uasyncio.run(do_timeout())
    pin.irq(handler=None)
