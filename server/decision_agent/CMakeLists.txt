add_library(decision_agent
    decision_agent.cpp
    policy_mixer.cpp
    guardrail.cpp
)

target_link_libraries(decision_agent
    core
    spdlog::spdlog
)
