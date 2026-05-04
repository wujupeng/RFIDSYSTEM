CREATE TABLE IF NOT EXISTS decisions (
    id SERIAL PRIMARY KEY,

    asset_id INT NOT NULL,
    asset_name VARCHAR(255),
    location VARCHAR(100),

    action VARCHAR(50) NOT NULL,
    risk_level VARCHAR(20),

    reason TEXT,

    executed BOOLEAN DEFAULT FALSE,
    ignored BOOLEAN DEFAULT FALSE,

    operator_name VARCHAR(100),

    created_at TIMESTAMP NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_decisions_created_at
ON decisions (created_at DESC);

CREATE INDEX IF NOT EXISTS idx_decisions_asset_id
ON decisions (asset_id);

CREATE INDEX IF NOT EXISTS idx_decisions_execution
ON decisions (executed, ignored);

CREATE INDEX IF NOT EXISTS idx_decisions_created_asset
ON decisions (created_at DESC, asset_id);

CREATE INDEX IF NOT EXISTS idx_decisions_recent
ON decisions (created_at)
WHERE created_at > NOW() - INTERVAL '7 days';
