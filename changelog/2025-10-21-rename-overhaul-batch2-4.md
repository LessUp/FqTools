# 批次2-4：实现重命名、目录重组与兼容清理（阶段性）

日期：2025-10-21

## 批次2：实现类重命名（不移动文件，先稳编译）
- 处理流水线实现类：`ProcessingPipeline` → `SequentialProcessingPipeline`
  - 文件：`src/modules/pipeline/processing/processing_pipeline.{h,cpp}`（仅类名与符号重命名）
  - 工厂：`make_processing_pipeline()` 实例化 `SequentialProcessingPipeline`
- 统计实现类：`FqStatistic` → `FastqStatisticCalculator`
  - 文件：`src/modules/pipeline/statistics/fq_statistic.{h,cpp}`（仅类名与符号重命名）
  - 工厂：`make_statistic_calculator()` 实例化 `FastqStatisticCalculator`
- 兼容包装：
  - `create_processing_pipeline()` 与 `create_statistic_calculator()` 保留，转发到 `make_*`，便于平滑迁移与测试通过。

## 批次3：目录重组（最小影响策略）
- 迁移源码目录：
  - `src/modules/pipeline/` → `src/pipeline/`
  - 包括：`processing/`、`statistics/`、`memory/`、`factory.cpp`
- 新增构建脚本：
  - `src/pipeline/CMakeLists.txt`（从旧 `src/modules/pipeline/CMakeLists.txt` 迁移并适配路径）
- CMake 更新：
  - 自 `src/modules/CMakeLists.txt` 移除 `add_subdirectory(pipeline)`
  - 在 `src/CMakeLists.txt` 添加 `add_subdirectory(pipeline)`
  - `target_include_directories(fq_lib INTERFACE ...)` 将 `src/modules/pipeline` 改为 `src/pipeline`
- 示例与命令包含路径调整：
  - `examples/tbb_pipeline_demo.cpp` 现在包含 `"pipeline/processing/tbb_processing_pipeline.h"`
  - `src/cli/commands/filter_command.cpp` 本地实现包含调整为 `"pipeline/processing/..."`

## 批次4：兼容清理（阶段性）
- 保留兼容期别名与旧工厂包装，待迁移完成并通过一次完整构建与测试后，计划移除：
  - 别名：`IProcessingPipeline`、`IReadMutator`、`IReadPredicate`、`IStatisticCalculator`、`IStatistic`
  - 工厂：`create_processing_pipeline()`、`create_statistic_calculator()`（当前仍有测试使用）

## 构建影响
- `src/CMakeLists.txt` 已加入 `add_subdirectory(pipeline)` 并更新 include 路径。
- `fq_cli` 依赖 `fq_lib`，公共 include 传递，`#include <fqtools/...>` 与模块内 `#include "processing/..."`/`#include "pipeline/..."` 保持可解析。

## 后续工作
- 运行一次全量构建与测试（建议）：
  - 构建：`bash scripts/build.sh`
  - 测试：`ctest --output-on-failure`（在构建目录中）
- 若构建通过：
  - 清理兼容别名与 `create_*` API
  - 更新示例与单测到 `make_*` API
  - 完成批次4最终清理的 changelog 记录
- 若构建失败：
  - 按编译输出逐个修正包含/符号/链接问题
