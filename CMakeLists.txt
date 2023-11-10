cmake_minimum_required(VERSION 3.10)
project(zeromq_study)

set(CMAKE_CXX_STANDARD 17)

add_compile_options("-Wno-unused-result" "-Wno-deprecated-declarations" "-O0" "-ggdb")

message(STATUS ${PROJECT_SOURCE_DIR})

include_directories(
    ${PROJECT_SOURCE_DIR}/../install/libzmq/include
    ${PROJECT_SOURCE_DIR}/../install/cppzmq/include)

link_directories(
    ${PROJECT_SOURCE_DIR}/../install/libzmq//lib
)

add_subdirectory(tcp_client)
add_subdirectory(tcp_server)
add_subdirectory(linux_server)






