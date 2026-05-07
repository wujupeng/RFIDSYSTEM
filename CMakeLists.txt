# CMakeLists.txt for RFID Asset Management Server v2.3

cmake_minimum_required(VERSION 3.16)

project(rfid_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Protobuf/gRPC paths
set(Protobuf_DIR "/usr/lib/x86_64-linux-gnu/cmake/protobuf")
set(gRPC_DIR "/usr/lib/x86_64-linux-gnu/cmake/grpc")

# Find dependencies
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(spdlog REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)
pkg_check_modules(CURL REQUIRED libcurl)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/server
    ${CMAKE_SOURCE_DIR}/proto
    ${Protobuf_INCLUDE_DIRS}
    ${gRPC_INCLUDE_DIRS}
    ${PQXX_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${CURL_INCLUDE_DIRS}
)

# Proto files
set(PROTO_FILES
    ${CMAKE_SOURCE_DIR}/proto/asset.proto
    ${CMAKE_SOURCE_DIR}/proto/decision.proto
)

# Protoc and grpc plugin paths
set(PROTOC_EXECUTABLE "/usr/bin/protoc")
set(GRPC_CPP_PLUGIN_EXECUTABLE "/usr/bin/grpc_cpp_plugin")

# Generate asset.proto
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/asset.pb.cc ${CMAKE_CURRENT_BINARY_DIR}/asset.pb.h
    COMMAND ${PROTOC_EXECUTABLE}
    ARGS --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
         -I${CMAKE_SOURCE_DIR}/proto
         ${CMAKE_SOURCE_DIR}/proto/asset.proto
    DEPENDS ${CMAKE_SOURCE_DIR}/proto/asset.proto
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/asset.grpc.pb.cc ${CMAKE_CURRENT_BINARY_DIR}/asset.grpc.pb.h
    COMMAND ${PROTOC_EXECUTABLE}
    ARGS --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
         --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
         -I${CMAKE_SOURCE_DIR}/proto
         --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXECUTABLE}
         ${CMAKE_SOURCE_DIR}/proto/asset.proto
    DEPENDS ${CMAKE_SOURCE_DIR}/proto/asset.proto
)

# Generate decision.proto
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/decision.pb.cc ${CMAKE_CURRENT_BINARY_DIR}/decision.pb.h
    COMMAND ${PROTOC_EXECUTABLE}
    ARGS --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
         -I${CMAKE_SOURCE_DIR}/proto
         ${CMAKE_SOURCE_DIR}/proto/decision.proto
    DEPENDS ${CMAKE_SOURCE_DIR}/proto/decision.proto
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/decision.grpc.pb.cc ${CMAKE_CURRENT_BINARY_DIR}/decision.grpc.pb.h
    COMMAND ${PROTOC_EXECUTABLE}
    ARGS --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
         --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
         -I${CMAKE_SOURCE_DIR}/proto
         --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXECUTABLE}
         ${CMAKE_SOURCE_DIR}/proto/decision.proto
    DEPENDS ${CMAKE_SOURCE_DIR}/proto/decision.proto
)

# Core library
set(CORE_SRCS
    server/core/logger.cpp
    server/core/epc_buffer.cpp
    server/core/rate_limiter.cpp
    server/core/metrics.cpp
    server/core/alert.cpp
    server/core/epc_validator.cpp
    server/core/idempotency_manager.cpp
)

set(CORE_HDRS
    server/core/logger.h
    server/core/epc_buffer.h
    server/core/rate_limiter.h
    server/core/metrics.h
    server/core/alert.h
    server/core/epc_validator.h
    server/core/idempotency_manager.h
)

add_library(core ${CORE_SRCS} ${CORE_HDRS})
target_link_libraries(core spdlog::spdlog pthread)

# Database library
set(DB_SRCS
    server/db/db_pool.cpp
)

set(DB_HDRS
    server/db/db_pool.h
)

add_library(db ${DB_SRCS} ${DB_HDRS})
target_link_libraries(db ${PQXX_LIBRARIES} core)

# Analytics library - 预测层、评分层、决策层
set(ANALYTICS_SRCS
    server/analytics/trajectory_engine.cpp
    server/analytics/behavior_analyzer.cpp
    server/analytics/anomaly_detector.cpp
    server/analytics/predictor/asset_predictor.cpp
    server/analytics/scoring/asset_score.cpp
    server/analytics/decision/decision_engine.cpp
    server/analytics/decision/action_generator.cpp
    server/analytics/decision/decision_stability.cpp
    server/analytics/decision/rule_impact_analyzer.cpp
)

set(ANALYTICS_HDRS
    server/analytics/trajectory_engine.h
    server/analytics/behavior_analyzer.h
    server/analytics/anomaly_detector.h
    server/analytics/predictor/asset_predictor.h
    server/analytics/scoring/asset_score.h
    server/analytics/decision/decision_engine.h
    server/analytics/decision/action_generator.h
    server/analytics/decision/decision_stability.h
    server/analytics/decision/rule_impact_analyzer.h
)

add_library(analytics ${ANALYTICS_SRCS} ${ANALYTICS_HDRS})
target_include_directories(analytics PUBLIC
    ${CMAKE_SOURCE_DIR}/server/analytics
    ${CMAKE_SOURCE_DIR}/server/repository
)
target_link_libraries(analytics db core ${PQXX_LIBRARIES} spdlog::spdlog)

# Tuning library - Auto-Tuning 自动调参系统
add_subdirectory(server/tuning)

# Server source files
set(SERVER_SRCS
    server/main.cpp
    server/models/asset.cpp
    server/models/inventory.cpp
    server/repository/decision_repository.cpp
    server/repository/rule_registry.cpp
    server/services/asset_service.cpp
    server/services/inventory_service.cpp
    server/services/auth_service.cpp
    server/services/audit_service.cpp
    server/services/reconciliation_service.cpp
    server/rpc/asset_service.cpp
    server/rpc/decision_service.cpp
    server/rfid/reader.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/asset.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/asset.grpc.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/decision.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/decision.grpc.pb.cc
)

# Server header files
set(SERVER_HDRS
    server/models/asset.h
    server/models/inventory.h
    server/services/asset_service.h
    server/services/inventory_service.h
    server/services/auth_service.h
    server/services/audit_service.h
    server/services/reconciliation_service.h
    server/rpc/asset_service.h
    server/rpc/decision_service.h
    server/rfid/reader.h
)

# Add server executable
add_executable(server ${SERVER_SRCS} ${SERVER_HDRS})

# Link libraries
target_link_libraries(server
    tuning
    analytics
    db
    core
    ${Protobuf_LIBRARIES}
    ${gRPC_LIBRARIES}
    ${PQXX_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${CURL_LIBRARIES}
    spdlog::spdlog
    pthread
)

# Tools
add_subdirectory(tools)

# Client
add_subdirectory(client)