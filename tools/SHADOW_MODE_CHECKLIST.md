# SHADOW 模式运行清单

## 📋 启动前检查（24-48 小时前）

### ✅ 代码部署

- [ ] 已拉取最新代码：`git pull origin main`
- [ ] 已重新编译：`cd build && cmake .. && make -j4`
- [ ] 数据库迁移已执行：`psql -f scripts/migration_v3.2_bandit.sql`
- [ ] 配置文件已更新（可选）：`server/config/bandit_config.json`

### ✅ 验证 SHADOW 模式

```bash
# 启动服务后检查日志
grep "BanditEngine initialized" server.log
# 应该看到：BanditEngine initialized in SHADOW mode
```

### ✅ 验证数据记录

```sql
-- 检查是否有 SHADOW 决策记录
SELECT COUNT(*) FROM decisions 
WHERE decision_source = 'SHADOW' 
  AND created_at > NOW() - INTERVAL '1 hour';
-- 应该 > 0
```

---

## 📊 运行期间监控（每 6 小时）

### 快速检查

```bash
# 运行监控脚本（单次模式）
cd tools
./monitor_shadow_mode.sh --once
```

### 关键指标检查

```sql
-- 1. 决策总数（过去 1 小时）
SELECT COUNT(*) FROM decisions 
WHERE created_at > NOW() - INTERVAL '1 hour';

-- 2. Guardrail 触发次数
SELECT COUNT(*) FROM decisions 
WHERE decision_source = 'GUARDRAIL'
  AND created_at > NOW() - INTERVAL '1 hour';

-- 3. 平均置信度
SELECT ROUND(AVG(confidence), 3) FROM decisions 
WHERE confidence > 0
  AND created_at > NOW() - INTERVAL '1 hour';
```

### 告警条件

如果出现以下情况，需要立即检查：

- ⚠️ Guardrail 高风险触发 > 100 次/小时
- ⚠️ 一致性率 < 50%
- ⚠️ 平均置信度 < 0.4
- ⚠️ 连续 1 小时无 SHADOW 决策

---

## 📈 24 小时后：深度分析

### 运行分析脚本

```bash
cd tools
./analyze_shadow_data.sh
```

这会生成 `shadow_analysis_report.md` 报告。

### 查看关键指标

```bash
cat shadow_analysis_report.md | grep -A 5 "关键指标"
```

### 决策标准

#### ✅ 可以切换到 SUGGESTION 模式：

- 一致性率 >= 80%
- 平均置信度 >= 0.6
- Guardrail 触发率正常（< 20%）
- 无严重告警

#### ⚠️ 继续 SHADOW 模式：

- 一致性率 60-80%
- 平均置信度 0.4-0.6
- 需要更多数据

#### ❌ 需要调整模型：

- 一致性率 < 60%
- 平均置信度 < 0.4
- Guardrail 频繁触发

---

## 🔄 48 小时后：最终评估

### 生成最终报告

```bash
# 导出 48 小时完整数据
psql -c "
COPY (
    SELECT * FROM decisions 
    WHERE created_at > NOW() - INTERVAL '48 hours'
    ORDER BY created_at
) TO STDOUT WITH CSV HEADER
" > shadow_48h_data.csv

# 运行深度分析
./analyze_shadow_data.sh
```

### 准备切换模式

如果指标良好，准备切换到 SUGGESTION 模式：

```bash
# 1. 备份当前配置
cp server/config/bandit_config.json server/config/bandit_config.json.backup

# 2. 修改配置
# 将 bandit.mode 从 SHADOW 改为 SUGGESTION

# 3. 重启服务
./restart_server.sh

# 4. 验证新模式
grep "BanditEngine initialized" server.log
# 应该看到：BanditEngine initialized in SUGGESTION mode
```

---

## 📞 问题排查

### 问题 1：没有 SHADOW 决策记录

```bash
# 检查 BanditEngine 是否启用
grep "bandit" server/config/bandit_config.json

# 检查日志
grep -i "bandit\|shadow" server.log | tail -20
```

### 问题 2：一致性率过低

```sql
-- 分析不一致的案例
SELECT 
    action_type,
    rule_based_action,
    missing_risk,
    confidence,
    created_at
FROM decisions 
WHERE action_type != rule_based_action
  AND created_at > NOW() - INTERVAL '24 hours'
LIMIT 10;
```

### 问题 3：Guardrail 频繁触发

```sql
-- 查看触发原因分布
SELECT 
    CASE 
        WHEN reason LIKE '%HIGH_RISK%' THEN '高风险'
        WHEN reason LIKE '%ILLEGAL_LOCATION%' THEN '非法位置'
        WHEN reason LIKE '%LOW_CONFIDENCE%' THEN '低置信度'
        ELSE '其他'
    END as reason_type,
    COUNT(*)
FROM decisions 
WHERE decision_source = 'GUARDRAIL'
GROUP BY 1;
```

---

## 📊 监控仪表板（可选）

如果有 Grafana，可以导入以下查询：

### 决策来源分布（饼图）

```sql
SELECT 
    decision_source,
    COUNT(*) as count
FROM decisions 
WHERE created_at > NOW() - INTERVAL '$__range'
GROUP BY 1
```

### 一致性趋势（折线图）

```sql
SELECT 
    DATE_TRUNC('hour', created_at) as time,
    ROUND(SUM(CASE WHEN action_type = rule_based_action THEN 1 ELSE 0 END) * 100.0 / COUNT(*), 2) as consistency_rate
FROM decisions 
WHERE decision_source IN ('BANDIT', 'SHADOW')
  AND rule_based_action IS NOT NULL
  AND created_at > NOW() - INTERVAL '$__range'
GROUP BY 1
ORDER BY 1
```

### Guardrail 触发（柱状图）

```sql
SELECT 
    DATE_TRUNC('hour', created_at) as time,
    COUNT(*) as triggers
FROM decisions 
WHERE decision_source = 'GUARDRAIL'
  AND created_at > NOW() - INTERVAL '$__range'
GROUP BY 1
ORDER BY 1
```

---

## ✅ 完成标志

SHADOW 模式运行成功的标志：

- [x] 连续 24-48 小时稳定运行
- [x] 收集到 1000+ SHADOW 决策样本
- [x] 一致性率 >= 80%
- [x] 平均置信度 >= 0.6
- [x] Guardrail 正常工作（高风险/非法位置触发率 > 95%）
- [x] 无严重告警或异常

**达到以上标准后，可以切换到 SUGGESTION 模式！** 🎉
