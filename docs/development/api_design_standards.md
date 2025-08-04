# FastQTools API 设计规范

## 📋 概述

本文档为 FastQTools 项目定义了统一的 API 设计规范，旨在提高代码的一致性、可维护性和用户体验。

## 🎯 设计原则

### 1. 一致性原则
- **命名统一**: 全项目使用一致的命名约定
- **接口统一**: 所有模块遵循相同的接口设计模式
- **错误处理统一**: 统一的异常处理和错误报告机制

### 2. 现代化原则
- **C++20 特性**: 充分利用现代 C++ 特性
- **类型安全**: 强类型设计，避免类型转换
- **资源安全**: RAII 模式，智能指针管理资源

### 3. 用户体验原则
- **易用性**: 简洁直观的 API 设计
- **可发现性**: 良好的 IDE 支持和文档
- **可扩展性**: 支持插件和自定义扩展

## 📝 命名约定

### 1. 文件和目录命名
```cpp
// 文件名：snake_case
src/modules/fastq/fastq.h
src/processing/pipeline.h
src/cli/commands/stat_command.h

// 目录名：snake_case
src/modules/
src/processing/
src/statistics/
```

### 2. 类型命名
```cpp
// 类名：PascalCase
class FastQReader;
class ProcessingPipeline;
class QualityTrimmer;

// 结构体名：PascalCase
struct FileAttributes;
struct ProcessingConfig;

// 枚举类名：PascalCase
enum class QScoreType;
enum class ProcessingStatus;

// 枚举值：UPPER_SNAKE_CASE
enum class QScoreType {
    Sanger,
    Illumina13,
    Illumina15,
    Illumina18
};
```

### 3. 函数和方法命名
```cpp
// 公共方法：snake_case
void set_input(const std::string& path);
auto get_config() const -> Config;
auto process_record(const FqRecord& record) -> Result<FqRecord>;

// 私有方法：snake_case（可选下划线前缀）
void validate_config();
auto calculate_statistics_() -> Statistics;

// 静态方法：snake_case
static auto create_pipeline() -> std::unique_ptr<IProcessingPipeline>;
```

### 4. 变量命名
```cpp
// 成员变量：m_ + snake_case
std::string m_input_path;
ProcessingConfig m_config;
std::unique_ptr<IProcessingPipeline> m_pipeline;

// 局部变量：snake_case
auto file_path = std::string{};
auto batch_size = std::size_t{0};

// 常量：UPPER_SNAKE_CASE
constexpr size_t DEFAULT_BATCH_SIZE = 10000;
constexpr double MIN_QUALITY_THRESHOLD = 20.0;

// 函数参数：snake_case
void process_file(const std::string& file_path, size_t batch_size);
```

### 5. 命名空间命名
```cpp
// 命名空间：snake_case
namespace fq::fastq {
namespace fq::processing {
namespace fq::statistics {
```

## 🏗️ 接口设计模式

### 1. 抽象接口设计
```cpp
// 统一的接口基类
class IProcessingPipeline {
public:
    virtual ~IProcessingPipeline() = default;
    
    // 配置方法
    virtual auto set_input(const std::string& path) -> void = 0;
    virtual auto set_output(const std::string& path) -> void = 0;
    virtual auto set_config(const ProcessingConfig& config) -> void = 0;
    
    // 执行方法
    virtual auto run() -> ProcessingResult = 0;
    
    // 状态查询
    virtual auto get_status() const -> ProcessingStatus = 0;
    virtual auto get_statistics() const -> ProcessingStatistics = 0;
};
```

### 2. 工厂模式
```cpp
// 工厂函数
namespace fq::processing {
    auto create_pipeline(PipelineType type = PipelineType::Default) 
        -> std::unique_ptr<IProcessingPipeline>;
    
    auto create_mutator(MutatorType type, const MutatorConfig& config)
        -> std::unique_ptr<IReadMutator>;
}
```

### 3. 构建器模式
```cpp
// 构建器模式
class ProcessingPipelineBuilder {
public:
    auto set_input(const std::string& path) -> ProcessingPipelineBuilder&;
    auto set_output(const std::string& path) -> ProcessingPipelineBuilder&;
    auto add_mutator(std::unique_ptr<IReadMutator> mutator) -> ProcessingPipelineBuilder&;
    auto add_predicate(std::unique_ptr<IReadPredicate> predicate) -> ProcessingPipelineBuilder&;
    auto set_config(const ProcessingConfig& config) -> ProcessingPipelineBuilder&;
    
    auto build() -> std::unique_ptr<IProcessingPipeline>;
    
private:
    std::string m_input_path;
    std::string m_output_path;
    std::vector<std::unique_ptr<IReadMutator>> m_mutators;
    std::vector<std::unique_ptr<IReadPredicate>> m_predicates;
    ProcessingConfig m_config;
};
```

### 4. 策略模式
```cpp
// 策略接口
class IReadMutator {
public:
    virtual ~IReadMutator() = default;
    virtual auto mutate(FqRecord& record) -> MutationResult = 0;
    virtual auto get_name() const -> std::string = 0;
};

// 具体策略
class QualityTrimmer : public IReadMutator {
public:
    explicit QualityTrimmer(double min_quality);
    auto mutate(FqRecord& record) -> MutationResult override;
    auto get_name() const -> std::string override;
};
```

## 🚨 错误处理规范

### 1. 异常层次结构
```cpp
// 基础异常类
class FastQException : public std::runtime_error {
public:
    explicit FastQException(const std::string& message);
    virtual auto get_error_code() const -> ErrorCode = 0;
    virtual auto get_severity() const -> ErrorSeverity = 0;
};

// 具体异常类
class IoException : public FastQException {
public:
    explicit IoException(const std::string& file_path, int error_code);
    auto get_error_code() const -> ErrorCode override;
    auto get_severity() const -> ErrorSeverity override;
    auto get_file_path() const -> std::string;
};

class ValidationException : public FastQException {
public:
    explicit ValidationException(const std::string& field_name, const std::string& error_message);
    auto get_error_code() const -> ErrorCode override;
    auto get_severity() const -> ErrorSeverity override;
    auto get_field_name() const -> std::string;
};
```

### 2. 错误代码定义
```cpp
enum class ErrorCode {
    // IO 错误 (1000-1999)
    FileNotFound = 1001,
    PermissionDenied = 1002,
    InvalidFormat = 1003,
    
    // 验证错误 (2000-2999)
    InvalidConfig = 2001,
    InvalidParameter = 2002,
    ValidationError = 2003,
    
    // 处理错误 (3000-3999)
    ProcessingFailed = 3001,
    MemoryAllocationFailed = 3002,
    Timeout = 3003
};

enum class ErrorSeverity {
    Info,
    Warning,
    Error,
    Critical
};
```

### 3. 错误处理宏
```cpp
// 统一的错误抛出宏
#define FQ_THROW_IO_ERROR(file_path, error_code) \
    throw IoException(file_path, error_code)

#define FQ_THROW_VALIDATION_ERROR(field_name, message) \
    throw ValidationException(field_name, message)

#define FQ_THROW_PROCESSING_ERROR(message) \
    throw ProcessingException(message)

// 错误检查宏
#define FQ_CHECK(condition, message) \
    do { \
        if (!(condition)) { \
            throw ProcessingException(message); \
        } \
    } while(0)

#define FQ_CHECK_IO(condition, file_path, error_code) \
    do { \
        if (!(condition)) { \
            FQ_THROW_IO_ERROR(file_path, error_code); \
        } \
    } while(0)
```

### 4. Result 类型
```cpp
// Result 类型用于可能失败的操作
template<typename T>
class Result {
public:
    static auto success(T value) -> Result<T>;
    static auto error(std::string message) -> Result<T>;
    
    auto is_success() const -> bool;
    auto is_error() const -> bool;
    auto get_value() const -> const T&;
    auto get_error() const -> const std::string&;
    
    // 函数式操作
    template<typename F>
    auto map(F&& func) const -> Result<decltype(func(std::declval<T>()))>;
    
    template<typename F>
    auto and_then(F&& func) const -> Result<typename std::invoke_result_t<F, T>>;

private:
    std::variant<T, std::string> m_data;
};
```

## 🧠 内存管理规范

### 1. 智能指针使用
```cpp
// 所有权明确时使用 unique_ptr
auto create_pipeline() -> std::unique_ptr<IProcessingPipeline>;

// 共享所有权时使用 shared_ptr
class SharedBuffer {
    std::shared_ptr<Data> m_data;
};

// 避免循环引用时使用 weak_ptr
class Cache {
    std::weak_ptr<ResourceManager> m_manager;
};
```

### 2. RAII 模式
```cpp
// 资源获取即初始化
class FileReader {
public:
    explicit FileReader(const std::string& path);
    ~FileReader();  // 自动关闭文件
    
    // 禁用拷贝
    FileReader(const FileReader&) = delete;
    FileReader& operator=(const FileReader&) = delete;
    
    // 允许移动
    FileReader(FileReader&&) noexcept = default;
    FileReader& operator=(FileReader&&) noexcept = default;
    
private:
    std::unique_ptr<std::ifstream> m_stream;
};
```

### 3. 对象池模式
```cpp
// 对象池用于重用昂贵的对象
class BatchPool {
public:
    auto acquire() -> std::unique_ptr<Batch>;
    void release(std::unique_ptr<Batch> batch);
    
private:
    std::stack<std::unique_ptr<Batch>> m_pool;
    std::mutex m_mutex;
};
```

## 🔒 线程安全规范

### 1. 线程安全设计原则
```cpp
// 原子操作用于简单计数器
class ProcessingStatistics {
    std::atomic<size_t> m_total_processed{0};
    std::atomic<size_t> m_error_count{0};
    
    auto increment_processed() -> void {
        m_total_processed.fetch_add(1, std::memory_order_relaxed);
    }
};

// 互斥锁保护复杂数据结构
class BatchPool {
    mutable std::mutex m_mutex;
    std::queue<std::unique_ptr<Batch>> m_pool;
    
    auto acquire() -> std::unique_ptr<Batch> {
        std::lock_guard<std::mutex> lock(m_mutex);
        // ... 操作
    }
};
```

### 2. 无锁数据结构
```cpp
// 使用无锁队列提高并发性能
#include <concurrent_queue.h>

class ConcurrentProcessor {
    moodycamel::ConcurrentQueue<Batch> m_input_queue;
    moodycamel::ConcurrentQueue<Batch> m_output_queue;
};
```

## 📊 配置管理规范

### 1. 配置结构体设计
```cpp
// 配置结构体使用默认值
struct ProcessingConfig {
    size_t batch_size = 10000;
    size_t thread_count = std::thread::hardware_concurrency();
    bool enable_compression = true;
    CompressionLevel compression_level = CompressionLevel::Default;
    std::string temp_directory = "/tmp";
    
    // 验证方法
    auto validate() const -> std::vector<std::string>;
};
```

### 2. 配置验证
```cpp
class ConfigValidator {
public:
    static auto validate(const ProcessingConfig& config) -> std::vector<std::string>;
    static auto validate(const StatisticConfig& config) -> std::vector<std::string>;
    
private:
    static auto validate_batch_size(size_t batch_size) -> std::optional<std::string>;
    static auto validate_thread_count(size_t thread_count) -> std::optional<std::string>;
    static auto validate_directory(const std::string& path) -> std::optional<std::string>;
};
```

## 📝 文档规范

### 1. 文档语言
- **统一使用中文**进行文档编写
- **技术术语**保持英文，但提供中文解释

### 2. 文档模板
```cpp
/**
 * @class ClassName
 * @brief 类的简要描述（一句话）。
 * @details 类的详细描述，包括其设计目的、核心职责和使用示例。
 * 
 * @warning 使用此类时需要特别注意的警告信息。
 * @note 补充说明或需要注意的细节。
 * 
 * @example
 * ```cpp
 * auto processor = ClassName();
 * auto result = processor.process(data);
 * ```
 */
class ClassName {
public:
    /**
     * @brief 函数功能的简要描述（一句话）。
     * @details 对函数行为、算法或复杂逻辑的详细描述。
     * 
     * @param param_name 参数的描述。
     * @return 返回值的描述。
     * 
     * @pre 使用该函数前必须满足的前置条件。
     * @post 函数成功执行后保证满足的后置条件。
     * 
     * @throw ExceptionType 在何种情况下会抛出此异常。
     * 
     * @threadsafe 描述该函数的线程安全性。
     */
    auto function_name(int param_name) -> ReturnType;
};
```

## 🔄 版本兼容性

### 1. 版本化 API
```cpp
// 使用版本化命名空间
namespace fq::v3 {
    class ProcessingPipeline;
}

// 提供向后兼容的头文件
#include <fastqtools/v2/compat.h>
```

### 2. 废弃标记
```cpp
// 标记废弃的 API
[[deprecated("使用 create_pipeline() 替代")]]
auto createProcessingPipeline() -> std::unique_ptr<IProcessingPipeline>;
```

## 🧪 测试规范

### 1. 测试命名
```cpp
// 测试文件：test_ + 模块名
test_fastq_processing.cpp
test_config_validation.cpp

// 测试类：模块名 + Test
class FastQProcessingTest : public ::testing::Test {};

// 测试方法：Test_ + 功能描述
TEST_F(FastQProcessingTest, Test_ProcessSingleRecord);
TEST_F(FastQProcessingTest, Test_ProcessBatchWithInvalidData);
```

### 2. 测试辅助
```cpp
// 测试辅助类
class TestHelper {
public:
    static auto create_test_record() -> FqRecord;
    static auto create_test_config() -> ProcessingConfig;
    static auto create_temp_file() -> std::string;
};
```

## 📋 实施检查清单

### 1. 代码审查检查项
- [ ] 命名约定是否符合规范
- [ ] 错误处理是否统一
- [ ] 内存管理是否安全
- [ ] 线程安全性是否保证
- [ ] 文档是否完整
- [ ] 测试覆盖率是否充足

### 2. 自动化检查
- [ ] 配置静态分析工具检查命名约定
- [ ] 设置 clang-tidy 检查现代 C++ 特性使用
- [ ] 配置单元测试覆盖率检查
- [ ] 设置文档生成和验证

## 🎯 总结

本 API 设计规范为 FastQTools 项目提供了统一的开发标准。通过遵循这些规范，可以：

1. **提高代码一致性**：统一的命名和设计模式
2. **增强可维护性**：清晰的接口和错误处理
3. **改善用户体验**：直观易用的 API 设计
4. **保证代码质量**：完善的测试和文档

所有开发者都应该熟悉并遵循这些规范，共同维护 FastQTools 项目的高质量标准。