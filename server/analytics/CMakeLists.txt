set(ANALYTICS_SRCS
    trajectory_engine.cpp
    behavior_analyzer.cpp
    anomaly_detector.cpp
)

add_library(analytics ${ANALYTICS_SRCS})

target_include_directories(analytics PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/server/core
    ${CMAKE_SOURCE_DIR}/server/db
    ${CMAKE_SOURCE_DIR}/server/models
)

target_link_libraries(analytics
    core
    db
    ${PQXX_LIBRARIES}
    spdlog::spdlog
)