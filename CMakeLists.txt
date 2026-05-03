# CMakeLists.txt for RFID Asset Management Server v1.4

cmake_minimum_required(VERSION 3.16)
project(rfid_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)
pkg_check_modules(OPENSSL REQUIRED openssl)
pkg_check_modules(CURL REQUIRED libcurl)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/server
    ${CMAKE_SOURCE_DIR}/proto
    ${Protobuf_INCLUDE_DIRS}
    ${gRPC_INCLUDE_DIRS}
    ${PQXX_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
)

# Proto files
set(PROTO_FILES ${CMAKE_SOURCE_DIR}/proto/asset.proto)

# Generate protobuf and gRPC files
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})
protobuf_generate_cpp(GRPC_SRCS GRPC_HDRS ${PROTO_FILES})

# gRPC plugin
get_target_property(gRPC_CPP_PLUGIN_EXECUTABLE gRPC::grpc_cpp_plugin LOCATION)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/asset.grpc.pb.cc ${CMAKE_CURRENT_BINARY_DIR}/asset.grpc.pb.h
    COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
    ARGS --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
         --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
         -I${CMAKE_SOURCE_DIR}/proto
         --plugin=protoc-gen-grpc=${gRPC_CPP_PLUGIN_EXECUTABLE}
         ${PROTO_FILES}
    DEPENDS ${PROTO_FILES}
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

# Analytics library
set(ANALYTICS_SRCS
    server/analytics/trajectory_engine.cpp
    server/analytics/behavior_analyzer.cpp
    server/analytics/anomaly_detector.cpp
)

set(ANALYTICS_HDRS
    server/analytics/trajectory_engine.h
    server/analytics/behavior_analyzer.h
    server/analytics/anomaly_detector.h
)

add_library(analytics ${ANALYTICS_SRCS} ${ANALYTICS_HDRS})
target_link_libraries(analytics db core)

# Server source files
set(SERVER_SRCS
    server/main.cpp
    server/models/asset.cpp
    server/models/inventory.cpp
    server/services/asset_service.cpp
    server/services/inventory_service.cpp
    server/services/auth_service.cpp
    server/services/audit_service.cpp
    server/services/reconciliation_service.cpp
    server/rpc/asset_service.cpp
    server/rfid/reader.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/asset.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/asset.grpc.pb.cc
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
    server/rfid/reader.h
)

# Add server executable
add_executable(server ${SERVER_SRCS} ${SERVER_HDRS})

# Link libraries
target_link_libraries(server
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