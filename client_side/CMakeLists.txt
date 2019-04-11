# Disable in-source builds to prevent source tree corruption.
cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project (ChatApp C)

find_package(fmt REQUIRED)

include_directories (include/client_utility.cpp)
add_library (utility SHARED include/client_utility.cpp)

set (CLIENT_SOURCE client.cpp)

add_executable (client ${CLIENT_SOURCE})

target_link_libraries (client utility pthread)
target_link_libraries (client PRIVATE fmt-header-only)
