#pragma once

#include "bandit_types.h"
#include "../../models/asset.h"
#include "../../analytics/scoring/asset_score.h"
#include <memory>

namespace bandit {

class ContextBuilder {
public:
    static ContextBuilder& instance();
    
    // 从资产和评分构建上下文特征
    ContextFeatures buildContext(const Asset& asset, const AssetScore& score);
    
    // 从资产ID构建上下文（需要查询数据库）
    ContextFeatures buildContextByAssetId(int asset_id);
    
private:
    ContextBuilder();
    ~ContextBuilder();
    
    ContextBuilder(const ContextBuilder&) = delete;
    ContextBuilder& operator=(const ContextBuilder&) = delete;
    
    // 获取每日平均扫描次数
    double getDailyAvgScans(int asset_id) const;
    
    // 获取每周平均扫描次数
    double getWeeklyAvgScans(int asset_id) const;
    
    // 获取24小时内移动次数
    int getMoveCount24h(int asset_id) const;
    
    // 获取距离上次扫描的小时数
    int getHoursSinceLastSeen(int asset_id) const;
    
    // 判断是否在非法位置
    bool isInIllegalLocation(int asset_id) const;
    
    // 判断是否为备用设备
    bool isBackupDevice(int asset_id) const;
    
    // 获取资产类型编码
    int getAssetTypeCode(const std::string& type) const;

    // 新增：获取最近扫描间隔波动
    double getLastSeenVariance(int asset_id) const;

    // 新增：获取位置稳定性
    double getLocationStability(int asset_id) const;

    // 新增：获取历史丢失率
    double getHistoricalMissingRate(int asset_id) const;
};

} // namespace bandit