# 全面命名与结构重构：批次1（接口与公共头前移与改名）

日期：2025-10-21

## 范围
- 去除 `i_*` 风格的接口命名，引入 `*Interface` 风格并提供兼容别名。
- 将接口头前移到公共头树 `include/fqtools/**` 并调整引用。
- 为处理/统计模块补充清晰的公共入口与转发头，保持外部包含路径稳定。

## 主要变更
- 公共接口与转发头
  - 新增 `include/fqtools/pipeline/processing_pipeline_interface.h`
    - 接口重命名：`IProcessingPipeline` → `ProcessingPipelineInterface`
    - 兼容别名与旧工厂：`using IProcessingPipeline = ProcessingPipelineInterface;`、`create_processing_pipeline()` 保留（返回类型更新为新接口）；新增 `make_processing_pipeline()`。
    - 引入新处理器接口：`read_mutator_interface.h`、`read_predicate_interface.h`，并提供别名 `IReadMutator`/`IReadPredicate`。
  - 新增 `include/fqtools/pipeline/read_mutator_interface.h`、`read_predicate_interface.h`。
  - 更新转发头：`include/fqtools/processing_pipeline.h` 指向新接口头。
  - 新增/迁移统计接口：`include/fqtools/statistics/statistic_calculator_interface.h`
    - 接口重命名：`IStatisticCalculator` → `StatisticCalculatorInterface`；保留 `create_statistic_calculator()`，新增 `make_statistic_calculator()`。
  - 迁移统计内部接口：`src/modules/pipeline/statistics/i_statistic.h` → `include/fqtools/statistics/statistic_interface.h`
    - 接口重命名：`IStatistic` → `StatisticInterface`，并提供兼容别名。
  - 更新转发头：`include/fqtools/statistic_calculator.h` 指向新接口头。
- 应用信息头
  - `include/fqtools/fq.h` → `include/fqtools/app/app_info.h`，并更新其内部包含到新接口。
  - 替换引用：`src/cli/main.cpp` 使用 `<fqtools/app/app_info.h>`。
- 实现与工厂
  - `src/modules/pipeline/processing/processing_pipeline.{h,cpp}`
    - 基类改为 `ProcessingPipelineInterface`；方法签名改用新接口类型。
  - `src/modules/pipeline/processing/tbb_processing_pipeline.{h,cpp}`
    - 基类改为 `ProcessingPipelineInterface`；方法签名改用新接口类型。
    - 工厂 `create_tbb_pipeline()` 返回类型更新为新接口。
  - `src/modules/pipeline/factory.cpp`
    - 统计/处理工厂返回类型统一到新接口；新增 `make_*` 别名实现，保留 `create_*` 兼容。
- 其他
  - `src/modules/pipeline/processing/i_read_processor.h` 由接口定义调整为适配层：仅包含新接口头并提供旧名别名。
  - 更新引用：`examples/tbb_pipeline_demo.cpp`、`src/cli/commands/filter_command.h`、`src/modules/pipeline/statistics/fq_statistic.h`、`src/modules/pipeline/statistics/fq_statistic_worker.h`。

## 注意事项
- 当前保留兼容别名与旧工厂函数声明，便于平滑迁移；后续批次将清理。
- 仍未进行目录扁平化与 `core_legacy/` → `legacy/` 的迁移（计划在批次3执行）。
- CMake 安装头策略与测试覆盖将在批次2/3同步完善。

## 迁移影响
- 外部包含路径仍可使用：`<fqtools/processing_pipeline.h>`、`<fqtools/statistic_calculator.h>`。
- 旧类型/工厂仍可用，但建议逐步切换至：`ProcessingPipelineInterface`、`StatisticCalculatorInterface`、`make_*` 系列。

## 后续计划
- 批次2：实现类重命名与目录扁平化前的准备（可选：`SequentialProcessingPipeline` 命名落地）。
- 批次3：目录重组（`src/modules/**` → `src/<domain>/**`，`core_legacy/` → `legacy/`），同步 CMake 与安装头策略。
- 批次4：清理兼容别名与旧 API，更新文档与示例。
