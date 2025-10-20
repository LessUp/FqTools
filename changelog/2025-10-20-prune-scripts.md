# 2025-10-20 脚本与示例精简（阶段三）

遵循 KISS 原则，清理未被构建/CI引用、与当前结构不一致或有误导性的脚本与示例，保持仓库精简。

## 删除项

- scripts/verify_refactor.py
  - 原因：对目录与模块的期望（app/commands、tools/benchmarks 等）与现状不符，未在 CI/构建中使用，为一次性重构辅助脚本。
- scripts/update_includes.py
  - 原因：一次性路径重写工具（将大写目录改为小写），当前源码已统一，不再需要。
- scripts/normalize_comments.py
  - 原因：批量生成注释，可能引入噪音，不在 CI/构建链路中使用。
- scripts/optimize_structure.py
  - 原因：危险的自动重组脚本，逻辑与当前结构不一致，且不在 CI/构建中使用。
- scripts/generators/create_module.py
  - 原因：生成到 src/<module> 与当前 src/modules/<module> 组织不一致，容易误导；不在 CI/构建中使用。
- examples/advanced/（仅 README）
  - 原因：README 引用的脚本/文件并不存在，信息误导，目录中仅包含此 README。

## 保留说明

- src/core_legacy/core.cpp / core.h / std.h
  - 仍被 `src/CMakeLists.txt` 中的 `fq_legacy_lib` 使用，当前构建所必需，待后续模块化迁移完成后再评估移除。

## 影响评估

- 不影响构建、测试与 CI。
- 移除内容均未被工作流/构建引用，且不属于用户必要文档。
