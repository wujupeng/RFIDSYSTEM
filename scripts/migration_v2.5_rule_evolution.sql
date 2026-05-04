CREATE TABLE IF NOT EXISTS decision_rules (
    id SERIAL PRIMARY KEY,

    rule_name VARCHAR(100) NOT NULL,
    rule_version VARCHAR(50) NOT NULL,

    rule_type VARCHAR(50),

    rule_json JSONB NOT NULL,

    description TEXT,
    enabled BOOLEAN DEFAULT TRUE,

    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP
);

CREATE TABLE IF NOT EXISTS rule_changes (
    id SERIAL PRIMARY KEY,

    from_version VARCHAR(50),
    to_version VARCHAR(50) NOT NULL,

    diff_summary TEXT,
    changed_rules JSONB,

    changed_by VARCHAR(100),
    change_reason TEXT,

    created_at TIMESTAMP DEFAULT NOW()
);

ALTER TABLE decision_snapshots
ADD COLUMN IF NOT EXISTS rule_snapshot_json JSONB;

CREATE INDEX IF NOT EXISTS idx_rules_name_version
ON decision_rules (rule_name, rule_version);

CREATE INDEX IF NOT EXISTS idx_rules_type
ON decision_rules (rule_type);

CREATE INDEX IF NOT EXISTS idx_rule_changes_version
ON rule_changes (to_version);
