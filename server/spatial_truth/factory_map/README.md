# Factory Map (P6)

P6 动态地图自动生成。自动构建工厂空间地图（Spatial Graph + Heatmap + AoA + Trail + Flow + NodeStatus）。

核心组件：
- `FactoryMap` - 工厂地图领域模型
- `HeatmapBuilder` - 热力场构建
- `AoAVectorBuilder` - AoA 方向向量构建
- `TrailCollector` - 移动轨迹收集
- `FlowCollector` - 流量收集
- `NodeStatusEncoder` - 颜色编码（绿/黄/红）
- `FactoryMapGenerator` - 自动全流程生成
- `FactoryMapPartitioner` - 按 Zone 分块输出