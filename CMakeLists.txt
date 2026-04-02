cmake_minimum_required(VERSION 3.10)
project(Lab2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Lab2 main.cpp)

if(WIN32)
    # No extra libs needed on Windows
else()
    find_package(Threads REQUIRED)
    target_link_libraries(Lab2 Threads::Threads)
endif()
