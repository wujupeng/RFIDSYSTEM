# Spatial Graph (P5)

P5 Spatial Graph 空间拓扑融合。融合 Reader Topology + Spatial Topology + RF Environment 三来源。

核心组件：
- `SpatialGraph` - 五类节点（Reader/Tag/Asset/Zone/Workshop）+ 五类边
- `ReaderTopologyAutoDiscovery` - Reader 上下线自动发现
- `TagAssetBindingSync` - Tag-Asset 绑定同步
- `SpatialOwnershipResolver` - Tag-Zone 动态归属判定
- `SpatialGraphBuilder` - 三来源融合构建
- `SpatialGraphProjector` - 降级投影为 TopologyGraph