#include "logger.h"
#include <vector>

std::shared_ptr<spdlog::logger> Logger::logger_;

void Logger::init(const std::string& appName) {
    std::vector<spdlog::sink_ptr> sinks;

    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_level(spdlog::level::info);
    consoleSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v");

    auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        "logs/" + appName + ".log",
        1024 * 1024 * 10,
        3
    );
    fileSink->set_level(spdlog::level::debug);
    fileSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] [%t] %v");

    sinks.push_back(consoleSink);
    sinks.push_back(fileSink);

    logger_ = std::make_shared<spdlog::logger>(appName, begin(sinks), end(sinks));
    logger_->set_level(spdlog::level::debug);
    logger_->flush_on(spdlog::level::info);

    spdlog::register_logger(logger_);

    spdlog::info("Logger initialized for {}", appName);
}

void Logger::shutdown() {
    if (logger_) {
        logger_->flush();
    }
    spdlog::drop_all();
}

spdlog::logger& Logger::getLogger() {
    return *logger_;
}