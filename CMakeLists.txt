cmake_minimum_required(VERSION 3.20)
project(Apriorit)

set(CMAKE_CXX_STANDARD 20)

add_executable(Apriorit
        CreateFileGuard.h
        main.cpp
        MySmartPointers.h)

target_link_libraries(Apriorit -lws2_32 -lwsock32)
