
set(MODULE_SRCS
${MODULE_DIR}/cball.c
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
        MICROPY_MODULE_DEFS "-DMODULE_CBALL_ENABLED=1"
)

message(status "cball module called")
