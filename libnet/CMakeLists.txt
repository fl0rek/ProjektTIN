cmake_minimum_required(VERSION 3.1.0)

add_library(net_client STATIC client.c common.c)
add_library(net_server STATIC server.c common.c)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_include_directories(net_client PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
	)

target_include_directories(net_server PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
	)

set(CLIENT_HEADER_FILES, include/client.h common.h include/fdebug.h)
set(SERVER_HEADER_FILES, include/server.h common.h include/fdebug.h)
set (CMAKE_C_FLAGS "-std=c11 -ggdb -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-declarations -Wmissing-include-dirs -Wredundant-decls -Wsign-conversion -Wstrict-overflow=5 -Wswitch-default -Wundef -Wno-error=unused-parameter -trigraphs -Wno-trigraphs -Wno-error=pedantic")

target_link_libraries(net_client "${CLIENT_HEADER_FILES}" Threads::Threads)
target_link_libraries(net_server "${SERVER_HEADER_FILES}" Threads::Threads)
