cmake_minimum_required(VERSION 3.16)

# Eigen3 - Use system installation at /usr/local/include/eigen
set(EIGEN3_INCLUDE_DIR "/usr/local/include/eigen")

# Project include paths
include_directories(
    ${EIGEN3_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/server
)

# Load Test
add_executable(load_test load_test.cpp)
target_link_libraries(load_test pthread)

# Decision Sandbox
add_executable(decision_sandbox decision_sandbox.cpp)
target_link_libraries(decision_sandbox
    analytics
    db
    core
    spdlog::spdlog
    pthread
)

# Bandit Performance Test (v3.2) - Optional (file might be missing)
# add_executable(bandit_test bandit_test.cpp)
# target_link_libraries(bandit_test pthread)
# if(Eigen3_FOUND)
#     target_include_directories(bandit_test PRIVATE ${EIGEN3_INCLUDE_DIRS})
# else()
#     target_include_directories(bandit_test PRIVATE /usr/local/include/eigen3)
# endif()

# Bandit V3.3 Test
add_executable(bandit_v33_test bandit_v33_test.cpp)
target_link_libraries(bandit_v33_test pthread)
if(Eigen3_FOUND)
    target_include_directories(bandit_v33_test PRIVATE ${EIGEN3_INCLUDE_DIRS})
else()
    target_include_directories(bandit_v33_test PRIVATE /usr/local/include/eigen3)
endif()

# Shadow Mode Test (Phase 1) - 独立测试，不依赖数据库
add_executable(shadow_test shadow_test.cpp)
target_link_libraries(shadow_test pthread)
if(Eigen3_FOUND)
    target_include_directories(shadow_test PRIVATE ${EIGEN3_INCLUDE_DIRS})
else()
    target_include_directories(shadow_test PRIVATE /usr/local/include/eigen3)
endif()

# Install
install(TARGETS load_test decision_sandbox bandit_v33_test shadow_test DESTINATION bin)