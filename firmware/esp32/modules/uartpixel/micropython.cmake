
add_library(uartpixel INTERFACE)

target_sources(uartpixel INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/uartpixel.c
)

target_link_libraries(usermod INTERFACE uartpixel)

add_compile_definitions(
    MODULE_UARTPIXEL_ENABLED=1
)

message(status "uartpixel module called")

