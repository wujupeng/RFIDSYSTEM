#pragma once

#include "calibration_session.h"
#include "accuracy_calculator.h"
#include "../ground_truth/ground_truth_service.h"
#include <vector>
#include <string>
#include <mutex>
#include <unordered_map>
#include <optional>
#include <atomic>

namespace pa::spatial_truth {

enum class CalibrationError {
    NONE,
    NO_GROUND_TRUTH,
    NO_ONLINE_READER,
    ALGORITHM_NOT_SUPPORTED,
    SESSION_NOT_FOUND,
    SESSION_ALREADY_COMPLETED,
    SESSION_TIMEOUT,
    REPEATABILITY_FAILED,
};

struct CreateSessionRequest {
    pa::LocalizationMode localization_mode = pa::LocalizationMode::RSSI_TRIANGULATION;
    std::string coordinate_system_id = "global";
    double target_accuracy = 0.1;
    std::optional<GroundTruthSource> source_filter;
};

struct CreateSessionResult {
    CalibrationError error = CalibrationError::NONE;
    std::string error_message;
    std::string session_id;
};

struct VersionInfo {
    uint64_t version = 0;
    uint64_t timestamp = 0;
    std::string session_id;
    double rmse = 0.0;
    double p95 = 0.0;
};

class CalibrationSessionManager {
public:
    static CalibrationSessionManager& instance();

    CreateSessionResult createSession(const CreateSessionRequest& req);
    bool startSession(const std::string& session_id);
    std::optional<CalibrationSession> getSession(const std::string& session_id) const;
    std::optional<CalibrationSessionStatus> getSessionStatus(const std::string& session_id) const;
    std::optional<AccuracyReport> getSessionReport(const std::string& session_id) const;
    std::vector<ComparisonRecord> getComparisonRecords(const std::string& session_id) const;
    std::vector<VersionInfo> listVersions(int offset = 0, int limit = 20) const;

    void setSessionTimeoutMs(uint64_t timeout_ms) { session_timeout_ms_ = timeout_ms; }

private:
    CalibrationSessionManager() = default;

    mutable std::mutex mutex_;
    std::unordered_map<std::string, CalibrationSession> sessions_;
    std::unordered_map<std::string, std::vector<ComparisonRecord>> comparison_records_;
    std::unordered_map<std::string, AccuracyReport> reports_;
    std::atomic<uint64_t> next_version_{1};
    uint64_t session_timeout_ms_ = 300000;

    std::string generateSessionId() const;
    bool hasOnlineReader() const;
    void executeCalibration(const std::string& session_id, const CreateSessionRequest& req);
    bool verifyRepeatability(const std::vector<ComparisonRecord>& records) const;
};

} // namespace pa::spatial_truth