
cmake_minimum_required(VERSION 3.10)


project(radixSA64
        VERSION 1.0
        LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(radixSA
               src/runtimes.cpp
               src/radix.h
               src/utils.h
               src/RadixLSDCache.h
               src/MergeSorter.h)


