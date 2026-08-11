# Calibration Session (P1)

P1 RFID 空间标定系统。管理标定会话全生命周期（created→running→analyzing→completed/failed）。

核心组件：
- `CalibrationSession` - 标定会话领域模型（状态机 + 领域模型）
- `AccuracyCalculator` - 十指标计算器（RMSE/MAE/P50/P90/P95/Max/Confidence）
- `CalibrationSessionManager` - 会话管理器（前置校验 + 比对主循环 + 可重复性验证 + 版本管理）