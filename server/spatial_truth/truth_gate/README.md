# Truth Gate (P7)

P7 Spatial Truth 接入 Runtime Kernel。真值比对判定 + 漂移事件发布 + 降级链路 + 自愈闭环。

核心组件：
- `TruthGate` - 真值比对判定（硬判定 + 软判定）
- `TruthGateAsyncRunner` - 异步执行器（不阻塞主循环）
- `LocalizationDriftEventPublisher` - 漂移事件发布
- `DriftDegradationChain` - 降级链路（连续 3 帧触发重标定 + Auto-Tuning）
- `SoftTruthGate` - 软判定模式（无 Ground Truth 时仅依据 confidence）
- `SpatialSelfHealingLoop` - 自愈闭环