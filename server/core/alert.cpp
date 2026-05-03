#include "alert.h"
#include "logger.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <curl/curl.h>

AlertManager& AlertManager::instance() {
    static AlertManager instance;
    return instance;
}

AlertManager::AlertManager() {
    spdlog::info("AlertManager initialized");
}

void AlertManager::sendAlert(const std::string& type, const std::string& message, AlertLevel level, const std::string& source) {
    Alert alert;
    alert.id = generateAlertId();
    alert.type = type;
    alert.message = message;
    alert.level = level;
    alert.timestamp = getCurrentTimestamp();
    alert.source = source;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        recentAlerts_.push_back(alert);
        if (recentAlerts_.size() > 1000) {
            recentAlerts_.erase(recentAlerts_.begin());
        }
    }

    if (alertCallback_) {
        alertCallback_(alert);
    }

    std::string levelStr;
    switch (level) {
        case AlertLevel::INFO: levelStr = "INFO"; break;
        case AlertLevel::WARNING: levelStr = "WARN"; break;
        case AlertLevel::ERROR: levelStr = "ERROR"; break;
        case AlertLevel::CRITICAL: levelStr = "CRITICAL"; break;
    }

    if (level >= AlertLevel::ERROR) {
        spdlog::error("ALERT [{}] {}: {} ({})", levelStr, type, message, source);
    } else {
        spdlog::warn("ALERT [{}] {}: {} ({})", levelStr, type, message, source);
    }

    if (emailEnabled_ && level >= AlertLevel::ERROR) {
        sendEmail(alert);
    }

    if (dingtalkEnabled_ && level >= AlertLevel::WARNING) {
        sendDingtalk(alert);
    }
}

void AlertManager::setEmailConfig(const std::string& smtpHost, int smtpPort, const std::string& from, const std::vector<std::string>& to) {
    smtpHost_ = smtpHost;
    smtpPort_ = smtpPort;
    emailFrom_ = from;
    emailTo_ = to;
    emailEnabled_ = !smtpHost.empty() && !from.empty() && !to.empty();
    spdlog::info("Email alerts configured: enabled={}", emailEnabled_);
}

void AlertManager::setDingtalkConfig(const std::string& webhookUrl) {
    dingtalkWebhook_ = webhookUrl;
    dingtalkEnabled_ = !webhookUrl.empty();
    spdlog::info("Dingtalk alerts configured: enabled={}", dingtalkEnabled_);
}

void AlertManager::setAlertCallback(AlertCallback callback) {
    alertCallback_ = callback;
}

std::vector<Alert> AlertManager::getRecentAlerts(int limit) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Alert> result;

    int count = std::min(limit, static_cast<int>(recentAlerts_.size()));
    if (count > 0) {
        result = std::vector<Alert>(recentAlerts_.end() - count, recentAlerts_.end());
    }

    return result;
}

void AlertManager::clearAlerts() {
    std::lock_guard<std::mutex> lock(mutex_);
    recentAlerts_.clear();
    spdlog::info("Alert history cleared");
}

void AlertManager::sendEmail(const Alert& alert) {
    spdlog::info("Would send email alert: {} - {}", alert.type, alert.message);
}

void AlertManager::sendDingtalk(const Alert& alert) {
    if (dingtalkWebhook_.empty()) {
        return;
    }

    std::string levelStr;
    switch (alert.level) {
        case AlertLevel::INFO: levelStr = "Info"; break;
        case AlertLevel::WARNING: levelStr = "Warning"; break;
        case AlertLevel::ERROR: levelStr = "Error"; break;
        case AlertLevel::CRITICAL: levelStr = "Critical"; break;
    }

    std::string jsonPayload = "{"
        "\"msgtype\": \"markdown\","
        "\"markdown\": {"
        "\"title\": \"RFID System Alert [" + levelStr + "]\","
        "\"text\": \"### RFID System Alert\\n\\n"
        + std::string("**Type:** ") + alert.type + "\\n\\n"
        + std::string("**Level:** ") + levelStr + "\\n\\n"
        + std::string("**Message:** ") + alert.message + "\\n\\n"
        + std::string("**Source:** ") + alert.source + "\\n\\n"
        + std::string("**Time:** ") + alert.timestamp + "\\n\""
        "}"
    "}";

    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, dingtalkWebhook_.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonPayload.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            spdlog::error("Failed to send Dingtalk alert: {}", curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
    }
}

std::string AlertManager::generateAlertId() {
    static int counter = 0;
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    return "ALT-" + std::to_string(timestamp) + "-" + std::to_string(++counter);
}

std::string AlertManager::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();

    return ss.str();
}