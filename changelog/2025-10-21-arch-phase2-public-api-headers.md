# 2025-10-21 架构重构 - 第二阶段（公共 API 头收口）

## 变更内容
- 公共头目录：新增 `include/fqtools/processing_pipeline.h`、`include/fqtools/statistic_calculator.h`
  - 转发导出接口：`IProcessingPipeline`、`create_processing_pipeline()`；`IStatisticCalculator`、`create_statistic_calculator()`。
  - 聚合头 `include/fqtools/fq.h` 切换为包含上述公共头。
- CLI 适配：
  - `src/cli/commands/stat_command.cpp` 改为 `#include <fqtools/statistic_calculator.h>`。
  - `src/cli/commands/filter_command.cpp` 增加 `#include <fqtools/processing_pipeline.h>`，继续保留实现头以获得 `ProcessingStatistics` 定义。
- 单测适配：
  - 新增/修改 `tests/unit/modules/pipeline/test_pipeline_smoke.cpp` 使用公共头 `fqtools/processing_pipeline.h`。
- 文档同步：
  - `docs/dev/architecture.md` 增补“公共 API 头目录”章节。
  - `docs/user/usage.md` 增补“公共 API 头引用”说明与示例。

## 兼容性
- 旧有包含路径（如 `#include "processing/..."`）仍可用（`fq_lib` 已将 `src/modules/pipeline` 纳入头路径）。
- 后续将把 `ProcessingStatistics::toString()` 提升到公共 API，以减少对实现头的依赖。

## 风险与验证
- 构建与测试通过脚本执行：`./scripts/build.sh` 与 `./scripts/test.sh`，自动解析依赖并运行单测。
- 该改动不改变行为，仅收口对外头文件，简化上层依赖关系。
