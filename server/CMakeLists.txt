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

# Generate bi.proto
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/bi.pb.cc ${CMAKE_CURRENT_BINARY_DIR}/bi.pb.h
           ${CMAKE_CURRENT_BINARY_DIR}/bi.grpc.pb.cc ${CMAKE_CURRENT_BINARY_DIR}/bi.grpc.pb.h
    COMMAND protobuf::protoc
    ARGS --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
         --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
         --plugin=protoc-gen-grpc=/usr/bin/grpc_cpp_plugin
         -I${PROTO_DIR}
         ${PROTO_DIR}/bi.proto
    DEPENDS ${PROTO_DIR}/bi.proto
)

# Generate trajectory.proto
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/trajectory.pb.cc ${CMAKE_CURRENT_BINARY_DIR}/trajectory.pb.h
           ${CMAKE_CURRENT_BINARY_DIR}/trajectory.grpc.pb.cc ${CMAKE_CURRENT_BINARY_DIR}/trajectory.grpc.pb.h
    COMMAND protobuf::protoc
    ARGS --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
         --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
         --plugin=protoc-gen-grpc=/usr/bin/grpc_cpp_plugin
         -I${PROTO_DIR}
         ${PROTO_DIR}/trajectory.proto
    DEPENDS ${PROTO_DIR}/trajectory.proto
)

add_custom_target(proto_gen DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/asset.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/asset.grpc.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/monitoring.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/monitoring.grpc.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/admin.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/admin.grpc.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/bi.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/bi.grpc.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/trajectory.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/trajectory.grpc.pb.cc
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
    rpc/bi_service.cpp
    rpc/trajectory_service.cpp
    monitoring/health_monitor.cpp
    monitoring/metrics.cpp
    repository/health_repository.cpp
    repository/metrics_repository.cpp
    repository/bandit_repository.cpp
    repository/decision_feedback_repository.cpp
    rfid/reader.cpp
    rfid/reader_config.cpp
    rfid/epc_processor.cpp
    bi/bi_service.cpp
    bi/task_scheduler.cpp
    analytics/trajectory_engine.cpp
    spatial_truth/ground_truth/ground_truth_store.cpp
    spatial_truth/ground_truth/ground_truth_service.cpp
    spatial_truth/calibration_session/accuracy_calculator.cpp
    spatial_truth/calibration_session/calibration_session_manager.cpp
    spatial_truth/spatial_estimate/error_radius_calculator.cpp
    spatial_truth/spatial_estimate/trusted_judge.cpp
    spatial_truth/spatial_estimate/source_hash_calculator.cpp
    spatial_truth/spatial_estimate/spatial_estimate_builder.cpp
    spatial_truth/spatial_estimate/spatial_estimate_projector.cpp
    spatial_truth/coordinate_system/coordinate_system_manager.cpp
    spatial_truth/coordinate_system/coordinate_transformer.cpp
    spatial_truth/spatial_graph/reader_topology_auto_discovery.cpp
    spatial_truth/spatial_graph/tag_asset_binding_sync.cpp
    spatial_truth/spatial_graph/spatial_graph_builder.cpp
    spatial_truth/spatial_graph/spatial_graph_projector.cpp
    spatial_truth/factory_map/heatmap_builder.cpp
    spatial_truth/factory_map/trail_collector.cpp
    spatial_truth/factory_map/node_status_encoder.cpp
    spatial_truth/factory_map/factory_map_generator.cpp
    spatial_truth/truth_gate/truth_gate.cpp
    spatial_truth/truth_gate/truth_gate_async_runner.cpp
    spatial_truth/truth_gate/localization_drift_event_publisher.cpp
    spatial_truth/truth_gate/drift_degradation_chain.cpp
    spatial_truth/truth_gate/spatial_self_healing_loop.cpp
    spatial_benchmark/scenarios/benchmark_scenario_registry.cpp
    spatial_benchmark/runner/scenario_runner.cpp
    spatial_benchmark/report/benchmark_report_generator.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/asset.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/asset.grpc.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/monitoring.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/monitoring.grpc.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/admin.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/admin.grpc.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/bi.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/bi.grpc.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/trajectory.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/trajectory.grpc.pb.cc
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
