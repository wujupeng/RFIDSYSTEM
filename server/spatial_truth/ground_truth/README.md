# Ground Truth (P0)

P0 基准建立模块。管理 Ground Truth 基准点的录入、存储、去重择优与检索。

核心组件：
- `GroundTruthPoint` - 基准点领域模型（五来源枚举 + 八字段）
- `GroundTruthStore` - 持久化存储（内存缓存 + PostgreSQL）
- `GroundTruthService` - 业务服务（录入校验 + 坐标系关联 + 删除校验）