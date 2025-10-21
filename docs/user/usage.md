# 使用说明

本指南详细介绍了 `FastQTools` 的命令行用法。

## `stat` 命令

`stat` 命令用于生成 FastQ 文件的综合统计报告。

### 基本用法

```bash
FastQTools stat [选项]
```

### 选项

- `-i, --input <文件路径>`
  **必需**。输入的 FastQ 文件路径。可以是压缩格式（`.gz`）。

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
FastQTools stat --input sample.fastq.gz --output sample.stats.txt
```

#### 示例 2: 使用4个线程分析文件

```bash
FastQTools stat \
    --input sample.fastq.gz \
    --output analysis.txt \
    --threads 4
```

## `filter` 命令

`filter` 命令用于对 FastQ 读段进行过滤与修剪。

### 基本用法

```bash
FastQTools filter [选项]
```

### 选项

- `-i, --input <文件路径>`
  **必需**。输入的 FastQ 文件路径（支持 `.gz`）。

- `-o, --output <文件路径>`
  **必需**。输出过滤/修剪后的 FastQ 文件路径。

- `-t, --threads <数量>`
  可选。处理线程数，默认 `1`。

- `--quality-encoding <33|64>`
  可选。质量值编码偏移，默认 `33`。

- `--min-quality <浮点数>`
  可选。最小平均质量阈值（过滤低于该值的读段）。

- `--min-length <整数>`
  可选。最小读长（过滤短于该值的读段）。

- `--max-length <整数>`
  可选。最大读长（过滤长于该值的读段）。

- `--max-n-ratio <0.0-1.0>`
  可选。读段中碱基 `N` 的最大比例。

- `--trim-quality <浮点数>`
  可选。质量修剪阈值（从读段两端/指定端修剪低质量碱基）。

- `--trim-mode <both|five|three>`
  可选。修剪模式，默认 `both`（两端）。`five` 表示 5' 端；`three` 表示 3' 端。

- `-h, --help`
  显示 `filter` 命令帮助信息。

### 示例

#### 示例 1: 质量阈值与长度过滤

```bash
FastQTools filter \
  --input sample.fastq.gz \
  --output sample.filtered.fastq.gz \
  --min-quality 20 \
  --min-length 50 \
  --threads 4
```

#### 示例 2: 仅进行质量修剪（3'端）

```bash
FastQTools filter \
  --input sample.fastq.gz \
  --output sample.trimmed.fastq.gz \
  --trim-quality 20 \
  --trim-mode three \
  --quality-encoding 33
```

## 全局选项

这些选项适用于所有 `FastQTools` 命令。

- `--help`
  显示所有可用命令的列表和它们的简要描述。

- `--version`
  显示 `FastQTools` 的版本信息。