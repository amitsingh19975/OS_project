# Disable in-source builds to prevent source tree corruption.
cmake_minimum_required(VERSION 3.0)

if(" ${CMAKE_SOURCE_DIR}" STREQUAL " ${CMAKE_BINARY_DIR}")
message(FATAL_ERROR "
FATAL: In-source builds are not allowed.
You should create a separate directory for build files.
")
endif()


project (ChatApp C)

add_subdirectory(client_side)

# set (EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
# set (LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
# include_directories (lib/include)
# add_library (utility SHARED lib/utility.c)

# set (SERVER_SOURCE server.c)

# add_executable (server ${SERVER_SOURCE})

# target_link_libraries (server utility pthread)
# target_link_libraries (client utility pthread)
# target_link_libraries (client PRIVATE fmt-header-only)
# # Copy users to build
# file (COPY ${CMAKE_SOURCE_DIR}/users DESTINATION ${CMAKE_BINARY_DIR})