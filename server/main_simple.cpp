#include "core/logger.h"
#include "db/db_pool.h"
#include "analytics/decision/decision_engine.h"
#include <iostream>
#include <csignal>
#include <thread>
#include <chrono>

namespace {
    volatile std::sig_atomic_t g_running = 1;
}

void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        g_running = 0;
    }
}

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    Logger::init("rfid_server");
    spdlog::info("Starting RFID Server (Simplified Version)...");

    try {
        DBPool& dbPool = DBPool::instance();
        spdlog::info("Database pool initialized");

        analytics::DecisionEngine& engine = analytics::DecisionEngine::instance();
        spdlog::info("Decision Engine initialized");

        spdlog::info("RFID Server is running in SHADOW mode");
        spdlog::info("Server ready on 0.0.0.0:50051 (gRPC disabled)");

        while (g_running) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            spdlog::debug("Server heartbeat...");
        }

        spdlog::info("Server shutdown initiated...");

    } catch (const std::exception& e) {
        spdlog::error("Server error: {}", e.what());
        return 1;
    }

    Logger::shutdown();
    return 0;
}
