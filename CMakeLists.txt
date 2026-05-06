# CMakeLists.txt for RFID Asset Management Server v2.3 - Debian Compatible

cmake_minimum_required(VERSION 3.16)

project(rfid_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies
find_package(spdlog REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)
pkg_check_modules(CURL REQUIRED libcurl)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/server
    ${PQXX_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${CURL_INCLUDE_DIRS}
)

# Source files
file(GLOB_RECURSE CORE_SRC "server/core/*.cpp")
file(GLOB_RECURSE DB_SRC "server/db/*.cpp")
file(GLOB_RECURSE ANALYTICS_SRC "server/analytics/*.cpp")
file(GLOB_RECURSE TUNING_SRC "server/tuning/*.cpp")

# Libraries
add_library(core ${CORE_SRC})
add_library(db ${DB_SRC})
add_library(analytics ${ANALYTICS_SRC})
add_library(tuning ${TUNING_SRC})

# Link libraries
target_link_libraries(db PRIVATE core ${PQXX_LIBRARIES})
target_link_libraries(analytics PRIVATE core db)
target_link_libraries(tuning PRIVATE core db analytics)

# Executable
add_executable(server server/main.cpp)
target_link_libraries(server 
    tuning 
    analytics 
    db 
    core 
    ${OPENSSL_LIBRARIES}
    ${CURL_LIBRARIES}
    pthread
)

# Install
install(TARGETS server DESTINATION bin)
