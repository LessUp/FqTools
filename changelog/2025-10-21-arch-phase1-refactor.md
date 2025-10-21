# 2025-10-21 架构重构 - 第一阶段

## 变更内容
- 新增公共聚合头：`include/fqtools/fq.h`
  - 导出稳定 API：`interfaces/i_processing_pipeline.h`、`interfaces/i_statistic_calculator.h`、`modules/common/common.h`
  - 提供与 legacy 兼容的薄封装：`fq::common::print_big_logo()`、`fq::common::software_info(const char*)`
- 异常宏一致性修复：`src/modules/error/exception_macros.h`
  - `FQ_THROW_VALIDATION_ERROR` 支持 2/3 参调用（可兼容 `fastq.h` 中两参调用）
- 模块使用场景修复：`src/modules/fastq/fastq.h`
  - 显式包含 `../error/exception_macros.h` 以启用验证抛错宏
- CLI 与 legacy 解耦：`src/cli/main.cpp`
  - 去除 `core_legacy/core.h` 直接包含，改为 `#include <fqtools/fq.h>`
- CMake 暴露头文件路径：`src/CMakeLists.txt`
  - 为 `fq_lib` 增加 `INTERFACE` 头文件目录：`${CMAKE_SOURCE_DIR}/include`、`${CMAKE_SOURCE_DIR}/src`

## 目的与影响
- 通过公共聚合头统一对外 API，降低 CLI 对实现细节（尤其是 legacy）的耦合。
- 统一异常抛错宏，消除 `fastq.h` 的两参调用潜在不一致。
- 为后续模块细化与 CMake 目标拆分打基础。

## 兼容性
- 编译路径保持，`fq_cli` 通过链接 `fq_lib` 获得 `include/` 与 `src/` 头路径。
- `print_big_logo()`/`software_info()` 提供薄封装，兼容旧调用。

## 后续计划（阶段二/三预告）
- 将 `interfaces/` 迁移为新模块（建议名：`fq_pipeline`），收敛处理与统计实现。
- 细化模块 CMake 目标与链路，逐步迁出 `fq_legacy_lib`。
- 为各模块新增/完善单测与 CI 目标，持续保障重构稳定性。
