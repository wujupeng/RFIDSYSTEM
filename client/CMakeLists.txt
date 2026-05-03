cmake_minimum_required(VERSION 3.16)
project(rfid_client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(gRPC REQUIRED)

set(PROTO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../proto/asset.proto)

protobuf_generate(TARGET client PROTO_SRC_FILES ${PROTO_FILES})
grpc_generate(TARGET client PROTO_SRC_FILES ${PROTO_FILES})

set(CLIENT_SOURCES
    main.cpp
    pages/mainwindow.cpp
    pages/dashboard/dashboard_page.cpp
    pages/asset/asset_page.cpp
    pages/inventory/inventory_page.cpp
    pages/repair/repair_page.cpp
    network/grpc_client.cpp
)

add_executable(client ${CLIENT_SOURCES})

target_link_libraries(client
    Qt6::Widgets
    gRPC::grpc++
    protobuf::libprotobuf
)

set_target_properties(client PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)