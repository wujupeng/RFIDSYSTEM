#include "context_builder.h"
#include "../../services/asset_service.h"
#include "../../services/inventory_service.h"
#include "../../db/db_pool.h"
#include <pqxx/pqxx>

namespace bandit {

ContextBuilder& ContextBuilder::instance() {
    static ContextBuilder instance;
    return instance;
}

ContextBuilder::ContextBuilder() {
}

ContextBuilder::~ContextBuilder() {
}

ContextFeatures ContextBuilder::buildContext(const Asset& asset, const AssetScore& score) {
    ContextFeatures context;
    
    // 风险特征
    context.missing_risk = score.missing_risk;
    context.inactivity_risk = score.inactivity_risk;
    context.abnormal_risk = score.abnormal_risk;
    
    // 扫描统计特征
    context.daily_avg_scans = getDailyAvgScans(asset.id);
    context.weekly_avg_scans = getWeeklyAvgScans(asset.id);
    
    // 行为特征
    context.move_count_24h = getMoveCount24h(asset.id);
    context.hours_since_last_seen = getHoursSinceLastSeen(asset.id);
    
    // 状态特征
    context.in_illegal_location = isInIllegalLocation(asset.id);
    context.is_backup = isBackupDevice(asset.id);
    
    // 类型特征
    context.asset_type = getAssetTypeCode(asset.type);

    // 新增：增强区分度的特征
    context.last_seen_variance = getLastSeenVariance(asset.id);
    context.location_stability = getLocationStability(asset.id);
    context.historical_missing_rate = getHistoricalMissingRate(asset.id);

    return context;
}

ContextFeatures ContextBuilder::buildContextByAssetId(int asset_id) {
    Asset asset = AssetService::instance().getAsset(asset_id);
    AssetScore score = AssetScoreEvaluator::instance().evaluate(asset_id);
    return buildContext(asset, score);
}

double ContextBuilder::getDailyAvgScans(int asset_id) const {
    try {
        auto conn = DBPool::instance().getConnection();
        pqxx::work txn(*conn);
        
        pqxx::result r = txn.exec(
            "SELECT AVG(scan_count) FROM ("
            "    SELECT COUNT(*) as scan_count, DATE(scanned_at) as day "
            "    FROM scan_logs "
            "    WHERE asset_id = $1 AND scanned_at >= NOW() - INTERVAL '7 days' "
            "    GROUP BY day"
            ") as daily_stats",
            asset_id
        );
        
        if (!r.empty() && !r[0][0].is_null()) {
            return r[0][0].as<double>();
        }
    } catch (...) {
        // 数据库查询失败时返回默认值
    }
    
    return 0.0;
}

double ContextBuilder::getWeeklyAvgScans(int asset_id) const {
    try {
        auto conn = DBPool::instance().getConnection();
        pqxx::work txn(*conn);
        
        pqxx::result r = txn.exec(
            "SELECT AVG(scan_count) FROM ("
            "    SELECT COUNT(*) as scan_count, DATE_TRUNC('week', scanned_at) as week "
            "    FROM scan_logs "
            "    WHERE asset_id = $1 AND scanned_at >= NOW() - INTERVAL '30 days' "
            "    GROUP BY week"
            ") as weekly_stats",
            asset_id
        );
        
        if (!r.empty() && !r[0][0].is_null()) {
            return r[0][0].as<double>();
        }
    } catch (...) {
        // 数据库查询失败时返回默认值
    }
    
    return 0.0;
}

int ContextBuilder::getMoveCount24h(int asset_id) const {
    try {
        auto conn = DBPool::instance().getConnection();
        pqxx::work txn(*conn);
        
        pqxx::result r = txn.exec(
            "SELECT COUNT(DISTINCT location) - 1 as move_count "
            "FROM scan_logs "
            "WHERE asset_id = $1 AND scanned_at >= NOW() - INTERVAL '24 hours'",
            asset_id
        );
        
        if (!r.empty() && !r[0][0].is_null()) {
            return std::max(0, r[0][0].as<int>());
        }
    } catch (...) {
        // 数据库查询失败时返回默认值
    }
    
    return 0;
}

int ContextBuilder::getHoursSinceLastSeen(int asset_id) const {
    try {
        auto conn = DBPool::instance().getConnection();
        pqxx::work txn(*conn);
        
        pqxx::result r = txn.exec(
            "SELECT EXTRACT(HOUR FROM (NOW() - MAX(scanned_at))) as hours "
            "FROM scan_logs "
            "WHERE asset_id = $1",
            asset_id
        );
        
        if (!r.empty() && !r[0][0].is_null()) {
            return static_cast<int>(r[0][0].as<double>());
        }
    } catch (...) {
        // 数据库查询失败时返回默认值
    }
    
    return 999; // 默认返回很大的值表示很久没看到
}

bool ContextBuilder::isInIllegalLocation(int asset_id) const {
    try {
        auto conn = DBPool::instance().getConnection();
        pqxx::work txn(*conn);
        
        pqxx::result r = txn.exec(
            "SELECT a.location, l.is_allowed "
            "FROM assets a "
            "LEFT JOIN location_rules l ON a.location = l.location AND a.type = l.asset_type "
            "WHERE a.id = $1",
            asset_id
        );
        
        if (!r.empty()) {
            auto is_allowed = r[0]["is_allowed"];
            if (!is_allowed.is_null()) {
                return !is_allowed.as<bool>();
            }
        }
    } catch (...) {
        // 数据库查询失败时返回默认值
    }
    
    return false;
}

bool ContextBuilder::isBackupDevice(int asset_id) const {
    try {
        auto conn = DBPool::instance().getConnection();
        pqxx::work txn(*conn);
        
        pqxx::result r = txn.exec(
            "SELECT is_backup FROM assets WHERE id = $1",
            asset_id
        );
        
        if (!r.empty() && !r[0][0].is_null()) {
            return r[0][0].as<bool>();
        }
    } catch (...) {
        // 数据库查询失败时返回默认值
    }
    
    return false;
}

int ContextBuilder::getAssetTypeCode(const std::string& type) const {
    if (type == "IT" || type == "computer" || type == "laptop" || type == "desktop") {
        return 0;
    } else if (type == "server" || type == "storage") {
        return 1;
    } else if (type == "EQ" || type == "equipment" || type == "machine") {
        return 2;
    }
    return 3;
}

double ContextBuilder::getLastSeenVariance(int asset_id) const {
    try {
        auto conn = DBPool::instance().getConnection();
        pqxx::work txn(*conn);

        pqxx::result r = txn.exec(
            "SELECT EXTRACT(EPOCH FROM (scanned_at - LAG(scanned_at) OVER (ORDER BY scanned_at))) as interval_sec "
            "FROM scan_logs "
            "WHERE asset_id = $1 AND scanned_at >= NOW() - INTERVAL '7 days' "
            "ORDER BY scanned_at DESC "
            "LIMIT 20",
            asset_id
        );

        if (r.size() < 3) return 0.5; // 数据不足时返回中等值

        std::vector<double> intervals;
        for (const auto& row : r) {
            if (!row[0].is_null()) {
                double sec = row[0].as<double>();
                if (sec > 0) intervals.push_back(sec / 3600.0); // 转换为小时
            }
        }

        if (intervals.empty()) return 0.5;

        // 计算方差
        double mean = 0.0;
        for (double v : intervals) mean += v;
        mean /= intervals.size();

        double variance = 0.0;
        for (double v : intervals) variance += (v - mean) * (v - mean);
        variance /= intervals.size();

        // 归一化到 [0, 1] 范围，方差越大表示波动越大
        return std::min(1.0, variance / 100.0);
    } catch (...) {
        // 数据库查询失败时返回默认值
    }
    return 0.5;
}

double ContextBuilder::getLocationStability(int asset_id) const {
    try {
        auto conn = DBPool::instance().getConnection();
        pqxx::work txn(*conn);

        pqxx::result r = txn.exec(
            "SELECT location, COUNT(*) as cnt "
            "FROM scan_logs "
            "WHERE asset_id = $1 AND scanned_at >= NOW() - INTERVAL '7 days' "
            "GROUP BY location "
            "ORDER BY cnt DESC",
            asset_id
        );

        if (r.empty()) return 0.5;

        int total = 0;
        int max_cnt = 0;
        for (const auto& row : r) {
            int cnt = row[1].as<int>();
            total += cnt;
            if (cnt > max_cnt) max_cnt = cnt;
        }

        // 位置稳定性：主要位置占比越高，越稳定
        double stability = (double)max_cnt / total;
        return stability;
    } catch (...) {
        // 数据库查询失败时返回默认值
    }
    return 0.5;
}

double ContextBuilder::getHistoricalMissingRate(int asset_id) const {
    try {
        auto conn = DBPool::instance().getConnection();
        pqxx::work txn(*conn);

        pqxx::result r = txn.exec(
            "SELECT "
            "  SUM(CASE WHEN risk_level = 'HIGH' AND risk_type = 'missing' THEN 1 ELSE 0 END) as missing_count, "
            "  COUNT(*) as total_count "
            "FROM decisions "
            "WHERE asset_id = $1 AND created_at >= NOW() - INTERVAL '30 days'",
            asset_id
        );

        if (!r.empty()) {
            int missing = r[0]["missing_count"].as<int>(0);
            int total = r[0]["total_count"].as<int>(0);
            if (total > 0) return (double)missing / total;
        }
    } catch (...) {
        // 数据库查询失败时返回默认值
    }
    return 0.05;
}

} // namespace bandit