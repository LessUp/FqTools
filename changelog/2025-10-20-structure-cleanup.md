# 2025-10-20 目录精简与结构重构（阶段一）

本次变更目标：遵循 KISS 原则，面向开源发布，精简非必要文件、统一第三方与脚本位置、改进构建与 CI、保持功能不变。

## 已完成变更

- **[文档迁移]** 根目录与 `docs/` 中的内部/阶段性文档迁移至 `docs/internal/`：
  - `BUILD_SYSTEM_MODERNIZATION_REPORT.md`
  - `FastQTools_Feature_Expansion_Analysis.md`
  - `CHANGELOG_TECHNICAL_DEBT.md`
  - `changelog_code_style.md`
  - `docs/ANNOTATION_PROGRESS_TRACKER.csv`
  - `docs/ANNOTATION_TASK_LIST.md`
  - `类结构体注释补全任务报告.md`
- **[第三方头文件规范化]** 将 `src/external/include/gzstream.h` 迁移至 `third_party/gzstream/include/gzstream.h`，并：
  - 修改 `src/core_legacy/core.h` 引用为 `#include "gzstream.h"`
  - 在 `src/CMakeLists.txt` 的 `fq_legacy_lib` 增加 `target_include_directories(… ${CMAKE_SOURCE_DIR}/third_party/gzstream/include)`
- **[移除硬编码路径]** 删除 `src/CMakeLists.txt` 中硬编码的本机 Conan 缓存 include 路径，提升可移植性。
- **[工具链探测放宽]** 顶层 `CMakeLists.txt`：
  - 优先包含 `${CMAKE_BINARY_DIR}/conan_toolchain.cmake`
  - 若未发现且存在 `VCPKG_ROOT`，包含 vcpkg 工具链
  - 否则以系统包方式继续（仅提示，不再 `FATAL_ERROR`）
- **[.gitignore 调整]**
  - 移除对 `config/dependencies/` 的忽略，避免丢失依赖声明更新
  - 新增构建/安装/报告/覆盖率产物忽略（`build*/`、`install*/`、`*.sarif`、`clang-tidy-fixes.yml`、`*.profraw`、`*.profdata` 等）
- **[CI 覆盖率]** `.github/workflows/ci.yml` 新增 `coverage` 任务，运行 `scripts/coverage.sh` 并上传 `coverage-filtered.info`。
- **[开发脚本收敛]** 将 `tools/development/` 中脚本迁移到 `scripts/`：
  - `tools/development/generators/create_module.py` → `scripts/generators/create_module.py`
  - `tools/development/validators/code_quality.py` → `scripts/validators/code_quality.py`

## 待删除项（需审批后执行）

- 目录：`.claude/`、`.clinerules/`、`.codebuddy/`、`.gemini/`、`.kiro/`
- 文件：`clang_tidy_report.txt`、`.github/workflows/ci-cd.yml`、`test.cpp`、`test.cppm`
- 已清空目录：`src/external/include/`、`src/external/`、`tools/development/generators/`、`tools/development/validators/`、`tools/development/`

## 注意事项

- 本次变更不影响功能与构建产物。首次构建仍建议通过 Conan/vcpkg 生成工具链。
- `third_party/` 中的第三方组件需按后续流程补充许可证与来源说明（若适用）。

## 后续计划（阶段二）

- 合并并精简 CI（保留单一工作流文件），增加 Conan/ccache 缓存以加速。
- 补充开源协作文件：`CONTRIBUTING.md`、`CODE_OF_CONDUCT.md`、`SECURITY.md`、`CODEOWNERS`（按需要）。
- 梳理 `tools/benchmark/` 与示例，评估是否升级为顶层 `benchmarks/` 与 `examples/` 的统一示例来源。
