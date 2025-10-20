# 构建系统说明

本README文件说明了FastQTools项目的构建系统，包括各种构建脚本和配置选项。

## 🚀 快速开始

### 基本构建

```bash
# 使用默认设置构建（Clang + Release）
./scripts/build.sh

# 使用GCC构建
./scripts/build.sh gcc Release

# 使用Clang构建
./scripts/build.sh clang Debug
```

### 专门的构建脚本

#### GCC 构建脚本

```bash
# 基本GCC构建
./scripts/build-gcc.sh

# GCC Debug构建 + ASAN
./scripts/build-gcc.sh Debug --asan

# GCC Coverage构建
./scripts/build-gcc.sh Coverage
```

#### Clang 构建脚本

```bash
# 基本Clang构建
./scripts/build-clang.sh

# Clang Debug构建 + 所有sanitizers
./scripts/build-clang.sh Sanitize

# Clang Release构建 + 静态分析
./scripts/build-clang.sh Release --static
```

## 🔧 高级构建选项

### 内存安全检查

```bash
# AddressSanitizer (ASAN) - 检测内存错误
./scripts/build.sh clang Debug --asan

# UndefinedBehaviorSanitizer (USAN) - 检测未定义行为
./scripts/build.sh clang Debug --usan

# ThreadSanitizer (TSAN) - 检测数据竞争
./scripts/build.sh clang Debug --tsan
```

### 代码覆盖率

```bash
# 生成代码覆盖率报告
./scripts/coverage.sh clang

# 或者使用构建脚本
./scripts/build.sh gcc Debug --coverage
```

### 静态分析

```bash
# 运行静态分析
./scripts/lint.sh

# 构建时自动运行静态分析
./scripts/build.sh clang Release --static
```

## 📊 质量保证脚本

### 测试脚本

```bash
# 运行所有测试
./scripts/test.sh

# 运行特定测试
./scripts/test.sh -f "*timer*"

# 运行测试并生成覆盖率报告
./scripts/test.sh -c
```

### Sanitizer测试

```bash
# 运行所有sanitizer测试
./scripts/sanitize.sh

# 运行特定sanitizer
./scripts/sanitize.sh clang asan
```

### 代码格式化

```bash
# 检查代码格式
./scripts/format.sh

# 自动修复格式问题
./scripts/lint.sh --fix
```

## 🏗️ 构建类型说明

### Debug构建
- 包含调试信息
- 优化级别：-O0
- 适用于开发和调试

### Release构建
- 不包含调试信息
- 优化级别：-O3
- 启用LTO（链接时优化）
- 适用于生产环境

### RelWithDebInfo构建
- 包含调试信息
- 优化级别：-O2
- 适用于需要调试的性能测试

### Coverage构建
- 包含调试信息和覆盖率 instrumentation
- 适用于代码覆盖率分析

### Sanitize构建
- 包含调试信息和sanitizer instrumentation
- 适用于内存安全检查

## 🔍 故障排除

### 常见问题

1. **找不到conan**
   ```bash
   # 安装conan
   pip install conan
   ```

2. **找不到clang-tidy**
   ```bash
   # Ubuntu/Debian
   sudo apt-get install clang-tidy
   
   # CentOS/RHEL
   sudo yum install clang-tidy
   ```

3. **找不到lcov**
   ```bash
   # Ubuntu/Debian
   sudo apt-get install lcov
   
   # CentOS/RHEL
   sudo yum install lcov
   ```

### 依赖问题

如果遇到依赖问题，请运行：
```bash
./scripts/install_dependencies.sh
```

### 清理构建

如果构建出现问题，可以尝试清理：
```bash
# 清理所有构建目录
rm -rf build-*

# 重新构建
./scripts/build.sh
```

## 📈 性能优化

### 编译器优化

Release构建自动启用以下优化：
- `-O3`: 最高级别的优化
- `-march=native`: 针对当前CPU架构优化
- `-flto`: 链接时优化

### 内存分配器

项目使用mimalloc作为高性能内存分配器，在Release构建中自动启用。

### 并行处理

项目使用Intel TBB进行并行处理，在所有构建类型中启用。

## 📋 CI/CD集成

项目包含完整的CI/CD流水线，包括：

1. **多平台构建测试**
   - GCC和Clang编译器
   - Debug和Release构建类型
   - 各种sanitizer检查

2. **代码质量检查**
   - 静态分析（clang-tidy）
   - 代码格式化（clang-format）
   - 代码覆盖率

3. **性能基准测试**
   - Release构建性能测试
   - 内存使用分析

4. **安全扫描**
   - 内存安全检查
   - 漏洞扫描

## 📚 更多信息

- [构建指南](../docs/dev_building.md)
- [开发者指南](../docs/dev_contributing.md)
- [编码标准](../docs/dev_coding_standards.md)
- [CI/CD配置](../.github/workflows/ci-cd.yml)