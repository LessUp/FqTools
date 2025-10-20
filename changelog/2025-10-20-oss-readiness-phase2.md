# 2025-10-20 开源就绪（阶段二）

本次变更遵循 KISS 原则，面向开源发布的最小必要补充与优化。

## 已完成变更

- **[协作文档]** 新增：`CONTRIBUTING.md`、`CODE_OF_CONDUCT.md`、`SECURITY.md`、`.github/CODEOWNERS`。
- **[第三方目录说明]** 新增：`third_party/README.md`，明确第三方头文件管理与合规指引。
- **[README 更新]** 同步当前目录结构与构建/运行指令（统一使用 `scripts/build.sh` 与 Conan toolchain 示例）。
- **[构建加速]** `scripts/build.sh` 自动启用 `ccache`（如存在）。
- **[CI 优化]** `.github/workflows/ci.yml`：
  - 在 `lint` 作业中配置 Conan 与 `ccache` 缓存（去重修正）。
  - 在 `build-and-test`、`coverage` 作业中添加 Conan 与 `ccache` 缓存，并在构建中启用 `-DCMAKE_CXX_COMPILER_LAUNCHER=ccache`。
  - 在相关步骤安装 `ccache` 与 `lcov`（覆盖率）。

## 不涉及的变更

- 未修改核心业务逻辑与库接口；不影响现有功能。

## 建议后续事项

- **[第三方合规]** 为 `third_party/gzstream` 补充来源链接与许可证条款，必要时引入 LICENSE 文件。
- **[维护者配置]** 更新 `.github/CODEOWNERS` 中的 GitHub 账号；在 `SECURITY.md` 中补充维护者联系方式。
- **[Conan 元信息]** 在 `conanfile.py` 中更新 `author` 与 `url` 为实际信息。
- **[CI 缓存]** 后续可加入 ccache 统计与命中率日志，以评估加速效果。
