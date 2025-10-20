# 安装指南

## 🚀 快速安装

### 系统要求

- **操作系统**: Linux, macOS, Windows
- **编译器**: GCC 15+, Clang 19+
- **CMake**: 3.28 或更高版本
- **内存**: 建议至少 4GB RAM
- **存储**: 至少 1GB 可用空间

### 从源码编译 (推荐)

我们推荐从源码编译以获得最佳性能和最新的功能。

```bash
# 1. 克隆本仓库
git clone https://github.com/your-org/fastqtools.git
cd fastqtools

# 2. 运行依赖安装脚本 (仅限首次配置)
# 这将安装与项目兼容的特定版本的编译器和工具
./scripts/install_dependencies.sh

# 3. 运行构建脚本
# 这将使用 Clang 以 Release 模式进行编译
./scripts/build.sh

# 4. (可选) 将可执行文件安装到系统路径
sudo cp build-clang-Release/FastQTools /usr/local/bin/

# 5. 验证安装
fastqtools --version
```

### 使用 Docker

对于希望在隔离环境中运行的用户，我们提供了 Dockerfile。

```bash
# 1. 克隆本仓库
git clone https://github.com/your-org/fastqtools.git
cd fastqtools

# 2. 构建 Docker 镜像
docker build -t fastqtools:latest -f config/deployment/Dockerfile .

# 3. 运行容器
docker run -it --rm fastqtools:latest --help
```

## 🎯 基本使用

安装完成后，您可以开始使用 `fastqtools`。最常用的功能是对 FastQ 文件进行统计分析：

```bash
# 对单个 FastQ 文件进行基本统计
fastqtools stat -i input.fastq.gz -o output.stat.txt

# 使用8个线程处理双端测序数据
fastqtools stat -i read1.fq.gz -I read2.fq.gz -o paired.stat.txt -t 8
```

## 🆘 获取帮助

如果您在安装或使用过程中遇到任何问题，请查阅以下资源：

- 🐛 **报告问题**: [GitHub Issues](https://github.com/your-org/fastqtools/issues)
- 📖 **查阅文档**: 阅读本目录下的其他文档以获取更详细的信息。