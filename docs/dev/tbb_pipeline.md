# TBB 流水线指南

## 概述

FastQTools 使用 Intel TBB (Threading Building Blocks) 的 `parallel_pipeline` 来实现高性能的并发处理。这套机制可以充分利用多核CPU的计算能力，并通过重叠 I/O 与计算来显著提升文件处理速度。

## 架构说明

流水线被设计为三个主要阶段：

1.  **输入阶段 (Input Filter)**
    - **职责**: 从文件读取原始数据块，并将其解析为 `FqInfoBatch` 对象。
    - **并发性**: 串行执行 (`serial_in_order`)，以保证文件读取的顺序性。
    - **优化**: 使用内存池来重用 `FqInfoBatch` 对象，减少内存分配开销。

2.  **处理阶段 (Processing Filter)**
    - **职责**: 对 `FqInfoBatch` 中的每一条 read 并行应用所有的过滤器 (Predicates) 和修改器 (Mutators)。
    - **并发性**: 并行执行 (`parallel`)，TBB 会自动将不同的数据批次分发给多个线程。
    - **核心**: 这是计算密集型阶段，也是流水线性能提升的关键。

3.  **输出阶段 (Output Filter)**
    - **职责**: 将处理完成的 `FqInfoBatch` 写入输出文件。
    - **并发性**: 串行执行 (`serial_in_order`)，以保证输出文件的顺序与输入一致。
    - **优化**: 将处理完成的 `FqInfoBatch` 对象释放回内存池。

## 性能与配置

### 关键配置参数

- `thread_count`: 流水线使用的线程数。设置为 `0` 将使用所有可用的硬件核心。
- `batch_size`: 每个 `FqInfoBatch` 中包含的 reads 数量。较大的批次可以减少调度开销，但会增加内存使用。
- `max_tokens`: 流水线中“飞行”的最大批次数。这是控制**背压 (back-pressure)** 的关键参数，可以防止因处理速度跟不上读取速度而导致的内存溢出。通常设置为线程数的2-4倍。

### 性能监控

`TbbProcessingPipeline` 类提供了 `get_performance_stats()` 方法，可以获取详细的性能指标，包括：
- 各阶段的处理时间
- 吞吐量 (MB/s 和 reads/sec)
- CPU 利用率
- 内存池的命中率

这些数据对于分析性能瓶颈和进行参数调优至关重要。
