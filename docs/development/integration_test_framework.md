# FastQTools 集成测试框架设计

## 📋 概述

本文档为 FastQTools 项目设计了完整的集成测试框架，旨在验证各个模块之间的协作和端到端功能。

## 🎯 设计目标

### 1. 测试覆盖目标
- **端到端测试**: 完整的工作流程验证
- **模块集成测试**: 验证模块间的正确协作
- **性能基准测试**: 建立性能回归检测
- **错误处理测试**: 验证系统在异常情况下的行为

### 2. 质量目标
- **测试覆盖率**: 达到 80%+ 的功能覆盖率
- **测试可靠性**: 减少偶发性测试失败
- **测试可维护性**: 清晰的测试结构和文档
- **测试执行效率**: 快速的测试反馈循环

## 🏗️ 集成测试架构

### 1. 测试层次结构
```
tests/
├── integration/              # 集成测试
│   ├── end_to_end/         # 端到端测试
│   ├── module_interaction/ # 模块交互测试
│   ├── performance/        # 性能基准测试
│   └── error_handling/     # 错误处理测试
├── fixtures/               # 测试数据和工具
│   ├── data/              # 测试数据文件
│   ├── helpers/           # 测试辅助类
│   └── mocks/             # 模拟对象
└── CI/                    # CI/CD 配置
    ├── integration_test.yml
    └── performance_test.yml
```

### 2. 测试框架组件

#### 2.1 测试基础设施
```cpp
// tests/integration/test_base.h
#pragma once

#include <gtest/gtest.h>
#include <filesystem>
#include <memory>
#include <string>

namespace fq::testing {

class IntegrationTestBase : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;
    
    // 测试环境管理
    auto create_test_directory() -> std::filesystem::path;
    auto cleanup_test_directory() -> void;
    
    // 测试数据生成
    auto create_test_fastq_file(size_t record_count, size_t read_length) 
        -> std::filesystem::path;
    auto create_test_config() -> fq::config::Config;
    
    // 测试辅助函数
    auto run_command(const std::vector<std::string>& args) -> int;
    auto verify_output_file(const std::filesystem::path& file) -> bool;
    
protected:
    std::filesystem::path m_test_dir;
    std::filesystem::path m_data_dir;
    std::filesystem::path m_output_dir;
};

} // namespace fq::testing
```

#### 2.2 测试数据管理
```cpp
// tests/fixtures/data/test_data_manager.h
#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace fq::testing {

class TestDataManager {
public:
    static auto get_instance() -> TestDataManager&;
    
    // 测试数据获取
    auto get_test_file(const std::string& name) -> std::filesystem::path;
    auto get_test_data(const std::string& category, const std::string& name) 
        -> std::vector<std::string>;
    
    // 测试数据生成
    auto generate_fastq_data(const std::string& pattern, size_t count) 
        -> std::filesystem::path;
    auto generate_corrupted_data() -> std::filesystem::path;
    
private:
    TestDataManager();
    std::filesystem::path m_data_root;
};

} // namespace fq::testing
```

#### 2.3 模拟对象框架
```cpp
// tests/fixtures/mocks/mock_file_reader.h
#pragma once

#include <gmock/gmock.h>
#include "src/modules/fastq/fastq.h"

namespace fq::testing {

class MockFileReader {
public:
    MOCK_METHOD(std::unique_ptr<fq::fastq::FqRecord>, read_record, ());
    MOCK_METHOD(bool, is_eof, (), (const));
    MOCK_METHOD(void, close, ());
    MOCK_METHOD(std::string, get_file_path, (), (const));
};

class MockProcessingPipeline : public fq::processing::IProcessingPipeline {
public:
    MOCK_METHOD(void, set_input, (const std::string& path));
    MOCK_METHOD(void, set_output, (const std::string& path));
    MOCK_METHOD(void, set_config, (const fq::config::Config& config));
    MOCK_METHOD(auto, run, (), (fq::processing::ProcessingResult));
    MOCK_METHOD(auto, get_status, (), (const, fq::processing::ProcessingStatus));
};

} // namespace fq::testing
```

## 📋 集成测试套件

### 1. 端到端测试 (End-to-End Tests)

#### 1.1 CLI 命令测试
```cpp
// tests/integration/end_to_end/test_cli_commands.cpp
#include "test_base.h"

namespace fq::testing {

class CliCommandTest : public IntegrationTestBase {};

TEST_F(CliCommandTest, StatCommand_BasicFunctionality) {
    // 准备测试数据
    auto test_file = create_test_fastq_file(1000, 150);
    
    // 执行统计命令
    auto result = run_command({
        "fastqtools", "stat", 
        "--input", test_file.string(),
        "--output", (m_output_dir / "stats.txt").string()
    });
    
    // 验证结果
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(verify_output_file(m_output_dir / "stats.txt"));
}

TEST_F(CliCommandTest, FilterCommand_QualityFiltering) {
    // 准备测试数据
    auto test_file = create_test_fastq_file(1000, 150);
    
    // 执行过滤命令
    auto result = run_command({
        "fastqtools", "filter",
        "--input", test_file.string(),
        "--output", (m_output_dir / "filtered.fastq").string(),
        "--min-quality", "20",
        "--min-length", "50"
    });
    
    // 验证结果
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(verify_output_file(m_output_dir / "filtered.fastq"));
}

TEST_F(CliCommandTest, HelpCommand_Display) {
    auto result = run_command({"fastqtools", "--help"});
    EXPECT_EQ(result, 0);
}

} // namespace fq::testing
```

#### 1.2 完整工作流测试
```cpp
// tests/integration/end_to_end/test_complete_workflow.cpp
#include "test_base.h"

namespace fq::testing {

class CompleteWorkflowTest : public IntegrationTestBase {};

TEST_F(CompleteWorkflowTest, StatThenFilterWorkflow) {
    // 步骤1：生成统计数据
    auto test_file = create_test_fastq_file(5000, 100);
    auto stat_result = run_command({
        "fastqtools", "stat",
        "--input", test_file.string(),
        "--output", (m_output_dir / "stats.json").string()
    });
    EXPECT_EQ(stat_result, 0);
    
    // 步骤2：基于统计结果进行过滤
    auto filter_result = run_command({
        "fastqtools", "filter",
        "--input", test_file.string(),
        "--output", (m_output_dir / "filtered.fastq").string(),
        "--min-quality", "25",
        "--trim-quality", "20"
    });
    EXPECT_EQ(filter_result, 0);
    
    // 步骤3：验证过滤结果
    auto final_stat_result = run_command({
        "fastqtools", "stat",
        "--input", (m_output_dir / "filtered.fastq").string(),
        "--output", (m_output_dir / "final_stats.json").string()
    });
    EXPECT_EQ(final_stat_result, 0);
    
    // 验证过滤后的质量提升
    verify_quality_improvement(
        m_output_dir / "stats.json",
        m_output_dir / "final_stats.json"
    );
}

} // namespace fq::testing
```

### 2. 模块交互测试 (Module Interaction Tests)

#### 2.1 处理流水线集成测试
```cpp
// tests/integration/module_interaction/test_processing_pipeline.cpp
#include "test_base.h"

namespace fq::testing {

class ProcessingPipelineIntegrationTest : public IntegrationTestBase {};

TEST_F(ProcessingPipelineIntegrationTest, MultipleMutatorsChain) {
    // 创建测试数据
    auto test_file = create_test_fastq_file(1000, 150);
    
    // 配置处理流水线
    auto config = create_test_config();
    config.pipeline_type = fq::processing::PipelineType::TBB;
    config.mutators = {
        {"quality_trimmer", {{"min_quality", 20}}},
        {"length_trimmer", {{"min_length", 50}}},
        {"adapter_trimmer", {{"adapter_sequence", "AGATCGGAAG"}}}
    };
    
    // 执行处理
    auto pipeline = fq::processing::create_pipeline(config.pipeline_type);
    pipeline->set_input(test_file.string());
    pipeline->set_output((m_output_dir / "processed.fastq").string());
    pipeline->set_config(config);
    
    auto result = pipeline->run();
    
    // 验证结果
    EXPECT_EQ(result.status, fq::processing::ProcessingStatus::Success);
    EXPECT_GT(result.statistics.processed_records, 0);
    EXPECT_TRUE(verify_output_file(m_output_dir / "processed.fastq"));
}

TEST_F(ProcessingPipelineIntegrationTest, ErrorHandlingInPipeline) {
    // 创建损坏的测试数据
    auto corrupted_file = generate_corrupted_data();
    
    // 配置处理流水线
    auto config = create_test_config();
    
    // 执行处理
    auto pipeline = fq::processing::create_pipeline(config.pipeline_type);
    pipeline->set_input(corrupted_file.string());
    pipeline->set_output((m_output_dir / "error_output.fastq").string());
    pipeline->set_config(config);
    
    auto result = pipeline->run();
    
    // 验证错误处理
    EXPECT_EQ(result.status, fq::processing::ProcessingStatus::Failed);
    EXPECT_FALSE(result.errors.empty());
}

} // namespace fq::testing
```

#### 2.2 统计模块集成测试
```cpp
// tests/integration/module_interaction/test_statistics_integration.cpp
#include "test_base.h"

namespace fq::testing {

class StatisticsIntegrationTest : public IntegrationTestBase {};

TEST_F(StatisticsIntegrationTest, LargeFileStatistics) {
    // 创建大型测试文件
    auto large_file = create_test_fastq_file(100000, 150);
    
    // 配置统计计算
    auto config = create_test_config();
    config.statistics.batch_size = 10000;
    config.statistics.thread_count = 4;
    
    // 执行统计计算
    auto calculator = fq::statistics::create_calculator(config);
    auto result = calculator->calculate(large_file.string());
    
    // 验证统计结果
    EXPECT_GT(result.total_records, 0);
    EXPECT_GT(result.total_bases, 0);
    EXPECT_GT(result.average_quality, 0);
    EXPECT_GE(result.gc_content, 0);
    EXPECT_LE(result.gc_content, 1);
    
    // 验证统计精度
    verify_statistics_accuracy(result);
}

} // namespace fq::testing
```

### 3. 性能基准测试 (Performance Benchmark Tests)

#### 3.1 性能基准框架
```cpp
// tests/integration/performance/test_performance_benchmarks.cpp
#include "test_base.h"
#include <benchmark/benchmark.h>

namespace fq::testing {

class PerformanceBenchmarkTest : public IntegrationTestBase {};

BENCHMARK_DEFINE_F(PerformanceBenchmarkTest, FileProcessing)(benchmark::State& state) {
    auto test_file = create_test_fastq_file(state.range(0), state.range(1));
    
    for (auto _ : state) {
        auto config = create_test_config();
        auto pipeline = fq::processing::create_pipeline(config.pipeline_type);
        pipeline->set_input(test_file.string());
        pipeline->set_output((m_output_dir / "benchmark_output.fastq").string());
        pipeline->set_config(config);
        
        auto result = pipeline->run();
        ASSERT_EQ(result.status, fq::processing::ProcessingStatus::Success);
    }
    
    state.SetItemsProcessed(state.range(0));
    state.SetBytesProcessed(state.range(0) * state.range(1) * 2); // *2 for quality
}

BENCHMARK_REGISTER_F(PerformanceBenchmarkTest, FileProcessing)
    ->Args({1000, 100})
    ->Args({10000, 100})
    ->Args({100000, 100})
    ->Args({1000000, 100})
    ->Unit(benchmark::kMillisecond);

BENCHMARK_DEFINE_F(PerformanceBenchmarkTest, MemoryUsage)(benchmark::State& state) {
    auto test_file = create_test_fastq_file(state.range(0), state.range(1));
    
    for (auto _ : state) {
        auto start_memory = get_current_memory_usage();
        
        auto config = create_test_config();
        auto pipeline = fq::processing::create_pipeline(config.pipeline_type);
        pipeline->set_input(test_file.string());
        pipeline->set_output((m_output_dir / "memory_test.fastq").string());
        pipeline->set_config(config);
        
        auto result = pipeline->run();
        
        auto end_memory = get_current_memory_usage();
        state.counters["MemoryBytes"] = end_memory - start_memory;
        
        ASSERT_EQ(result.status, fq::processing::ProcessingStatus::Success);
    }
}

BENCHMARK_REGISTER_F(PerformanceBenchmarkTest, MemoryUsage)
    ->Args({10000, 100})
    ->Args({100000, 100})
    ->Unit(benchmark::kMillisecond);

} // namespace fq::testing
```

#### 3.2 并发性能测试
```cpp
// tests/integration/performance/test_concurrent_processing.cpp
#include "test_base.h"

namespace fq::testing {

class ConcurrentProcessingTest : public IntegrationTestBase {};

TEST_F(ConcurrentProcessingTest, ThreadScalingTest) {
    auto test_file = create_test_fastq_file(50000, 150);
    
    std::vector<size_t> thread_counts = {1, 2, 4, 8, 16};
    std::vector<double> processing_times;
    
    for (auto thread_count : thread_counts) {
        auto config = create_test_config();
        config.thread_count = thread_count;
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        auto pipeline = fq::processing::create_pipeline(config.pipeline_type);
        pipeline->set_input(test_file.string());
        pipeline->set_output((m_output_dir / ("concurrent_" + std::to_string(thread_count) + ".fastq")).string());
        pipeline->set_config(config);
        
        auto result = pipeline->run();
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<double>(end_time - start_time).count();
        
        processing_times.push_back(duration);
        ASSERT_EQ(result.status, fq::processing::ProcessingStatus::Success);
    }
    
    // 验证线程扩展性
    verify_thread_scaling(thread_counts, processing_times);
}

} // namespace fq::testing
```

### 4. 错误处理测试 (Error Handling Tests)

#### 4.1 系统错误处理测试
```cpp
// tests/integration/error_handling/test_system_errors.cpp
#include "test_base.h"

namespace fq::testing {

class SystemErrorHandlingTest : public IntegrationTestBase {};

TEST_F(SystemErrorHandlingTest, MissingInputFile) {
    auto config = create_test_config();
    auto pipeline = fq::processing::create_pipeline(config.pipeline_type);
    pipeline->set_input("/nonexistent/file.fastq");
    pipeline->set_output((m_output_dir / "error_test.fastq").string());
    pipeline->set_config(config);
    
    auto result = pipeline->run();
    
    EXPECT_EQ(result.status, fq::processing::ProcessingStatus::Failed);
    EXPECT_FALSE(result.errors.empty());
    EXPECT_TRUE(contains_error(result.errors, "File not found"));
}

TEST_F(SystemErrorHandlingTest, PermissionDenied) {
    // 创建无权限的文件
    auto restricted_file = m_test_dir / "restricted.fastq";
    create_test_fastq_file(100, 100, restricted_file);
    std::filesystem::permissions(restricted_file, 
        std::filesystem::perms::owner_read | 
        std::filesystem::perms::owner_write);
    
    auto config = create_test_config();
    auto pipeline = fq::processing::create_pipeline(config.pipeline_type);
    pipeline->set_input(restricted_file.string());
    pipeline->set_output((m_output_dir / "permission_test.fastq").string());
    pipeline->set_config(config);
    
    auto result = pipeline->run();
    
    EXPECT_EQ(result.status, fq::processing::ProcessingStatus::Failed);
    EXPECT_TRUE(contains_error(result.errors, "Permission denied"));
}

TEST_F(SystemErrorHandlingTest, DiskSpaceFull) {
    // 模拟磁盘空间不足的情况
    auto config = create_test_config();
    config.temp_directory = m_test_dir.string();
    
    // 创建大型测试文件
    auto large_file = create_test_fastq_file(1000000, 1000);
    
    // 限制测试目录大小
    limit_directory_size(m_test_dir, 1024 * 1024); // 1MB
    
    auto pipeline = fq::processing::create_pipeline(config.pipeline_type);
    pipeline->set_input(large_file.string());
    pipeline->set_output((m_output_dir / "disk_full_test.fastq").string());
    pipeline->set_config(config);
    
    auto result = pipeline->run();
    
    EXPECT_EQ(result.status, fq::processing::ProcessingStatus::Failed);
    EXPECT_TRUE(contains_error(result.errors, "Disk space"));
}

} // namespace fq::testing
```

#### 4.2 数据错误处理测试
```cpp
// tests/integration/error_handling/test_data_errors.cpp
#include "test_base.h"

namespace fq::testing {

class DataErrorHandlingTest : public IntegrationTestBase {};

TEST_F(DataErrorHandlingTest, CorruptedFastqFile) {
    auto corrupted_file = generate_corrupted_data();
    
    auto config = create_test_config();
    auto pipeline = fq::processing::create_pipeline(config.pipeline_type);
    pipeline->set_input(corrupted_file.string());
    pipeline->set_output((m_output_dir / "corrupted_test.fastq").string());
    pipeline->set_config(config);
    
    auto result = pipeline->run();
    
    EXPECT_EQ(result.status, fq::processing::ProcessingStatus::Failed);
    EXPECT_TRUE(contains_error(result.errors, "Invalid format"));
}

TEST_F(DataErrorHandlingTest, InvalidQualityScores) {
    // 创建包含无效质量分数的文件
    auto invalid_file = create_invalid_quality_file();
    
    auto config = create_test_config();
    auto pipeline = fq::processing::create_pipeline(config.pipeline_type);
    pipeline->set_input(invalid_file.string());
    pipeline->set_output((m_output_dir / "invalid_quality_test.fastq").string());
    pipeline->set_config(config);
    
    auto result = pipeline->run();
    
    EXPECT_EQ(result.status, fq::processing::ProcessingStatus::Failed);
    EXPECT_TRUE(contains_error(result.errors, "Invalid quality"));
}

TEST_F(DataErrorHandlingTest, SequenceQualityMismatch) {
    // 创建序列和质量长度不匹配的文件
    auto mismatch_file = create_mismatch_file();
    
    auto config = create_test_config();
    auto pipeline = fq::processing::create_pipeline(config.pipeline_type);
    pipeline->set_input(mismatch_file.string());
    pipeline->set_output((m_output_dir / "mismatch_test.fastq").string());
    pipeline->set_config(config);
    
    auto result = pipeline->run();
    
    EXPECT_EQ(result.status, fq::processing::ProcessingStatus::Failed);
    EXPECT_TRUE(contains_error(result.errors, "Sequence quality mismatch"));
}

} // namespace fq::testing
```

## 🔧 测试工具和辅助函数

### 1. 测试辅助函数
```cpp
// tests/fixtures/helpers/test_helpers.h
#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <chrono>

namespace fq::testing {

// 文件操作辅助函数
auto create_test_fastq_file(size_t record_count, size_t read_length, 
                           const std::filesystem::path& path = {}) 
    -> std::filesystem::path;

auto generate_corrupted_data() -> std::filesystem::path;
auto create_invalid_quality_file() -> std::filesystem::path;
auto create_mismatch_file() -> std::filesystem::path;

// 性能监控辅助函数
auto get_current_memory_usage() -> size_t;
auto get_cpu_usage() -> double;
auto time_function(const std::function<void()>& func) 
    -> std::chrono::duration<double>;

// 验证辅助函数
auto verify_output_file(const std::filesystem::path& file) -> bool;
auto verify_statistics_accuracy(const fq::statistics::StatisticsResult& result) 
    -> bool;
auto verify_quality_improvement(const std::filesystem::path& before,
                               const std::filesystem::path& after) 
    -> bool;
auto verify_thread_scaling(const std::vector<size_t>& thread_counts,
                         const std::vector<double>& processing_times) 
    -> bool;

// 错误处理辅助函数
auto contains_error(const std::vector<std::string>& errors, 
                   const std::string& keyword) -> bool;
auto limit_directory_size(const std::filesystem::path& dir, size_t max_size) 
    -> void;

} // namespace fq::testing
```

### 2. 测试数据生成器
```cpp
// tests/fixtures/helpers/data_generator.h
#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace fq::testing {

class FastQDataGenerator {
public:
    struct Record {
        std::string name;
        std::string sequence;
        std::string quality;
    };
    
    static auto generate_standard_records(size_t count, size_t length) 
        -> std::vector<Record>;
    
    static auto generate_variable_length_records(size_t count, 
                                                size_t min_length, 
                                                size_t max_length) 
        -> std::vector<Record>;
    
    static auto generate_low_quality_records(size_t count, size_t length) 
        -> std::vector<Record>;
    
    static auto generate_adapter_contaminated_records(size_t count, 
                                                     size_t length,
                                                     const std::string& adapter) 
        -> std::vector<Record>;
    
    static auto write_records(const std::vector<Record>& records,
                             const std::filesystem::path& file_path) 
        -> void;
    
    static auto write_gzipped_records(const std::vector<Record>& records,
                                   const std::filesystem::path& file_path) 
        -> void;
};

} // namespace fq::testing
```

## 📊 测试执行和报告

### 1. CMake 配置
```cmake
# tests/integration/CMakeLists.txt
add_subdirectory(end_to_end)
add_subdirectory(module_interaction)
add_subdirectory(performance)
add_subdirectory(error_handling)

# 集成测试目标
add_custom_target(integration_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -L integration
    DEPENDS 
        end_to_end_tests
        module_interaction_tests
        performance_tests
        error_handling_tests
)

# 性能基准测试目标
add_custom_target(benchmark_tests
    COMMAND ${CMAKE_BINARY_DIR}/tools/benchmark/performance_benchmark
    DEPENDS performance_benchmark
)
```

### 2. 测试报告生成
```bash
#!/bin/bash
# scripts/run_integration_tests.sh

set -e

echo "=== FastQTools 集成测试套件 ==="
echo "开始时间: $(date)"

# 创建测试环境
mkdir -p build/integration_test_results
cd build

# 运行单元测试
echo "运行单元测试..."
ctest --output-on-failure -L unit

# 运行集成测试
echo "运行集成测试..."
ctest --output-on-failure -L integration --timeout 300

# 运行性能基准测试
echo "运行性能基准测试..."
./tools/benchmark/performance_benchmark --benchmark_format=json > \
    integration_test_results/performance_results.json

# 生成测试覆盖率报告
echo "生成测试覆盖率报告..."
gcovr --root .. --xml integration_test_results/coverage.xml
gcovr --root .. --html integration_test_results/coverage.html

# 生成测试报告
echo "生成测试报告..."
python3 ../scripts/generate_test_report.py \
    integration_test_results/ \
    integration_test_results/test_report.html

echo "集成测试完成！"
echo "报告位置: build/integration_test_results/"
echo "结束时间: $(date)"
```

## 🎯 测试质量保证

### 1. 测试质量检查清单
- [ ] 所有测试用例都有清晰的描述性名称
- [ ] 测试覆盖了正常流程和异常情况
- [ ] 测试数据和测试逻辑分离
- [ ] 测试有适当的设置和清理
- [ ] 测试断言准确且有意义
- [ ] 测试执行时间合理（< 10秒）
- [ ] 测试之间相互独立
- [ ] 错误处理测试覆盖所有错误类型

### 2. 测试维护策略
- **定期审查**: 每月审查测试覆盖率和质量
- **性能回归检测**: 每次发布前运行性能基准测试
- **测试数据更新**: 根据功能更新定期更新测试数据
- **文档维护**: 保持测试文档和代码同步更新

## 📈 预期效果

通过实施这个集成测试框架，FastQTools 项目将获得：

1. **更高的代码质量**: 全面的测试覆盖减少生产环境问题
2. **更快的开发周期**: 自动化测试提供快速反馈
3. **更好的可维护性**: 清晰的测试结构便于长期维护
4. **更强的用户信心**: 完善的测试体系提升软件可靠性
5. **持续的性能监控**: 性能基准测试防止性能退化

这个集成测试框架为 FastQTools 项目提供了企业级的质量保证体系。