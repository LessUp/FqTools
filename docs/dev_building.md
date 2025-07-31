# 构建指南

## 🏗️ 构建系统概览

FastQTools 使用现代化的构建系统：

- **CMake**: >= 3.20
- **Conan**: >= 2.19
- **编译器**: GCC >= 11.0 或 Clang >= 12.0
- **C++标准**: C++20

## 🚀 快速构建

### 使用构建脚本 (推荐)

我们提供了一个统一的构建脚本来简化编译过程。

```bash
# 使用 Clang 以 Release 模式构建 (默认)
./scripts/build.sh

# 使用 GCC 以 Debug 模式构建
./scripts/build.sh gcc Debug

# 清理构建目录
# rm -rf build-*
```

构建脚本包含以下功能：
- 自动检查依赖项是否安装
- 验证编译器可用性
- 自动清理之前的构建
- 错误处理和验证
- 构建完成后验证可执行文件

### 手动构建

您也可以手动执行 CMake 命令来进行更精细的控制。

```bash
# 1. 配置 (以Clang Debug为例)
cmake -S . -B build-clang-debug \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_BUILD_TYPE=Debug \
    -G "Ninja"

# 2. 构建
cmake --build build-clang-debug

# 3. 运行测试
ctest --test-dir build-clang-debug
```

## ⚙️ 构建选项

### 构建类型

| 类型 | 优化 | 调试信息 | 用途 |
|:--- |:--- |:--- |
| `Debug` | `-O0` | 完整 | 开发和调试 |
| `Release` | `-O3` | 无 | 生产发布 |

### CMake 选项

| 选项 | 默认值 | 描述 |
|:--- |:--- |:--- |
| `BUILD_TESTING` | `ON` | 是否构建测试套件 |

## 🧪 测试

```bash
# 运行所有测试 (假设已在Debug模式下构建)
ctest --test-dir build-clang-debug --output-on-failure
```

## 🔍 代码质量检查

我们提供了完整的代码质量检查工具：

### 运行 lint 检查

```bash
# 检查代码格式和静态分析
./scripts/lint.sh

# 自动修复格式问题
./scripts/lint.sh --fix

# 指定构建目录
./scripts/lint.sh --build-dir build-clang-release
```

### 代码质量工具

- **clang-format**: 自动代码格式化
- **clang-tidy**: 静态代码分析
- **编译器警告**: 启用所有警告 (`-Wall -Wextra -Wpedantic`)
- **构建优化**: Release模式包含性能优化选项

## 📦 打包

我们提供了一个脚本来创建可分发的压缩包。

```bash
# 创建一个版本为 3.0.0 的发布包
./scripts/package.sh 3.0.0
```

此命令将在 `dist/` 目录下生成一个 `fastqtools-v3.0.0-linux-x86_64.tar.gz` 文件。

## 🔄 CI/CD

项目配置了完整的CI/CD流水线，包括：

### GitHub Actions

- **代码质量检查**: 自动运行clang-format和clang-tidy
- **多平台构建**: 测试GCC和Clang编译器
- **多种构建模式**: Debug和Release构建
- **自动化测试**: 运行完整测试套件
- **Docker构建**: 构建和测试Docker镜像

### 构建矩阵

| 操作系统 | 编译器 | 构建模式 |
|:---|:---|:---|
| Ubuntu | GCC 11+ | Release |
| Ubuntu | Clang 12+ | Debug/Release |

### 触发条件

- **推送**: master/main分支
- **Pull Request**: 针对master/main分支的PR
- **定时构建**: 可配置定时检查