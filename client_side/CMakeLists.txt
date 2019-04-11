# Disable in-source builds to prevent source tree corruption.
cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project (ChatApp CXX)

set (CLIENT_SOURCE client.cpp)

add_executable (client ${CLIENT_SOURCE})

target_link_libraries (client pthread)

