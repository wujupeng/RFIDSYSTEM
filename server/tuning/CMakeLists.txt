set(TUNING_SRCS
    auto_tuner.cpp
    reward_evaluator.cpp
    tuning_strategy.cpp
    parameter_space.cpp
)

set(TUNING_BAYES_SRCS
    bayes/gaussian_process.cpp
    bayes/surrogate_model.cpp
    bayes/acquisition_function.cpp
    bayes/bayesian_optimizer.cpp
)

add_library(tuning ${TUNING_SRCS} ${TUNING_BAYES_SRCS})

target_include_directories(tuning PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/bayes
    ${CMAKE_SOURCE_DIR}/server/core
    ${CMAKE_SOURCE_DIR}/server/db
    ${CMAKE_SOURCE_DIR}/server/models
    ${CMAKE_SOURCE_DIR}/server/repository
    ${CMAKE_SOURCE_DIR}/server/analytics
)

target_link_libraries(tuning
    analytics
    db
    core
    ${PQXX_LIBRARIES}
    spdlog::spdlog
)