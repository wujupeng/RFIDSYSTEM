-- v3.3.1 可观测性系统数据库升级

-- 1. 补全 health_logs 表字段
ALTER TABLE health_logs 
ADD COLUMN component VARCHAR(50),
ADD COLUMN status VARCHAR(20),  -- OK / WARN / ERROR
ADD COLUMN latency_ms INT,
ADD COLUMN error_message TEXT;

-- 2. 新增 system_metrics 表
CREATE TABLE system_metrics (
    id SERIAL PRIMARY KEY,
    metric_name VARCHAR(100),
    metric_value DOUBLE PRECISION,
    tags JSONB,
    created_at TIMESTAMP DEFAULT NOW()
);

-- 3. 新增 bandit_logs 表
CREATE TABLE bandit_logs (
    id SERIAL PRIMARY KEY,
    asset_id INT,
    action VARCHAR(50),
    score DOUBLE PRECISION,
    uncertainty DOUBLE PRECISION,
    confidence DOUBLE PRECISION,
    created_at TIMESTAMP DEFAULT NOW()
);

-- 4. 创建索引
CREATE INDEX IF NOT EXISTS idx_system_metrics_name ON system_metrics(metric_name);
CREATE INDEX IF NOT EXISTS idx_system_metrics_time ON system_metrics(created_at);
CREATE INDEX IF NOT EXISTS idx_bandit_logs_asset_id ON bandit_logs(asset_id);
CREATE INDEX IF NOT EXISTS idx_bandit_logs_action ON bandit_logs(action);
CREATE INDEX IF NOT EXISTS idx_bandit_logs_time ON bandit_logs(created_at);
CREATE INDEX IF NOT EXISTS idx_health_logs_component ON health_logs(component);
CREATE INDEX IF NOT EXISTS idx_health_logs_time ON health_logs(created_at);

-- 5. 决策反馈表（用于Adoption Rate计算）
CREATE TABLE IF NOT EXISTS decision_feedback (
    id SERIAL PRIMARY KEY,
    decision_id INT,
    asset_id INT,
    executed BOOLEAN DEFAULT FALSE,
    ignored BOOLEAN DEFAULT FALSE,
    user_id INT,
    feedback_time TIMESTAMP DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_decision_feedback_decision_id ON decision_feedback(decision_id);
CREATE INDEX IF NOT EXISTS idx_decision_feedback_asset_id ON decision_feedback(asset_id);
CREATE INDEX IF NOT EXISTS idx_decision_feedback_time ON decision_feedback(feedback_time);
