# 2025-10-20 脚本精简与合并（阶段五）

遵循 KISS 原则，统一构建与测试入口，删除冗余脚本，避免重复维护与歧义，提升开源友好度。

## 删除脚本

- scripts/build-clang.sh
- scripts/build-gcc.sh
- scripts/build-clang-simple.sh
- scripts/sanitize.sh
- scripts/dev.sh
- scripts/validators/code_quality.py（移除后空目录一并清理）

## 保留脚本

- scripts/build.sh（统一构建入口）
- scripts/coverage.sh（覆盖率生成）
- scripts/lint.sh（静态检查）
- scripts/format.sh（代码格式化）
- scripts/test.sh（测试驱动）
- scripts/package.sh（打包）
- scripts/install_dependencies.sh（依赖安装）
- scripts/dev-container-setup.sh（Dev Container 初始化）

## 文档同步

- 更新 `docs/dev/build_system.md`：
  - 移除对 `build-*.sh` 专门脚本与 `sanitize.sh` 的引用
  - 统一为 `scripts/build.sh` 与 `scripts/coverage.sh` 用法

## 影响评估

- 不影响 CI 与 README 使用路径。
- 开发者与用户仅需记忆统一入口脚本，降低学习成本。
