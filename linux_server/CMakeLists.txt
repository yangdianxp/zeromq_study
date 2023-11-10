add_executable(linux_server main.cpp)
target_include_directories(linux_server PRIVATE .)
target_link_libraries(linux_server PRIVATE pthread)








