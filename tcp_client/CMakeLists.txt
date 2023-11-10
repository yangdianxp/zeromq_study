add_executable(tcp_client main.cpp)
target_include_directories(tcp_client PRIVATE .)
target_link_libraries(tcp_client PRIVATE libzmq.a pthread)








