# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [3.0.0] - 2025-07-31

### Refactored
- **构建系统**: 对CMake构建系统进行了彻底的现代化重构。放弃了旧的、基于`include()`的脚本模式，全面转向基于目标的（target-based）配置。为每个逻辑组件（如`fq_common`, `fq_cli`）创建了独立的库，明确了依赖关系，极大地提升了构建系统的可维护性和清晰度。
- **核心库结构**: 为了解决因编译器对C++20模块支持不完善而导致的严重编译问题，对核心库进行了重大重构。放弃了实验性的C++20模块（`.cppm`），回归到稳定、可靠的传统头文件/源文件（`.h`/`.cpp`）结构。此举保证了项目的健壮性和在当前工具链下的可编译性。
- **C++标准升级**: 将C++标准从C++20升级到C++26，并添加了相应的编译器版本检查。

### Fixed
- **编译与链接**: 解决了一系列复杂的编译和链接错误。这包括修正因API不匹配导致的测试失败、修复头文件循环依赖和路径问题，以及为一个被声明但未被定义的`operator+=`函数提供了实现。经过此番修复，项目现在可以被成功编译和链接。
- **头文件依赖**: 修复了配置模块中缺失的`<format>`头文件，解决了`fmt::format`相关的编译错误。
- **函数重复定义**: 移除了`config.cpp`中重复定义的`has_key`方法，解决了编译器重复定义错误。
- **主程序构建**: 修复了主程序`FastQTools`的构建问题，确保程序可以正常编译和运行。

### Docs
- **架构文档**: 评审并整合了所有分散的设计文档，创建了一份全新的、统一的架构文档 `docs/design/01_Architecture.md`。该文档不仅包含了项目的设计原则和技术选型，还详细记录了从尝试C++20模块到回归传统结构的决策过程，为项目的未来发展提供了宝贵的参考。

## [Unreleased]

### Changed
- **[兼容性]** 将C++标准从C++26降级到C++20，以提高编译器兼容性
- **[构建系统]** 更新了编译器版本要求：GCC从15.0降至11.0，Clang从19.0降至12.0
- **[构建系统]** 添加了编译器特定的优化选项，包括Release模式的`-O3 -march=native`和LTO支持
- **[构建系统]** 移除了CMakeLists.txt中的硬编码路径，提高了构建系统的可移植性
- **[构建系统]** 优化了构建脚本`build.sh`，增加了错误处理、依赖检查和构建验证
- **[构建系统]** 更新了依赖安装脚本，增加了系统兼容性检查和更好的错误处理
- **[代码质量]** 优化了clang-tidy配置，添加了命名规范检查和排除不必要的规则
- **[CI/CD]** 创建了完整的GitHub Actions CI配置，包括代码质量检查、多平台构建和Docker支持
- **[文档]** 更新了构建文档，添加了代码质量检查和CI/CD流水线说明

### Fixed
- **[性能]** 优化了多个构造函数中的参数传递方式，通过使用按值传递和 `std::move`，减少了不必要的对象拷贝，提升了性能。
- **[健壮性]** 修复了代码中所有危险的窄化转换（narrowing conversions）。对可能导致数据截断的 `size_t` -> `int` 转换添加了安全检查，并对所有整数到浮点数的转换使用了显式的 `static_cast`，以消除潜在的精度损失和未定义行为。
- **[性能]** 将代码库中所有 `std::endl` 替换为 `'
'`，以避免不必要的输出流刷新操作，从而提升在高 I/O 负载下的性能。

### Changed
- **[可读性]** 通过用命名常量替换硬编码的“魔术数字”（如 Phred 分数、缓冲区大小），提高了代码的可读性和可维护性。
- **[可读性]** 重构了整个代码库，将过短的变量名（如 `s`, `it`, `fc`, `c`）替换为更具描述性的名称（如 `source_string`, `command_iterator`, `flow_control`, `character`），显著提高了代码的可读性和可维护性。

### Added
- **[重构]** 集成了 `mimalloc` 高性能内存分配器，作为性能优化的第一阶段。
- **[重构]** 建立了端到端的性能基准测试环境 (`tools/benchmark/performance_benchmark.cpp`)，为即将进行的性能优化提供可靠的性能基准线。
- 为即将进行的性能与并发架构升级创建了详细的设计方案 (`docs/design/performance_and_concurrency_refactor.md`)。

### Changed
- **[重构]** 使用 `tbb::parallel_pipeline` 重构了 `FqStatistic` 的核心处理流程，移除了旧的线程池和 `std::future` 机制，实现了更简洁、更高效的并行处理。
- **[重构]** 使用 `tbb::concurrent_queue` 完全重构了 `common/Conveyor`，移除了自定义的RingBuffer和同步原语，以提升并发性能和代码健壮性。
- **[进行中]** 启动了性能与并发模型的重大重构。该计划包括：
    - 升级到高性能内存分配器 (`mimalloc`)。
    - 使用 Intel TBB 和现代C++并发原语替换自定义实现。
    - 采用 `tbb::parallel_pipeline` 重新设计核心处理流水线。

### Removed
- 移除了对 `bshoshany-thread-pool` 库的依赖，因其功能已被 TBB 流水线完全取代。

- 重构项目目录结构，采用现代化布局
- 建立完整的测试框架（单元测试、集成测试）
- 添加开发工具和脚本（代码生成、质量检查、性能测试）
- 创建分层文档体系（用户指南、开发者指南、API文档）
- 添加使用示例和教程

### Changed
- **[文档]** 将 `docs/dev/coding-standards.md` 翻译为中文。
- 统一目录命名规范（全部使用小写+下划线）
- 重组文档结构，按用户类型分类
- 优化构建系统，支持多种构建预设
- 改进CMake配置，支持现代C++20特性

### Fixed
- 修复包含路径问题
- 统一代码格式化规范
- 改进错误处理和日志输出

## [2.0.0] - 2024-07-29

### Added
- 初始版本发布
- FastQ文件统计分析功能
- 多线程处理支持
- 压缩文件格式支持
- 命令行界面

### Technical Details
- 基于C++20标准
- 使用CMake 3.20+构建系统
- 集成Conan 2.0包管理
- 支持跨平台编译（Linux, macOS, Windows）
