# FastQTools 基本使用示例

本目录包含 FastQTools 的基本使用示例，帮助用户快速上手。

## 📁 示例列表

### 1. 基本统计分析
- **文件**: `basic_stats.sh`
- **功能**: 演示如何对 FastQ 文件进行基本统计分析
- **适用**: 初学者

### 2. 批量处理
- **文件**: `batch_processing.sh`
- **功能**: 演示如何批量处理多个 FastQ 文件
- **适用**: 需要处理大量文件的用户

### 3. 双端数据处理
- **文件**: `paired_end_analysis.sh`
- **功能**: 演示如何处理双端测序数据
- **适用**: 处理配对读取的用户

### 4. 自定义输出格式
- **文件**: `custom_output.sh`
- **功能**: 演示如何自定义统计结果的输出格式
- **适用**: 需要特定输出格式的用户

## 🚀 快速开始

### 准备测试数据

```bash
# 下载示例数据
wget https://github.com/your-org/fastqtools/raw/main/examples/data/sample.fastq.gz

# 或者生成测试数据
./generate_test_data.sh
```

### 运行示例

```bash
# 基本统计
./basic_stats.sh sample.fastq.gz

# 批量处理
./batch_processing.sh *.fastq.gz

# 双端数据
./paired_end_analysis.sh read1.fq.gz read2.fq.gz
```

## 📊 预期输出

每个示例都会生成相应的统计报告文件，包含：

- 读取数量和长度分布
- 碱基组成和 GC 含量
- 质量分数分布
- 错误率估算

## 🔧 自定义参数

所有示例都支持通过环境变量自定义参数：

```bash
# 设置线程数
export FASTQTOOLS_THREADS=8

# 设置批处理大小
export FASTQTOOLS_BATCH_SIZE=100000

# 运行示例
./basic_stats.sh input.fastq.gz
```

## 📝 注意事项

1. 确保 FastQTools 已正确安装并在 PATH 中
2. 示例脚本需要可执行权限：`chmod +x *.sh`
3. 某些示例可能需要较大内存，建议在配置充足的机器上运行
4. 处理大文件时，建议使用 SSD 存储以提高性能

## 🆘 故障排除

如果遇到问题，请检查：

1. FastQTools 版本是否兼容
2. 输入文件格式是否正确
3. 系统资源是否充足
4. 权限设置是否正确

更多帮助请参考 [用户指南](../../docs/user-guide/) 或提交 [Issue](https://github.com/your-org/fastqtools/issues)。
