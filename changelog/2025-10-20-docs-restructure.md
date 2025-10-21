# 2025-10-20 文档重组与教学友好化（阶段四）

遵循 KISS 原则，面向教学与开源使用者，精简并重组 `docs/`，同时清理未被引用的遗留目录。

## 变更

- **[文档重组]** 建立三层结构并迁移：
  - 用户（User）：`docs/user/installation.md`、`docs/user/quick_start.md`、`docs/user/usage.md`
  - 开发（Dev）：`docs/dev/architecture.md`、`docs/dev/building.md`、`docs/dev/coding_standards.md`、`docs/dev/contributing.md`、`docs/dev/git_commit_guidelines.md`、`docs/dev/tbb_pipeline.md`、`docs/dev/build_system.md`、`docs/dev/api_design_standards.md`、`docs/dev/cpp20_modules_migration_plan.md`、`docs/dev/integration_test_framework.md`、`docs/dev/performance_benchmark_system.md`、`docs/dev/unified_exception_handling_strategy.md`
  - 内部（Internal）：`docs/internal/`（如 `TASK_SUMMARY.md` 等）
- **[新增]** `docs/README.md` 文档索引（教学导航）。
- **[删除]** `src/common/`（改由 `src/modules/common/` 提供 `fq_common` 目标；该目录未被引用）。

## 影响

- 不影响现有构建/测试/CI。
- 文档导航清晰，便于教学与使用者快速上手。

## 后续建议

- 统一示例与教程的文档风格与最小代码片段，避免大段未验证的伪代码。
- 若新增文档，请归入 `docs/user/` 或 `docs/dev/`，并更新 `docs/README.md` 索引。
