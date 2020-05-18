cmake_minimum_required(VERSION 3.16)
project(mudpp)

set(CMAKE_CXX_STANDARD 20)

if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
else()
    message(WARNING "The file conanbuildinfo.cmake doesn't exist, you have to run conan install first")
endif()

set(CMAKE_VERBOSE_MAKEFILE ON)

add_executable(mudpp main.cpp app.cpp app.h net.cpp net.h)
#target_link_libraries(mudpp ${CONAN_LIBS})
conan_target_link_libraries(mudpp)