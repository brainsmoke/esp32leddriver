
add_library(cball INTERFACE)

target_sources(cball INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/cball.c
)

target_link_libraries(usermod INTERFACE cball)

add_compile_definitions(
    MODULE_CBALL_ENABLED=1
)

message(status "cball module called")
