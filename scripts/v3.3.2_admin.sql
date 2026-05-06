-- 用户表
CREATE TABLE IF NOT EXISTS users (
    id SERIAL PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    password VARCHAR(255) NOT NULL,
    email VARCHAR(100) UNIQUE,
    name VARCHAR(100),
    role_id INT REFERENCES roles(id),
    active BOOLEAN DEFAULT true,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);

-- 角色表
CREATE TABLE IF NOT EXISTS roles (
    id SERIAL PRIMARY KEY,
    name VARCHAR(50) UNIQUE NOT NULL,
    description VARCHAR(255),
    active BOOLEAN DEFAULT true,
    created_at TIMESTAMP DEFAULT NOW()
);

-- 权限表
CREATE TABLE IF NOT EXISTS permissions (
    id SERIAL PRIMARY KEY,
    permission_id VARCHAR(50) UNIQUE NOT NULL,
    name VARCHAR(100),
    description VARCHAR(255),
    category VARCHAR(50)
);

-- 角色权限关联表
CREATE TABLE IF NOT EXISTS role_permissions (
    role_id INT REFERENCES roles(id),
    permission_id VARCHAR(50) REFERENCES permissions(permission_id),
    PRIMARY KEY (role_id, permission_id)
);

-- 插入默认角色
INSERT INTO roles (name, description) VALUES 
('超级管理员', '系统最高权限管理员'),
('资产管理员', '资产管理相关权限'),
('运维人员', '系统运维监控权限'),
('普通用户', '基础操作权限')
ON CONFLICT DO NOTHING;

-- 插入默认权限
INSERT INTO permissions (permission_id, name, description, category) VALUES 
('asset.view', '查看资产', '资产模块', 'asset'),
('asset.create', '创建资产', '资产模块', 'asset'),
('asset.edit', '编辑资产', '资产模块', 'asset'),
('asset.delete', '删除资产', '资产模块', 'asset'),
('inventory.view', '查看盘点', '盘点模块', 'inventory'),
('inventory.create', '创建盘点', '盘点模块', 'inventory'),
('repair.view', '查看维修', '维修模块', 'repair'),
('repair.create', '创建维修', '维修模块', 'repair'),
('repair.approve', '审批维修', '维修模块', 'repair'),
('admin.user.view', '查看用户', '管理模块', 'admin'),
('admin.user.create', '创建用户', '管理模块', 'admin'),
('admin.user.edit', '编辑用户', '管理模块', 'admin'),
('admin.user.delete', '删除用户', '管理模块', 'admin'),
('admin.role.view', '查看角色', '管理模块', 'admin'),
('admin.role.create', '创建角色', '管理模块', 'admin'),
('admin.role.edit', '编辑角色', '管理模块', 'admin'),
('admin.role.delete', '删除角色', '管理模块', 'admin'),
('admin.settings', '系统设置', '管理模块', 'admin'),
('monitoring.view', '查看监控', '监控模块', 'monitoring'),
('report.view', '查看报表', '报表模块', 'report')
ON CONFLICT DO NOTHING;

-- 插入默认超级管理员用户
INSERT INTO users (username, password, email, name, role_id) 
SELECT 'admin', 'admin123', 'admin@example.com', '超级管理员', id 
FROM roles WHERE name = '超级管理员'
ON CONFLICT DO NOTHING;
