
set(MODULE_SRCS
${MODULE_DIR}/esphttpd.c
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
        MICROPY_MODULE_DEFS "-DMODULE_ESPHTTPD_ENABLED=1"
)

message(status "esphttpd module called")
