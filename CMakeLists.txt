cmake_minimum_required(VERSION 3.10)
project(zeromq_study)

set(CMAKE_CXX_STANDARD 17)

add_compile_options("-Wno-unused-result" "-Wno-deprecated-declarations")

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/../zeromq_install/zeromq-4.1.8/include
    ${PROJECT_SOURCE_DIR}/../zeromq_install/cppzmq-4.10.0)

link_directories(
    ${PROJECT_SOURCE_DIR}/../zeromq_install/zeromq-4.1.8/.libs
)

add_executable(hwclient example001/hwclient.cpp)
add_executable(hwserver example001/hwserver.cpp)
target_link_libraries(hwclient PRIVATE zmq)
target_link_libraries(hwserver PRIVATE zmq)

add_executable(version example002/version.cpp)
target_link_libraries(version PRIVATE zmq)

add_executable(wuserver example003/wuserver.cpp)
target_link_libraries(wuserver PRIVATE zmq)
add_executable(wuclient example003/wuclient.cpp)
target_link_libraries(wuclient PRIVATE zmq)

add_executable(taskvent example004/taskvent.cpp)
target_link_libraries(taskvent PRIVATE zmq)
add_executable(taskwork example004/taskwork.cpp)
target_link_libraries(taskwork PRIVATE zmq)
add_executable(tasksink example004/tasksink.cpp)
target_link_libraries(tasksink PRIVATE zmq)



