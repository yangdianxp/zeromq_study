add_executable(tcp_server main.cpp)
target_include_directories(tcp_server PRIVATE .)
target_link_libraries(tcp_server PRIVATE libzmq.a pthread)








