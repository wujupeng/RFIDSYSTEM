cmake_minimum_required(VERSION 3.16)
project(rfid_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(nlohmann_json REQUIRED)
pkg_check_modules(SPDLOG REQUIRED spdlog)
pkg_check_modules(PQXX REQUIRED libpqxx)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PQXX_INCLUDE_DIRS}
    ${SPDLOG_INCLUDE_DIRS}
)

set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../proto)

# Generate protobuf and grpc code for asset.proto
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/asset.pb.cc ${CMAKE_CURRENT_BINARY_DIR}/asset.pb.h
           ${CMAKE_CURRENT_BINARY_DIR}/asset.grpc.pb.cc ${CMAKE_CURRENT_BINARY_DIR}/asset.grpc.pb.h
    COMMAND protobuf::protoc
    ARGS --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
         --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
         --plugin=protoc-gen-grpc=/usr/bin/grpc_cpp_plugin
         -I${PROTO_DIR}
         ${PROTO_DIR}/asset.proto
    DEPENDS ${PROTO_DIR}/asset.proto
)

# Generate monitoring.proto
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/monitoring.pb.cc ${CMAKE_CURRENT_BINARY_DIR}/monitoring.pb.h
           ${CMAKE_CURRENT_BINARY_DIR}/monitoring.grpc.pb.cc ${CMAKE_CURRENT_BINARY_DIR}/monitoring.grpc.pb.h
    COMMAND protobuf::protoc
    ARGS --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
         --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
         --plugin=protoc-gen-grpc=/usr/bin/grpc_cpp_plugin
         -I${PROTO_DIR}
         ${PROTO_DIR}/monitoring.proto
    DEPENDS ${PROTO_DIR}/monitoring.proto
)

# Generate admin.proto
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/admin.pb.cc ${CMAKE_CURRENT_BINARY_DIR}/admin.pb.h
           ${CMAKE_CURRENT_BINARY_DIR}/admin.grpc.pb.cc ${CMAKE_CURRENT_BINARY_DIR}/admin.grpc.pb.h
    COMMAND protobuf::protoc
    ARGS --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
         --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
         --plugin=protoc-gen-grpc=/usr/bin/grpc_cpp_plugin
         -I${PROTO_DIR}
         ${PROTO_DIR}/admin.proto
    DEPENDS ${PROTO_DIR}/admin.proto
)

add_custom_target(proto_gen DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/asset.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/asset.grpc.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/monitoring.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/monitoring.grpc.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/admin.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/admin.grpc.pb.cc
)

# Minimal server executable
add_executable(rfid-server
    main.cpp
    core/logger.cpp
    core/epc_validator.cpp
    core/idempotency_manager.cpp
    db/db_pool.cpp
    models/asset.cpp
    services/asset_service.cpp
    services/asset_code_generator.cpp
    services/inventory_service.cpp
    services/reconciliation_service.cpp
    rpc/asset_service.cpp
    rpc/monitoring_service.cpp
    rpc/admin_service.cpp
    monitoring/health_monitor.cpp
    monitoring/metrics.cpp
    repository/health_repository.cpp
    repository/metrics_repository.cpp
    repository/bandit_repository.cpp
    repository/decision_feedback_repository.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/asset.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/asset.grpc.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/monitoring.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/monitoring.grpc.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/admin.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/admin.grpc.pb.cc
)

add_dependencies(rfid-server proto_gen)

target_link_libraries(rfid-server
    gRPC::grpc++
    protobuf::libprotobuf
    nlohmann_json::nlohmann_json
    ${PQXX_LIBRARIES}
    ${SPDLOG_LIBRARIES}
    pthread
)

target_compile_definitions(rfid-server PRIVATE SPDLOG_FMT_EXTERNAL)
