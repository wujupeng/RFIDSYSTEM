# RFIDSYSTEM v3.3.2
# RFID 资产全生命周期 + 决策智能管理系统

> **v3.3.2 Admin Console** | RFID 驱动的 EAM + ITAM 融合平台，覆盖 IT 设备与生产设备的完整生命周期管理，具备 AI 决策能力和完善的管理后台。

---

## 版本历史

| 版本 | 日期 | 说明 |
|------|------|------|
| [v1.0](https://github.com/wujupeng/rfidsystem/releases/tag/v1.0) | 2026-01 | 基础资产管理，入库/领用/查询 |
| [v2.0](https://github.com/wujupeng/rfidsystem/releases/tag/v2.0) | 2026-02 | RFID 盘点功能，盘盈/盘亏对比 |
| [v2.3](https://github.com/wujupeng/rfidsystem/releases/tag/v2.3) | 2026-03 | gRPC 通信层，Qt 客户端，决策引擎 |
| [v2.4](https://github.com/wujupeng/rfidsystem/releases/tag/v2.4) | 2026-03 | 决策可解释层（Snapshot + Risk Breakdown） |
| [v2.5](https://github.com/wujupeng/rfidsystem/releases/tag/v2.5) | 2026-04 | 规则演进系统（Rule Registry + Versioning） |
| [v2.6](https://github.com/wujupeng/rfidsystem/releases/tag/v2.6) | 2026-04 | 规则影响分析（Rule Impact Analysis） |
| [v3.0](https://github.com/wujupeng/rfidsystem/releases/tag/v3.0) | 2026-05 | **自动调参系统（Random/Grid/Gradient Search）** |
| [v3.1](https://github.com/wujupeng/rfidsystem/releases/tag/v3.1) | 2026-05 | **Bayesian Auto-Tuning（高斯过程 + 获取函数）** |
| [v3.3.1](https://github.com/wujupeng/rfidsystem/releases/tag/v3.3.1) | 2026-05 | **可观测性系统（监控 + 运维控制台）** |
| **[v3.3.2](https://github.com/wujupeng/rfidsystem/releases/tag/v3.3.2)** | 2026-05 | **管理后台（RBAC用户管理 + 角色管理 + 系统设置）** |

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
                         │  (v3.0+v3.1)           │
                         └────────────────────────┘
                                    ↓
                         ┌────────────────────────┐
                         │    可观测性系统        │
                         │  (v3.3.1 运维控制台)   │
                         └────────────────────────┘
```

---

### 整体架构（七层）

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        Decision Intelligence Platform                        │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                    Layer 7: 管理后台层 (v3.3.2)                       │    │
│  │  ┌─────────────┐  ┌─────────────┐  ┌──────────────────────────┐   │    │
│  │  │User Mgmt    │  │Role Mgmt    │  │ System Settings          │   │    │
│  │  │(用户管理)   │  │(角色管理)   │  │ (系统配置)               │   │    │
│  │  └─────────────┘  └─────────────┘  └──────────────────────────┘   │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                     ↓                                       │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                    Layer 6: 可观测性层 (v3.3.1)                      │    │
│  │  ┌─────────────┐  ┌─────────────┐  ┌──────────────────────────┐   │    │
│  │  │Health Monitor│  │Metrics Rec  │  │ Bandit Behavior Logger   │   │    │
│  │  │(健康监控)   │  │(指标记录)   │  │ (Bandit行为日志)         │   │    │
│  │  └─────────────┘  └─────────────┘  └──────────────────────────┘   │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                     ↓                                       │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                    Layer 5: AI 决策层 (v3.0+v3.1)                    │    │
│  │  ┌─────────────┐  ┌─────────────┐  ┌──────────────────────────┐   │    │
│  │  │Auto-Tuner   │  │Reward System│  │ Bayesian Optimizer       │   │    │
│  │  │(自动调参)   │  │(奖励函数)   │  │ Gaussian Process         │   │    │
│  │  └─────────────┘  └─────────────┘  └──────────────────────────┘   │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                     ↓                                       │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                    Layer 4: 规则演进层 (v2.5+v2.6)                    │    │
│  │  ┌─────────────┐  ┌─────────────┐  ┌──────────────────────────┐   │    │
│  │  │Rule Registry│  │Rule Version  │  │ Rule Impact Analyzer     │   │    │
│  │  │(规则注册)   │  │Control      │  │ (规则效果归因)           │   │    │
│  │  └─────────────┘  └─────────────┘  └──────────────────────────┘   │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                     ↓                                       │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                    Layer 3: 决策引擎层 (v2.3+v2.4)                    │    │
│  │  ┌─────────────┐  ┌─────────────┐  ┌──────────────────────────┐   │    │
│  │  │Decision     │  │Decision     │  │ Action Generator        │   │    │
│  │  │Engine       │  │Stability    │  │ (INSPECT/ALERT/IGNORE)  │   │    │
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
│       ┌────────────────┬────────────────────────────┼────────────────┐    │
│       ▼                ▼                            ▼                ▼    │
│  ┌──────────┐  ┌──────────────┐            ┌──────────────┐ ┌──────────┐ │
│  │ RFID盘点 │  │  定期巡检    │            │  故障报修    │ │ 维修管理 │ │
│  │ (核心)   │  │  (决策建议)  │            │  REPAIR     │ │          │ │
│  └────┬─────┘  └──────────────┘            └──────┬───────┘ └────┬─────┘ │
│       │                         │                          │              │
│       └─────────────────────────┴──────────────────────────┘              │
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
│  └──────────────────────────────────────────────────────────────────────┘  │  │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## 核心功能模块

### v3.3.2 管理后台系统

```cpp
// RBAC 角色权限管理
enum class PermissionType {
    ASSET_VIEW,      // 资产查看
    ASSET_EDIT,      // 资产编辑
    INVENTORY,       // 盘点操作
    USER_MANAGE,     // 用户管理
    ROLE_MANAGE,     // 角色管理
    SYSTEM_SETTINGS  // 系统设置
};

// 用户管理功能
- 用户列表展示与搜索
- 用户创建/编辑/删除
- 角色分配
- 状态管理（启用/禁用）

// 角色管理功能
- 角色列表展示
- 角色创建/编辑/删除
- 权限配置
```

### v3.3.1 运维控制台

```cpp
// 系统健康监控
struct HealthStatus {
    bool server_ok;      // 服务器状态
    bool grpc_ok;        // gRPC服务状态
    bool bandit_ok;      // Bandit算法状态
    int latency_ms;      // 延迟(ms)
};

// 决策分布统计
struct ActionDistribution {
    int inspect;         // 立即巡检
    int no_action;       // 无需行动
    int alert;           // 安全告警
};

// 用户采纳率趋势
struct AdoptionRate {
    double current_rate; // 当前采纳率
    vector<double> history; // 历史趋势
};
```

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

---

## 技术选型

| 层级 | 技术 |
|------|------|
| **客户端** | Qt 6 + C++17（Win11 / Linux / macOS） |
| **服务端** | C++（gRPC）+ Debian 13 / Ubuntu |
| **接口协议** | gRPC + Protobuf |
| **数据库** | PostgreSQL 15+ |
| **缓存** | Redis（进阶） |
| **AI 引擎** | Gaussian Process + Bayesian Optimization |
| **图表库** | QtCharts（饼图/折线图） |
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
│   ├── db/                         # 数据库层
│   │   └── db_pool.h/cpp           # 连接池
│   │
│   ├── services/                   # 业务服务
│   │   ├── asset_service.h/cpp     # 资产服务
│   │   ├── inventory_service.h/cpp # 盘点服务
│   │   └── reconciliation_service.h/cpp # 对账服务
│   │
│   ├── monitoring/                 # ⭐ 可观测性模块 (v3.3.1)
│   │   ├── health_monitor.h/cpp    # 健康监控
│   │   └── metrics.h/cpp           # 指标记录
│   │
│   ├── repository/                 # 数据仓库层
│   │   ├── health_repository.h/cpp
│   │   ├── metrics_repository.h/cpp
│   │   ├── bandit_repository.h/cpp
│   │   └── decision_feedback_repository.h/cpp
│   │
│   ├── tuning/                     # ⭐ Auto-Tuning (v3.0+v3.1)
│   │   ├── auto_tuner.h/cpp        # 自动调参器
│   │   ├── parameter_space.h/cpp   # 参数空间
│   │   ├── reward_evaluator.h/cpp  # 奖励评估
│   │   ├── tuning_strategy.h/cpp   # 调参策略
│   │   └── bayes/                  # 贝叶斯优化
│   │
│   └── rpc/                        # gRPC 服务
│       ├── asset_service.h/cpp     # 资产服务
│       ├── monitoring_service.h/cpp # ⭐ 监控服务 (v3.3.1)
│       └── admin_service.h/cpp     # ⭐ 管理后台服务 (v3.3.2)
│
├── client/                         # Qt 客户端
│   ├── main.cpp
│   ├── CMakeLists.txt
│   ├── pages/                      # 页面
│   │   ├── dashboard/              # 仪表盘
│   │   ├── asset/                  # 资产管理
│   │   ├── purchase/               # ⭐ 采购入库 (v3.3.2)
│   │   ├── allocation/             # ⭐ 领用出库 (v3.3.2)
│   │   ├── inventory/              # RFID盘点
│   │   ├── inspection/             # ⭐ 定期巡检 (v3.3.2)
│   │   ├── repair/                 # 维修管理
│   │   ├── recommendations/        # 系统建议
│   │   ├── monitoring/             # ⭐ 运维控制台 (v3.3.1)
│   │   └── admin/                  # ⭐ 管理后台 (v3.3.2)
│   │       ├── user_management_page.h/cpp
│   │       ├── role_management_page.h/cpp
│   │       └── system_settings_page.h/cpp
│   └── network/                    # 网络层
│       ├── grpc_client.h/cpp       # gRPC 客户端
│       ├── monitoring_client.h/cpp # ⭐ 监控客户端 (v3.3.1)
│       └── admin_client.h/cpp      # ⭐ 管理后台客户端 (v3.3.2)
│
├── proto/                          # gRPC 接口定义
│   ├── asset.proto                 # 资产服务
│   ├── decision.proto              # 决策服务
│   ├── monitoring.proto            # ⭐ 监控服务 (v3.3.1)
│   └── admin.proto                 # ⭐ 管理后台服务 (v3.3.2)
│
├── scripts/                        # 脚本
│   ├── init_db.sql                 # 初始化数据库
│   ├── v3.3.1_observability.sql    # ⭐ 可观测性表结构 (v3.3.1)
│   ├── v3.3.2_admin.sql            # ⭐ 管理后台表结构 (v3.3.2)
│   └── fix_health_logs.sh          # 健康日志修复脚本
│
└── tools/                          # 工具
    └── seed_data.py                # 数据初始化工具（支持50000+设备）
```

---

## 核心数据库表

### 可观测性表结构 (v3.3.1)

```sql
-- 健康日志
CREATE TABLE health_logs (
    id SERIAL PRIMARY KEY,
    component TEXT NOT NULL,       -- server/grpc/bandit
    status TEXT NOT NULL,          -- OK/WARN/ERROR
    latency_ms INT,
    message TEXT,
    created_at TIMESTAMP DEFAULT NOW()
);

-- 系统指标
CREATE TABLE system_metrics (
    id SERIAL PRIMARY KEY,
    metric_name TEXT NOT NULL,
    metric_value DOUBLE PRECISION NOT NULL,
    created_at TIMESTAMP DEFAULT NOW()
);

-- Bandit行为日志
CREATE TABLE bandit_logs (
    id SERIAL PRIMARY KEY,
    action_type TEXT NOT NULL,      -- INSPECT/NO_ACTION/ALERT
    asset_id INT REFERENCES assets(id),
    reward DOUBLE PRECISION,
    context JSONB,
    created_at TIMESTAMP DEFAULT NOW()
);

-- 决策反馈
CREATE TABLE decision_feedback (
    id SERIAL PRIMARY KEY,
    decision_id INT REFERENCES decisions(id),
    asset_id INT REFERENCES assets(id),
    user_id INT REFERENCES users(id),
    adopted BOOLEAN NOT NULL,       -- 是否采纳
    feedback TEXT,
    created_at TIMESTAMP DEFAULT NOW()
);
```

### 管理后台表结构 (v3.3.2)

```sql
-- 用户表
CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    username TEXT UNIQUE NOT NULL,
    email TEXT,
    name TEXT,
    role_id INT REFERENCES roles(id),
    active BOOLEAN DEFAULT TRUE,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);

-- 角色表
CREATE TABLE roles (
    id SERIAL PRIMARY KEY,
    name TEXT UNIQUE NOT NULL,
    description TEXT,
    active BOOLEAN DEFAULT TRUE,
    created_at TIMESTAMP DEFAULT NOW()
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

-- 系统设置
CREATE TABLE system_settings (
    id SERIAL PRIMARY KEY,
    setting_key TEXT UNIQUE NOT NULL,
    setting_value TEXT,
    description TEXT,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
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
}
```

### MonitoringService (v3.3.1)

```protobuf
service MonitoringService {
  rpc GetSystemHealth (Empty) returns (HealthResponse);
  rpc GetActionDistribution (Empty) returns (ActionDistributionResponse);
  rpc GetAdoptionRate (Empty) returns (AdoptionResponse);
}
```

### AdminService (v3.3.2)

```protobuf
service AdminService {
  rpc GetUsers (GetUsersRequest) returns (GetUsersResponse);
  rpc CreateUser (CreateUserRequest) returns (User);
  rpc UpdateUser (UpdateUserRequest) returns (User);
  rpc DeleteUser (DeleteUserRequest) returns (EmptyResponse);
  
  rpc GetRoles (GetRolesRequest) returns (GetRolesResponse);
  rpc CreateRole (CreateRoleRequest) returns (Role);
  rpc UpdateRole (UpdateRoleRequest) returns (Role);
  rpc DeleteRole (DeleteRoleRequest) returns (EmptyResponse);
  
  rpc GetSystemSettings (EmptyRequest) returns (SystemSettings);
  rpc UpdateSystemSettings (UpdateSystemSettingsRequest) returns (SystemSettings);
}
```

---

## 快速启动

### 环境要求

| 组件 | 要求 |
|------|------|
| 操作系统 | Linux (Debian 13/Ubuntu) / macOS / Windows 11 |
| C++ 编译器 | GCC 11+ / Clang 15+ / MSVC 2022+ |
| CMake | 3.16+ |
| Qt | Qt 6.2+ (含 QtCharts) |
| PostgreSQL | 15+ |
| gRPC | 1.50+ |
| Protobuf | 3.19+ |

### 服务端编译

```bash
# Linux/WSL 环境
sudo apt-get update && sudo apt-get install -y \
    cmake build-essential libpqxx-dev \
    libgrpc++-dev protobuf-compiler-grpc \
    qt6-charts-dev

# macOS 环境
brew install cmake protobuf grpc libpqxx qt@6

# 编译
cd server
mkdir build && cd build
cmake ..
make -j$(nproc)

# 初始化数据库
createdb rfid
psql rfid -f ../scripts/init_db.sql
psql rfid -f ../scripts/v3.3.1_observability.sql
psql rfid -f ../scripts/v3.3.2_admin.sql

# 运行
./rfid-server
# Server listening on 0.0.0.0:50051
```

### Qt 客户端编译

```bash
cd client
mkdir build && cd build

# Linux/WSL
cmake .. -DCMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake/Qt6
make -j$(nproc)

# macOS
cmake .. -DCMAKE_PREFIX_PATH=/usr/local/opt/qt/lib/cmake/Qt6
make -j$(nproc)

# 运行 (Linux/WSL 需要配置 DISPLAY)
export DISPLAY=:0
./rfid-client
```

---

## 环境差异说明

| 项目 | **Linux/WSL 环境** | **MacOS 环境** |
|------|-------------------|---------------|
| **依赖安装** | `apt-get install qt6-charts-dev libpqxx-dev libgrpc++-dev protobuf-compiler-grpc` | `brew install qt@6 libpqxx grpc protobuf` |
| **Qt 显示** | 需要配置 DISPLAY (`export DISPLAY=:0`) | 无需额外配置 |
| **PostgreSQL 连接** | 默认使用 Unix socket `/var/run/postgresql` | 默认使用 TCP `localhost:5432` |
| **CMake Qt 路径** | `/usr/lib/x86_64-linux-gnu/cmake/Qt6` | `/usr/local/opt/qt/lib/cmake/Qt6` |
| **编译命令** | `cmake .. && make -j4` | `cmake .. && make -j4` |

---

## 配置说明

### 默认配置

| 项目 | 值 |
|------|------|
| **数据库名称** | rfid |
| **数据库用户** | postgres |
| **数据库密码** | 123456 |
| **gRPC 端口** | 50051 |
| **默认管理员账号** | admin / admin123 |

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
| **Phase 4** | v3.0-v3.1 | Auto-Tuning，贝叶斯优化 |
| **Phase 5** | **v3.3.1-v3.3.2** | **可观测性系统，管理后台** |

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
