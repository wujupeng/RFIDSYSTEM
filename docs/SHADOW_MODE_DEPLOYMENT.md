# v3.3 Bandit SHADOW 模式部署指南

## 📋 前置条件

1. ✅ 代码已更新到最新版本 (commit 1e6521d)
2. ✅ 数据库迁移脚本已执行 (`scripts/migration_v3.2_bandit.sql`)
3. ✅ 项目已重新编译

## 🚀 生产环境开启步骤

### 步骤 1: 拉取最新代码

```bash
cd /path/to/RFIDSYSTEM
git pull origin main
```

### 步骤 2: 重新编译

```bash
cd build
cmake ..
make -j4
```

### 步骤 3: 配置 SHADOW 模式

在 `server/config/bandit_config.json` 中配置：

```json
{
  "bandit": {
    "enabled": true,
    "mode": "SHADOW",
    "alpha": 1.5,
    "epsilon": 0.1,
    "feature_dim": 14
  },
  "guardrail": {
    "enabled": true,
    "high_risk_threshold": 0.9,
    "low_confidence_threshold": 0.6,
    "uncertainty_threshold": 0.4
  },
  "policy_mixer": {
    "top_k": 2,
    "fallback_to_rule": true
  }
}
```

### 步骤 4: 启动服务

```bash
./server
```

## 📊 监控指标

### Guardrail 触发统计

```sql
-- 查看 Guardrail 触发情况
SELECT
    decision_source,
    COUNT(*) as count,
    AVG(confidence) as avg_confidence,
    AVG(uncertainty) as avg_uncertainty
FROM decisions
WHERE created_at > NOW() - INTERVAL '24 hours'
GROUP BY decision_source;
```

### SHADOW 模式一致性

```sql
-- 查看 Bandit 与规则的一致性
SELECT
    CASE
        WHEN bandit_action = rule_action THEN 'MATCH'
        ELSE 'DIFFER'
    END as match_status,
    COUNT(*) as count
FROM (
    SELECT
        action_type as bandit_action,
        rule_based_action as rule_action
    FROM decisions
    WHERE decision_source = 'BANDIT'
      AND created_at > NOW() - INTERVAL '24 hours'
) sub
GROUP BY match_status;
```

### 置信度分布

```sql
-- 查看置信度分布
SELECT
    CASE
        WHEN confidence >= 0.8 THEN 'HIGH (>=0.8)'
        WHEN confidence >= 0.6 THEN 'MEDIUM (0.6-0.8)'
        WHEN confidence >= 0.4 THEN 'LOW (0.4-0.6)'
        ELSE 'VERY_LOW (<0.4)'
    END as confidence_bucket,
    COUNT(*) as count
FROM decisions
WHERE created_at > NOW() - INTERVAL '24 hours'
GROUP BY confidence_bucket
ORDER BY count DESC;
```

## 🎯 关键监控指标

| 指标 | 目标值 | 告警阈值 | 说明 |
|------|--------|----------|------|
| Guardrail 高风险触发率 | > 99% | < 95% | missing_risk > 0.9 时必须触发 |
| Guardrail 非法位置触发率 | > 99% | < 95% | in_illegal_location 时必须触发 |
| Bandit 与规则一致性 | > 80% | < 50% | 一致性过低说明模型偏离业务规则 |
| 平均置信度 | > 0.6 | < 0.4 | 置信度过低说明模型不确定性高 |
| 模型更新次数 | 持续增长 | 连续 24h 无更新 | 模型没有学习新数据 |

## 🔧 故障排查

### 问题 1: BanditEngine 未初始化

```
检查日志中是否有: "BanditEngine initialized in SHADOW mode"
```

### 问题 2: 决策始终来自 GUARDRAIL

```
这可能是预期行为，检查:
1. missing_risk 值是否普遍较高
2. in_illegal_location 标记是否过多
```

### 问题 3: 一致性过低 (< 50%)

```
可能原因:
1. 模型尚未充分训练
2. 奖励函数设置不当
3. 特征维度不匹配

解决方案:
1. 继续收集 SHADOW 模式数据
2. 检查奖励函数配置
3. 确认 ContextFeatures 维度为 14
```

## 📈 收集到足够数据后

当 SHADOW 模式收集到 1000+ 样本，且一致性 > 80% 时，可切换到 SUGGESTION 模式：

```bash
# 方式 1: 通过 API 切换
curl -X PUT /api/bandit/mode -d '{"mode": "SUGGESTION"}'

# 方式 2: 修改配置文件后重启
```

## 🔄 回滚方案

如需回滚到纯规则引擎：

```bash
# 方式 1: API 切换
curl -X PUT /api/bandit/mode -d '{"mode": "DISABLED"}'

# 方式 2: 修改配置
"bandit": {
  "enabled": false
}
```

## 📞 支持

如遇问题，请检查:
1. `server/logs/bandit.log` - Bandit 相关日志
2. `server/logs/decision.log` - 决策相关日志
3. PostgreSQL `decisions` 表 - 决策记录
