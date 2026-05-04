CREATE TABLE IF NOT EXISTS decision_snapshots (
    id SERIAL PRIMARY KEY,

    decision_id INT REFERENCES decisions(id),

    asset_id INT NOT NULL,

    risk_missing DOUBLE PRECISION,
    risk_inactivity DOUBLE PRECISION,
    risk_abnormal DOUBLE PRECISION,

    score INT,

    rule_version VARCHAR(50),
    threshold_snapshot JSONB,
    engine_version VARCHAR(50),

    created_at TIMESTAMP DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_snapshots_decision_id
ON decision_snapshots (decision_id);

CREATE INDEX IF NOT EXISTS idx_snapshots_asset_id
ON decision_snapshots (asset_id);

CREATE INDEX IF NOT EXISTS idx_snapshots_created_at
ON decision_snapshots (created_at DESC);
