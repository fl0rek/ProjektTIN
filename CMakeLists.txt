cmake_minimum_required(VERSION 3.1.0)

project(tintin)

set(CMAKE_C_FLAGS "-std=c11 -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-declarations -Wmissing-include-dirs -Wredundant-decls -Wsign-conversion -Wstrict-overflow=5 -Wswitch-default -Wundef -Werror -Wno-error=unused-parameter -trigraphs -Wno-trigraphs -D_POSIX_C_SOURCE")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_subdirectory(libnet)
add_subdirectory(ServerAPP)
add_subdirectory(libtlv)
