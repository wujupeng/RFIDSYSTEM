#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>
#include <string>

class Logger {
public:
    static void init(const std::string& appName = "rfid_server");
    static void shutdown();

    static spdlog::logger& getLogger();

private:
    static std::shared_ptr<spdlog::logger> logger_;
};