-- v1.3 数据质量增强

-- 创建脏数据表（隔离异常数据）
CREATE TABLE IF NOT EXISTS dirty_epcs (
    id SERIAL PRIMARY KEY,
    raw_epc TEXT NOT NULL,
    reason VARCHAR(255) NOT NULL,
    source_reader VARCHAR(100),
    timestamp TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- 创建索引
CREATE INDEX IF NOT EXISTS idx_dirty_epcs_timestamp ON dirty_epcs(timestamp);
CREATE INDEX IF NOT EXISTS idx_dirty_epcs_source ON dirty_epcs(source_reader);

-- 为inventory_scans添加唯一约束（幂等性保证）
ALTER TABLE IF EXISTS inventory_scans
ADD CONSTRAINT IF NOT EXISTS unique_task_epc UNIQUE (task_id, epc);

-- 创建扫描记录表（用于幂等性）
CREATE TABLE IF NOT EXISTS scan_records (
    id SERIAL PRIMARY KEY,
    task_id INT NOT NULL REFERENCES inventory_tasks(id),
    epc VARCHAR(24) NOT NULL,
    scan_count INT NOT NULL DEFAULT 1,
    first_scan_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    last_scan_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    source_reader VARCHAR(100),
    rssi INT,
    CONSTRAINT unique_task_epc_record UNIQUE (task_id, epc)
);

-- 创建索引
CREATE INDEX IF NOT EXISTS idx_scan_records_task ON scan_records(task_id);
CREATE INDEX IF NOT EXISTS idx_scan_records_epc ON scan_records(epc);

-- 更新inventory_tasks添加对账字段
ALTER TABLE IF EXISTS inventory_tasks
ADD COLUMN IF NOT EXISTS accuracy_rate NUMERIC(5,2),
ADD COLUMN IF NOT EXISTS profit_rate NUMERIC(5,2),
ADD COLUMN IF NOT EXISTS loss_rate NUMERIC(5,2),
ADD COLUMN IF NOT EXISTS total_expected INT DEFAULT 0;

COMMIT;