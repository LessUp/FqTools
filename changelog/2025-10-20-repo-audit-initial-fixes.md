# 2025-10-20 仓库体检与初始修复

## 概览
- 完成首轮仓库体检与一批构建/脚本/文档一致性修复，遵循 KISS 原则。

## Fixed
- `README.md`
  - 修正编译器要求为 `GCC 11+ / Clang 12+`。
  - 修复用户指南链接：`docs/user/overview.md` → `docs/user/usage.md`。
- `scripts/build.sh`
  - `conan install` 强制使用 `compiler.cppstd=20` 与 `compiler.libcxx=libstdc++11`，避免工具链降回 C++17。
  - 修复 ASAN/USAN/TSAN/coverage 标志拼接方式，避免错误级联。
- `scripts/package.sh`
  - 修复产物路径大小写，使用 `build-clang-release/FastQTools`。
- `scripts/test.sh`
  - 与 `build.sh` 目录命名统一：`build-<compiler>-<build_type>`，使用 `ctest --test-dir` 运行测试。
  - 支持 `-c/--compiler`、`-t/--type`、`-C/--coverage`、`-j/--jobs` 等选项。
- `scripts/coverage.sh`
  - 统一覆盖率构建为 `Coverage`，用 `ctest --test-dir` 执行测试。
  - 修复 Python 覆盖率汇总的 here-doc 传参和多余引号问题。
- `src/CMakeLists.txt`
  - 将 `mimalloc-static` 替换为 `mimalloc::mimalloc`，匹配 Conan 导出的 CMake 目标。
- `tests/utils/CMakeLists.txt`
  - 统一 GTest 目标名为 `GTest::gtest`，与上层一致。
- `config/dependencies/conanfile.py`
  - 仅在 `clang` 且 `compiler.libcxx=libc++` 时追加 `-stdlib=libc++`，避免 Linux 下与 `libstdc++11` 冲突。

## 后续建议
- 清理误提交的生成文件（如根目录 `conan_toolchain.cmake`）。
- 在 CI 中增加 `scripts/test.sh` 与 `scripts/coverage.sh` 的使用，统一本地/CI 入口。
- 为发行包增加“可移植构建”（禁用 `-march=native`），保留当前“本机优化构建”。
- 评审 `config/dependencies/` 版本与项目版本号一致性（`vcpkg.json`、`conandata.yml`）。
