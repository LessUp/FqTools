# 文档清理与合并（2025-10-20）

## 背景
- 依据请求：删除不必要/过时文档，整合同主题，统一描述与教学内容，遵循 KISS。

## 本次变更
- 编辑：
  - `docs/user/usage.md`：移除 `--input2`，修正多线程示例。
  - `docs/user/quick_start.md`：移除 `--input2`，修复链接，统一构建版本展示。
  - `docs/user/installation.md`：统一系统要求为 C++20 / GCC>=11 / Clang>=12 / CMake>=3.20。
  - `docs/dev/architecture.md`：统一为 C++20；更新技术栈表述。
  - `docs/dev/coding_standards.md`：统一为 C++20 / GCC>=11 / Clang>=12。
  - `docs/README.md`：简化导航，仅保留用户/开发/内部三类核心入口。
- 新增：
  - `docs/dev/build.md`：合并原 `build_system.md` 与 `building.md` 的核心操作，突出脚本用法。
  - `docs/internal/annotation_tasks.md`：合并内部注释任务清单至单页。
- 计划删除（待确认命令执行后生效）：
  - 开发重复/过时：
    - `docs/dev/build_system.md`
    - `docs/dev/building.md`
    - `docs/dev/tbb_pipeline.md`
    - `docs/dev/commenting_standards.md`
  - 规划/方案（对外不再展示）：
    - `docs/dev/api_design_standards.md`
    - `docs/dev/unified_exception_handling_strategy.md`
    - `docs/dev/integration_test_framework.md`
    - `docs/dev/performance_benchmark_system.md`
  - 内部重复：
    - `docs/internal/ANNOTATION_TASK_LIST.md`
    - `docs/internal/TASK_SUMMARY.md`

## 影响范围
- 用户入口与示例与实际源码一致（`src/cli/commands/stat_command.cpp`）。
- 开发者入口更聚焦（架构/构建/规范）。
- 内部规划与历史方案从对外导航移除，减少维护负担。

## 兼容性
- 统一到 C++20 要求，降低工具链门槛；不影响现有脚本。

## 回退方案
- 如需恢复被删除文档，可从版本控制历史恢复。
