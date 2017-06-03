cmake_minimum_required(VERSION 3.1.0)
include_directories(include ..)
file(GLOB SOURCES "src/*.cpp" "../libnet/*.cpp")
add_definitions(-std=c++14)

add_executable(client src/ClientAPP.cpp src/Client.cpp)

set (CMAKE_C_FLAGS "-std=c++14 -ggdb -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-declarations -Wmissing-include-dirs -Wredundant-decls -Wsign-conversion -Wstrict-overflow=5 -Wswitch-default -Wundef -Wno-error=unused-parameter -trigraphs -Wno-trigraphs -Wno-error=pedantic")

target_link_libraries(client net_client tlv tags)

install(TARGETS client DESTINATION bin)

