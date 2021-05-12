
set(MODULE_SRCS
${MODULE_DIR}/uartpixel.c
)

set_property(
    GLOBAL
    APPEND
    PROPERTY
        MICROPY_MODULE_SRCS ${MODULE_SRCS}
)

set_property(
    GLOBAL
    APPEND
    PROPERTY
        MICROPY_MODULE_DEFS "-DMODULE_UARTPIXEL_ENABLED=1"
)

message(status "uartpixel module called")
