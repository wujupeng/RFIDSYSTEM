-- RFID Asset Management System Database Schema v1.0

-- Assets table
CREATE TABLE IF NOT EXISTS assets (
    id SERIAL PRIMARY KEY,
    name TEXT NOT NULL,
    type TEXT NOT NULL,
    asset_code TEXT UNIQUE NOT NULL,
    rfid_epc TEXT UNIQUE,
    location TEXT,
    status TEXT DEFAULT 'IN_STOCK',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Unique index for RFID EPC
CREATE UNIQUE INDEX IF NOT EXISTS idx_assets_rfid_epc ON assets(rfid_epc);

-- Status index for faster queries
CREATE INDEX IF NOT EXISTS idx_assets_status ON assets(status);

-- Operation logs table
CREATE TABLE IF NOT EXISTS operation_logs (
    id SERIAL PRIMARY KEY,
    asset_id INT REFERENCES assets(id),
    operation_type TEXT NOT NULL,
    operator TEXT NOT NULL,
    old_status TEXT,
    new_status TEXT,
    remark TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Inventory tasks table
CREATE TABLE IF NOT EXISTS inventory_tasks (
    id SERIAL PRIMARY KEY,
    task_name TEXT NOT NULL,
    status TEXT DEFAULT 'PENDING',
    scanned_count INT DEFAULT 0,
    found_count INT DEFAULT 0,
    missing_count INT DEFAULT 0,
    extra_count INT DEFAULT 0,
    location TEXT,
    operator TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    completed_at TIMESTAMP
);

-- Inventory results table
CREATE TABLE IF NOT EXISTS inventory_results (
    id SERIAL PRIMARY KEY,
    task_id INT REFERENCES inventory_tasks(id),
    epc TEXT NOT NULL,
    asset_id INT REFERENCES assets(id),
    status TEXT NOT NULL,
    reason TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Index for inventory task queries
CREATE INDEX IF NOT EXISTS idx_inventory_results_task_id ON inventory_results(task_id);
CREATE INDEX IF NOT EXISTS idx_inventory_results_epc ON inventory_results(epc);

-- RBAC: Users table
CREATE TABLE IF NOT EXISTS users (
    id SERIAL PRIMARY KEY,
    username TEXT UNIQUE NOT NULL,
    password_hash TEXT NOT NULL,
    real_name TEXT,
    email TEXT,
    status TEXT DEFAULT 'ACTIVE',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- RBAC: Roles table
CREATE TABLE IF NOT EXISTS roles (
    id SERIAL PRIMARY KEY,
    name TEXT UNIQUE NOT NULL,
    description TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- RBAC: User-Role mapping
CREATE TABLE IF NOT EXISTS user_roles (
    id SERIAL PRIMARY KEY,
    user_id INT REFERENCES users(id) ON DELETE CASCADE,
    role_id INT REFERENCES roles(id) ON DELETE CASCADE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(user_id, role_id)
);

-- RBAC: Permissions table
CREATE TABLE IF NOT EXISTS permissions (
    id SERIAL PRIMARY KEY,
    name TEXT UNIQUE NOT NULL,
    resource TEXT NOT NULL,
    action TEXT NOT NULL,
    description TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- RBAC: Role-Permission mapping
CREATE TABLE IF NOT EXISTS role_permissions (
    id SERIAL PRIMARY KEY,
    role_id INT REFERENCES roles(id) ON DELETE CASCADE,
    permission_id INT REFERENCES permissions(id) ON DELETE CASCADE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(role_id, permission_id)
);

-- RBAC: Default permissions
INSERT INTO permissions (name, resource, action, description) VALUES
    ('asset:create', 'asset', 'create', 'Create new assets'),
    ('asset:read', 'asset', 'read', 'View asset details'),
    ('asset:update', 'asset', 'update', 'Update asset information'),
    ('asset:delete', 'asset', 'delete', 'Delete assets'),
    ('inventory:create', 'inventory', 'create', 'Create inventory tasks'),
    ('inventory:start', 'inventory', 'start', 'Start inventory scanning'),
    ('inventory:view', 'inventory', 'view', 'View inventory results'),
    ('user:create', 'user', 'create', 'Create users'),
    ('user:manage', 'user', 'manage', 'Manage user accounts')
ON CONFLICT (name) DO NOTHING;

-- RBAC: Default roles
INSERT INTO roles (name, description) VALUES
    ('admin', 'System administrator with full access'),
    ('operator', 'Warehouse operator with inventory access'),
    ('viewer', 'Read-only access to view assets and inventory')
ON CONFLICT (name) DO NOTHING;

-- RBAC: Admin role gets all permissions
INSERT INTO role_permissions (role_id, permission_id)
SELECT r.id, p.id FROM roles r, permissions p WHERE r.name = 'admin'
ON CONFLICT DO NOTHING;

-- RBAC: Operator role gets asset and inventory permissions
INSERT INTO role_permissions (role_id, permission_id)
SELECT r.id, p.id FROM roles r, permissions p
WHERE r.name = 'operator' AND p.name IN ('asset:create', 'asset:read', 'asset:update', 'inventory:create', 'inventory:start', 'inventory:view')
ON CONFLICT DO NOTHING;

-- RBAC: Viewer role gets read-only permissions
INSERT INTO role_permissions (role_id, permission_id)
SELECT r.id, p.id FROM roles r, permissions p
WHERE r.name = 'viewer' AND p.name IN ('asset:read', 'inventory:view')
ON CONFLICT DO NOTHING;

-- Inventory snapshots: lock asset list at task start
CREATE TABLE IF NOT EXISTS inventory_snapshots (
    id SERIAL PRIMARY KEY,
    task_id INT REFERENCES inventory_tasks(id) ON DELETE CASCADE,
    asset_id INT REFERENCES assets(id),
    rfid_epc TEXT,
    status TEXT,
    location TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_inventory_snapshots_task_id ON inventory_snapshots(task_id);

-- System health log
CREATE TABLE IF NOT EXISTS health_logs (
    id SERIAL PRIMARY KEY,
    status TEXT NOT NULL,
    service TEXT NOT NULL,
    message TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Audit logs table - Enterprise compliance requirement
CREATE TABLE IF NOT EXISTS audit_logs (
    id SERIAL PRIMARY KEY,
    user_id INT REFERENCES users(id),
    action TEXT NOT NULL,
    target_type TEXT NOT NULL,
    target_id INT,
    result TEXT NOT NULL,
    ip_address TEXT,
    details TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Index for audit log queries
CREATE INDEX IF NOT EXISTS idx_audit_logs_user_id ON audit_logs(user_id);
CREATE INDEX IF NOT EXISTS idx_audit_logs_action ON audit_logs(action);
CREATE INDEX IF NOT EXISTS idx_audit_logs_target ON audit_logs(target_type, target_id);
CREATE INDEX IF NOT EXISTS idx_audit_logs_created_at ON audit_logs(created_at);