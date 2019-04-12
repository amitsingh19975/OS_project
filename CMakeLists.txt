# Disable in-source builds to prevent source tree corruption.
cmake_minimum_required(VERSION 3.0)

if(" ${CMAKE_SOURCE_DIR}" STREQUAL " ${CMAKE_BINARY_DIR}")
message(FATAL_ERROR "
FATAL: In-source builds are not allowed.
You should create a separate directory for build files.
")
endif()


add_subdirectory(client_side)
add_subdirectory(server_side)
