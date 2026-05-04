CREATE TABLE IF NOT EXISTS rule_impact_analysis (
    id SERIAL PRIMARY KEY,

    rule_version VARCHAR(50) NOT NULL,
    rule_name VARCHAR(100) NOT NULL,

    impact_score DOUBLE PRECISION,
    precision_delta DOUBLE PRECISION,
    recall_delta DOUBLE PRECISION,
    f1_delta DOUBLE PRECISION,

    adoption_rate_delta DOUBLE PRECISION,
    false_positive_delta DOUBLE PRECISION,
    false_negative_delta DOUBLE PRECISION,

    sample_size INT,
    confidence_interval DOUBLE PRECISION,

    analysis_summary TEXT,

    evaluated_at TIMESTAMP DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS decision_attribution (
    id SERIAL PRIMARY KEY,

    decision_id INT REFERENCES decisions(id),
    snapshot_id INT REFERENCES decision_snapshots(id),

    primary_rule VARCHAR(100),
    primary_rule_contribution DOUBLE PRECISION,

    contributing_rules JSONB,

    attribution_confidence DOUBLE PRECISION,

    created_at TIMESTAMP DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_impact_version
ON rule_impact_analysis (rule_version);

CREATE INDEX IF NOT EXISTS idx_impact_rule_name
ON rule_impact_analysis (rule_name);

CREATE INDEX IF NOT EXISTS idx_attribution_decision
ON decision_attribution (decision_id);
