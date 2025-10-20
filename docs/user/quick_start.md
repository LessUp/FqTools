# 快速入门

## 🎯 项目简介

FastQTools 是一个现代化的生物信息学命令行工具集，专注于高性能的 FastQ 文件处理和分析。本项目采用 C++26 标准开发，旨在提供卓越的性能和现代化的代码实现。

### 核心价值
- **高性能**: 利用 Intel TBB 实现多线程并行处理，最大化利用计算资源。
- **现代化**: 基于 C++26 标准，采用现代 CMake 和 Conan 2.0 进行构建和依赖管理。
- **易用性**: 简洁、直观的命令行接口。
- **可扩展性**: 采用基于接口和工厂模式的模块化设计，易于添加新功能。

## 🚀 可用功能

### `fastqtools stat` - FastQ 统计分析

这是目前已实现的核心功能，用于对 FastQ 文件进行全面的质量和内容统计。

#### 使用示例
```bash
# 基本用法：对单个压缩的 FastQ 文件进行统计
fastqtools stat --input input.fastq.gz --output output.stat.txt

# 使用8个线程处理双端测序数据
fastqtools stat --input read1.fq.gz --input2 read2.fq.gz --output paired.stat.txt --threads 8
```

#### 输出指标
- **读数统计**: 总读数、有效读数
- **长度分布**: 最小/最大/平均长度
- **质量分析**: Q20/Q30 碱基的百分比
- **碱基组成**: A, T, C, G, N 碱基的比例和分布
- **GC 含量**: 整体和位置特异性 GC 含量

## 🛠️ 技术栈

- **语言标准**: C++26
- **构建系统**: CMake (>= 3.28)
- **包管理**: Conan (>= 2.19)
- **并行计算**: Intel TBB
- **命令行解析**: cxxopts
- **日志**: spdlog

## 下一步

- 阅读 **[安装指南](./user_installation.md)** 了解如何安装本工具。
- 阅读 **[使用说明](./user_usage.md)** 了解所有命令和参数的详细信息。
