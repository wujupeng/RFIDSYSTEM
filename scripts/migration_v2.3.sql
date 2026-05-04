-- v2.3: 预测、评分、决策系统

-- 资产预测记录表
CREATE TABLE IF NOT EXISTS asset_predictions (
    id SERIAL PRIMARY KEY,
    asset_id INTEGER NOT NULL,
    epc VARCHAR(24) NOT NULL,
    predicted_status VARCHAR(50) NOT NULL,
    missing_risk NUMERIC(5,2) DEFAULT 0,
    inactivity_risk NUMERIC(5,2) DEFAULT 0,
    abnormal_risk NUMERIC(5,2) DEFAULT 0,
    total_risk NUMERIC(5,2) DEFAULT 0,
    usage_forecast NUMERIC(10,2) DEFAULT 0,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- 索引
CREATE INDEX IF NOT EXISTS idx_asset_predictions_asset_id ON asset_predictions(asset_id);
CREATE INDEX IF NOT EXISTS idx_asset_predictions_epc ON asset_predictions(epc);
CREATE INDEX IF NOT EXISTS idx_asset_predictions_status ON asset_predictions(predicted_status);
CREATE INDEX IF NOT EXISTS idx_asset_predictions_created ON asset_predictions(created_at);

-- 资产评分记录表
CREATE TABLE IF NOT EXISTS asset_scores (
    id SERIAL PRIMARY KEY,
    asset_id INTEGER NOT NULL,
    epc VARCHAR(24) NOT NULL,
    health_score NUMERIC(5,2) NOT NULL,
    risk_score NUMERIC(5,2) NOT NULL,
    risk_level VARCHAR(20) NOT NULL,
    risk_summary TEXT,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- 索引
CREATE INDEX IF NOT EXISTS idx_asset_scores_asset_id ON asset_scores(asset_id);
CREATE INDEX IF NOT EXISTS idx_asset_scores_epc ON asset_scores(epc);
CREATE INDEX IF NOT EXISTS idx_asset_scores_level ON asset_scores(risk_level);
CREATE INDEX IF NOT EXISTS idx_asset_scores_created ON asset_scores(created_at);

-- 决策记录表
CREATE TABLE IF NOT EXISTS decisions (
    id SERIAL PRIMARY KEY,
    asset_id INTEGER NOT NULL,
    asset_name VARCHAR(255) NOT NULL,
    action_type VARCHAR(50) NOT NULL,
    action VARCHAR(255) NOT NULL,
    reason TEXT NOT NULL,
    priority INTEGER NOT NULL DEFAULT 5,
    current_location VARCHAR(100),
    suggested_location VARCHAR(100),
    is_resolved BOOLEAN DEFAULT FALSE,
    resolved_at TIMESTAMP,
    resolved_by VARCHAR(100),
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- 索引
CREATE INDEX IF NOT EXISTS idx_decisions_asset_id ON decisions(asset_id);
CREATE INDEX IF NOT EXISTS idx_decisions_priority ON decisions(priority);
CREATE INDEX IF NOT EXISTS idx_decisions_resolved ON decisions(is_resolved);
CREATE INDEX IF NOT EXISTS idx_decisions_created ON decisions(created_at);

-- 自动任务表（未来扩展）
CREATE TABLE IF NOT EXISTS auto_tasks (
    id SERIAL PRIMARY KEY,
    task_type VARCHAR(50) NOT NULL,
    asset_id INTEGER,
    decision_id INTEGER REFERENCES decisions(id),
    status VARCHAR(50) NOT NULL DEFAULT 'PENDING',
    assigned_to VARCHAR(100),
    location VARCHAR(100),
    description TEXT,
    scheduled_at TIMESTAMP,
    completed_at TIMESTAMP,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- 索引
CREATE INDEX IF NOT EXISTS idx_auto_tasks_type ON auto_tasks(task_type);
CREATE INDEX IF NOT EXISTS idx_auto_tasks_status ON auto_tasks(status);
CREATE INDEX IF NOT EXISTS idx_auto_tasks_asset ON auto_tasks(asset_id);
CREATE INDEX IF NOT EXISTS idx_auto_tasks_created ON auto_tasks(created_at);

COMMIT;