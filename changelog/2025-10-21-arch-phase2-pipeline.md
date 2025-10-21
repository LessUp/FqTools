# 2025-10-21 架构重构 - 第二阶段（初始落地）

## 变更内容
- 新增模块：`src/modules/pipeline/`
  - 新建 `CMakeLists.txt`，定义目标 `fq_pipeline`（STATIC），暂编译 `src/interfaces/factory.cpp`
  - 纳入构建：`src/processing/*.cpp`、`src/memory/batch_memory_manager.cpp`、`src/statistics/*.cpp`
  - 链接：`fq_core`、`fq_io`、`fq_fastq`、`spdlog::spdlog`、`TBB::tbb`、`fmt::fmt`、`fq_legacy_lib`
- 聚合调整：`src/CMakeLists.txt`
  - `fq_lib` 聚合新增：`fq_pipeline`，并显式聚合 `fq_common`、`fq_error`、`fq_core`、`fq_io`、`fq_fastq`
  - 从 `fq_legacy_lib` 移除与 `processing/statistics/memory` 重复编译的源，避免重复定义
  - 暴露 INTERFACE 头目录：`${CMAKE_SOURCE_DIR}/include`、`${CMAKE_SOURCE_DIR}/src`
- 模块入口：`src/modules/CMakeLists.txt` 增加 `add_subdirectory(pipeline)`
- 宏一致性微调：`exception_macros.h` 支持 `FQ_THROW_VALIDATION_ERROR` 的 2/3 参用法；二参解释为 `(field, value)`，三参为 `(field, value, rule)`

## 目的与影响
- 落地 `fq_pipeline` 作为统一工厂/实现聚合点，为后续迁移 `processing/`、`tbb_processing_pipeline.*`、`memory/` 做准备
- `fq_lib` 显式聚合模块目标，清晰表达依赖

## 兼容性
- CLI 仍通过 `fq_lib` 获取头路径与链接，行为保持
- 工厂 API 不变（`interfaces/` 仍保留头，先由 `fq_pipeline` 进行实现侧“承载”）

## 后续计划
- 逐步迁移 `processing/`、`tbb_processing_pipeline.*`、`memory/` 至 `fq_pipeline` 及其子模块
- 为 `fq_pipeline` 与各模块补齐/完善单测与 CI 目标
- 同步更新 `docs/dev/architecture.md` 的模块与依赖矩阵
