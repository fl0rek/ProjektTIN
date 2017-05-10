cmake_minimum_required(VERSION 2.8.0)
find_package(Threads REQUIRED)
add_library(net STATIC server.c)

include_directories(include)

set(HEADER_FILES, include/server.h include/fdebug.h)
set (CMAKE_C_FLAGS "-std=c11 -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wredundant-decls -Wsign-conversion -Wstrict-overflow=5 -Wswitch-default -Wundef -Werror -Wno-error=unused-parameter -trigraphs -Wno-trigraphs")


target_link_libraries(net Threads ${HEADER_FILES})
