#pragma once

#include <string>
#include <vector>
#include <functional>
#include <mutex>

enum class AlertLevel {
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

struct Alert {
    std::string id;
    std::string type;
    std::string message;
    AlertLevel level;
    std::string timestamp;
    std::string source;
};

class AlertManager {
public:
    static AlertManager& instance();

    void sendAlert(const std::string& type, const std::string& message, AlertLevel level, const std::string& source = "system");

    void setEmailConfig(const std::string& smtpHost, int smtpPort, const std::string& from, const std::vector<std::string>& to);
    void setDingtalkConfig(const std::string& webhookUrl);

    using AlertCallback = std::function<void(const Alert&)>;
    void setAlertCallback(AlertCallback callback);

    std::vector<Alert> getRecentAlerts(int limit = 100);
    void clearAlerts();

private:
    AlertManager();
    ~AlertManager() = default;
    AlertManager(const AlertManager&) = delete;
    AlertManager& operator=(const AlertManager&) = delete;

    void sendEmail(const Alert& alert);
    void sendDingtalk(const Alert& alert);
    std::string generateAlertId();
    std::string getCurrentTimestamp();

    std::vector<Alert> recentAlerts_;
    mutable std::mutex mutex_;

    std::string smtpHost_;
    int smtpPort_ = 25;
    std::string emailFrom_;
    std::vector<std::string> emailTo_;
    std::string dingtalkWebhook_;

    bool emailEnabled_ = false;
    bool dingtalkEnabled_ = false;

    AlertCallback alertCallback_;
};

#define ALERT_INFO(type, msg) AlertManager::instance().sendAlert(type, msg, AlertLevel::INFO)
#define ALERT_WARN(type, msg) AlertManager::instance().sendAlert(type, msg, AlertLevel::WARNING)
#define ALERT_ERROR(type, msg) AlertManager::instance().sendAlert(type, msg, AlertLevel::ERROR)
#define ALERT_CRITICAL(type, msg) AlertManager::instance().sendAlert(type, msg, AlertLevel::CRITICAL)