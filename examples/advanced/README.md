# FastQTools 高级使用示例

本目录包含 FastQTools 的高级使用示例，展示复杂的分析流程和自定义功能。

## 📁 高级示例列表

### 1. 性能优化分析
- **文件**: `performance_analysis.py`
- **功能**: 分析不同参数对性能的影响，找到最优配置
- **适用**: 需要处理大量数据的高级用户

### 2. 质量控制流水线
- **文件**: `quality_control_pipeline.sh`
- **功能**: 完整的质量控制流程，包括预处理、分析和报告
- **适用**: 生产环境的质量控制

### 3. 多样本比较分析
- **文件**: `multi_sample_comparison.py`
- **功能**: 比较多个样本的统计特征，生成比较报告
- **适用**: 需要比较不同样本的研究

### 4. 自定义统计指标
- **文件**: `custom_metrics.cpp`
- **功能**: 演示如何扩展 FastQTools 添加自定义统计指标
- **适用**: 需要特殊分析的开发者

### 5. 大数据处理策略
- **文件**: `big_data_processing.sh`
- **功能**: 处理超大文件的策略和技巧
- **适用**: 处理 TB 级数据的用户

## 🚀 运行要求

### 系统要求
- **内存**: 至少 8GB RAM（推荐 16GB+）
- **存储**: 至少 100GB 可用空间
- **CPU**: 多核处理器（推荐 8+ 核心）

### 软件依赖
- FastQTools 2.0+
- Python 3.8+（用于 Python 脚本）
- R 4.0+（用于统计分析，可选）
- GNU parallel（用于并行处理，可选）

### 安装依赖

```bash
# Python 依赖
pip install -r requirements.txt

# R 依赖（可选）
Rscript install_packages.R

# 系统工具
sudo apt-get install parallel
```

## 📊 示例详情

### 性能优化分析

```bash
# 运行性能分析
python3 performance_analysis.py \
    --input large_dataset.fastq.gz \
    --output performance_report.html \
    --test-threads 1,2,4,8,16 \
    --test-batch-sizes 10000,50000,100000
```

**输出**: 详细的性能报告，包括最优参数建议

### 质量控制流水线

```bash
# 运行完整的质量控制流程
./quality_control_pipeline.sh \
    --input-dir /path/to/fastq/files \
    --output-dir /path/to/results \
    --config qc_config.yaml
```

**输出**: 
- 质量控制报告
- 过滤后的数据
- 可视化图表

### 多样本比较

```bash
# 比较多个样本
python3 multi_sample_comparison.py \
    --samples sample1.fq.gz,sample2.fq.gz,sample3.fq.gz \
    --labels "Control,Treatment1,Treatment2" \
    --output comparison_report.html
```

**输出**: 交互式比较报告

## 🔧 配置文件

### 性能配置 (`performance_config.yaml`)

```yaml
performance:
  threads:
    min: 1
    max: 32
    step: 2
  batch_sizes:
    - 10000
    - 50000
    - 100000
    - 500000
  memory_limit: "16GB"
  
benchmarks:
  file_sizes:
    - "1GB"
    - "10GB"
    - "100GB"
  iterations: 3
```

### 质量控制配置 (`qc_config.yaml`)

```yaml
quality_control:
  filters:
    min_length: 50
    max_length: 300
    min_quality: 20
    max_n_content: 0.1
  
  statistics:
    - basic_stats
    - quality_distribution
    - length_distribution
    - gc_content
    - duplication_rate
  
  output:
    format: ["txt", "json", "html"]
    plots: true
    summary: true
```

## 📈 性能基准

### 测试环境
- **CPU**: Intel Xeon E5-2680 v4 (14 cores)
- **内存**: 64GB DDR4
- **存储**: NVMe SSD

### 基准结果

| 文件大小 | 线程数 | 批大小 | 处理时间 | 内存使用 |
|----------|--------|--------|----------|----------|
| 1GB      | 8      | 50K    | 2.3 min  | 1.2GB    |
| 10GB     | 16     | 100K   | 18.7 min | 3.8GB    |
| 100GB    | 32     | 500K   | 2.8 hrs  | 12.1GB   |

## 🛠️ 自定义扩展

### 添加自定义统计指标

```cpp
// custom_metrics.cpp
#include "statistics/IStatistic.h"

class CustomMetric : public fq::statistics::IStatistic {
public:
    Result calculate(const Batch& batch) override {
        // 实现自定义统计逻辑
        return result;
    }
};
```

### 编译自定义扩展

```bash
# 编译自定义模块
g++ -std=c++20 -shared -fPIC \
    -I../../src \
    custom_metrics.cpp \
    -o libcustom_metrics.so

# 使用自定义模块
export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
fastqtools stat --plugin libcustom_metrics.so -i input.fq.gz
```

## 🚨 注意事项

1. **内存管理**: 大文件处理时注意内存使用，适当调整批大小
2. **并行处理**: 线程数不宜超过 CPU 核心数的 2 倍
3. **存储 I/O**: 使用 SSD 可显著提升性能
4. **网络存储**: 避免在网络存储上直接处理大文件

## 🔍 故障排除

### 内存不足
```bash
# 减少批大小
fastqtools stat -i large.fq.gz -b 10000

# 使用流式处理
fastqtools stat -i large.fq.gz --streaming
```

### 性能问题
```bash
# 启用性能分析
fastqtools stat -i input.fq.gz --profile

# 查看系统资源使用
htop
iotop
```

### 错误诊断
```bash
# 启用详细日志
export FASTQTOOLS_LOG_LEVEL=debug
fastqtools stat -i input.fq.gz -v

# 检查文件完整性
fastqtools validate -i input.fq.gz
```

## 📚 进一步学习

- [开发者指南](../../docs/developer-guide/) - 了解内部实现
- [API 文档](../../docs/api/) - 编程接口参考
- [性能优化指南](../../docs/performance/) - 深入的性能优化技巧
- [社区示例](https://github.com/fastqtools/community-examples) - 更多社区贡献的示例
