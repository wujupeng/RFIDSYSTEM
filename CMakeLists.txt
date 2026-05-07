# CMakeLists.txt for RFID Asset Management Server v3.3 - Debian Compatible

cmake_minimum_required(VERSION 3.16)

project(rfid_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add fmt path for CMake
list(APPEND CMAKE_MODULE_PATH "/usr/lib/x86_64-linux-gnu/cmake/fmt")

# Find dependencies
find_package(Fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)
pkg_check_modules(CURL REQUIRED libcurl)
find_package(Eigen3 REQUIRED NO_MODULE)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/server
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PQXX_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${CURL_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIR}
    ${Fmt_INCLUDE_DIRS}
)

# Source files
file(GLOB_RECURSE CORE_SRC "server/core/*.cpp")
file(GLOB_RECURSE DB_SRC "server/db/*.cpp")
file(GLOB_RECURSE ANALYTICS_SRC "server/analytics/*.cpp")
file(GLOB_RECURSE MODELS_SRC "server/models/*.cpp")
file(GLOB_RECURSE REPOSITORY_SRC "server/repository/*.cpp")

# v3.3 Decision Agent modules
file(GLOB_RECURSE DECISION_AGENT_SRC "server/decision_agent/*.cpp")
file(GLOB_RECURSE FEEDBACK_SRC "server/feedback/*.cpp")
file(GLOB_RECURSE REWARD_SRC "server/reward/*.cpp")

# Pre-compiled protobuf files (already on server)
set(PROTO_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/asset.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/asset.grpc.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/decision.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/decision.grpc.pb.cc
)

# Libraries
add_library(core ${CORE_SRC})
add_library(db ${DB_SRC})
add_library(analytics ${ANALYTICS_SRC})
add_library(models ${MODELS_SRC})
add_library(repository ${REPOSITORY_SRC})
add_library(decision_agent ${DECISION_AGENT_SRC})
add_library(feedback ${FEEDBACK_SRC})
add_library(reward ${REWARD_SRC})

# Link libraries
target_link_libraries(db PRIVATE core ${PQXX_LIBRARIES})
target_link_libraries(analytics PRIVATE core models repository db)
target_link_libraries(decision_agent PRIVATE core)
target_link_libraries(feedback PRIVATE core db)
target_link_libraries(reward PRIVATE core)
target_link_libraries(core PRIVATE spdlog::spdlog)

# Executable - use simplified main without gRPC
add_executable(server server/main_simple.cpp)
target_link_libraries(server
    decision_agent
    feedback
    reward
    spdlog::spdlog
    analytics
    models
    repository
    db
    core
    ${OPENSSL_LIBRARIES}
    ${CURL_LIBRARIES}
    ${Fmt_LIBRARIES}
    pthread
)

# Install
install(TARGETS server DESTINATION bin)
