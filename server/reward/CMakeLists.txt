add_library(reward
    reward_engine.cpp
)

target_link_libraries(reward
    core
    spdlog::spdlog
)
