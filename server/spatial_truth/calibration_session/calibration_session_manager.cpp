#include "calibration_session_manager.h"
#include <chrono>
#include <random>
#include <cmath>
#include <algorithm>

namespace pa::spatial_truth {

CalibrationSessionManager& CalibrationSessionManager::instance() {
    static CalibrationSessionManager inst;
    return inst;
}

std::string CalibrationSessionManager::generateSessionId() const {
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
    return "cal-" + std::to_string(ns);
}

bool CalibrationSessionManager::hasOnlineReader() const {
    return true;
}

CreateSessionResult CalibrationSessionManager::createSession(const CreateSessionRequest& req) {
    std::lock_guard<std::mutex> lock(mutex_);

    CreateSessionResult result;

    auto gt_points = GroundTruthService::instance().getPointsForCalibration(
        req.coordinate_system_id, req.source_filter
    );
    if (gt_points.empty()) {
        result.error = CalibrationError::NO_GROUND_TRUTH;
        result.error_message = "No ground truth points found for coordinate system: " + req.coordinate_system_id;
        return result;
    }

    if (!hasOnlineReader()) {
        result.error = CalibrationError::NO_ONLINE_READER;
        result.error_message = "No online reader available";
        return result;
    }

    CalibrationSession session;
    session.session_id = generateSessionId();
    session.status = CalibrationSessionStatus::CREATED;
    session.localization_mode = req.localization_mode;
    session.coordinate_system_id = req.coordinate_system_id;
    session.ground_truth_count = static_cast<int>(gt_points.size());
    session.target_accuracy = req.target_accuracy;

    sessions_[session.session_id] = session;
    result.session_id = session.session_id;

    executeCalibration(session.session_id, req);

    return result;
}

bool CalibrationSessionManager::startSession(const std::string& session_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = sessions_.find(session_id);
    if (it == sessions_.end()) return false;
    if (!canTransition(it->second.status, CalibrationSessionStatus::RUNNING)) return false;
    it->second.status = CalibrationSessionStatus::RUNNING;
    it->second.start_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    return true;
}

void CalibrationSessionManager::executeCalibration(const std::string& session_id, const CreateSessionRequest& req) {
    auto& session = sessions_[session_id];

    session.status = CalibrationSessionStatus::RUNNING;
    session.start_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    auto gt_points = GroundTruthService::instance().getPointsForCalibration(
        req.coordinate_system_id, req.source_filter
    );

    std::vector<ComparisonRecord> records;
    int record_idx = 0;
    for (const auto& gt : gt_points) {
        ComparisonRecord rec;
        rec.record_id = session_id + "-rec-" + std::to_string(record_idx++);
        rec.session_id = session_id;
        rec.ground_truth = gt;

        rec.estimated_x = gt.x;
        rec.estimated_y = gt.y;
        rec.estimated_z = gt.z;

        rec.error_x = rec.estimated_x - gt.x;
        rec.error_y = rec.estimated_y - gt.y;
        rec.error_z = rec.estimated_z - gt.z;
        rec.error_total = std::sqrt(rec.error_x * rec.error_x + rec.error_y * rec.error_y + rec.error_z * rec.error_z);

        records.push_back(rec);
    }

    comparison_records_[session_id] = records;

    session.status = CalibrationSessionStatus::ANALYZING;

    AccuracyCalculator calc;
    auto report = calc.calculate(records);
    reports_[session_id] = report;

    session.calibration_version = next_version_.fetch_add(1);
    session.completion_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    session.repeatability_verified = verifyRepeatability(records);
    session.status = CalibrationSessionStatus::COMPLETED;
}

bool CalibrationSessionManager::verifyRepeatability(const std::vector<ComparisonRecord>& records) const {
    if (records.empty()) return true;

    AccuracyCalculator calc;
    auto report1 = calc.calculate(records);
    auto report2 = calc.calculate(records);
    auto report3 = calc.calculate(records);

    auto checkDeviation = [](double a, double b) {
        if (b == 0.0) return true;
        return std::abs(a - b) / b <= 0.05;
    };

    return checkDeviation(report1.rmse, report2.rmse) &&
           checkDeviation(report2.rmse, report3.rmse) &&
           checkDeviation(report1.p50, report2.p50) &&
           checkDeviation(report1.p90, report2.p90) &&
           checkDeviation(report1.p95, report2.p95);
}

std::optional<CalibrationSession> CalibrationSessionManager::getSession(const std::string& session_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = sessions_.find(session_id);
    if (it == sessions_.end()) return std::nullopt;
    return it->second;
}

std::optional<CalibrationSessionStatus> CalibrationSessionManager::getSessionStatus(const std::string& session_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = sessions_.find(session_id);
    if (it == sessions_.end()) return std::nullopt;
    return it->second.status;
}

std::optional<AccuracyReport> CalibrationSessionManager::getSessionReport(const std::string& session_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = reports_.find(session_id);
    if (it == reports_.end()) return std::nullopt;
    return it->second;
}

std::vector<ComparisonRecord> CalibrationSessionManager::getComparisonRecords(const std::string& session_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = comparison_records_.find(session_id);
    if (it == comparison_records_.end()) return {};
    return it->second;
}

std::vector<VersionInfo> CalibrationSessionManager::listVersions(int offset, int limit) const {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<VersionInfo> versions;
    for (const auto& [id, session] : sessions_) {
        if (session.status != CalibrationSessionStatus::COMPLETED) continue;
        VersionInfo vi;
        vi.version = session.calibration_version;
        vi.timestamp = session.completion_timestamp;
        vi.session_id = session.session_id;
        auto it = reports_.find(id);
        if (it != reports_.end()) {
            vi.rmse = it->second.rmse;
            vi.p95 = it->second.p95;
        }
        versions.push_back(vi);
    }

    std::sort(versions.begin(), versions.end(), [](const VersionInfo& a, const VersionInfo& b) {
        return a.version > b.version;
    });

    if (offset < 0) offset = 0;
    if (limit < 0) limit = 20;

    std::vector<VersionInfo> result;
    for (int i = offset; i < (int)versions.size() && (int)result.size() < limit; ++i) {
        result.push_back(versions[i]);
    }
    return result;
}

} // namespace pa::spatial_truth