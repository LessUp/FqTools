# 使用说明

本指南详细介绍了 `fastqtools` 的命令行用法。

## `stat` 命令

`stat` 命令用于生成 FastQ 文件的综合统计报告。

### 基本用法

```bash
fastqtools stat [选项]
```

### 选项

- `-i, --input <文件路径>`
  **必需**。输入的 FastQ 文件路径。可以是压缩格式（`.gz`）。

- `-I, --input2 <文件路径>`
  可选。用于双端测序的第二个 FastQ 文件路径。

- `-o, --output <文件路径>`
  **必需**。输出统计报告的文件路径。

- `-t, --threads <数量>`
  可选。用于处理的线程数。默认为 `1`。建议设置为可用的 CPU 核心数以获得最佳性能。

- `-h, --help`
  显示 `stat` 命令的帮助信息。

### 示例

#### 示例 1: 单端文件分析

对一个名为 `sample.fastq.gz` 的文件进行统计，并将结果保存到 `sample.stats.txt`。

```bash
fastqtools stat --input sample.fastq.gz --output sample.stats.txt
```

#### 示例 2: 使用4个线程分析双端文件

```bash
fastqtools stat \
    --input sample_R1.fastq.gz \
    --input2 sample_R2.fastq.gz \
    --output paired_analysis.txt \
    --threads 4
```

## 全局选项

这些选项适用于所有 `fastqtools` 命令。

- `--help`
  显示所有可用命令的列表和它们的简要描述。

- `--version`
  显示 `fastqtools` 的版本信息。

```