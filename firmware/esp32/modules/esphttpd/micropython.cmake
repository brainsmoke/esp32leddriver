
add_library(esphttpd INTERFACE)

target_sources(esphttpd INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/esphttpd.c
)

target_link_libraries(usermod INTERFACE esphttpd)

add_compile_definitions(
    MODULE_ESPHTTPD_ENABLED=1
)

message(status "esphttpd module called")

