cmake_minimum_required(VERSION 3.16)
project(rfid_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SPDLOG REQUIRED spdlog)
pkg_check_modules(PQXX REQUIRED libpqxx)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PQXX_INCLUDE_DIRS}
    ${SPDLOG_INCLUDE_DIRS}
)

set(PROTO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../proto/asset.proto)

protobuf_generate(TARGET server PROTO_SRC_FILES ${PROTO_FILES})
grpc_generate(TARGET server PROTO_SRC_FILES ${PROTO_FILES})

add_executable(server
    main.cpp
    core/logger.cpp
    db/db_pool.cpp
    models/asset.cpp
    services/asset_service.cpp
    services/asset_code_generator.cpp
    rpc/asset_service.cpp
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)

target_link_libraries(server
    gRPC::grpc++
    protobuf::libprotobuf
    ${PQXX_LIBRARIES}
    ${SPDLOG_LIBRARIES}
    pthread
)

target_compile_definitions(server PRIVATE SPDLOG_ERR_)