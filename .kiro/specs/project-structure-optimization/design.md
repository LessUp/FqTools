# 项目目录结构优化设计文档

## 概述

本设计文档详细描述了FastQTools 2.0项目目录结构优化的技术方案。通过重新组织项目结构，我们将创建一个更加清晰、标准化和易于维护的项目布局。

## 架构

### 目标目录结构

```
fastqtools/
├── README.md                    # 项目主要说明文档
├── LICENSE                      # 许可证文件
├── CHANGELOG.md                 # 变更日志
├── .gitignore                   # Git忽略规则
├── .clang-format               # 代码格式化配置
│
├── build/                      # 构建输出目录（gitignore）
├── cmake-build-*/              # IDE构建目录（gitignore）
│
├── cmake/                      # CMake模块和配置
│   ├── FastQToolsConfig.cmake.in
│   ├── modules/                # 自定义CMake模块
│   └── toolchain/              # 工具链配置
│
├── scripts/                    # 构建和开发脚本
│   ├── build.sh               # 主构建脚本
│   ├── dev.sh                 # 开发辅助脚本
│   ├── format.sh              # 代码格式化脚本
│   └── test.sh                # 测试运行脚本
│
├── config/                     # 项目配置文件
│   ├── CMakeLists.txt         # 根CMake配置
│   ├── CMakePresets.json      # CMake预设配置
│   ├── conanfile.py           # Conan依赖配置
│   ├── conandata.yml          # Conan数据配置
│   ├── vcpkg.json             # vcpkg依赖配置
│   └── Dockerfile             # Docker配置
│
├── docs/                       # 项目文档
│   ├── README.md              # 文档索引
│   ├── user/                  # 用户文档
│   │   ├── installation.md    # 安装指南
│   │   ├── usage.md           # 使用指南
│   │   └── examples/          # 使用示例
│   ├── dev/                   # 开发者文档
│   │   ├── architecture.md    # 架构设计
│   │   ├── contributing.md    # 贡献指南
│   │   ├── coding-standards.md # 编码规范
│   │   └── api/               # API文档
│   └── design/                # 设计文档
│       ├── requirements.md    # 需求文档
│       └── specifications.md  # 技术规格
│
├── src/                        # 源代码
│   ├── CMakeLists.txt         # 源码CMake配置
│   ├── common/                # 通用模块（重命名）
│   ├── fastq/                 # FastQ处理模块（重命名）
│   ├── encoder/               # 编码模块
│   ├── statistics/            # 统计模块（重命名）
│   └── processing/            # 处理流水线模块
│
├── app/                        # 应用程序入口
│   ├── CMakeLists.txt
│   ├── main.cpp
│   └── commands/              # 命令实现（重命名）
│
├── tests/                      # 测试代码
│   ├── CMakeLists.txt
│   ├── unit/                  # 单元测试
│   ├── integration/           # 集成测试
│   ├── fixtures/              # 测试数据
│   └── utils/                 # 测试工具
│
├── tools/                      # 开发工具和实用程序
│   ├── benchmarks/            # 性能基准测试
│   ├── profiling/             # 性能分析工具
│   └── generators/            # 代码生成工具
│
└── third_party/               # 第三方依赖（如果需要）
    └── README.md              # 第三方依赖说明
```

## 组件和接口

### 目录重组策略

#### 1. 根目录清理
- 移动构建脚本到 `scripts/` 目录
- 移动配置文件到 `config/` 目录
- 保留核心项目文件在根目录

#### 2. 文档重组
- 合并冗余的设计文档
- 按用户类型重新组织文档结构
- 建立清晰的文档导航

#### 3. 源码目录标准化
- 使用小写目录名（符合Unix约定）
- 重命名模块目录使其更加直观
- 建立清晰的模块边界

#### 4. 测试结构优化
- 按测试类型分组
- 添加测试数据管理
- 改进测试工具组织

## 数据模型

### 文件移动映射

```yaml
移动操作:
  构建脚本:
    - build.sh → scripts/build.sh
    - dev.sh → scripts/dev.sh
  
  配置文件:
    - CMakeLists.txt → config/CMakeLists.txt
    - CMakePresets.json → config/CMakePresets.json
    - conanfile.py → config/conanfile.py
    - conandata.yml → config/conandata.yml
    - vcpkg.json → config/vcpkg.json
    - Dockerfile → config/Dockerfile
  
  文档重组:
    - doc/ → docs/
    - 合并重复的设计文档
    - 创建用户和开发者文档分类
  
  源码重命名:
    - src/Common/ → src/common/
    - src/FastQ/ → src/fastq/
    - src/FqStatistic/ → src/statistics/
    - app/Commands/ → app/commands/

删除操作:
  废弃文件:
    - CMakeUserPresets.json
    - doc/Software_Engineering_Detailed_Design.md
    - doc/Ultra_Detailed_Software_Design.md
  
  IDE文件:
    - .idea/ (添加到.gitignore)
    - cmake-build-debug/ (添加到.gitignore)
```

## 错误处理

### 文件移动错误处理
- 检查目标目录是否存在，不存在则创建
- 验证文件移动操作的完整性
- 提供回滚机制以防操作失败

### 依赖更新错误处理
- 自动更新CMakeLists.txt中的路径引用
- 更新包含路径和链接配置
- 验证构建系统的完整性

## 测试策略

### 结构验证测试
1. 验证所有必需的目录都已创建
2. 验证文件移动操作的完整性
3. 验证CMake配置的正确性

### 构建系统测试
1. 测试重组后的项目能够正常构建
2. 测试所有构建预设仍然有效
3. 测试依赖管理系统正常工作

### 文档完整性测试
1. 验证所有文档链接的有效性
2. 验证文档结构的逻辑性
3. 验证文档内容的一致性

## 实施计划

### 阶段1：准备工作
1. 备份当前项目状态
2. 创建新的目录结构
3. 准备文件移动脚本

### 阶段2：文件重组
1. 移动构建脚本和配置文件
2. 重组文档结构
3. 重命名源码目录

### 阶段3：配置更新
1. 更新CMakeLists.txt文件
2. 更新构建脚本中的路径
3. 更新.gitignore规则

### 阶段4：验证和测试
1. 验证项目构建正常
2. 运行测试套件
3. 验证文档完整性

### 阶段5：清理和优化
1. 删除废弃文件
2. 优化.gitignore规则
3. 更新项目文档