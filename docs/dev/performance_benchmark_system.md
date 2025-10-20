# FastQTools 性能基准测试系统

## 📋 概述

本文档为 FastQTools 项目设计了完整的性能基准测试系统，用于监控性能变化、识别性能瓶颈和验证优化效果。

## 🎯 性能测试目标

### 1. 测试覆盖范围
- **文件 I/O 性能**: 读取、写入、压缩处理
- **内存使用效率**: 内存分配、缓存策略、对象池效率
- **CPU 计算性能**: 序列处理、质量计算、统计算法
- **并发处理性能**: 线程扩展性、锁竞争、同步开销
- **端到端性能**: 完整工作流程的性能表现

### 2. 性能指标
- **吞吐量**: 每秒处理的记录数和碱基数
- **延迟**: 单个记录的处理时间
- **内存使用**: 峰值内存使用和内存分配频率
- **CPU 利用率**: 多核利用效率和计算密度
- **可扩展性**: 随数据量和线程数变化的性能表现

## 🏗️ 性能测试架构

### 1. 测试框架结构
```
tools/benchmark/
├── performance_benchmark.cpp      # 主基准测试程序
├── benchmark_suite.h             # 基准测试套件
├── benchmark_cases/              # 具体测试用例
│   ├── file_io_benchmarks.cpp
│   ├── memory_benchmarks.cpp
│   ├── cpu_benchmarks.cpp
│   ├── concurrent_benchmarks.cpp
│   └── end_to_end_benchmarks.cpp
├── benchmark_reporters/          # 结果报告器
│   ├── console_reporter.h
│   ├── json_reporter.h
│   └── html_reporter.h
├── benchmark_data/               # 测试数据生成器
│   ├── data_generator.h
│   └── workload_profiles.h
└── benchmark_utils/              # 工具函数
    ├── performance_counter.h
    ├── memory_tracker.h
    └── statistics.h
```

### 2. 基准测试核心类
```cpp
// tools/benchmark/benchmark_suite.h
#pragma once

#include <benchmark/benchmark.h>
#include <memory>
#include <vector>
#include <string>
#include <map>

namespace fq::benchmark {

class BenchmarkSuite {
public:
    static auto get_instance() -> BenchmarkSuite&;
    
    // 测试注册
    auto register_benchmark(const std::string& name,
                          std::unique_ptr<BenchmarkCase> benchmark) -> void;
    
    // 测试执行
    auto run_benchmarks(const BenchmarkConfig& config) -> BenchmarkResults;
    
    // 结果报告
    auto generate_report(const BenchmarkResults& results,
                        const std::string& format) -> std::string;
    
    // 性能比较
    auto compare_with_baseline(const BenchmarkResults& current,
                              const BenchmarkResults& baseline) -> PerformanceComparison;
    
private:
    BenchmarkSuite() = default;
    std::map<std::string, std::unique_ptr<BenchmarkCase>> m_benchmarks;
};

// 基准测试基类
class BenchmarkCase {
public:
    virtual ~BenchmarkCase() = default;
    virtual auto run(benchmark::State& state) -> void = 0;
    virtual auto get_name() const -> std::string = 0;
    virtual auto get_description() const -> std::string = 0;
    virtual auto get_category() const -> std::string = 0;
};

// 基准测试配置
struct BenchmarkConfig {
    size_t min_time = 1;           // 最小运行时间（秒）
    size_t iterations = 0;         // 迭代次数（0表示自动）
    size_t warmup_iterations = 3;  // 预热迭代次数
    bool use_real_time = true;     // 使用真实时间
    bool measure_memory = true;   // 测量内存使用
    std::string output_format = "console"; // 输出格式
};

// 基准测试结果
struct BenchmarkResult {
    std::string name;
    std::string category;
    double mean_time = 0.0;
    double std_dev = 0.0;
    size_t iterations = 0;
    size_t memory_bytes = 0;
    std::map<std::string, double> additional_metrics;
};

struct BenchmarkResults {
    std::vector<BenchmarkResult> results;
    std::string timestamp;
    std::string git_commit;
    std::string system_info;
};

// 性能比较结果
struct PerformanceComparison {
    std::vector<std::string> improved;
    std::vector<std::string> regressed;
    std::vector<std::string> unchanged;
    double threshold_percent = 5.0;
};

} // namespace fq::benchmark
```

## 📊 详细测试用例

### 1. 文件 I/O 性能测试
```cpp
// tools/benchmark/benchmark_cases/file_io_benchmarks.cpp
#include "benchmark_suite.h"
#include "src/modules/io/io.h"
#include "src/modules/fastq/fastq.h"

namespace fq::benchmark {

class FileReadBenchmark : public BenchmarkCase {
public:
    auto get_name() const -> std::string override { return "FileRead"; }
    auto get_description() const -> std::string override { 
        return "Measure FASTQ file reading performance"; 
    }
    auto get_category() const -> std::string override { return "I/O"; }
    
    auto run(benchmark::State& state) -> void override {
        auto file_size = state.range(0);
        auto test_file = generate_test_fastq_file(file_size);
        
        for (auto _ : state) {
            auto reader = std::make_unique<FastQReader>(test_file);
            size_t total_records = 0;
            
            while (auto record = reader->read_record()) {
                total_records++;
                state.PauseTiming(); // 暂停计时，避免测量验证开销
                verify_record(*record);
                state.ResumeTiming(); // 恢复计时
            }
            
            state.SetItemsProcessed(total_records);
            state.SetBytesProcessed(file_size);
        }
    }
};

class FileWriteBenchmark : public BenchmarkCase {
public:
    auto get_name() const -> std::string override { return "FileWrite"; }
    auto get_description() const -> std::string override { 
        return "Measure FASTQ file writing performance"; 
    }
    auto get_category() const -> std::string override { return "I/O"; }
    
    auto run(benchmark::State& state) -> void override {
        auto record_count = state.range(0);
        auto read_length = state.range(1);
        auto records = generate_test_records(record_count, read_length);
        auto output_file = get_temp_file_path();
        
        for (auto _ : state) {
            auto writer = std::make_unique<FastQWriter>(output_file);
            
            for (const auto& record : records) {
                state.PauseTiming();
                auto copy = record; // 避免测量拷贝开销
                state.ResumeTiming();
                
                writer->write_record(copy);
            }
            
            state.SetItemsProcessed(record_count);
            state.SetBytesProcessed(record_count * read_length * 2); // sequence + quality
        }
    }
};

class CompressionBenchmark : public BenchmarkCase {
public:
    auto get_name() const -> std::string override { return "Compression"; }
    auto get_description() const -> std::string override { 
        return "Measure compressed file processing performance"; 
    }
    auto get_category() const -> std::string override { return "I/O"; }
    
    auto run(benchmark::State& state) -> void override {
        auto compression_type = static_cast<CompressionType>(state.range(0));
        auto file_size = state.range(1);
        auto test_file = generate_compressed_test_file(file_size, compression_type);
        
        for (auto _ : state) {
            auto reader = std::make_unique<FastQReader>(test_file);
            size_t total_records = 0;
            
            while (auto record = reader->read_record()) {
                total_records++;
                verify_record(*record);
            }
            
            state.SetItemsProcessed(total_records);
            state.SetBytesProcessed(file_size);
        }
    }
};

BENCHMARK_REGISTER_F(FileReadBenchmark, FileRead)
    ->Args({1024 * 1024})    // 1MB
    ->Args({10 * 1024 * 1024})  // 10MB
    ->Args({100 * 1024 * 1024}) // 100MB
    ->Unit(benchmark::kMillisecond);

BENCHMARK_REGISTER_F(FileWriteBenchmark, FileWrite)
    ->Args({1000, 100})     // 1000 records, 100bp
    ->Args({10000, 100})    // 10000 records, 100bp
    ->Args({100000, 100})   // 100000 records, 100bp
    ->Unit(benchmark::kMillisecond);

BENCHMARK_REGISTER_F(CompressionBenchmark, Compression)
    ->Args({static_cast<int>(CompressionType::Gzip), 10 * 1024 * 1024})
    ->Args({static_cast<int>(CompressionType::Bzip2), 10 * 1024 * 1024})
    ->Args({static_cast<int>(CompressionType::Xz), 10 * 1024 * 1024})
    ->Unit(benchmark::kMillisecond);

} // namespace fq::benchmark
```

### 2. 内存性能测试
```cpp
// tools/benchmark/benchmark_cases/memory_benchmarks.cpp
#include "benchmark_suite.h"
#include "src/memory/batch_memory_manager.h"
#include "src/modules/fastq/fastq.h"

namespace fq::benchmark {

class MemoryAllocationBenchmark : public BenchmarkCase {
public:
    auto get_name() const -> std::string override { return "MemoryAllocation"; }
    auto get_description() const -> std::string override { 
        return "Measure memory allocation and deallocation performance"; 
    }
    auto get_category() const -> std::string override { return "Memory"; }
    
    auto run(benchmark::State& state) -> void override {
        auto batch_size = state.range(0);
        auto allocation_count = state.range(1);
        
        for (auto _ : state) {
            std::vector<std::unique_ptr<FqBatch>> batches;
            
            for (size_t i = 0; i < allocation_count; ++i) {
                state.PauseTiming();
                // 预先分配内存以避免测量分配开销
                batches.reserve(allocation_count);
                state.ResumeTiming();
                
                batches.push_back(std::make_unique<FqBatch>());
                batches.back()->reserve(batch_size);
            }
            
            state.PauseTiming();
            batches.clear(); // 清理内存
            state.ResumeTiming();
        }
        
        state.SetItemsProcessed(allocation_count);
    }
};

class ObjectPoolBenchmark : public BenchmarkCase {
public:
    auto get_name() const -> std::string override { return "ObjectPool"; }
    auto get_description() const -> std::string override { 
        return "Measure object pool performance vs direct allocation"; 
    }
    auto get_category() const -> std::string override { return "Memory"; }
    
    auto run(benchmark::State& state) -> void override {
        auto use_pool = state.range(0);
        auto operation_count = state.range(1);
        
        auto pool = std::make_unique<FqBatchPool>();
        
        for (auto _ : state) {
            if (use_pool) {
                std::vector<std::unique_ptr<FqBatch>> batches;
                
                for (size_t i = 0; i < operation_count; ++i) {
                    auto batch = pool->acquire();
                    // 使用对象池
                    batches.push_back(std::move(batch));
                }
                
                for (auto& batch : batches) {
                    pool->release(std::move(batch));
                }
            } else {
                std::vector<std::unique_ptr<FqBatch>> batches;
                
                for (size_t i = 0; i < operation_count; ++i) {
                    // 直接分配
                    batches.push_back(std::make_unique<FqBatch>());
                }
            }
            
            state.SetItemsProcessed(operation_count);
        }
    }
};

class MemoryAccessPatternBenchmark : public BenchmarkCase {
public:
    auto get_name() const -> std::string override { return "MemoryAccessPattern"; }
    auto get_description() const -> std::string override { 
        return "Measure different memory access patterns"; 
    }
    auto get_category() const -> std::string override { return "Memory"; }
    
    auto run(benchmark::State& state) -> void override {
        auto access_pattern = static_cast<AccessPattern>(state.range(0));
        auto data_size = state.range(1);
        
        auto data = generate_sequential_data(data_size);
        
        for (auto _ : state) {
            switch (access_pattern) {
                case AccessPattern::Sequential:
                    for (size_t i = 0; i < data.size(); ++i) {
                        benchmark::DoNotOptimize(data[i]);
                    }
                    break;
                    
                case AccessPattern::Random:
                    for (size_t i = 0; i < data.size(); ++i) {
                        auto index = rand() % data.size();
                        benchmark::DoNotOptimize(data[index]);
                    }
                    break;
                    
                case AccessPattern::Strided:
                    for (size_t i = 0; i < data.size(); i += 16) {
                        benchmark::DoNotOptimize(data[i]);
                    }
                    break;
            }
            
            state.SetItemsProcessed(data.size());
        }
    }
};

BENCHMARK_REGISTER_F(MemoryAllocationBenchmark, MemoryAllocation)
    ->Args({1000, 1000})     // 1000 records per batch, 1000 allocations
    ->Args({10000, 1000})    // 10000 records per batch, 1000 allocations
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_REGISTER_F(ObjectPoolBenchmark, ObjectPool)
    ->Args({0, 1000})        // Direct allocation
    ->Args({1, 1000})        // Use object pool
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_REGISTER_F(MemoryAccessPatternBenchmark, MemoryAccessPattern)
    ->Args({static_cast<int>(AccessPattern::Sequential), 1000000})
    ->Args({static_cast<int>(AccessPattern::Random), 1000000})
    ->Args({static_cast<int>(AccessPattern::Strided), 1000000})
    ->Unit(benchmark::kMicrosecond);

} // namespace fq::benchmark
```

### 3. CPU 计算性能测试
```cpp
// tools/benchmark/benchmark_cases/cpu_benchmarks.cpp
#include "benchmark_suite.h"
#include "src/modules/core/core.h"
#include "src/modules/fastq/fastq.h"

namespace fq::benchmark {

class QualityCalculationBenchmark : public BenchmarkCase {
public:
    auto get_name() const -> std::string override { return "QualityCalculation"; }
    auto get_description() const -> std::string override { 
        return "Measure quality score calculation performance"; 
    }
    auto get_category() const -> std::string override { return "CPU"; }
    
    auto run(benchmark::State& state) -> void override {
        auto sequence_length = state.range(0);
        auto iterations = state.range(1);
        
        auto quality_string = generate_quality_string(sequence_length);
        
        for (auto _ : state) {
            double total_quality = 0.0;
            
            for (size_t i = 0; i < iterations; ++i) {
                total_quality += QualityScore::calculate_average_quality(quality_string);
            }
            
            benchmark::DoNotOptimize(total_quality);
            state.SetItemsProcessed(iterations);
            state.SetBytesProcessed(iterations * sequence_length);
        }
    }
};

class SequenceValidationBenchmark : public BenchmarkCase {
public:
    auto get_name() const -> std::string override { return "SequenceValidation"; }
    auto get_description() const -> std::string override { 
        return "Measure DNA sequence validation performance"; 
    }
    auto get_category() const -> std::string override { return "CPU"; }
    
    auto run(benchmark::State& state) -> void override {
        auto sequence_length = state.range(0);
        auto iterations = state.range(1);
        
        auto sequence = generate_dna_sequence(sequence_length);
        
        for (auto _ : state) {
            size_t valid_count = 0;
            
            for (size_t i = 0; i < iterations; ++i) {
                if (SequenceUtils::is_valid_dna(sequence)) {
                    valid_count++;
                }
            }
            
            benchmark::DoNotOptimize(valid_count);
            state.SetItemsProcessed(iterations);
            state.SetBytesProcessed(iterations * sequence_length);
        }
    }
};

class GcContentBenchmark : public BenchmarkCase {
public:
    auto get_name() const -> std::string override { return "GcContent"; }
    auto get_description() const -> std::string override { 
        return "Measure GC content calculation performance"; 
    }
    auto get_category() const -> std::string override { return "CPU"; }
    
    auto run(benchmark::State& state) -> void override {
        auto sequence_length = state.range(0);
        auto iterations = state.range(1);
        
        auto sequence = generate_dna_sequence(sequence_length);
        
        for (auto _ : state) {
            double total_gc = 0.0;
            
            for (size_t i = 0; i < iterations; ++i) {
                total_gc += SequenceUtils::calculate_gc_content(sequence);
            }
            
            benchmark::DoNotOptimize(total_gc);
            state.SetItemsProcessed(iterations);
            state.SetBytesProcessed(iterations * sequence_length);
        }
    }
};

BENCHMARK_REGISTER_F(QualityCalculationBenchmark, QualityCalculation)
    ->Args({100, 10000})     // 100bp, 10000 iterations
    ->Args({150, 10000})     // 150bp, 10000 iterations
    ->Args({250, 10000})     // 250bp, 10000 iterations
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_REGISTER_F(SequenceValidationBenchmark, SequenceValidation)
    ->Args({100, 10000})     // 100bp, 10000 iterations
    ->Args({150, 10000})     // 150bp, 10000 iterations
    ->Args({250, 10000})     // 250bp, 10000 iterations
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_REGISTER_F(GcContentBenchmark, GcContent)
    ->Args({100, 10000})     // 100bp, 10000 iterations
    ->Args({150, 10000})     // 150bp, 10000 iterations
    ->Args({250, 10000})     // 250bp, 10000 iterations
    ->Unit(benchmark::kMicrosecond);

} // namespace fq::benchmark
```

### 4. 并发性能测试
```cpp
// tools/benchmark/benchmark_cases/concurrent_benchmarks.cpp
#include "benchmark_suite.h"
#include "src/processing/tbb_processing_pipeline.h"
#include "src/modules/fastq/fastq.h"

namespace fq::benchmark {

class ThreadScalingBenchmark : public BenchmarkCase {
public:
    auto get_name() const -> std::string override { return "ThreadScaling"; }
    auto get_description() const -> std::string override { 
        return "Measure thread scaling performance"; 
    }
    auto get_category() const -> std::string override { return "Concurrency"; }
    
    auto run(benchmark::State& state) -> void override {
        auto thread_count = state.range(0);
        auto file_size = state.range(1);
        
        auto test_file = generate_test_fastq_file(file_size);
        
        for (auto _ : state) {
            auto config = create_processing_config();
            config.thread_count = thread_count;
            
            auto pipeline = std::make_unique<TbbProcessingPipeline>(config);
            pipeline->set_input(test_file.string());
            pipeline->set_output(get_temp_file_path());
            
            auto start_time = std::chrono::high_resolution_clock::now();
            auto result = pipeline->run();
            auto end_time = std::chrono::high_resolution_clock::now();
            
            auto duration = std::chrono::duration<double>(end_time - start_time).count();
            state.SetIterationTime(duration);
            
            ASSERT_EQ(result.status, ProcessingStatus::Success);
        }
        
        state.SetItemsProcessed(file_size / 150); // Assuming 150bp reads
        state.SetBytesProcessed(file_size);
    }
};

class LockContentionBenchmark : public BenchmarkCase {
public:
    auto get_name() const -> std::string override { return "LockContention"; }
    auto get_description() const -> std::string override { 
        return "Measure lock contention under high concurrency"; 
    }
    auto get_category() const -> std::string override { return "Concurrency"; }
    
    auto run(benchmark::State& state) -> void override {
        auto thread_count = state.range(0);
        auto operation_count = state.range(1);
        
        auto shared_counter = std::make_shared<std::atomic<size_t>>(0);
        auto shared_pool = std::make_shared<FqBatchPool>();
        
        for (auto _ : state) {
            std::vector<std::thread> threads;
            
            auto start_time = std::chrono::high_resolution_clock::now();
            
            for (size_t i = 0; i < thread_count; ++i) {
                threads.emplace_back([operation_count, shared_counter, shared_pool]() {
                    for (size_t j = 0; j < operation_count; ++j) {
                        auto batch = shared_pool->acquire();
                        shared_counter->fetch_add(1);
                        shared_pool->release(std::move(batch));
                    }
                });
            }
            
            for (auto& thread : threads) {
                thread.join();
            }
            
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration<double>(end_time - start_time).count();
            state.SetIterationTime(duration);
            
            state.SetItemsProcessed(thread_count * operation_count);
        }
    }
};

class ConcurrentStatisticsBenchmark : public BenchmarkCase {
public:
    auto get_name() const -> std::string override { return "ConcurrentStatistics"; }
    auto get_description() const -> std::string override { 
        return "Measure concurrent statistics calculation performance"; 
    }
    auto get_category() const -> std::string override { return "Concurrency"; }
    
    auto run(benchmark::State& state) -> void override {
        auto thread_count = state.range(0);
        auto batch_count = state.range(1);
        
        for (auto _ : state) {
            auto calculator = std::make_unique<FqStatistic>(thread_count);
            std::vector<std::future<StatisticsResult>> futures;
            
            auto start_time = std::chrono::high_resolution_clock::now();
            
            for (size_t i = 0; i < thread_count; ++i) {
                auto batch = generate_test_batch(batch_count / thread_count, 150);
                futures.push_back(std::async(std::launch::async, [&calculator, &batch]() {
                    return calculator->calculate_batch(batch);
                }));
            }
            
            StatisticsResult total_result;
            for (auto& future : futures) {
                auto result = future.get();
                total_result.merge(result);
            }
            
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration<double>(end_time - start_time).count();
            state.SetIterationTime(duration);
            
            state.SetItemsProcessed(batch_count);
            state.SetBytesProcessed(batch_count * 150 * 2); // sequence + quality
        }
    }
};

BENCHMARK_REGISTER_F(ThreadScalingBenchmark, ThreadScaling)
    ->Args({1, 10 * 1024 * 1024})      // 1 thread, 10MB
    ->Args({2, 10 * 1024 * 1024})      // 2 threads, 10MB
    ->Args({4, 10 * 1024 * 1024})      // 4 threads, 10MB
    ->Args({8, 10 * 1024 * 1024})      // 8 threads, 10MB
    ->Args({16, 10 * 1024 * 1024})     // 16 threads, 10MB
    ->Unit(benchmark::kMillisecond);

BENCHMARK_REGISTER_F(LockContentionBenchmark, LockContention)
    ->Args({2, 1000})        // 2 threads, 1000 operations
    ->Args({4, 1000})        // 4 threads, 1000 operations
    ->Args({8, 1000})        // 8 threads, 1000 operations
    ->Args({16, 1000})       // 16 threads, 1000 operations
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_REGISTER_F(ConcurrentStatisticsBenchmark, ConcurrentStatistics)
    ->Args({1, 10000})       // 1 thread, 10000 batches
    ->Args({2, 10000})       // 2 threads, 10000 batches
    ->Args({4, 10000})       // 4 threads, 10000 batches
    ->Args({8, 10000})       // 8 threads, 10000 batches
    ->Unit(benchmark::kMillisecond);

} // namespace fq::benchmark
```

### 5. 端到端性能测试
```cpp
// tools/benchmark/benchmark_cases/end_to_end_benchmarks.cpp
#include "benchmark_suite.h"
#include "src/processing/processing_pipeline.h"
#include "src/cli/commands/stat_command.h"

namespace fq::benchmark {

class StatCommandBenchmark : public BenchmarkCase {
public:
    auto get_name() const -> std::string override { return "StatCommand"; }
    auto get_description() const -> std::string override { 
        return "Measure end-to-end stat command performance"; 
    }
    auto get_category() const -> std::string override { return "EndToEnd"; }
    
    auto run(benchmark::State& state) -> void override {
        auto file_size = state.range(0);
        auto test_file = generate_test_fastq_file(file_size);
        
        for (auto _ : state) {
            fq::cli::StatCommand stat_command;
            
            std::vector<std::string> args = {
                "fastqtools", "stat",
                "--input", test_file.string(),
                "--output", get_temp_file_path().string()
            };
            
            auto start_time = std::chrono::high_resolution_clock::now();
            
            // Parse arguments and execute
            cxxopts::Options options("stat", "FastQ file statistics");
            // Add options similar to main program
            auto result = options.parse(args.size(), args.data());
            
            stat_command.execute(args.size(), const_cast<char**>(args.data()));
            
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration<double>(end_time - start_time).count();
            state.SetIterationTime(duration);
            
            state.SetItemsProcessed(file_size / 150); // Assuming 150bp reads
            state.SetBytesProcessed(file_size);
        }
    }
};

class FilterCommandBenchmark : public BenchmarkCase {
public:
    auto get_name() const -> std::string override { return "FilterCommand"; }
    auto get_description() const -> std::string override { 
        return "Measure end-to-end filter command performance"; 
    }
    auto get_category() const -> std::string override { return "EndToEnd"; }
    
    auto run(benchmark::State& state) -> void override {
        auto file_size = state.range(0);
        auto test_file = generate_test_fastq_file(file_size);
        
        for (auto _ : state) {
            fq::cli::FilterCommand filter_command;
            
            std::vector<std::string> args = {
                "fastqtools", "filter",
                "--input", test_file.string(),
                "--output", get_temp_file_path().string(),
                "--min-quality", "20",
                "--min-length", "50"
            };
            
            auto start_time = std::chrono::high_resolution_clock::now();
            
            filter_command.execute(args.size(), const_cast<char**>(args.data()));
            
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration<double>(end_time - start_time).count();
            state.SetIterationTime(duration);
            
            state.SetItemsProcessed(file_size / 150);
            state.SetBytesProcessed(file_size);
        }
    }
};

class ProcessingPipelineBenchmark : public BenchmarkCase {
public:
    auto get_name() const -> std::string override { return "ProcessingPipeline"; }
    auto get_description() const -> std::string override { 
        return "Measure complete processing pipeline performance"; 
    }
    auto get_category() const -> std::string override { return "EndToEnd"; }
    
    auto run(benchmark::State& state) -> void override {
        auto file_size = state.range(0);
        auto complexity = state.range(1); // 1=simple, 2=medium, 3=complex
        
        auto test_file = generate_test_fastq_file(file_size);
        
        for (auto _ : state) {
            auto config = create_processing_config();
            
            // Add processing steps based on complexity
            if (complexity >= 1) {
                config.mutators.push_back({"quality_trimmer", {{"min_quality", 20}}});
            }
            if (complexity >= 2) {
                config.mutators.push_back({"length_trimmer", {{"min_length", 50}}});
            }
            if (complexity >= 3) {
                config.mutators.push_back({"adapter_trimmer", {{"adapter_sequence", "AGATCGGAAG"}}});
                config.predicates.push_back({"max_n_ratio", {{"max_ratio", 0.1}}});
            }
            
            auto pipeline = std::make_unique<TbbProcessingPipeline>(config);
            pipeline->set_input(test_file.string());
            pipeline->set_output(get_temp_file_path());
            
            auto start_time = std::chrono::high_resolution_clock::now();
            auto result = pipeline->run();
            auto end_time = std::chrono::high_resolution_clock::now();
            
            auto duration = std::chrono::duration<double>(end_time - start_time).count();
            state.SetIterationTime(duration);
            
            state.SetItemsProcessed(file_size / 150);
            state.SetBytesProcessed(file_size);
            
            ASSERT_EQ(result.status, ProcessingStatus::Success);
        }
    }
};

BENCHMARK_REGISTER_F(StatCommandBenchmark, StatCommand)
    ->Args({10 * 1024 * 1024})    // 10MB
    ->Args({100 * 1024 * 1024})   // 100MB
    ->Args({1000 * 1024 * 1024})  // 1GB
    ->Unit(benchmark::kMillisecond);

BENCHMARK_REGISTER_F(FilterCommandBenchmark, FilterCommand)
    ->Args({10 * 1024 * 1024})    // 10MB
    ->Args({100 * 1024 * 1024})   // 100MB
    ->Args({1000 * 1024 * 1024})  // 1GB
    ->Unit(benchmark::kMillisecond);

BENCHMARK_REGISTER_F(ProcessingPipelineBenchmark, ProcessingPipeline)
    ->Args({10 * 1024 * 1024, 1})  // 10MB, simple
    ->Args({10 * 1024 * 1024, 2})  // 10MB, medium
    ->Args({10 * 1024 * 1024, 3})  // 10MB, complex
    ->Args({100 * 1024 * 1024, 1}) // 100MB, simple
    ->Args({100 * 1024 * 1024, 2}) // 100MB, medium
    ->Args({100 * 1024 * 1024, 3}) // 100MB, complex
    ->Unit(benchmark::kMillisecond);

} // namespace fq::benchmark
```

## 🔧 性能测试工具

### 1. 性能计数器
```cpp
// tools/benchmark/benchmark_utils/performance_counter.h
#pragma once

#include <chrono>
#include <memory>
#include <string>

namespace fq::benchmark {

class PerformanceCounter {
public:
    static auto get_instance() -> PerformanceCounter&;
    
    // CPU 性能计数器
    auto get_cpu_usage() -> double;
    auto get_process_cpu_time() -> double;
    auto get_system_cpu_time() -> double;
    
    // 内存性能计数器
    auto get_memory_usage() -> size_t;
    auto get_peak_memory_usage() -> size_t;
    auto get_page_faults() -> size_t;
    
    // I/O 性能计数器
    auto get_bytes_read() -> size_t;
    auto get_bytes_written() -> size_t;
    auto get_disk_operations() -> size_t;
    
    // 自定义计数器
    auto start_timing(const std::string& name) -> void;
    auto stop_timing(const std::string& name) -> double;
    auto get_timing(const std::string& name) -> double;
    
    // 统计信息
    auto get_system_info() -> std::string;
    auto get_process_info() -> std::string;
    
private:
    PerformanceCounter();
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

// RAII 计时器
class ScopedTimer {
public:
    explicit ScopedTimer(const std::string& name);
    ~ScopedTimer();
    
private:
    std::string m_name;
    std::chrono::high_resolution_clock::time_point m_start_time;
};

} // namespace fq::benchmark
```

### 2. 内存跟踪器
```cpp
// tools/benchmark/benchmark_utils/memory_tracker.h
#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace fq::benchmark {

class MemoryTracker {
public:
    static auto get_instance() -> MemoryTracker&;
    
    // 内存分配跟踪
    auto allocation_hook() -> void;
    auto deallocation_hook() -> void;
    
    // 内存统计
    auto get_total_allocated() -> size_t;
    auto get_total_freed() -> size_t;
    auto get_current_usage() -> size_t;
    auto get_peak_usage() -> size_t;
    auto get_allocation_count() -> size_t;
    auto get_deallocation_count() -> size_t;
    
    // 内存分配分析
    auto get_allocation_size_distribution() -> std::vector<std::pair<size_t, size_t>>;
    auto get_allocation_hotspots() -> std::vector<std::pair<std::string, size_t>>;
    
    // 重置统计
    auto reset() -> void;
    
    // 生成报告
    auto generate_report() -> std::string;
    
private:
    MemoryTracker() = default;
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

// RAII 内存跟踪器
class ScopedMemoryTracker {
public:
    explicit ScopedMemoryTracker(const std::string& name);
    ~ScopedMemoryTracker();
    
    auto get_peak_memory() const -> size_t;
    auto get_total_allocations() const -> size_t;
    
private:
    std::string m_name;
    size_t m_start_memory;
    size_t m_start_allocations;
};

} // namespace fq::benchmark
```

### 3. 测试数据生成器
```cpp
// tools/benchmark/benchmark_data/data_generator.h
#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace fq::benchmark {

class TestDataGenerator {
public:
    struct DataSpec {
        size_t record_count = 0;
        size_t read_length = 0;
        double quality_mean = 30.0;
        double quality_std = 5.0;
        double gc_content = 0.5;
        bool variable_length = false;
        size_t min_length = 0;
        size_t max_length = 0;
        double adapter_contamination = 0.0;
        std::string adapter_sequence = "AGATCGGAAG";
    };
    
    // 生成标准 FASTQ 文件
    static auto generate_fastq_file(const DataSpec& spec,
                                   const std::filesystem::path& output_path) -> void;
    
    // 生成压缩文件
    static auto generate_compressed_file(const DataSpec& spec,
                                        const std::filesystem::path& output_path,
                                        CompressionType type) -> void;
    
    // 生成损坏数据
    static auto generate_corrupted_file(const DataSpec& spec,
                                      const std::filesystem::path& output_path,
                                      CorruptionType type) -> void;
    
    // 生成不同负载特征的数据
    static auto generate_workload_profile(WorkloadProfile profile,
                                        const std::filesystem::path& output_path) -> void;
    
private:
    static auto generate_records(const DataSpec& spec) -> std::vector<FqRecord>;
    static auto add_corruption(std::vector<FqRecord>& records, 
                             CorruptionType type) -> void;
};

} // namespace fq::benchmark
```

## 📊 性能报告生成

### 1. 报告生成器
```cpp
// tools/benchmark/benchmark_reporters/html_reporter.h
#pragma once

#include "benchmark_suite.h"
#include <string>

namespace fq::benchmark {

class HtmlReporter {
public:
    static auto generate_report(const BenchmarkResults& results,
                              const std::filesystem::path& output_path) -> void;
    
    static auto generate_comparison_report(const BenchmarkResults& current,
                                         const BenchmarkResults& baseline,
                                         const std::filesystem::path& output_path) -> void;
    
    static auto generate_trend_report(const std::vector<BenchmarkResults>& history,
                                   const std::filesystem::path& output_path) -> void;
    
private:
    static auto generate_html_header(const std::string& title) -> std::string;
    static auto generate_summary_table(const BenchmarkResults& results) -> std::string;
    static auto generate_performance_charts(const BenchmarkResults& results) -> std::string;
    static auto generate_system_info(const BenchmarkResults& results) -> std::string;
    static auto generate_html_footer() -> std::string;
};

} // namespace fq::benchmark
```

### 2. 持续集成集成
```yaml
# .github/workflows/benchmark.yml
name: Performance Benchmark

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]

jobs:
  benchmark:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Build
      run: |
        mkdir -p build
        cd build
        cmake .. -DCMAKE_BUILD_TYPE=Release
        make -j$(nproc)
    
    - name: Run Benchmarks
      run: |
        cd build
        ./tools/benchmark/performance_benchmark \
          --benchmark_format=json \
          --benchmark_out=benchmark_results.json
    
    - name: Compare with Baseline
      run: |
        cd build
        python3 ../scripts/compare_benchmarks.py \
          benchmark_results.json \
          baseline_results.json \
          --threshold=5.0 \
          --output=comparison_report.html
    
    - name: Upload Results
      uses: actions/upload-artifact@v3
      with:
        name: benchmark-results
        path: |
          build/benchmark_results.json
          build/comparison_report.html
    
    - name: Comment on PR
      if: github.event_name == 'pull_request'
      uses: actions/github-script@v6
      with:
        script: |
          const fs = require('fs');
          const results = JSON.parse(fs.readFileSync('build/benchmark_results.json', 'utf8'));
          const comparison = JSON.parse(fs.readFileSync('build/comparison_report.json', 'utf8'));
          
          let comment = `## 📊 Performance Benchmark Results\n\n`;
          comment += `### Summary\n`;
          comment += `- **Improved**: ${comparison.improved.length} benchmarks\n`;
          comment += `- **Regressed**: ${comparison.regressed.length} benchmarks\n`;
          comment += `- **Unchanged**: ${comparison.unchanged.length} benchmarks\n\n`;
          
          if (comparison.regressed.length > 0) {
            comment += `### ⚠️ Performance Regressions\n`;
            comparison.regressed.forEach(regression => {
              comment += `- ${regression.name}: ${regression.change}%\n`;
            });
          }
          
          github.rest.issues.createComment({
            issue_number: context.issue.number,
            owner: context.repo.owner,
            repo: context.repo.repo,
            body: comment
          });
```

## 🎯 使用指南

### 1. 运行基准测试
```bash
# 运行所有基准测试
./tools/benchmark/performance_benchmark

# 运行特定类别的测试
./tools/benchmark/performance_benchmark --benchmark_filter=I/O

# 生成 JSON 报告
./tools/benchmark/performance_benchmark --benchmark_format=json --benchmark_out=results.json

# 生成 HTML 报告
./tools/benchmark/performance_benchmark --benchmark_format=html --benchmark_out=results.html
```

### 2. 性能回归检测
```bash
# 生成基线结果
./tools/benchmark/performance_benchmark --benchmark_out=baseline.json

# 运行当前版本
./tools/benchmark/performance_benchmark --benchmark_out=current.json

# 比较结果
python3 scripts/compare_benchmarks.py current.json baseline.json --threshold=5.0
```

### 3. 持续监控
```bash
# 设置定时任务
crontab -e
# 添加：每天凌晨运行性能测试
0 0 * * * /path/to/fastqtools/scripts/daily_benchmark.sh
```

## 📈 预期效果

通过实施这个性能基准测试系统，FastQTools 项目将获得：

1. **性能回归检测**: 自动检测性能退化
2. **瓶颈识别**: 精确定位性能瓶颈
3. **优化验证**: 验证性能优化效果
4. **持续监控**: 长期性能趋势监控
5. **文档化性能**: 建立性能基线和目标

这个性能基准测试系统为 FastQTools 项目提供了企业级的性能保证体系。