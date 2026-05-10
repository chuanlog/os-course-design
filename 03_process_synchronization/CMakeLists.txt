cmake_minimum_required(VERSION 3.10)
project(process_synchronization LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(process_synchronization main.cpp)
find_package(Threads REQUIRED)
target_link_libraries(process_synchronization PRIVATE Threads::Threads)
