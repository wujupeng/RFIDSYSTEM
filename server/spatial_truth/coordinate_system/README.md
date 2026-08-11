# Coordinate System (P4)

P4 空间坐标系正式化。管理 Factory→Building→Floor→Zone→Local 五级坐标系层级。

核心组件：
- `SpatialCoordinateSystem` - 坐标系领域模型（id/parent_id/origin/rotation/unit/version）
- `CoordinateSystemManager` - 层级与版本管理（循环引用检测 + 版本单调递增 + 默认全局坐标系）
- `CoordinateTransformer` - 链式变换（父子变换 + 多级合成 + 单位转换 + 往返一致性）