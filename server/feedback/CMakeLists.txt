add_library(feedback
    feedback_collector.cpp
)

target_link_libraries(feedback
    core
    db
    spdlog::spdlog
)
