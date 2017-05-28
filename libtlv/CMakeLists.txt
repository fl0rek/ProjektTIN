cmake_minimum_required(VERSION 3.1)

include_directories(include)
file(GLOB SOURCES "src/*.cpp")

add_compile_options(-std=c++14)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)

add_library(tlv STATIC src/Tlv.cpp)

add_executable(TlvTests ${SOURCES})

enable_testing()
target_link_libraries(TlvTests ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
add_test(TlvTests TlvTests)
