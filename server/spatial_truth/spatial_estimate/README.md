# Spatial Estimate (P2)

P2 SpatialEstimate 升级。将定位输出从 TagPosition 升级为 13 字段 SpatialEstimate。

核心组件：
- `SpatialEstimate` - 13 字段结构（x/y/z/confidence/covariance/error_radius/source_hash/environment_hash/calibration_version/mode/trusted）
- `SpatialEstimateBuilder` - 构建器（从 FusionResult + SpatialProbability 组装）
- `ErrorRadiusCalculator` - 误差半径计算（协方差特征值或 P95）
- `TrustedJudge` - 可信判定（confidence + error_radius + calibration_version 三条件）
- `SourceHashCalculator` - 观测数据哈希 + RF 环境哈希
- `SpatialEstimateProjector` - 向后兼容投影为 TagPosition