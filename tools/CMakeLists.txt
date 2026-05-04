cmake_minimum_required(VERSION 3.16)

add_executable(load_test load_test.cpp)
target_link_libraries(load_test pthread)

# Decision Sandbox - 决策验证工具
add_executable(decision_sandbox decision_sandbox.cpp)
target_link_libraries(decision_sandbox
    analytics
    db
    core
    spdlog::spdlog
    pthread
)

install(TARGETS load_test decision_sandbox DESTINATION bin)