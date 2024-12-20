cmake_minimum_required(VERSION 3.28.3)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

#project name
project (code_cmake)

# Set the target architecture
set(CMAKE_SYSTEM_NAME Linux)
#set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
add_executable(indexed.out indexed.c )
add_executable(inode.out inode.c )
add_executable(linked.out linked.c)
add_executable(linked-fat.out linked-fat.c)
add_executable(sequential.out sequential.c)
