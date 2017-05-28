cmake_minimum_required(VERSION 3.1.0)

find_package(Threads REQUIRED)

add_executable(server server.cc)

set (CMAKE_C_FLAGS "-std=c11 -ggdb -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-declarations -Wmissing-include-dirs -Wredundant-decls -Wsign-conversion -Wstrict-overflow=5 -Wswitch-default -Wundef -Wno-error=unused-parameter -trigraphs -Wno-trigraphs -Wno-error=pedantic")


#set_target_properties(net_server PROPERTIES
#IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/libnet/libnet_server.a" )

target_link_libraries(server net_server)

install(TARGETS server DESTINATION bin)
