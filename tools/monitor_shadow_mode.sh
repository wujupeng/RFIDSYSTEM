#!/bin/bash

# SHADOW 模式监控脚本
# 用于在生产环境运行期间收集和分析 Bandit 决策数据

set -e

# 配置
DB_HOST="${DB_HOST:-localhost}"
DB_NAME="${DB_NAME:-rfid_db}"
DB_USER="${DB_USER:-rfid_user}"
LOG_FILE="${LOG_FILE:-shadow_monitor.log}"
REPORT_DIR="${REPORT_DIR:-./shadow_reports}"

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

log() {
    echo -e "[$(date '+%Y-%m-%d %H:%M:%S')] $1" | tee -a "$LOG_FILE"
}

# 创建报告目录
mkdir -p "$REPORT_DIR"

log "${BLUE}═══════════════════════════════════════════════════════════════${NC}"
log "${BLUE}           SHADOW 模式监控工具 - 启动                          ${NC}"
log "${BLUE}═══════════════════════════════════════════════════════════════${NC}"

# 检查数据库连接
check_db() {
    if ! psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" -c "SELECT 1" > /dev/null 2>&1; then
        log "${RED}错误：无法连接到数据库${NC}"
        exit 1
    fi
    log "${GREEN}✓ 数据库连接成功${NC}"
}

# 检查 SHADOW 模式是否启用
check_shadow_mode() {
    log "${YELLOW}检查 SHADOW 模式状态...${NC}"
    
    # 检查最近 1 小时是否有 SHADOW 决策
    count=$(psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" -t -c "
        SELECT COUNT(*) 
        FROM decisions 
        WHERE decision_source = 'SHADOW' 
          AND created_at > NOW() - INTERVAL '1 hour';
    " | tr -d ' ')
    
    if [ "$count" -eq 0 ]; then
        log "${YELLOW}⚠️  警告：过去 1 小时内没有 SHADOW 决策记录${NC}"
        log "${YELLOW}   请确认：${NC}"
        log "${YELLOW}   1. BanditEngine 已启用 SHADOW 模式${NC}"
        log "${YELLOW}   2. 决策已正确记录到数据库${NC}"
    else
        log "${GREEN}✓ SHADOW 模式正常运行 (${count} 决策/小时)${NC}"
    fi
}

# 生成实时统计报告
generate_realtime_report() {
    log "${BLUE}═══════════════════════════════════════════════════════════════${NC}"
    log "${BLUE}                    实时统计报告                              ${NC}"
    log "${BLUE}═══════════════════════════════════════════════════════════════${NC}"
    
    # 总决策数
    total=$(psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" -t -c "
        SELECT COUNT(*) 
        FROM decisions 
        WHERE created_at > NOW() - INTERVAL '24 hours';
    " | tr -d ' ')
    
    log "过去 24 小时总决策数：${GREEN}$total${NC}"
    
    # 决策来源分布
    log "${YELLOW}【决策来源分布】${NC}"
    psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" -c "
        SELECT 
            decision_source as 来源，
            COUNT(*) as 数量，
            ROUND(COUNT(*) * 100.0 / SUM(COUNT(*)) OVER (), 2) as 百分比
        FROM decisions 
        WHERE created_at > NOW() - INTERVAL '24 hours'
        GROUP BY decision_source
        ORDER BY 数量 DESC;
    "
    
    # Guardrail 触发统计
    log "${YELLOW}【Guardrail 触发统计】${NC}"
    psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" -c "
        SELECT 
            CASE 
                WHEN reason LIKE '%HIGH_RISK%' THEN '高风险'
                WHEN reason LIKE '%ILLEGAL_LOCATION%' THEN '非法位置'
                WHEN reason LIKE '%LOW_CONFIDENCE%' THEN '低置信度'
                ELSE '其他'
            END as 触发类型，
            COUNT(*) as 触发次数
        FROM decisions 
        WHERE decision_source = 'GUARDRAIL'
          AND created_at > NOW() - INTERVAL '24 hours'
        GROUP BY 1
        ORDER BY 触发次数 DESC;
    "
    
    # 置信度分布
    log "${YELLOW}【置信度分布】${NC}"
    psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" -c "
        SELECT 
            CASE 
                WHEN confidence >= 0.8 THEN '高 (>=0.8)'
                WHEN confidence >= 0.6 THEN '中 (0.6-0.8)'
                WHEN confidence >= 0.4 THEN '低 (0.4-0.6)'
                ELSE '极低 (<0.4)'
            END as 置信度等级，
            COUNT(*) as 数量，
            ROUND(AVG(confidence), 4) as 平均置信度
        FROM decisions 
        WHERE confidence > 0
          AND created_at > NOW() - INTERVAL '24 hours'
        GROUP BY 1
        ORDER BY 数量 DESC;
    "
    
    # 不确定性分布
    log "${YELLOW}【不确定性分布】${NC}"
    psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" -c "
        SELECT 
            CASE 
                WHEN uncertainty >= 1000 THEN '极高 (>=1000)'
                WHEN uncertainty >= 500 THEN '高 (500-1000)'
                WHEN uncertainty >= 100 THEN '中 (100-500)'
                ELSE '低 (<100)'
            END as 不确定性等级，
            COUNT(*) as 数量，
            ROUND(AVG(uncertainty), 2) as 平均不确定性
        FROM decisions 
        WHERE uncertainty > 0
          AND created_at > NOW() - INTERVAL '24 hours'
        GROUP BY 1
        ORDER BY 数量 DESC;
    "
}

# 生成一致性分析报告
generate_consistency_report() {
    log "${BLUE}═══════════════════════════════════════════════════════════════${NC}"
    log "${BLUE}                  一致性分析报告                              ${NC}"
    log "${BLUE}═══════════════════════════════════════════════════════════════${NC}"
    
    # 计算一致性（需要 rule_based_action 字段）
    psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" -c "
        SELECT 
            CASE 
                WHEN action_type = rule_based_action THEN '一致'
                ELSE '不同'
            END as 一致性状态，
            COUNT(*) as 数量，
            ROUND(COUNT(*) * 100.0 / SUM(COUNT(*)) OVER (), 2) as 百分比
        FROM decisions 
        WHERE decision_source IN ('BANDIT', 'SHADOW')
          AND rule_based_action IS NOT NULL
          AND created_at > NOW() - INTERVAL '24 hours'
        GROUP BY 1
        ORDER BY 数量 DESC;
    "
    
    # 按资产类型分析一致性
    log "${YELLOW}【按资产类型分析一致性】${NC}"
    psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" -c "
        SELECT 
            asset_type as 资产类型，
            COUNT(*) as 总决策数，
            SUM(CASE WHEN action_type = rule_based_action THEN 1 ELSE 0 END) as 一致数，
            ROUND(SUM(CASE WHEN action_type = rule_based_action THEN 1 ELSE 0 END) * 100.0 / COUNT(*), 2) as 一致率
        FROM decisions 
        WHERE decision_source IN ('BANDIT', 'SHADOW')
          AND rule_based_action IS NOT NULL
          AND created_at > NOW() - INTERVAL '24 hours'
        GROUP BY 1
        ORDER BY 总决策数 DESC;
    "
    
    # 按风险等级分析一致性
    log "${YELLOW}【按风险等级分析一致性】${NC}"
    psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" -c "
        SELECT 
            CASE 
                WHEN missing_risk >= 0.8 THEN '高风险 (>=0.8)'
                WHEN missing_risk >= 0.5 THEN '中风险 (0.5-0.8)'
                ELSE '低风险 (<0.5)'
            END as 风险等级，
            COUNT(*) as 总决策数，
            SUM(CASE WHEN action_type = rule_based_action THEN 1 ELSE 0 END) as 一致数，
            ROUND(SUM(CASE WHEN action_type = rule_based_action THEN 1 ELSE 0 END) * 100.0 / COUNT(*), 2) as 一致率
        FROM decisions 
        WHERE decision_source IN ('BANDIT', 'SHADOW')
          AND rule_based_action IS NOT NULL
          AND created_at > NOW() - INTERVAL '24 hours'
        GROUP BY 1
        ORDER BY 总决策数 DESC;
    "
}

# 导出 CSV 报告
export_csv_report() {
    local timestamp=$(date '+%Y%m%d_%H%M%S')
    local csv_file="$REPORT_DIR/shadow_report_$timestamp.csv"
    
    log "${YELLOW}导出 CSV 报告：$csv_file${NC}"
    
    psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" -c "
        COPY (
            SELECT 
                id,
                asset_id,
                decision_source,
                action_type,
                rule_based_action,
                confidence,
                uncertainty,
                created_at
            FROM decisions 
            WHERE created_at > NOW() - INTERVAL '24 hours'
            ORDER BY created_at DESC
        ) TO STDOUT WITH CSV HEADER
    " > "$csv_file"
    
    log "${GREEN}✓ CSV 报告已导出：$csv_file${NC}"
}

# 生成告警
generate_alerts() {
    log "${YELLOW}检查告警条件...${NC}"
    
    # 检查 Guardrail 高风险触发率
    high_risk_rate=$(psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" -t -c "
        SELECT COUNT(*) 
        FROM decisions 
        WHERE reason LIKE '%HIGH_RISK%'
          AND created_at > NOW() - INTERVAL '1 hour';
    " | tr -d ' ')
    
    if [ "$high_risk_rate" -gt 100 ]; then
        log "${RED}⚠️  告警：过去 1 小时内高风险触发 $high_risk_rate 次${NC}"
    fi
    
    # 检查一致性是否过低
    consistency_rate=$(psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" -t -c "
        SELECT ROUND(SUM(CASE WHEN action_type = rule_based_action THEN 1 ELSE 0 END) * 100.0 / COUNT(*), 2)
        FROM decisions 
        WHERE decision_source IN ('BANDIT', 'SHADOW')
          AND rule_based_action IS NOT NULL
          AND created_at > NOW() - INTERVAL '24 hours';
    " | tr -d ' ')
    
    if [ -n "$consistency_rate" ] && [ "$consistency_rate" -lt 50 ]; then
        log "${RED}⚠️  告警：一致性率过低 (${consistency_rate}%)${NC}"
    fi
}

# 主循环
main() {
    check_db
    
    # 单次运行模式
    if [ "$1" = "--once" ]; then
        check_shadow_mode
        generate_realtime_report
        generate_consistency_report
        export_csv_report
        generate_alerts
        exit 0
    fi
    
    # 持续监控模式（每 5 分钟更新一次）
    log "${GREEN}启动持续监控模式（每 5 分钟更新）...${NC}"
    log "${YELLOW}按 Ctrl+C 停止${NC}"
    
    while true; do
        clear
        check_shadow_mode
        generate_realtime_report
        generate_consistency_report
        generate_alerts
        
        log "${BLUE}下次更新：5 分钟后...${NC}"
        sleep 300
    done
}

# 显示帮助
show_help() {
    echo "SHADOW 模式监控工具"
    echo ""
    echo "用法：$0 [选项]"
    echo ""
    echo "选项:"
    echo "  --once      单次运行，不循环"
    echo "  --help      显示此帮助信息"
    echo ""
    echo "环境变量:"
    echo "  DB_HOST     数据库主机 (默认：localhost)"
    echo "  DB_NAME     数据库名 (默认：rfid_db)"
    echo "  DB_USER     数据库用户 (默认：rfid_user)"
    echo "  LOG_FILE    日志文件 (默认：shadow_monitor.log)"
    echo "  REPORT_DIR  报告目录 (默认：./shadow_reports)"
}

# 解析参数
case "$1" in
    --help|-h)
        show_help
        exit 0
        ;;
    *)
        main "$@"
        ;;
esac
