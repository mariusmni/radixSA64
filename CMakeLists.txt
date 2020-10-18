
cmake_minimum_required(VERSION 3.16)

project(radixSA64
        VERSION 1.0
        LANGUAGES CXX)

add_executable(radixSA
               src/runtimes.cpp
               src/radix.h
               src/utils.h
               src/RadixLSDCache.h
               src/MergeSorter.h)


