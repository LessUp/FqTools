# 构建与脚本指南

## 快速开始

```bash
# 推荐：Clang + Release（默认）
./scripts/build.sh

# GCC + Debug
./scripts/build.sh gcc Debug

# 代码格式/静态检查
./scripts/format.sh
./scripts/lint.sh
```

## 构建选项

- 线程/并发相关：由源码实现决定，无需单独配置。
- Sanitizers：
```bash
./scripts/build.sh clang Debug --asan
./scripts/build.sh clang Debug --usan
./scripts/build.sh clang Debug --tsan
```
- 覆盖率：
```bash
./scripts/build.sh gcc Debug --coverage
./scripts/coverage.sh clang
```

## 故障排除

- 依赖问题：
```bash
./scripts/install_dependencies.sh
```
- 清理构建：
```bash
rm -rf build-*
./scripts/build.sh
```

## 参考

- 开发指南：`docs/dev/coding_standards.md`
- 架构说明：`docs/dev/architecture.md`
