cmake_minimum_required (VERSION 2.6)
project (rdwipe)
add_executable(rdwipe rdwipe.c)

include(CheckCCompilerFlag)
CHECK_C_COMPILER_FLAG("-std=c99" COMPILER_SUPPORTS_C99)
if(COMPILER_SUPPORTS_C99)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C99 support. Please use a different C compiler.")
endif()

target_link_libraries(rdwipe pthread)
