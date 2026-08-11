# Spatial Benchmark (P3)

P3 定位算法 Benchmark。预置五场景（A-E）× 四算法对比测试。

核心组件：
- `BenchmarkScenario` - 场景领域模型
- `BenchmarkScenarioRegistry` - 五预置场景注册（不可修改）
- `ScenarioRunner` - 场景执行器（超时 + 算法不支持处理）
- `BenchmarkReportGenerator` - 对比表生成 + 持久化 + 导出