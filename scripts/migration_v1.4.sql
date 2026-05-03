-- v1.4 轨迹分析与异常检测

-- 创建资产位置历史表
CREATE TABLE IF NOT EXISTS asset_location_history (
    id SERIAL PRIMARY KEY,
    asset_id INT NOT NULL REFERENCES assets(id),
    epc VARCHAR(24) NOT NULL,
    location VARCHAR(100) NOT NULL,
    previous_location VARCHAR(100),
    entry_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    exit_time TIMESTAMP,
    duration_seconds INT DEFAULT 0,
    source_reader VARCHAR(100),
    confidence NUMERIC(5,2) DEFAULT 1.0,
    event_type VARCHAR(50) DEFAULT 'SCAN'
);

-- 创建索引
CREATE INDEX IF NOT EXISTS idx_location_history_asset ON asset_location_history(asset_id);
CREATE INDEX IF NOT EXISTS idx_location_history_epc ON asset_location_history(epc);
CREATE INDEX IF NOT EXISTS idx_location_history_location ON asset_location_history(location);
CREATE INDEX IF NOT EXISTS idx_location_history_time ON asset_location_history(entry_time);

-- 创建异常事件表
CREATE TABLE IF NOT EXISTS anomaly_events (
    id SERIAL PRIMARY KEY,
    asset_id INT REFERENCES assets(id),
    epc VARCHAR(24) NOT NULL,
    anomaly_type VARCHAR(50) NOT NULL,
    severity VARCHAR(20) NOT NULL DEFAULT 'WARNING',
    message TEXT NOT NULL,
    location VARCHAR(100),
    detected_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    resolved_at TIMESTAMP,
    resolved_by VARCHAR(100),
    is_resolved BOOLEAN DEFAULT FALSE
);

-- 创建索引
CREATE INDEX IF NOT EXISTS idx_anomaly_asset ON anomaly_events(asset_id);
CREATE INDEX IF NOT EXISTS idx_anomaly_epc ON anomaly_events(epc);
CREATE INDEX IF NOT EXISTS idx_anomaly_type ON anomaly_events(anomaly_type);
CREATE INDEX IF NOT EXISTS idx_anomaly_resolved ON anomaly_events(is_resolved);

-- 创建行为统计视图
CREATE VIEW IF NOT EXISTS asset_behavior_stats AS
SELECT
    asset_id,
    epc,
    COUNT(DISTINCT location) AS unique_locations,
    COUNT(*) AS total_moves,
    AVG(duration_seconds) AS avg_stay_duration,
    MAX(duration_seconds) AS max_stay_duration,
    MIN(duration_seconds) AS min_stay_duration,
    FIRST_VALUE(location) OVER (PARTITION BY asset_id ORDER BY entry_time) AS first_location,
    LAST_VALUE(location) OVER (PARTITION BY asset_id ORDER BY entry_time) AS last_location
FROM asset_location_history
GROUP BY asset_id, epc;

COMMIT;