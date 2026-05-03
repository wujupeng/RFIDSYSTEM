# RFIDSYSTEM
# RFID 资产全生命周期管理系统

> RFID 驱动的 EAM + ITAM 融合平台，覆盖 IT 设备与生产设备的完整生命周期管理。

---

## 系统架构总览

```
采购 → 入库 → 贴标(RFID) → 领用 → 使用 → 巡检 → 报修 → 维修 → 盘点 → 报废
```

### 整体架构（四层）

<!-- SVG: 系统架构图 -->
<p align="center">
<svg width="100%" viewBox="0 0 680 580" xmlns="http://www.w3.org/2000/svg" style="max-width:800px;font-family:sans-serif">
  <defs>
    <marker id="arrow" viewBox="0 0 10 10" refX="8" refY="5" markerWidth="6" markerHeight="6" orient="auto-start-reverse">
      <path d="M2 1L8 5L2 9" fill="none" stroke="#888" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"/>
    </marker>
  </defs>

  <!-- Layer 1: 设备层 -->
  <rect x="40" y="30" width="600" height="90" rx="12" fill="#f1efe8" stroke="#b4b2a9" stroke-width="0.8"/>
  <text x="340" y="54" text-anchor="middle" font-size="14" font-weight="600" fill="#2c2c2a">设备层（RFID 硬件）</text>
  <rect x="58" y="64" width="138" height="38" rx="6" fill="#d3d1c7" stroke="#888780" stroke-width="0.5"/>
  <text x="127" y="87" text-anchor="middle" font-size="12" fill="#2c2c2a">固定读写器（仓库/通道）</text>
  <rect x="210" y="64" width="128" height="38" rx="6" fill="#d3d1c7" stroke="#888780" stroke-width="0.5"/>
  <text x="274" y="87" text-anchor="middle" font-size="12" fill="#2c2c2a">手持 PDA 读写器</text>
  <rect x="354" y="64" width="128" height="38" rx="6" fill="#d3d1c7" stroke="#888780" stroke-width="0.5"/>
  <text x="418" y="87" text-anchor="middle" font-size="12" fill="#2c2c2a">UHF RFID 标签（Impinj）</text>
  <rect x="498" y="64" width="120" height="38" rx="6" fill="#d3d1c7" stroke="#888780" stroke-width="0.5"/>
  <text x="558" y="87" text-anchor="middle" font-size="12" fill="#2c2c2a">天线阵列</text>

  <!-- Arrow 1→2 -->
  <line x1="340" y1="120" x2="340" y2="148" stroke="#888" stroke-width="1" marker-end="url(#arrow)"/>
  <text x="354" y="138" font-size="11" fill="#888">gRPC</text>

  <!-- Layer 2: 客户端层 -->
  <rect x="40" y="150" width="600" height="100" rx="12" fill="#e1f5ee" stroke="#1d9e75" stroke-width="0.8"/>
  <text x="340" y="174" text-anchor="middle" font-size="14" font-weight="600" fill="#085041">客户端层（Qt + C++，Win11）</text>
  <rect x="58" y="184" width="170" height="50" rx="6" fill="#9fe1cb" stroke="#0f6e56" stroke-width="0.5"/>
  <text x="143" y="204" text-anchor="middle" font-size="13" font-weight="600" fill="#04342c">资产管理客户端</text>
  <text x="143" y="222" text-anchor="middle" font-size="11" fill="#085041">主系统，全功能模块</text>
  <rect x="248" y="184" width="170" height="50" rx="6" fill="#9fe1cb" stroke="#0f6e56" stroke-width="0.5"/>
  <text x="333" y="204" text-anchor="middle" font-size="13" font-weight="600" fill="#04342c">RFID 盘点工具</text>
  <text x="333" y="222" text-anchor="middle" font-size="11" fill="#085041">手持枪对接，批量盘点</text>
  <rect x="438" y="184" width="170" height="50" rx="6" fill="#9fe1cb" stroke="#0f6e56" stroke-width="0.5"/>
  <text x="523" y="204" text-anchor="middle" font-size="13" font-weight="600" fill="#04342c">运维客户端</text>
  <text x="523" y="222" text-anchor="middle" font-size="11" fill="#085041">报修 / 巡检入口</text>

  <!-- Arrow 2→3 -->
  <line x1="340" y1="250" x2="340" y2="278" stroke="#888" stroke-width="1" marker-end="url(#arrow)"/>
  <text x="354" y="268" font-size="11" fill="#888">gRPC</text>

  <!-- Layer 3: 服务端层 -->
  <rect x="40" y="280" width="600" height="130" rx="12" fill="#eeedfe" stroke="#534ab7" stroke-width="0.8"/>
  <text x="340" y="304" text-anchor="middle" font-size="14" font-weight="600" fill="#26215c">服务端层（C++ + Debian 13）</text>
  <rect x="58" y="316" width="128" height="76" rx="6" fill="#cec bf6" stroke="#534ab7" stroke-width="0.5"/>
  <rect x="58" y="316" width="128" height="76" rx="6" fill="#cecbf6" stroke="#534ab7" stroke-width="0.5"/>
  <text x="122" y="344" text-anchor="middle" font-size="13" font-weight="600" fill="#26215c">gRPC 服务</text>
  <text x="122" y="362" text-anchor="middle" font-size="11" fill="#3c3489">Boost.Asio</text>
  <text x="122" y="378" text-anchor="middle" font-size="11" fill="#3c3489">主通信框架</text>
  <rect x="202" y="316" width="128" height="76" rx="6" fill="#cecbf6" stroke="#534ab7" stroke-width="0.5"/>
  <text x="266" y="344" text-anchor="middle" font-size="13" font-weight="600" fill="#26215c">业务服务</text>
  <text x="266" y="362" text-anchor="middle" font-size="11" fill="#3c3489">资产/报修/巡检</text>
  <text x="266" y="378" text-anchor="middle" font-size="11" fill="#3c3489">盘点逻辑</text>
  <rect x="346" y="316" width="128" height="76" rx="6" fill="#cecbf6" stroke="#534ab7" stroke-width="0.5"/>
  <text x="410" y="344" text-anchor="middle" font-size="13" font-weight="600" fill="#26215c">PostgreSQL</text>
  <text x="410" y="362" text-anchor="middle" font-size="11" fill="#3c3489">主数据库</text>
  <text x="410" y="378" text-anchor="middle" font-size="11" fill="#3c3489">资产全量数据</text>
  <rect x="490" y="316" width="138" height="76" rx="6" fill="#cecbf6" stroke="#534ab7" stroke-width="0.5"/>
  <text x="559" y="344" text-anchor="middle" font-size="13" font-weight="600" fill="#26215c">Redis / NATS</text>
  <text x="559" y="362" text-anchor="middle" font-size="11" fill="#3c3489">缓存 + 消息队列</text>
  <text x="559" y="378" text-anchor="middle" font-size="11" fill="#3c3489">进阶扩展</text>

  <!-- Arrow 3→4 -->
  <line x1="340" y1="410" x2="340" y2="438" stroke="#888" stroke-width="1" marker-end="url(#arrow)"/>

  <!-- Layer 4: 管理后台 -->
  <rect x="40" y="440" width="600" height="80" rx="12" fill="#faece7" stroke="#993c1d" stroke-width="0.8"/>
  <text x="340" y="466" text-anchor="middle" font-size="14" font-weight="600" fill="#4a1b0c">管理后台（Qt Admin 客户端）</text>
  <text x="340" y="486" text-anchor="middle" font-size="12" fill="#712b13">超级管理员 · RBAC 权限管理 · 仪表盘 · 报表 · 系统设置 · 用户管理</text>
  <text x="340" y="505" text-anchor="middle" font-size="11" fill="#993c1d">权限更高，类运维控制台</text>

  <!-- 技术栈说明 -->
  <text x="340" y="548" text-anchor="middle" font-size="11" fill="#888">协议: EPC Gen2 UHF 860–960MHz · 芯片: Impinj · 接口: gRPC · 系统: Debian 13</text>
</svg>
</p>

---

### 资产全生命周期流程

<!-- SVG: 生命周期流程图 -->
<p align="center">
<svg width="100%" viewBox="0 0 680 430" xmlns="http://www.w3.org/2000/svg" style="max-width:800px;font-family:sans-serif">
  <defs>
    <marker id="arr2" viewBox="0 0 10 10" refX="8" refY="5" markerWidth="6" markerHeight="6" orient="auto-start-reverse">
      <path d="M2 1L8 5L2 9" fill="none" stroke="#888" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"/>
    </marker>
  </defs>

  <text x="340" y="28" text-anchor="middle" font-size="14" font-weight="600" fill="#333">资产全生命周期闭环</text>

  <!-- Row 1 -->
  <rect x="40" y="46" width="108" height="50" rx="8" fill="#f1efe8" stroke="#888780" stroke-width="0.8"/>
  <text x="94" y="66" text-anchor="middle" font-size="13" font-weight="600" fill="#2c2c2a">采购入库</text>
  <text x="94" y="84" text-anchor="middle" font-size="11" fill="#5f5e5a">采购单→到货录入</text>
  <line x1="148" y1="71" x2="168" y2="71" stroke="#888" stroke-width="1" marker-end="url(#arr2)"/>

  <rect x="170" y="46" width="108" height="50" rx="8" fill="#e1f5ee" stroke="#1d9e75" stroke-width="0.8"/>
  <text x="224" y="66" text-anchor="middle" font-size="13" font-weight="600" fill="#085041">生成编号</text>
  <text x="224" y="84" text-anchor="middle" font-size="11" fill="#0f6e56">IT-SZ-2026-XXXX</text>
  <line x1="278" y1="71" x2="298" y2="71" stroke="#888" stroke-width="1" marker-end="url(#arr2)"/>

  <rect x="300" y="46" width="108" height="50" rx="8" fill="#eeedfe" stroke="#534ab7" stroke-width="0.8"/>
  <text x="354" y="66" text-anchor="middle" font-size="13" font-weight="600" fill="#26215c">RFID 写卡</text>
  <text x="354" y="84" text-anchor="middle" font-size="11" fill="#3c3489">EPC → AssetID 贴标</text>
  <line x1="408" y1="71" x2="428" y2="71" stroke="#888" stroke-width="1" marker-end="url(#arr2)"/>

  <rect x="430" y="46" width="108" height="50" rx="8" fill="#e1f5ee" stroke="#1d9e75" stroke-width="0.8"/>
  <text x="484" y="66" text-anchor="middle" font-size="13" font-weight="600" fill="#085041">领用出库</text>
  <text x="484" y="84" text-anchor="middle" font-size="11" fill="#0f6e56">申请→审批→绑定</text>

  <!-- 状态标签 Row 1 -->
  <rect x="40" y="104" width="50" height="20" rx="4" fill="#e6f1fb" stroke="#185fa5" stroke-width="0.5"/>
  <text x="65" y="118" text-anchor="middle" font-size="10" fill="#185fa5">在库</text>
  <rect x="170" y="104" width="50" height="20" rx="4" fill="#e6f1fb" stroke="#185fa5" stroke-width="0.5"/>
  <text x="195" y="118" text-anchor="middle" font-size="10" fill="#185fa5">在库</text>
  <rect x="300" y="104" width="50" height="20" rx="4" fill="#e6f1fb" stroke="#185fa5" stroke-width="0.5"/>
  <text x="325" y="118" text-anchor="middle" font-size="10" fill="#185fa5">在库</text>
  <rect x="430" y="104" width="60" height="20" rx="4" fill="#eaf3de" stroke="#3b6d11" stroke-width="0.5"/>
  <text x="460" y="118" text-anchor="middle" font-size="10" fill="#3b6d11">使用中</text>

  <!-- Arrow down 领用→巡检 -->
  <line x1="484" y1="96" x2="484" y2="150" stroke="#888" stroke-width="1" marker-end="url(#arr2)"/>

  <!-- Row 2 -->
  <rect x="430" y="152" width="108" height="50" rx="8" fill="#e1f5ee" stroke="#1d9e75" stroke-width="0.8"/>
  <text x="484" y="172" text-anchor="middle" font-size="13" font-weight="600" fill="#085041">定期巡检</text>
  <text x="484" y="190" text-anchor="middle" font-size="11" fill="#0f6e56">扫标签→自动记录</text>
  <line x1="430" y1="177" x2="408" y2="177" stroke="#888" stroke-width="1" marker-end="url(#arr2)"/>

  <rect x="300" y="152" width="108" height="50" rx="8" fill="#faeeda" stroke="#ba7517" stroke-width="0.8"/>
  <text x="354" y="172" text-anchor="middle" font-size="13" font-weight="600" fill="#412402">故障报修</text>
  <text x="354" y="190" text-anchor="middle" font-size="11" fill="#854f0b">报修→派单→SLA</text>
  <line x1="300" y1="177" x2="278" y2="177" stroke="#888" stroke-width="1" marker-end="url(#arr2)"/>

  <rect x="170" y="152" width="108" height="50" rx="8" fill="#faece7" stroke="#993c1d" stroke-width="0.8"/>
  <text x="224" y="172" text-anchor="middle" font-size="13" font-weight="600" fill="#4a1b0c">送修维修</text>
  <text x="224" y="190" text-anchor="middle" font-size="11" fill="#712b13">状态: 维修中</text>
  <line x1="170" y1="177" x2="148" y2="177" stroke="#888" stroke-width="1" marker-end="url(#arr2)"/>

  <rect x="40" y="152" width="108" height="50" rx="8" fill="#e1f5ee" stroke="#1d9e75" stroke-width="0.8"/>
  <text x="94" y="172" text-anchor="middle" font-size="13" font-weight="600" fill="#085041">验收归还</text>
  <text x="94" y="190" text-anchor="middle" font-size="11" fill="#0f6e56">验收→恢复使用</text>

  <!-- 状态标签 Row 2 -->
  <rect x="430" y="210" width="60" height="20" rx="4" fill="#eaf3de" stroke="#3b6d11" stroke-width="0.5"/>
  <text x="460" y="224" text-anchor="middle" font-size="10" fill="#3b6d11">使用中</text>
  <rect x="300" y="210" width="60" height="20" rx="4" fill="#faeeda" stroke="#ba7517" stroke-width="0.5"/>
  <text x="330" y="224" text-anchor="middle" font-size="10" fill="#854f0b">待维修</text>
  <rect x="170" y="210" width="60" height="20" rx="4" fill="#faeeda" stroke="#ba7517" stroke-width="0.5"/>
  <text x="200" y="224" text-anchor="middle" font-size="10" fill="#854f0b">维修中</text>
  <rect x="40" y="210" width="60" height="20" rx="4" fill="#eaf3de" stroke="#3b6d11" stroke-width="0.5"/>
  <text x="70" y="224" text-anchor="middle" font-size="10" fill="#3b6d11">使用中</text>

  <!-- Arrow down 验收→盘点 -->
  <line x1="94" y1="202" x2="94" y2="258" stroke="#888" stroke-width="1" marker-end="url(#arr2)"/>

  <!-- Row 3 -->
  <rect x="40" y="260" width="172" height="62" rx="8" fill="#eeedfe" stroke="#534ab7" stroke-width="0.8"/>
  <text x="126" y="282" text-anchor="middle" font-size="13" font-weight="600" fill="#26215c">RFID 盘点（核心亮点）</text>
  <text x="126" y="300" text-anchor="middle" font-size="11" fill="#3c3489">手持 PDA / 固定通道</text>
  <text x="126" y="316" text-anchor="middle" font-size="11" fill="#3c3489">盘盈 / 盘亏 / 位置异常</text>
  <line x1="212" y1="291" x2="248" y2="291" stroke="#888" stroke-width="1" marker-end="url(#arr2)"/>

  <rect x="250" y="260" width="148" height="62" rx="8" fill="#e1f5ee" stroke="#1d9e75" stroke-width="0.8"/>
  <text x="324" y="282" text-anchor="middle" font-size="13" font-weight="600" fill="#085041">自动数据对比</text>
  <text x="324" y="300" text-anchor="middle" font-size="11" fill="#0f6e56">对比系统资产台账</text>
  <text x="324" y="316" text-anchor="middle" font-size="11" fill="#0f6e56">生成差异报告</text>
  <line x1="398" y1="291" x2="434" y2="291" stroke="#888" stroke-width="1" marker-end="url(#arr2)"/>

  <rect x="436" y="260" width="172" height="62" rx="8" fill="#faece7" stroke="#993c1d" stroke-width="0.8"/>
  <text x="522" y="282" text-anchor="middle" font-size="13" font-weight="600" fill="#4a1b0c">报废 / 处置</text>
  <text x="522" y="300" text-anchor="middle" font-size="11" fill="#712b13">审批→报废→RFID 注销</text>
  <text x="522" y="316" text-anchor="middle" font-size="11" fill="#712b13">已处置（终态）</text>

  <!-- 状态标签 Row 3 -->
  <rect x="40" y="330" width="60" height="20" rx="4" fill="#e6f1fb" stroke="#185fa5" stroke-width="0.5"/>
  <text x="70" y="344" text-anchor="middle" font-size="10" fill="#185fa5">任意状态</text>
  <rect x="436" y="330" width="60" height="20" rx="4" fill="#fcebeb" stroke="#a32d2d" stroke-width="0.5"/>
  <text x="466" y="344" text-anchor="middle" font-size="10" fill="#a32d2d">已报废</text>

  <!-- MVP 说明 -->
  <rect x="40" y="368" width="598" height="48" rx="8" fill="#f7f6f2" stroke="#d3d1c7" stroke-width="0.5"/>
  <text x="340" y="388" text-anchor="middle" font-size="12" font-weight="600" fill="#444">MVP 优先路径</text>
  <text x="340" y="408" text-anchor="middle" font-size="11" fill="#888">采购入库 → RFID 写卡 → 领用 → 盘点  |  Phase 2: 报修 → 巡检  |  Phase 3: 自动化通道机 + BI 报表</text>
</svg>
</p>

---

## 技术选型

| 层级 | 技术 |
|------|------|
| 客户端 | Qt 6 + C++17（Win11） |
| 服务端 | C++（Boost.Asio）+ Debian 13 |
| 接口协议 | gRPC + Protobuf |
| 数据库 | PostgreSQL |
| 缓存 | Redis |
| 消息队列 | NATS（进阶） |
| RFID 协议 | EPC Gen2 UHF 860–960MHz（Impinj） |

---

## 工程结构

```
rfid-system/
├── server/                 # C++ 服务端
│   ├── CMakeLists.txt
│   ├── main.cpp
│   ├── config/
│   ├── core/
│   ├── db/                 # PostgreSQL 封装（连接池）
│   ├── rpc/                # gRPC 服务实现
│   ├── models/
│   └── services/
│
├── client/                 # Qt 客户端（Win11）
│   ├── CMakeLists.txt
│   ├── main.cpp
│   ├── ui/
│   ├── pages/
│   └── network/            # gRPC 客户端封装
│
├── proto/                  # gRPC 接口定义
│   └── asset.proto
│
├── scripts/
│   ├── init_db.sql
│   └── run_server.sh
│
└── README.md
```

---

## 核心数据库表

```sql
-- 资产主表
CREATE TABLE assets (
    id          SERIAL PRIMARY KEY,
    asset_code  TEXT UNIQUE,          -- IT-SZ-2026-000123
    rfid_epc    TEXT UNIQUE,          -- RFID EPC 绑定
    name        TEXT,
    type        TEXT,                 -- IT / EQ
    status      TEXT DEFAULT 'in_stock', -- in_stock / in_use / repairing / scrapped
    location    TEXT,
    owner       TEXT,
    created_at  TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 全量操作日志
CREATE TABLE asset_logs (
    id          SERIAL PRIMARY KEY,
    asset_id    INTEGER REFERENCES assets(id),
    action      TEXT,                 -- 入库/领用/维修/盘点
    operator    TEXT,
    timestamp   TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 维修单
CREATE TABLE repair_orders (
    id          SERIAL PRIMARY KEY,
    asset_id    INTEGER REFERENCES assets(id),
    issue       TEXT,
    status      TEXT DEFAULT 'open', -- open / in_progress / closed
    assigned_to TEXT,
    created_at  TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 盘点任务
CREATE TABLE inventory_tasks (
    id          SERIAL PRIMARY KEY,
    area        TEXT,
    status      TEXT DEFAULT 'pending',
    created_at  TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

---

## 资产编码规则

```
[类型]-[工厂]-[年份]-[序号]

IT-SZ-2026-000123   # IT 设备，深圳工厂
EQ-SZ-2026-000456   # 生产设备，深圳工厂
```

- `IT`：IT 设备（电脑/交换机/服务器）  
- `EQ`：生产设备（机床/工控/仪器）  
- RFID Tag 绑定：EPC → AssetID

---

## 权限体系（RBAC）

| 角色 | 说明 |
|------|------|
| 超级管理员 | 全部权限 |
| IT 管理员 | IT 资产管理 |
| 设备管理员 | 生产设备管理 |
| 维修人员 | 报修/维修单操作 |
| 普通员工 | 申请领用/报修 |

---

## 实施路线

| 阶段 | 周期 | 目标 |
|------|------|------|
| Phase 1 | 第 1 个月 | 资产管理、入库/领用、基础 RFID 盘点（MVP） |
| Phase 2 | 第 2–3 个月 | 报修系统、巡检系统、完整盘点 |
| Phase 3 | 第 4–6 个月 | 自动化通道机、数据分析、BI 报表 |

> **建议**：先跑通 `客户端 ⇄ gRPC ⇄ PostgreSQL`，带一个最小业务（资产新增 + 查询），再逐步叠加 RFID / 巡检 / 报修模块。

---

## 快速启动

### 服务端（Debian 13）

```bash
sudo apt install build-essential cmake libpqxx-dev protobuf-compiler grpc

createdb rfid
psql rfid < scripts/init_db.sql

cd server
mkdir build && cd build
cmake ..
make
./server
# Server listening on 0.0.0.0:50051
```

### 客户端（Win11 + Qt6）

```bash
cd client
cmake -B build
cmake --build build
./build/client.exe
```

---

## License

MIT
