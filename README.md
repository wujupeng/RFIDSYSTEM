# RFIDSYSTEM v3.1
# RFID 资产全生命周期 + 决策智能管理系统

> **v3.1 Bayesian Auto-Tuning** | RFID 驱动的 EAM + ITAM 融合平台，覆盖 IT 设备与生产设备的完整生命周期管理，具备 AI 决策能力。

---

## 版本历史

| 版本 | 日期 | 说明 |
|------|------|------|
| [v1.0](https://github.com/example/rfid-system/releases/tag/v1.0) | 2026-01 | 基础资产管理，入库/领用/查询 |
| [v2.0](https://github.com/example/rfid-system/releases/tag/v2.0) | 2026-02 | RFID 盘点功能，盘盈/盘亏对比 |
| [v2.3](https://github.com/example/rfid-system/releases/tag/v2.3) | 2026-03 | gRPC 通信层，Qt 客户端，决策引擎 |
| [v2.4](https://github.com/example/rfid-system/releases/tag/v2.4) | 2026-03 | 决策可解释层（Snapshot + Risk Breakdown） |
| [v2.5](https://github.com/example/rfid-system/releases/tag/v2.5) | 2026-04 | 规则演进系统（Rule Registry + Versioning） |
| [v2.6](https://github.com/example/rfid-system/releases/tag/v2.6) | 2026-04 | 规则影响分析（Rule Impact Analysis） |
| **[v3.0](https://github.com/example/rfid-system/releases/tag/v3.0)** | 2026-05 | **自动调参系统（Random/Grid/Gradient Search）** |
| **[v3.1](https://github.com/example/rfid-system/releases/tag/v3.1)** | 2026-05 | **Bayesian Auto-Tuning（高斯过程 + 获取函数）** |

---

## 系统架构总览

```
采购 → 入库 → 贴标(RFID) → 领用 → 使用 → 巡检 → 报修 → 维修 → 盘点 → 报废
                                    ↓
                              ┌─────────────┐
                              │  决策引擎   │ ← AI 核心
                              └─────────────┘
                                    ↓
                         ┌────────────────────────┐
                         │  Bayesian Auto-Tuner   │
                         │  (v3.1 新增)          │
                         └────────────────────────┘
```

---

### 整体架构（六层）

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        Decision Intelligence Platform                        │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                    Layer 6: AI 决策层 (v3.0+v3.1)                    │    │
│  │  ┌─────────────┐  ┌─────────────┐  ┌──────────────────────────┐   │    │
│  │  │Auto-Tuner   │  │Reward System│  │ Bayesian Optimizer (v3.1) │   │    │
│  │  │(随机搜索)   │  │(奖励函数)   │  │ Gaussian Process         │   │    │
│  │  └─────────────┘  └─────────────┘  └──────────────────────────┘   │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                     ↓                                       │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                    Layer 5: 规则演进层 (v2.5+v2.6)                    │    │
│  │  ┌─────────────┐  ┌─────────────┐  ┌──────────────────────────┐   │    │
│  │  │Rule Registry│  │Rule Version  │  │ Rule Impact Analyzer     │   │    │
│  │  │(规则注册)   │  │Control      │  │ (规则效果归因)           │   │    │
│  │  └─────────────┘  └─────────────┘  └──────────────────────────┘   │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                     ↓                                       │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                    Layer 4: 决策引擎层 (v2.3+v2.4)                    │    │
│  │  ┌─────────────┐  ┌─────────────┐  ┌──────────────────────────┐   │    │
│  │  │Decision     │  │Decision     │  │ Action Generator        │   │    │
│  │  │Engine       │  │Stability    │  │ (INSPECT/ALERT/IGNORE)  │   │    │
│  │  └─────────────┘  └─────────────┘  └──────────────────────────┘   │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                     ↓                                       │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                    Layer 3: 分析层                                    │    │
│  │  ┌─────────────┐  ┌─────────────┐  ┌──────────────────────────┐   │    │
│  │  │Trajectory   │  │Anomaly      │  │ Asset Predictor          │   │    │
│  │  │Engine       │  │Detector     │  │ (风险预测)               │   │    │
│  │  └─────────────┘  └─────────────┘  └──────────────────────────┘   │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                     ↓                                       │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                    Layer 2: RFID 核心层                              │    │
│  │  ┌─────────────┐  ┌─────────────┐  ┌──────────────────────────┐   │    │
│  │  │Reader       │  │EPC Buffer   │  │ EPC Validator           │   │    │
│  │  │Manager      │  │(去重/限流)  │  │ (格式验证)               │   │    │
│  │  └─────────────┘  └─────────────┘  └──────────────────────────┘   │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                     ↓                                       │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                    Layer 1: 设备层                                   │    │
│  │  ┌─────────────┐  ┌─────────────┐  ┌──────────────────────────┐   │    │
│  │  │固定读写器   │  │手持 PDA     │  │ UHF RFID Tag (Impinj)   │   │    │
│  │  └─────────────┘  └─────────────┘  └──────────────────────────┘   │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## 资产全生命周期流程

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        资产全生命周期闭环                                    │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  ┌──────────────┐     ┌──────────────┐     ┌──────────────┐              │
│  │  采购入库    │────▶│ RFID 写卡   │────▶│  领用出库    │              │
│  │  IN_STOCK   │     │ EPC 绑定    │     │  IN_USE     │              │
│  └──────────────┘     └──────────────┘     └──────┬───────┘              │
│                                                      │                     │
│                         ┌────────────────────────────┼────────────────┐    │
│                         ▼                            ▼                ▼    │
│                  ┌──────────────┐            ┌──────────────┐ ┌──────────┐ │
│                  │  定期巡检    │            │  故障报修    │ │ RFID盘点 │ │
│                  │  (决策建议)  │            │  REPAIR     │ │ (核心)   │ │
│                  └──────────────┘            └──────┬───────┘ └────┬─────┘ │
│                         │                          │              │       │
│                         └──────────────────────────┴──────────────┘       │
│                                          │                                  │
│                                          ▼                                  │
│                                 ┌──────────────┐                           │
│                                 │  报废处置    │                           │
│                                 │  SCRAPPED   │                           │
│                                 └──────────────┘                           │
│                                                                              │
│  ┌──────────────────────────────────────────────────────────────────────┐  │
│  │                         决策反馈闭环 (AI Core)                         │  │
│  │                                                                       │  │
│  │   RFID数据 ──▶ 决策引擎 ──▶ 系统建议 ──▶ 用户反馈 ──▶ 奖励评估      │  │
│  │                                            │               │          │  │
│  │                                            ▼               ▼          │  │
│  │                                    ┌─────────────┐ ┌────────────┐    │  │
│  │                                    │ Adoption    │ │ Bayesian   │    │  │
│  │                                    │ Rate        │ │ Auto-Tuner│    │  │
│  │                                    └─────────────┘ └─────┬──────┘    │  │
│  │                                                           │           │  │
│  │                                              参数自动优化 ◀───────────┘  │  │
│  └──────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## 核心功能模块

### v3.1 Bayesian Auto-Tuning 系统

```cpp
// 奖励函数
total_reward =
    0.4 × accuracy_score      // 准确率
  + 0.3 × adoption_rate        // 采纳率
  - 0.2 × false_positive_rate // 误报惩罚
  + 0.1 × stability_score     // 稳定性

// 可调参数（11个）
- missing_hours_threshold      [24-168h]
- inactivity_hours_threshold   [12-120h]
- abnormal_score_threshold     [0.3-0.95]
- cooldown_alert_minutes       [30-120min]
- missing_risk_weight          [0.1-0.8]
- alert_priority_threshold     [1.0-4.0]
// ... 共11个可调参数

// 调参策略
- random     : 随机搜索（v3.0）
- grid       : 网格搜索（v3.0）
- gradient   : 梯度上升（v3.0）
- bayesian   : 贝叶斯优化（v3.1 ⭐）
```

### 决策引擎 (Decision Engine v2.3)

```cpp
enum class ActionType {
    INSPECT,        // 立即巡检
    CHECK_USAGE,    // 检查使用情况
    SECURITY_ALERT, // 安全告警
    NO_ACTION       // 无需行动
};

// 决策考虑因素
- 丢失风险 (missing_risk)
- 闲置风险 (inactivity_risk)
- 异常行为风险 (abnormal_risk)
- 位置合法性 (illegal_location)
- 设备健康状态 (health_score)
```

### 规则演进系统 (Rule Evolution v2.5)

```cpp
// 规则版本控制
- 规则注册表 (RuleRegistry)
- 版本对比 (Rule Diff)
- 版本回滚 (Rollback)
- 规则变更历史 (rule_changes)

// 规则影响分析
- impact_score      : 规则影响分数
- precision_delta   : 精确率变化
- recall_delta      : 召回率变化
```

---

## 技术选型

| 层级 | 技术 |
|------|------|
| **客户端** | Qt 6 + C++17（Win11） |
| **服务端** | C++（gRPC）+ Debian 13 |
| **接口协议** | gRPC + Protobuf |
| **数据库** | PostgreSQL 15+ |
| **缓存** | Redis（进阶） |
| **AI 引擎** | Gaussian Process + Bayesian Optimization |
| **RFID 协议** | EPC Gen2 UHF 860–960MHz（Impinj） |

---

## 工程结构

```
rfidsystem/
├── server/                         # C++ 服务端
│   ├── main.cpp                    # 服务入口
│   ├── CMakeLists.txt
│   │
│   ├── core/                       # 核心组件
│   │   ├── logger.h/cpp            # 日志系统
│   │   ├── epc_buffer.h/cpp        # EPC 缓存/去重
│   │   ├── epc_validator.h/cpp    # EPC 格式验证
│   │   ├── rate_limiter.h/cpp      # 限流器
│   │   └── idempotency_manager.h  # 幂等管理
│   │
│   ├── rfid/                       # RFID 层
│   │   └── reader.h/cpp            # 读写器管理
│   │
│   ├── db/                         # 数据库层
│   │   └── db_pool.h/cpp           # 连接池
│   │
│   ├── analytics/                  # 分析层
│   │   ├── trajectory_engine.h     # 轨迹分析
│   │   ├── anomaly_detector.h      # 异常检测
│   │   ├── predictor/              # 预测模块
│   │   │   └── asset_predictor.h
│   │   ├── scoring/                # 评分模块
│   │   │   └── asset_score.h
│   │   └── decision/               # 决策模块
│   │       ├── decision_engine.h   # 决策引擎
│   │       ├── decision_stability.h# 决策稳定性
│   │       ├── action_generator.h  # 动作生成
│   │       └── rule_impact_analyzer.h # 规则影响分析
│   │
│   ├── repository/                  # 数据仓库层
│   │   ├── decision_repository.h   # 决策仓储
│   │   └── rule_registry.h         # 规则注册表
│   │
│   ├── services/                   # 业务服务
│   │   ├── asset_service.h         # 资产服务
│   │   ├── inventory_service.h     # 盘点服务
│   │   ├── auth_service.h          # 认证服务
│   │   └── audit_service.h         # 审计服务
│   │
│   ├── tuning/                      # ⭐ Auto-Tuning (v3.0+v3.1)
│   │   ├── auto_tuner.h/cpp         # 自动调参器
│   │   ├── parameter_space.h/cpp   # 参数空间
│   │   ├── reward_evaluator.h/cpp  # 奖励评估
│   │   ├── tuning_strategy.h/cpp   # 调参策略
│   │   └── bayes/                  # 贝叶斯优化
│   │       ├── gaussian_process.h  # 高斯过程
│   │       ├── surrogate_model.h   # 代理模型
│   │       ├── acquisition_function.h # 获取函数
│   │       └── bayesian_optimizer.h # 贝叶斯优化器
│   │
│   └── rpc/                         # gRPC 服务
│       ├── asset_service.h/cpp     # 资产服务
│       └── decision_service.h/cpp  # 决策服务
│
├── client/                         # Qt 客户端
│   ├── main.cpp
│   ├── CMakeLists.txt
│   ├── pages/                      # 页面
│   │   ├── dashboard/              # 仪表盘
│   │   ├── asset/                  # 资产管理
│   │   ├── inventory/              # 盘点
│   │   │   └── inventory_scan_page.h # 盘点扫描页
│   │   ├── recommendations/        # ⭐ 系统建议页
│   │   │   └── system_recommendations_page.h
│   │   └── repair/                 # 报修
│   └── network/                    # 网络层
│       └── grpc_client.h/cpp       # gRPC 客户端
│
├── proto/                          # gRPC 接口定义
│   ├── asset.proto                 # 资产服务
│   └── decision.proto              # 决策服务
│
└── scripts/                        # 脚本
    ├── init_db.sql                 # 初始化数据库
    ├── migration_*.sql             # 数据库迁移
    └── run_server.sh               # 启动脚本
```

---

## 核心数据库表

### 资产与操作

```sql
-- 资产主表
CREATE TABLE assets (
    id SERIAL PRIMARY KEY,
    asset_code TEXT UNIQUE NOT NULL,  -- IT-SZ-2026-000123
    rfid_epc TEXT UNIQUE,             -- RFID EPC 绑定
    name TEXT NOT NULL,
    type TEXT,                        -- IT / EQ
    status TEXT DEFAULT 'IN_STOCK',   -- IN_STOCK/IN_USE/REPAIR/SCRAPPED
    location TEXT,
    owner TEXT,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);

-- 操作日志
CREATE TABLE operation_logs (
    id SERIAL PRIMARY KEY,
    asset_id INT REFERENCES assets(id),
    operation_type TEXT NOT NULL,      -- 入库/领用/维修/盘点
    operator TEXT NOT NULL,
    old_status TEXT,
    new_status TEXT,
    remark TEXT,
    created_at TIMESTAMP DEFAULT NOW()
);

-- 盘点任务
CREATE TABLE inventory_tasks (
    id SERIAL PRIMARY KEY,
    task_name TEXT NOT NULL,
    status TEXT DEFAULT 'PENDING',     -- PENDING/IN_PROGRESS/COMPLETED
    scanned_count INT DEFAULT 0,
    found_count INT DEFAULT 0,
    missing_count INT DEFAULT 0,
    extra_count INT DEFAULT 0,
    location TEXT,
    operator TEXT,
    created_at TIMESTAMP DEFAULT NOW(),
    completed_at TIMESTAMP
);

-- 盘点结果
CREATE TABLE inventory_results (
    id SERIAL PRIMARY KEY,
    task_id INT REFERENCES inventory_tasks(id),
    epc TEXT NOT NULL,
    asset_id INT REFERENCES assets(id),
    status TEXT NOT NULL,              -- FOUND/MISSING/EXTRA
    reason TEXT,
    created_at TIMESTAMP DEFAULT NOW()
);
```

### 决策系统 (v2.3+)

```sql
-- 决策表
CREATE TABLE decisions (
    id SERIAL PRIMARY KEY,
    asset_id INT REFERENCES assets(id),
    action_type TEXT NOT NULL,        -- INSPECT/CHECK_USAGE/SECURITY_ALERT/NO_ACTION
    action_reason TEXT,
    priority INT,                      -- 1-5, 1为最高
    risk_level TEXT,                   -- HIGH/MEDIUM/LOW
    is_shadow_mode BOOLEAN DEFAULT FALSE,
    is_handled BOOLEAN DEFAULT FALSE,
    is_executed BOOLEAN DEFAULT FALSE,
    is_ignored BOOLEAN DEFAULT FALSE,
    created_at TIMESTAMP DEFAULT NOW(),
    handled_at TIMESTAMP
);

-- 决策快照 (v2.4 可解释层)
CREATE TABLE decision_snapshots (
    id SERIAL PRIMARY KEY,
    decision_id INT REFERENCES decisions(id),
    asset_id INT REFERENCES assets(id),
    risk_missing DOUBLE PRECISION,
    risk_inactivity DOUBLE PRECISION,
    risk_abnormal DOUBLE PRECISION,
    score INT,
    rule_version VARCHAR(50),
    threshold_snapshot JSONB,
    engine_version VARCHAR(50),
    created_at TIMESTAMP DEFAULT NOW()
);

-- 规则表 (v2.5 规则演进)
CREATE TABLE decision_rules (
    id SERIAL PRIMARY KEY,
    rule_name VARCHAR(100) UNIQUE NOT NULL,
    rule_version VARCHAR(50) NOT NULL,
    description TEXT,
    parameters JSONB,
    is_active BOOLEAN DEFAULT TRUE,
    created_at TIMESTAMP DEFAULT NOW(),
    activated_at TIMESTAMP
);

-- 规则变更历史
CREATE TABLE rule_changes (
    id SERIAL PRIMARY KEY,
    rule_id INT REFERENCES decision_rules(id),
    change_type VARCHAR(50),           -- CREATED/UPDATED/ACTIVATED/DEACTIVATED
    old_version VARCHAR(50),
    new_version VARCHAR(50),
    change_details JSONB,
    created_at TIMESTAMP DEFAULT NOW()
);

-- 规则影响分析 (v2.6)
CREATE TABLE rule_impact_analysis (
    id SERIAL PRIMARY KEY,
    rule_version VARCHAR(50) NOT NULL,
    rule_name VARCHAR(100) NOT NULL,
    impact_score DOUBLE PRECISION,
    precision_delta DOUBLE PRECISION,
    recall_delta DOUBLE PRECISION,
    f1_delta DOUBLE PRECISION,
    adoption_rate_delta DOUBLE PRECISION,
    evaluated_at TIMESTAMP DEFAULT NOW()
);
```

### RBAC 权限系统

```sql
-- 用户表
CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    username TEXT UNIQUE NOT NULL,
    password_hash TEXT NOT NULL,
    real_name TEXT,
    email TEXT,
    status TEXT DEFAULT 'ACTIVE',
    created_at TIMESTAMP DEFAULT NOW()
);

-- 角色表
CREATE TABLE roles (
    id SERIAL PRIMARY KEY,
    name TEXT UNIQUE NOT NULL,
    description TEXT
);

-- 权限表
CREATE TABLE permissions (
    id SERIAL PRIMARY KEY,
    name TEXT UNIQUE NOT NULL,
    resource TEXT NOT NULL,
    action TEXT NOT NULL
);

-- 角色-权限映射
CREATE TABLE role_permissions (
    role_id INT REFERENCES roles(id),
    permission_id INT REFERENCES permissions(id),
    PRIMARY KEY (role_id, permission_id)
);

-- 用户-角色映射
CREATE TABLE user_roles (
    user_id INT REFERENCES users(id),
    role_id INT REFERENCES roles(id),
    PRIMARY KEY (user_id, role_id)
);

-- 审计日志
CREATE TABLE audit_logs (
    id SERIAL PRIMARY KEY,
    user_id INT REFERENCES users(id),
    action TEXT NOT NULL,
    target_type TEXT NOT NULL,
    target_id INT,
    result TEXT NOT NULL,
    ip_address TEXT,
    details TEXT,
    created_at TIMESTAMP DEFAULT NOW()
);
```

---

## 资产编码规则

```
[类型]-[工厂]-[年份]-[序号]

IT-SZ-2026-000123   # IT 设备，深圳工厂
EQ-SZ-2026-000456   # 生产设备，深圳工厂
```

| 前缀 | 说明 |
|------|------|
| `IT` | IT 设备（电脑/交换机/服务器） |
| `EQ` | 生产设备（机床/工控/仪器） |

---

## API 接口 (gRPC)

### AssetService

```protobuf
service AssetService {
  rpc CreateAsset (CreateAssetRequest) returns (CreateAssetResponse);
  rpc GetAsset (GetAssetRequest) returns (GetAssetResponse);
  rpc UpdateAssetStatus (UpdateAssetStatusRequest) returns (UpdateAssetStatusResponse);
  rpc ListAssets (ListAssetsRequest) returns (ListAssetsResponse);
  rpc BatchScanEPC (BatchScanRequest) returns (BatchScanResponse);
  rpc StartInventoryTask (StartInventoryTaskRequest) returns (StartInventoryTaskResponse);
  rpc GetRecentDecisions (GetRecentDecisionsRequest) returns (GetRecentDecisionsResponse);
  rpc ReportDecision (ReportDecisionRequest) returns (ReportDecisionResponse);
  rpc HealthCheck (HealthCheckRequest) returns (HealthCheckResponse);
}
```

### 决策报告接口

```protobuf
// 获取系统决策建议
rpc GetRecentDecisions (GetRecentDecisionsRequest) returns (GetRecentDecisionsResponse) {
  // Request: limit, user_filter
  // Response: decisions[], total_count, adoption_rate
}

// 用户反馈决策
rpc ReportDecision (ReportDecisionRequest) returns (ReportDecisionResponse) {
  // Request: asset_id, executed, ignored, user_name
  // Response: success, message
}
```

---

## 快速启动

### 环境要求

| 组件 | 要求 |
|------|------|
| 操作系统 | macOS / Linux (Debian 13) / Windows 11 |
| C++ 编译器 | GCC 11+ / Clang 15+ / MSVC 2022+ |
| CMake | 3.16+ |
| Qt | Qt 6.2+ |
| PostgreSQL | 15+ |
| gRPC | 1.50+ |
| Protobuf | 3.19+ |

### 服务端编译

```bash
# 1. 安装依赖 (macOS)
brew install cmake protobuf grpc pqxx

# 2. 编译
cd server
mkdir build && cd build
cmake ..
make -j$(nproc)

# 3. 初始化数据库
createdb rfid
psql rfid -f ../scripts/init_db.sql

# 4. 运行
./server
# Server listening on 0.0.0.0:50051
```

### Qt 客户端编译

```bash
# 1. 安装 Qt6
# macOS: brew install qt

# 2. 编译
cd client
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/qt
make -j$(nproc)

# 3. 运行
./client
```

### 数据库迁移

```bash
# 执行所有迁移
psql rfid -f scripts/init_db.sql
psql rfid -f scripts/migration_v2.3.sql
psql rfid -f scripts/migration_v2.4_decision_snapshot.sql
psql rfid -f scripts/migration_v2.5_rule_evolution.sql
psql rfid -f scripts/migration_v2.6_rule_impact.sql
```

---

## 使用示例

### 1. 创建资产

```bash
# 通过 gRPC 创建
grpcurl -plaintext -d '{
  "name": "Dell Laptop XPS 15",
  "type": "IT",
  "asset_code": "IT-SZ-2026-000001",
  "rfid_epc": "300833B2DDE9014000000001",
  "location": "仓库A-01-03",
  "operator_name": "admin"
}' localhost:50051 asset.AssetService/CreateAsset
```

### 2. 批量扫描盘点

```bash
# 盘点扫描
grpcurl -plaintext -d '{
  "epcs": ["300833B2DDE9014000000001", "300833B2DDE9014000000002"],
  "task_id": 1
}' localhost:50051 asset.AssetService/BatchScanEPC
```

### 3. 获取决策建议

```bash
# 获取系统建议
grpcurl -plaintext -d '{
  "limit": 10
}' localhost:50051 asset.AssetService/GetRecentDecisions
```

### 4. 反馈决策

```bash
# 用户采纳
grpcurl -plaintext -d '{
  "asset_id": 1,
  "executed": true,
  "ignored": false,
  "user_name": "operator1"
}' localhost:50051 asset.AssetService/ReportDecision

# 用户忽略
grpcurl -plaintext -d '{
  "asset_id": 2,
  "executed": false,
  "ignored": true,
  "user_name": "operator1"
}' localhost:50051 asset.AssetService/ReportDecision
```

---

## 配置说明

### RFID 读写器配置

```ini
[reader.main]
type = simulator  # simulator/serial/tcp
reader_id = READER-001

[reader.warehouse]
type = tcp
host = 192.168.1.100
port = 5000
rssi_threshold = -70
```

### 决策阈值配置

```ini
[decision]
missing_hours_threshold = 72
inactivity_hours_threshold = 48
abnormal_score_threshold = 0.8

[decision.cooldown]
alert_minutes = 60
inspect_minutes = 30
noaction_minutes = 15
```

### Auto-Tuning 配置

```ini
[tuning]
strategy = bayesian  # random/grid/gradient/bayesian
iterations = 100
learning_rate = 0.1
exploration_rate = 0.2

[tuning.bayesian]
beta = 2.0
noise_smoothing = 0.1

[tuning.reward]
accuracy_weight = 0.4
adoption_weight = 0.3
fp_penalty_weight = 0.2
stability_weight = 0.1
```

---

## 性能指标

| 指标 | 目标值 | 说明 |
|------|--------|------|
| EPC 处理速度 | > 1000 EPC/s | 峰值处理能力 |
| API 响应时间 | < 50ms (P99) | gRPC 接口延迟 |
| 数据库查询 | < 10ms (P99) | 索引优化后 |
| 盘点准确率 | > 99.5% | RFID 读取准确率 |
| 系统可用性 | > 99.9% | SLA 目标 |

---

## 实施路线图

| 阶段 | 版本 | 目标 |
|------|------|------|
| **Phase 1** | v1.0-v2.0 | 资产管理基础功能，RFID 盘点 |
| **Phase 2** | v2.3-v2.4 | gRPC 通信，决策引擎，可解释层 |
| **Phase 3** | v2.5-v2.6 | 规则演进，影响分析 |
| **Phase 4** | **v3.0-v3.1** | **Auto-Tuning，贝叶斯优化** |
| **Phase 5** | v3.2 | RL Policy Learning |

---

## License

MIT License

Copyright (c) 2026 RFIDSYSTEM

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
