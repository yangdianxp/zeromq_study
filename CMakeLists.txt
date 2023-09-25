cmake_minimum_required(VERSION 3.10)
project(zeromq_study)

set(CMAKE_CXX_STANDARD 17)

include_directories(
    ${PROJECT_SOURCE_DIR}/../zeromq_install/zeromq-4.1.8/include
    ${PROJECT_SOURCE_DIR}/../zeromq_install/cppzmq-4.10.0)

link_directories(
    ${PROJECT_SOURCE_DIR}/../zeromq_install/zeromq-4.1.8/.libs
)

add_executable(hwclient samp1_1/hwclient.cpp)
add_executable(hwserver samp1_1/hwserver.cpp)
target_link_libraries(hwclient PRIVATE zmq)
target_link_libraries(hwserver PRIVATE zmq)
