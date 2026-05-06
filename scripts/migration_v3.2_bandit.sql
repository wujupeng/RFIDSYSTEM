-- v3.2 Contextual Bandit 数据库迁移
-- 执行顺序: psql rfid -f scripts/migration_v3.2_bandit.sql

-- Bandit 决策日志表
CREATE TABLE IF NOT EXISTS bandit_logs (
    id SERIAL PRIMARY KEY,
    decision_id INT REFERENCES decisions(id),
    context JSONB NOT NULL,
    action VARCHAR(50) NOT NULL,
    reward DOUBLE PRECISION,
    mode VARCHAR(20) NOT NULL,  -- SHADOW, SUGGESTION, AUTO
    created_at TIMESTAMP DEFAULT NOW()
);

-- Bandit 模型持久化表
CREATE TABLE IF NOT EXISTS bandit_models (
    id SERIAL PRIMARY KEY,
    action VARCHAR(50) NOT NULL UNIQUE,
    A_matrix JSONB NOT NULL,
    b_vector JSONB NOT NULL,
    sample_count INT DEFAULT 0,
    last_updated TIMESTAMP DEFAULT NOW()
);

-- Bandit 延迟奖励表
CREATE TABLE IF NOT EXISTS bandit_delayed_rewards (
    id SERIAL PRIMARY KEY,
    decision_id INT REFERENCES decisions(id),
    reward DOUBLE PRECISION NOT NULL,
    delay_hours INT NOT NULL,
    status VARCHAR(20) DEFAULT 'pending',  -- pending, processed, expired, cancelled
    trigger_time TIMESTAMP NOT NULL,
    created_at TIMESTAMP DEFAULT NOW(),
    processed_at TIMESTAMP
);

-- 扫描日志表（用于上下文特征计算）
CREATE TABLE IF NOT EXISTS scan_logs (
    id SERIAL PRIMARY KEY,
    asset_id INT REFERENCES assets(id),
    rfid_epc TEXT,
    location TEXT,
    rssi INT,
    scanned_at TIMESTAMP DEFAULT NOW(),
    reader_id TEXT
);

-- 位置规则表（用于判断非法位置）
CREATE TABLE IF NOT EXISTS location_rules (
    id SERIAL PRIMARY KEY,
    location TEXT NOT NULL,
    asset_type TEXT NOT NULL,
    is_allowed BOOLEAN NOT NULL DEFAULT TRUE,
    description TEXT,
    created_at TIMESTAMP DEFAULT NOW()
);

-- 创建索引
CREATE INDEX IF NOT EXISTS idx_bandit_logs_decision_id ON bandit_logs(decision_id);
CREATE INDEX IF NOT EXISTS idx_bandit_logs_created_at ON bandit_logs(created_at);
CREATE INDEX IF NOT EXISTS idx_bandit_models_action ON bandit_models(action);
CREATE INDEX IF NOT EXISTS idx_bandit_delayed_status ON bandit_delayed_rewards(status);
CREATE INDEX IF NOT EXISTS idx_bandit_delayed_trigger ON bandit_delayed_rewards(trigger_time);
CREATE INDEX IF NOT EXISTS idx_scan_logs_asset_id ON scan_logs(asset_id);
CREATE INDEX IF NOT EXISTS idx_scan_logs_scanned_at ON scan_logs(scanned_at);
CREATE INDEX IF NOT EXISTS idx_location_rules_location_type ON location_rules(location, asset_type);

-- 插入默认位置规则示例
INSERT INTO location_rules (location, asset_type, is_allowed, description) VALUES
('仓库A', 'IT', true, 'IT设备允许存放'),
('仓库A', 'EQ', true, '生产设备允许存放'),
('办公室', 'IT', true, 'IT设备允许使用'),
('办公室', 'EQ', false, '生产设备不允许在办公室'),
('维修区', 'IT', true, '维修区允许'),
('维修区', 'EQ', true, '维修区允许'),
('报废区', 'IT', false, '非报废设备不允许'),
('报废区', 'EQ', false, '非报废设备不允许')
ON CONFLICT DO NOTHING;

-- 更新资产表添加 is_backup 字段
ALTER TABLE assets ADD COLUMN IF NOT EXISTS is_backup BOOLEAN DEFAULT FALSE;

-- 创建视图：Bandit 统计视图
CREATE VIEW IF NOT EXISTS bandit_statistics AS
SELECT
    COUNT(*) as total_decisions,
    SUM(CASE WHEN mode = 'SHADOW' THEN 1 ELSE 0 END) as shadow_decisions,
    SUM(CASE WHEN mode = 'SUGGESTION' THEN 1 ELSE 0 END) as suggestion_decisions,
    SUM(CASE WHEN mode = 'AUTO' THEN 1 ELSE 0 END) as auto_decisions,
    AVG(reward) as avg_reward,
    SUM(CASE WHEN reward IS NOT NULL THEN 1 ELSE 0 END) as rewarded_decisions
FROM bandit_logs;