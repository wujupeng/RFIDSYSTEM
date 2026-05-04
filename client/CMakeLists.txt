cmake_minimum_required(VERSION 3.16)
project(rfid_client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Protobuf/GraPC paths
set(Protobuf_DIR "/usr/local/opt/protobuf@34/lib/cmake/protobuf")
set(gRPC_DIR "/usr/local/opt/grpc/lib/cmake/grpc")

find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(Protobuf REQUIRED CONFIG)
find_package(gRPC REQUIRED CONFIG)

# Protoc and grpc plugin paths
set(PROTOC_EXECUTABLE "/usr/local/opt/protobuf@34/bin/protoc")
set(GRPC_CPP_PLUGIN "/usr/local/opt/grpc/bin/grpc_cpp_plugin")

# Generate asset.proto
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/asset.pb.cc ${CMAKE_CURRENT_BINARY_DIR}/asset.pb.h
    COMMAND ${PROTOC_EXECUTABLE}
    ARGS --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
         -I${CMAKE_CURRENT_SOURCE_DIR}/../proto
         ${CMAKE_CURRENT_SOURCE_DIR}/../proto/asset.proto
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../proto/asset.proto
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/asset.grpc.pb.cc ${CMAKE_CURRENT_BINARY_DIR}/asset.grpc.pb.h
    COMMAND ${PROTOC_EXECUTABLE}
    ARGS --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
         --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
         -I${CMAKE_CURRENT_SOURCE_DIR}/../proto
         --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
         ${CMAKE_CURRENT_SOURCE_DIR}/../proto/asset.proto
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../proto/asset.proto
)

# Generate decision.proto
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/decision.pb.cc ${CMAKE_CURRENT_BINARY_DIR}/decision.pb.h
    COMMAND ${PROTOC_EXECUTABLE}
    ARGS --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
         -I${CMAKE_CURRENT_SOURCE_DIR}/../proto
         ${CMAKE_CURRENT_SOURCE_DIR}/../proto/decision.proto
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../proto/decision.proto
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/decision.grpc.pb.cc ${CMAKE_CURRENT_BINARY_DIR}/decision.grpc.pb.h
    COMMAND ${PROTOC_EXECUTABLE}
    ARGS --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
         --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
         -I${CMAKE_CURRENT_SOURCE_DIR}/../proto
         --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
         ${CMAKE_CURRENT_SOURCE_DIR}/../proto/decision.proto
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../proto/decision.proto
)

set(CLIENT_SOURCES
    main.cpp
    pages/mainwindow.cpp
    pages/dashboard/dashboard_page.cpp
    pages/asset/asset_page.cpp
    pages/inventory/inventory_page.cpp
    pages/repair/repair_page.cpp
    pages/recommendations/system_recommendations_page.cpp
    network/grpc_client.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/asset.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/asset.grpc.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/decision.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/decision.grpc.pb.cc
)

add_executable(client ${CLIENT_SOURCES})

target_include_directories(client PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(client
    Qt6::Widgets
    gRPC::grpc++
    protobuf::libprotobuf
)
