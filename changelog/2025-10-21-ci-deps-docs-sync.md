# 2025-10-21 CI/依赖/文档同步

## Fixed
- `.github/workflows/ci.yml`
  - `build-and-test` 作业改为统一调用 `scripts/build.sh` 与 `scripts/test.sh`，与本地保持一致。
- `.gitignore`
  - 新增忽略 `conan_toolchain.cmake`，避免误提交。
- `scripts/coverage.sh`
  - 统一使用 `Coverage` 构建并以 `ctest --test-dir` 运行测试。
  - 修复 Python 覆盖率汇总的 here-doc 参数与多余引号问题。

## Changed
- `config/dependencies/vcpkg.json`
  - `version-string` 更新为 `3.1.0`，补全依赖至与 CMake 一致（`fmt`、`nlohmann-json`、`bzip2`、`xz`、`mimalloc`、`onetbb` 等）。
- `config/dependencies/conandata.yml`
  - 依赖版本与 `conanfile.py` 保持一致（`xz_utils/5.4.5`、`nlohmann_json/3.11.3`、`fmt/10.2.1`）。
- `config/dependencies/conanfile.py`
  - 仅在 `clang` 且 `compiler.libcxx=libc++` 时追加 `-stdlib=libc++`，避免 Linux `libstdc++11` 下冲突。
- `scripts/build.sh`
  - Conan 安装前自动探测并显式传入 `-s compiler` 与 `-s compiler.version`，确保与实际编译器一致。
- `docs/dev/architecture.md`
  - 纠正表述为 C++20，保持与项目构建标准一致。
- `docs/user/usage.md`
  - 统一命令名称为 `FastQTools`，新增并对齐 `filter` 命令选项与示例。

## Note
- 所有修改遵循 KISS 原则，减少配置分叉，统一本地与 CI 的构建入口。
