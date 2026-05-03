cmake_minimum_required(VERSION 3.16)

add_executable(load_test load_test.cpp)

target_link_libraries(load_test
    pthread
)

install(TARGETS load_test DESTINATION bin)