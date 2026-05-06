#!/bin/bash

# SHADOW 模式深度分析脚本
# 用于分析 24-48 小时收集的数据，生成详细报告

set -e

# 配置
DB_HOST="${DB_HOST:-localhost}"
DB_NAME="${DB_NAME:-rfid_db}"
DB_USER="${DB_USER:-rfid_user}"
REPORT_FILE="${REPORT_FILE:-shadow_analysis_report.md}"

log() {
    echo -e "$1" | tee -a "$REPORT_FILE"
}

# 生成 Markdown 报告头部
generate_header() {
    cat > "$REPORT_FILE" << 'EOF'
# SHADOW 模式深度分析报告

**生成时间**: $(date '+%Y-%m-%d %H:%M:%S')  
**分析周期**: 过去 24 小时  
**数据库**: rfid_db

---

## 执行摘要

EOF
}

# 关键指标
generate_executive_summary() {
    log "### 📊 关键指标"
    log ""
    
    # 总决策数
    total=$(psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" -t -c "
        SELECT COUNT(*) FROM decisions 
        WHERE created_at > NOW() - INTERVAL '24 hours';
    " | tr -d ' ')
    log "- **总决策数**: $total"
    
    # SHADOW 决策数
    shadow=$(psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" -t -c "
        SELECT COUNT(*) FROM decisions 
        WHERE decision_source = 'SHADOW' 
          AND created_at > NOW() - INTERVAL '24 hours';
    " | tr -d ' ')
    log "- **SHADOW 决策数**: $shadow"
    
    # Guardrail 触发数
    guardrail=$(psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" -t -c "
        SELECT COUNT(*) FROM decisions 
        WHERE decision_source = 'GUARDRAIL' 
          AND created_at > NOW() - INTERVAL '24 hours';
    " | tr -d ' ')
    log "- **Guardrail 触发数**: $guardrail"
    
    # 平均一致性
    consistency=$(psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" -t -c "
        SELECT ROUND(SUM(CASE WHEN action_type = rule_based_action THEN 1 ELSE 0 END) * 100.0 / COUNT(*), 2)
        FROM decisions 
        WHERE decision_source IN ('BANDIT', 'SHADOW')
          AND rule_based_action IS NOT NULL
          AND created_at > NOW() - INTERVAL '24 hours';
    " | tr -d ' ')
    log "- **平均一致性**: ${consistency}%"
    
    # 平均置信度
    avg_conf=$(psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" -t -c "
        SELECT ROUND(AVG(confidence), 4)
        FROM decisions 
        WHERE confidence > 0
          AND created_at > NOW() - INTERVAL '24 hours';
    " | tr -d ' ')
    log "- **平均置信度**: $avg_conf"
    
    log ""
    log "---"
    log ""
}

# 决策质量分析
analyze_decision_quality() {
    log "## 🎯 决策质量分析"
    log ""
    
    log "### 决策来源分布"
    log ""
    log "| 来源 | 数量 | 百分比 |"
    log "|------|------|--------|"
    
    psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" -t -c "
        SELECT 
            decision_source,
            COUNT(*),
            ROUND(COUNT(*) * 100.0 / SUM(COUNT(*)) OVER (), 2)
        FROM decisions 
        WHERE created_at > NOW() - INTERVAL '24 hours'
        GROUP BY decision_source
        ORDER BY COUNT(*) DESC;
    " | while IFS='|' read -r source count pct; do
        source=$(echo "$source" | xargs)
        log "| $source | $count | ${pct}% |"
    done
    
    log ""
    log "### Guardrail 触发原因分析"
    log ""
    log "| 触发原因 | 次数 | 占比 |"
    log "|----------|------|------|"
    
    psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" -t -c "
        SELECT 
            CASE 
                WHEN reason LIKE '%HIGH_RISK%' THEN '高风险 (HIGH_RISK)'
                WHEN reason LIKE '%ILLEGAL_LOCATION%' THEN '非法位置 (ILLEGAL_LOCATION)'
                WHEN reason LIKE '%LOW_CONFIDENCE%' THEN '低置信度 (LOW_CONFIDENCE)'
                ELSE '其他'
            END,
            COUNT(*),
            ROUND(COUNT(*) * 100.0 / SUM(COUNT(*)) OVER (), 2)
        FROM decisions 
        WHERE decision_source = 'GUARDRAIL'
          AND created_at > NOW() - INTERVAL '24 hours'
        GROUP BY 1
        ORDER BY COUNT(*) DESC;
    " | while IFS='|' read -r reason count pct; do
        reason=$(echo "$reason" | xargs)
        log "| $reason | $count | ${pct}% |"
    done
    
    log ""
    log "### 置信度与不确定性分析"
    log ""
    log "| 指标 | 平均值 | 中位数 | 最小值 | 最大值 |"
    log "|------|--------|--------|--------|--------|"
    
    # 置信度统计
    conf_stats=$(psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" -t -c "
        SELECT 
            ROUND(AVG(confidence), 4),
            ROUND(PERCENTILE_CONT(0.5) WITHIN GROUP (ORDER BY confidence), 4),
            ROUND(MIN(confidence), 4),
            ROUND(MAX(confidence), 4)
        FROM decisions 
        WHERE confidence > 0
          AND created_at > NOW() - INTERVAL '24 hours';
    ")
    
    log "| 置信度 |$(echo "$conf_stats" | tr '|' ' |') |"
    
    # 不确定性统计
    unc_stats=$(psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" -t -c "
        SELECT 
            ROUND(AVG(uncertainty), 2),
            ROUND(PERCENTILE_CONT(0.5) WITHIN GROUP (ORDER BY uncertainty), 2),
            ROUND(MIN(uncertainty), 2),
            ROUND(MAX(uncertainty), 2)
        FROM decisions 
        WHERE uncertainty > 0
          AND created_at > NOW() - INTERVAL '24 hours';
    ")
    
    log "| 不确定性 |$(echo "$unc_stats" | tr '|' ' |') |"
    
    log ""
    log "---"
    log ""
}

# 一致性深度分析
analyze_consistency() {
    log "## 📈 一致性深度分析"
    log ""
    
    log "### 总体一致性"
    log ""
    
    # 一致性矩阵
    log "#### 一致性矩阵"
    log ""
    log "| Bandit \\ 规则 | INSPECT | ALERT | NO_ACTION | REALLOCATE | 总计 |"
    log "|----------------|---------|-------|-----------|------------|------|"
    
    # 这里可以扩展为完整的矩阵分析
    psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" -c "
        SELECT 
            action_type as bandit_action,
            rule_based_action,
            COUNT(*)
        FROM decisions 
        WHERE decision_source IN ('BANDIT', 'SHADOW')
          AND rule_based_action IS NOT NULL
          AND created_at > NOW() - INTERVAL '24 hours'
        GROUP BY 1, 2
        ORDER BY 1, 2;
    "
    
    log ""
    log "### 按场景分析一致性"
    log ""
    
    log "#### 按风险等级"
    log ""
    log "| 风险等级 | 总决策 | 一致 | 一致率 |"
    log "|----------|--------|------|--------|"
    
    psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" -t -c "
        SELECT 
            CASE 
                WHEN missing_risk >= 0.8 THEN '高风险 (>=0.8)'
                WHEN missing_risk >= 0.5 THEN '中风险 (0.5-0.8)'
                ELSE '低风险 (<0.5)'
            END,
            COUNT(*),
            SUM(CASE WHEN action_type = rule_based_action THEN 1 ELSE 0 END),
            ROUND(SUM(CASE WHEN action_type = rule_based_action THEN 1 ELSE 0 END) * 100.0 / COUNT(*), 2)
        FROM decisions 
        WHERE decision_source IN ('BANDIT', 'SHADOW')
          AND rule_based_action IS NOT NULL
          AND created_at > NOW() - INTERVAL '24 hours'
        GROUP BY 1
        ORDER BY 1;
    " | while IFS='|' read -r risk total match rate; do
        risk=$(echo "$risk" | xargs)
        log "| $risk | $total | $match | ${rate}% |"
    done
    
    log ""
    log "#### 按资产类型"
    log ""
    log "| 资产类型 | 总决策 | 一致 | 一致率 |"
    log "|----------|--------|------|--------|"
    
    psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" -t -c "
        SELECT 
            asset_type,
            COUNT(*),
            SUM(CASE WHEN action_type = rule_based_action THEN 1 ELSE 0 END),
            ROUND(SUM(CASE WHEN action_type = rule_based_action THEN 1 ELSE 0 END) * 100.0 / COUNT(*), 2)
        FROM decisions 
        WHERE decision_source IN ('BANDIT', 'SHADOW')
          AND rule_based_action IS NOT NULL
          AND created_at > NOW() - INTERVAL '24 hours'
        GROUP BY 1
        ORDER BY 1;
    " | while IFS='|' read -r type total match rate; do
        type=$(echo "$type" | xargs)
        log "| $type | $total | $match | ${rate}% |"
    done
    
    log ""
    log "---"
    log ""
}

# 时间趋势分析
analyze_time_trend() {
    log "## 📉 时间趋势分析"
    log ""
    
    log "### 每小时决策数趋势"
    log ""
    
    psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" -c "
        SELECT 
            DATE_TRUNC('hour', created_at) as 小时，
            COUNT(*) as 决策数，
            SUM(CASE WHEN decision_source = 'GUARDRAIL' THEN 1 ELSE 0 END) as Guardrail触发，
            ROUND(AVG(confidence), 3) as 平均置信度
        FROM decisions 
        WHERE created_at > NOW() - INTERVAL '24 hours'
        GROUP BY 1
        ORDER BY 1;
    "
    
    log ""
    log "### 一致性随时间变化"
    log ""
    
    psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" -c "
        SELECT 
            DATE_TRUNC('hour', created_at) as 小时，
            COUNT(*) as 总决策，
            SUM(CASE WHEN action_type = rule_based_action THEN 1 ELSE 0 END) as 一致，
            ROUND(SUM(CASE WHEN action_type = rule_based_action THEN 1 ELSE 0 END) * 100.0 / COUNT(*), 2) as 一致率
        FROM decisions 
        WHERE decision_source IN ('BANDIT', 'SHADOW')
          AND rule_based_action IS NOT NULL
          AND created_at > NOW() - INTERVAL '24 hours'
        GROUP BY 1
        ORDER BY 1;
    "
    
    log ""
    log "---"
    log ""
}

# 建议和下一步
generate_recommendations() {
    log "## 💡 建议和下一步"
    log ""
    
    # 获取关键指标
    consistency=$(psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" -t -c "
        SELECT ROUND(SUM(CASE WHEN action_type = rule_based_action THEN 1 ELSE 0 END) * 100.0 / COUNT(*), 2)
        FROM decisions 
        WHERE decision_source IN ('BANDIT', 'SHADOW')
          AND rule_based_action IS NOT NULL
          AND created_at > NOW() - INTERVAL '24 hours';
    " | tr -d ' ')
    
    guardrail_rate=$(psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" -t -c "
        SELECT ROUND(COUNT(*) * 100.0 / SUM(COUNT(*)) OVER (), 2)
        FROM decisions 
        WHERE decision_source = 'GUARDRAIL'
          AND created_at > NOW() - INTERVAL '24 hours'
        GROUP BY decision_source;
    " | tr -d ' ')
    
    avg_conf=$(psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" -t -c "
        SELECT ROUND(AVG(confidence), 2)
        FROM decisions 
        WHERE confidence > 0
          AND created_at > NOW() - INTERVAL '24 hours';
    " | tr -d ' ')
    
    log "### 评估结果"
    log ""
    
    # 一致性评估
    if (( $(echo "$consistency >= 80" | bc -l) )); then
        log "✅ **一致性优秀** (${consistency}%)：可以考虑切换到 SUGGESTION 模式"
    elif (( $(echo "$consistency >= 60" | bc -l) )); then
        log "⚠️  **一致性中等** (${consistency}%)：建议继续收集数据 24 小时"
    else
        log "❌ **一致性较低** (${consistency}%)：需要检查模型配置或奖励函数"
    fi
    
    # Guardrail 评估
    if (( $(echo "$guardrail_rate > 20" | bc -l) )); then
        log "⚠️  **Guardrail 触发率高** (${guardrail_rate}%)：检查业务场景风险是否普遍较高"
    fi
    
    # 置信度评估
    if (( $(echo "$avg_conf >= 0.7" | bc -l) )); then
        log "✅ **置信度良好** (${avg_conf})：模型学习状态正常"
    elif (( $(echo "$avg_conf >= 0.5" | bc -l) )); then
        log "⚠️  **置信度中等** (${avg_conf})：建议增加训练样本"
    else
        log "❌ **置信度较低** (${avg_conf})：模型可能欠拟合"
    fi
    
    log ""
    log "### 下一步行动"
    log ""
    log "1. **如果一致性 >= 80%**："
    log "   - 准备切换到 SUGGESTION 模式"
    log "   - 更新配置：\`bandit.mode = SUGGESTION\`"
    log "   - 密切监控用户反馈"
    log ""
    log "2. **如果一致性 60-80%**："
    log "   - 继续 SHADOW 模式运行 24-48 小时"
    log "   - 收集更多训练数据"
    log "   - 分析不一致案例的原因"
    log ""
    log "3. **如果一致性 < 60%**："
    log "   - 检查奖励函数配置"
    log "   - 验证特征工程是否正确"
    log "   - 考虑调整超参数（alpha, epsilon）"
    log ""
}

# 主函数
main() {
    echo "正在生成 SHADOW 模式深度分析报告..."
    echo ""
    
    generate_header
    generate_executive_summary
    analyze_decision_quality
    analyze_consistency
    analyze_time_trend
    generate_recommendations
    
    echo ""
    echo "✅ 报告已生成：$REPORT_FILE"
    echo ""
    echo "查看报告：cat $REPORT_FILE"
}

# 执行
main
