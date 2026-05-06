cmake_minimum_required(VERSION 3.16)
project(rfid_client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Charts)
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../proto)

# Generate asset.proto
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

# Generate decision.proto
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/decision.pb.cc ${CMAKE_CURRENT_BINARY_DIR}/decision.pb.h
           ${CMAKE_CURRENT_BINARY_DIR}/decision.grpc.pb.cc ${CMAKE_CURRENT_BINARY_DIR}/decision.grpc.pb.h
    COMMAND protobuf::protoc
    ARGS --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
         --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
         --plugin=protoc-gen-grpc=/usr/bin/grpc_cpp_plugin
         -I${PROTO_DIR}
         ${PROTO_DIR}/decision.proto
    DEPENDS ${PROTO_DIR}/decision.proto
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
    ${CMAKE_CURRENT_BINARY_DIR}/decision.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/decision.grpc.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/monitoring.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/monitoring.grpc.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/admin.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/admin.grpc.pb.cc
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(SOURCES
    main.cpp
    pages/mainwindow.cpp
    pages/dashboard/dashboard_page.cpp
    pages/asset/asset_page.cpp
    pages/purchase/purchase_page.cpp
    pages/allocation/allocation_page.cpp
    pages/inventory/inventory_page.cpp
    pages/inventory/inventory_scan_page.cpp
    pages/repair/repair_page.cpp
    pages/inspection/inspection_page.cpp
    pages/recommendations/system_recommendations_page.cpp
    pages/monitoring/monitoring_page.cpp
    pages/admin/user_management_page.cpp
    pages/admin/role_management_page.cpp
    pages/admin/system_settings_page.cpp
    network/grpc_client.cpp
    network/monitoring_client.cpp
    network/admin_client.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/asset.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/asset.grpc.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/decision.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/decision.grpc.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/monitoring.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/monitoring.grpc.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/admin.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/admin.grpc.pb.cc
)

set(HEADERS
    pages/mainwindow.h
    pages/dashboard/dashboard_page.h
    pages/asset/asset_page.h
    pages/purchase/purchase_page.h
    pages/allocation/allocation_page.h
    pages/inventory/inventory_page.h
    pages/inventory/inventory_scan_page.h
    pages/repair/repair_page.h
    pages/inspection/inspection_page.h
    pages/recommendations/system_recommendations_page.h
    pages/monitoring/monitoring_page.h
    pages/admin/user_management_page.h
    pages/admin/role_management_page.h
    pages/admin/system_settings_page.h
    network/grpc_client.h
    network/monitoring_client.h
    network/admin_client.h
)

add_executable(rfid-client ${SOURCES} ${HEADERS})
add_dependencies(rfid-client proto_gen)

target_link_libraries(rfid-client
    Qt6::Widgets
    Qt6::Charts
    gRPC::grpc++
    protobuf::libprotobuf
    pthread
)
