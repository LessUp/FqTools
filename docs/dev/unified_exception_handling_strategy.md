# FastQTools 统一异常处理策略

## 📋 概述

本文档为 FastQTools 项目设计了完整的统一异常处理策略，旨在提供一致的错误处理模式、清晰的错误信息传递和可靠的错误恢复机制。

## 🎯 设计目标

### 1. 错误处理原则
- **一致性**: 全项目使用统一的异常处理模式
- **清晰性**: 错误信息清晰易懂，包含足够的上下文
- **可恢复性**: 支持错误恢复和继续执行
- **可追踪性**: 错误信息支持调试和问题追踪
- **国际化**: 支持多语言错误信息

### 2. 异常处理目标
- **错误分类**: 按严重程度和类型分类错误
- **上下文信息**: 提供丰富的错误上下文
- **恢复策略**: 定义清晰的错误恢复机制
- **日志记录**: 统一的错误日志记录
- **用户友好**: 面向用户的友好错误信息

## 🏗️ 异常处理架构

### 1. 异常层次结构
```cpp
// src/modules/error/exception_hierarchy.h
#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace fq::error {

// 基础异常类
class FastQException : public std::runtime_error {
public:
    FastQException(const std::string& message, 
                  ErrorCode code = ErrorCode::Unknown,
                  ErrorSeverity severity = ErrorSeverity::Error);
    
    virtual ~FastQException() = default;
    
    // 获取错误代码
    auto get_error_code() const noexcept -> ErrorCode;
    
    // 获取错误严重程度
    auto get_severity() const noexcept -> ErrorSeverity;
    
    // 获取错误上下文
    auto get_context() const noexcept -> const ErrorContext&;
    
    // 获取错误时间戳
    auto get_timestamp() const noexcept -> std::chrono::system_clock::time_point;
    
    // 获取堆栈跟踪
    auto get_stack_trace() const noexcept -> const std::string&;
    
    // 获取错误建议
    auto get_suggestions() const noexcept -> const std::vector<std::string>&;
    
    // 添加上下文信息
    auto add_context(const std::string& key, const std::string& value) -> void;
    
    // 添加修复建议
    auto add_suggestion(const std::string& suggestion) -> void;
    
    // 转换为用户友好的消息
    auto get_user_message() const -> std::string;
    
    // 转换为日志消息
    auto get_log_message() const -> std::string;

protected:
    ErrorCode m_code;
    ErrorSeverity m_severity;
    ErrorContext m_context;
    std::chrono::system_clock::time_point m_timestamp;
    std::string m_stack_trace;
    std::vector<std::string> m_suggestions;
};

// IO 相关异常
class IoException : public FastQException {
public:
    IoException(const std::string& file_path, 
               int error_code,
               const std::string& operation = "file operation");
    
    auto get_file_path() const noexcept -> const std::string&;
    auto get_system_error_code() const noexcept -> int;
    auto get_operation() const noexcept -> const std::string&;
};

// 配置相关异常
class ConfigurationException : public FastQException {
public:
    ConfigurationException(const std::string& config_key,
                         const std::string& config_value,
                         const std::string& reason);
    
    auto get_config_key() const noexcept -> const std::string&;
    auto get_config_value() const noexcept -> const std::string&;
    auto get_reason() const noexcept -> const std::string&;
};

// 数据验证异常
class ValidationException : public FastQException {
public:
    ValidationException(const std::string& field_name,
                      const std::string& field_value,
                      const std::string& validation_rule);
    
    auto get_field_name() const noexcept -> const std::string&;
    auto get_field_value() const noexcept -> const std::string&;
    auto get_validation_rule() const noexcept -> const std::string&;
};

// 处理流程异常
class ProcessingException : public FastQException {
public:
    ProcessingException(const std::string& operation,
                       size_t processed_count,
                       size_t failed_count,
                       const std::string& details);
    
    auto get_operation() const noexcept -> const std::string&;
    auto get_processed_count() const noexcept -> size_t;
    auto get_failed_count() const noexcept -> size_t;
    auto get_success_rate() const noexcept -> double;
};

// 内存管理异常
class MemoryException : public FastQException {
public:
    MemoryException(size_t requested_size,
                  size_t available_size,
                  const std::string& allocation_type);
    
    auto get_requested_size() const noexcept -> size_t;
    auto get_available_size() const noexcept -> size_t;
    auto get_allocation_type() const noexcept -> const std::string&;
};

// 并发处理异常
class ConcurrencyException : public FastQException {
public:
    ConcurrencyException(const std::string& operation,
                        const std::string& resource_name,
                        int thread_count);
    
    auto get_operation() const noexcept -> const std::string&;
    auto get_resource_name() const noexcept -> const std::string&;
    auto get_thread_count() const noexcept -> int;
};

} // namespace fq::error
```

### 2. 错误代码系统
```cpp
// src/modules/error/error_codes.h
#pragma once

#include <string>
#include <map>

namespace fq::error {

enum class ErrorCode {
    // 通用错误 (1000-1999)
    Unknown = 1000,
    InternalError = 1001,
    NotImplemented = 1002,
    Timeout = 1003,
    
    // IO 错误 (2000-2999)
    FileNotFound = 2001,
    PermissionDenied = 2002,
    InvalidFormat = 2003,
    FileCorrupted = 2004,
    DiskFull = 2005,
    NetworkError = 2006,
    
    // 配置错误 (3000-3999)
    InvalidConfig = 3001,
    MissingConfig = 3002,
    ConfigOutOfRange = 3003,
    ConfigTypeMismatch = 3004,
    
    // 验证错误 (4000-4999)
    InvalidParameter = 4001,
    InvalidRange = 4002,
    InvalidFormat = 4003,
    MissingRequiredField = 4004,
    ValidationFailed = 4005,
    
    // 数据错误 (5000-5999)
    DataCorrupted = 5001,
    DataInconsistent = 5002,
    DataTooLarge = 5003,
    DataEmpty = 5004,
    
    // 处理错误 (6000-6999)
    ProcessingFailed = 6001,
    ProcessingTimeout = 6002,
    ProcessingInterrupted = 6003,
    ResourceBusy = 6004,
    
    // 内存错误 (7000-7999)
    MemoryAllocationFailed = 7001,
    MemoryAccessViolation = 7002,
    MemoryLeakDetected = 7003,
    MemoryLimitExceeded = 7004,
    
    // 并发错误 (8000-8999)
    DeadlockDetected = 8001,
    RaceCondition = 8002,
    ThreadCreationFailed = 8003,
    SynchronizationError = 8004
};

enum class ErrorSeverity {
    Info = 0,      // 信息性消息
    Warning = 1,   // 警告，程序可以继续
    Error = 2,     // 错误，当前操作失败
    Critical = 3,  // 严重错误，程序可能需要终止
    Fatal = 4      // 致命错误，程序必须终止
};

// 错误代码信息
struct ErrorInfo {
    ErrorCode code;
    std::string name;
    std::string description;
    ErrorSeverity default_severity;
    std::vector<std::string> suggestions;
};

// 错误代码注册表
class ErrorCodeRegistry {
public:
    static auto get_instance() -> ErrorCodeRegistry&;
    
    auto register_error(const ErrorInfo& info) -> void;
    auto get_error_info(ErrorCode code) const -> const ErrorInfo&;
    auto get_all_errors() const -> std::vector<ErrorInfo>;
    
    auto get_user_message(ErrorCode code) const -> std::string;
    auto get_suggestions(ErrorCode code) const -> std::vector<std::string>;
    
private:
    ErrorCodeRegistry();
    std::map<ErrorCode, ErrorInfo> m_error_registry;
};

} // namespace fq::error
```

### 3. 错误上下文系统
```cpp
// src/modules/error/error_context.h
#pragma once

#include <string>
#include <map>
#include <variant>
#include <vector>

namespace fq::error {

// 错误上下文信息
class ErrorContext {
public:
    using ContextValue = std::variant<
        std::string,
        int,
        size_t,
        double,
        bool
    >;
    
    // 添加上下文信息
    auto add(const std::string& key, const ContextValue& value) -> void;
    auto add(const std::string& key, const std::string& value) -> void;
    auto add(const std::string& key, int value) -> void;
    auto add(const std::string& key, size_t value) -> void;
    auto add(const std::string& key, double value) -> void;
    auto add(const std::string& key, bool value) -> void;
    
    // 获取上下文信息
    auto get(const std::string& key) const -> std::optional<ContextValue>;
    auto get_string(const std::string& key) const -> std::optional<std::string>;
    auto get_int(const std::string& key) const -> std::optional<int>;
    auto get_size(const std::string& key) const -> std::optional<size_t>;
    auto get_double(const std::string& key) const -> std::optional<double>;
    auto get_bool(const std::string& key) const -> std::optional<bool>;
    
    // 检查是否存在
    auto contains(const std::string& key) const -> bool;
    
    // 获取所有键
    auto get_keys() const -> std::vector<std::string>;
    
    // 清空上下文
    auto clear() -> void;
    
    // 格式化为字符串
    auto format() const -> std::string;

private:
    std::map<std::string, ContextValue> m_context;
};

// 错误上下文构建器
class ErrorContextBuilder {
public:
    auto add(const std::string& key, const ErrorContext::ContextValue& value) -> ErrorContextBuilder&;
    auto add_file_info(const std::string& file_path, int line_number) -> ErrorContextBuilder&;
    auto add_system_info(int error_code, const std::string& error_message) -> ErrorContextBuilder&;
    auto add_operation_info(const std::string& operation, const std::string& details) -> ErrorContextBuilder&;
    auto add_performance_info(size_t processed_count, double elapsed_time) -> ErrorContextBuilder&;
    
    auto build() const -> ErrorContext;

private:
    ErrorContext m_context;
};

// 常用上下文键
namespace ContextKeys {
    constexpr const char* FILE_PATH = "file_path";
    constexpr const char* LINE_NUMBER = "line_number";
    constexpr const char* FUNCTION_NAME = "function_name";
    constexpr const char* ERROR_CODE = "error_code";
    constexpr const char* ERROR_MESSAGE = "error_message";
    constexpr const char* OPERATION = "operation";
    constexpr const char* PROCESSED_COUNT = "processed_count";
    constexpr const char* FAILED_COUNT = "failed_count";
    constexpr const char* ELAPSED_TIME = "elapsed_time";
    constexpr const char* MEMORY_USAGE = "memory_usage";
    constexpr const char* THREAD_COUNT = "thread_count";
    constexpr const char* CONFIG_KEY = "config_key";
    constexpr const char* CONFIG_VALUE = "config_value";
}

} // namespace fq::error
```

## 🔧 异常处理工具

### 1. 异常抛出宏
```cpp
// src/modules/error/exception_macros.h
#pragma once

#include "exception_hierarchy.h"
#include "error_context.h"
#include <sstream>

// 基础异常抛出宏
#define FQ_THROW(exception_type, message) \
    throw exception_type(message, fq::error::ErrorCode::exception_type##Error)

// 带上下文的异常抛出宏
#define FQ_THROW_WITH_CONTEXT(exception_type, message, context) \
    do { \
        auto ex = exception_type(message, fq::error::ErrorCode::exception_type##Error); \
        ex.add_context(context); \
        throw ex; \
    } while(0)

// IO 异常宏
#define FQ_THROW_IO_ERROR(file_path, system_error) \
    throw fq::error::IoException(file_path, system_error)

#define FQ_THROW_FILE_NOT_FOUND(file_path) \
    FQ_THROW_IO_ERROR(file_path, ENOENT)

#define FQ_THROW_PERMISSION_DENIED(file_path) \
    FQ_THROW_IO_ERROR(file_path, EACCES)

// 配置异常宏
#define FQ_THROW_CONFIG_ERROR(key, value, reason) \
    throw fq::error::ConfigurationException(key, value, reason)

#define FQ_THROW_MISSING_CONFIG(key) \
    FQ_THROW_CONFIG_ERROR(key, "", "Missing required configuration")

// 验证异常宏
#define FQ_THROW_VALIDATION_ERROR(field, value, rule) \
    throw fq::error::ValidationException(field, value, rule)

// 处理异常宏
#define FQ_THROW_PROCESSING_ERROR(operation, processed, failed, details) \
    throw fq::error::ProcessingException(operation, processed, failed, details)

// 内存异常宏
#define FQ_THROW_MEMORY_ERROR(requested, available, type) \
    throw fq::error::MemoryException(requested, available, type)

// 并发异常宏
#define FQ_THROW_CONCURRENCY_ERROR(operation, resource, threads) \
    throw fq::error::ConcurrencyException(operation, resource, threads)

// 错误检查宏
#define FQ_CHECK(condition, message) \
    do { \
        if (!(condition)) { \
            FQ_THROW(fq::error::FastQException, message); \
        } \
    } while(0)

#define FQ_CHECK_IO(condition, file_path, error_code) \
    do { \
        if (!(condition)) { \
            FQ_THROW_IO_ERROR(file_path, error_code); \
        } \
    } while(0)

#define FQ_CHECK_VALID(condition, field, value, rule) \
    do { \
        if (!(condition)) { \
            FQ_THROW_VALIDATION_ERROR(field, value, rule); \
        } \
    } while(0)

// 带错误代码的检查宏
#define FQ_CHECK_CODE(condition, code, message) \
    do { \
        if (!(condition)) { \
            throw fq::error::FastQException(message, code); \
        } \
    } while(0)

// 函数异常处理宏
#define FQ_TRY(function_call) \
    try { \
        function_call; \
    } catch (const fq::error::FastQException& ex) { \
        /* 处理 FastQ 异常 */ \
        handle_fastq_exception(ex); \
    } catch (const std::exception& ex) { \
        /* 处理标准异常 */ \
        handle_std_exception(ex); \
    } catch (...) { \
        /* 处理未知异常 */ \
        handle_unknown_exception(); \
    }

// 返回值包装宏
#define FQ_RETURN_IF_ERROR(expression) \
    do { \
        auto result = expression; \
        if (!result.is_success()) { \
            return result.error(); \
        } \
    } while(0)

#define FQ_RETURN_OR_THROW(expression) \
    do { \
        auto result = expression; \
        if (!result.is_success()) { \
            throw fq::error::FastQException(result.error().message()); \
        } \
        return result.value(); \
    } while(0)
```

### 2. 错误恢复机制
```cpp
// src/modules/error/error_recovery.h
#pragma once

#include <functional>
#include <memory>
#include <vector>

namespace fq::error {

// 错误恢复策略
enum class RecoveryStrategy {
    Retry,          // 重试操作
    Skip,           // 跳过当前项
    UseDefault,     // 使用默认值
    Fallback,       // 使用备用方案
    Abort,          // 中止操作
    Continue        // 继续执行
};

// 错误恢复结果
enum class RecoveryResult {
    Success,        // 恢复成功
    Failed,         // 恢复失败
    Skipped,        // 跳过处理
    Retrying,       // 正在重试
    Aborted         // 操作中止
};

// 错误恢复处理器
class ErrorRecoveryHandler {
public:
    using RecoveryFunction = std::function<RecoveryResult(const FastQException&)>;
    
    // 注册恢复处理器
    auto register_handler(ErrorCode code, RecoveryFunction handler) -> void;
    auto register_handler(ErrorSeverity severity, RecoveryFunction handler) -> void;
    auto register_default_handler(RecoveryFunction handler) -> void;
    
    // 尝试恢复
    auto try_recover(const FastQException& ex) -> RecoveryResult;
    
    // 设置重试策略
    auto set_retry_policy(ErrorCode code, size_t max_retries, std::chrono::milliseconds delay) -> void;
    
    // 获取恢复统计
    auto get_recovery_stats() const -> RecoveryStats;

private:
    std::map<ErrorCode, RecoveryFunction> m_code_handlers;
    std::map<ErrorSeverity, RecoveryFunction> m_severity_handlers;
    RecoveryFunction m_default_handler;
    std::map<ErrorCode, RetryPolicy> m_retry_policies;
    RecoveryStats m_stats;
};

// 错误恢复策略构建器
class RecoveryStrategyBuilder {
public:
    auto on_error(ErrorCode code) -> RecoveryStrategyBuilder&;
    auto on_severity(ErrorSeverity severity) -> RecoveryStrategyBuilder&;
    auto retry(size_t max_attempts, std::chrono::milliseconds delay) -> RecoveryStrategyBuilder&;
    auto skip() -> RecoveryStrategyBuilder&;
    auto use_default_value() -> RecoveryStrategyBuilder&;
    auto fallback_to(std::function<void()> fallback) -> RecoveryStrategyBuilder&;
    auto abort() -> RecoveryStrategyBuilder&;
    auto continue_execution() -> RecoveryStrategyBuilder&;
    
    auto build() const -> std::function<RecoveryResult(const FastQException&)>;

private:
    std::vector<ErrorCode> m_error_codes;
    std::vector<ErrorSeverity> m_severities;
    RecoveryStrategy m_strategy;
    std::function<void()> m_fallback;
    size_t m_max_retries = 0;
    std::chrono::milliseconds m_retry_delay{0};
};

// 预定义的恢复策略
namespace RecoveryStrategies {
    // 文件操作恢复策略
    auto file_read_retry_strategy() -> std::function<RecoveryResult(const FastQException&)>;
    auto file_write_skip_strategy() -> std::function<RecoveryResult(const FastQException&)>;
    
    // 处理流程恢复策略
    auto record_skip_strategy() -> std::function<RecoveryResult(const FastQException&)>;
    auto batch_continue_strategy() -> std::function<RecoveryResult(const FastQException&)>;
    
    // 配置恢复策略
    auto config_use_default_strategy() -> std::function<RecoveryResult(const FastQException&)>;
    auto config_abort_strategy() -> std::function<RecoveryResult(const FastQException&)>;
    
    // 内存恢复策略
    auto memory_reduce_batch_strategy() -> std::function<RecoveryResult(const FastQException&)>;
    auto memory_abort_strategy() -> std::function<RecoveryResult(const FastQException&)>;
}

} // namespace fq::error
```

### 3. 错误日志系统
```cpp
// src/modules/error/error_logger.h
#pragma once

#include <string>
#include <memory>
#include <vector>
#include <chrono>

namespace fq::error {

// 日志级别
enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Critical
};

// 日志条目
struct LogEntry {
    std::chrono::system_clock::time_point timestamp;
    LogLevel level;
    std::string message;
    std::string category;
    ErrorContext context;
    std::string thread_id;
    std::string file_path;
    int line_number;
    std::string function_name;
};

// 错误日志记录器
class ErrorLogger {
public:
    static auto get_instance() -> ErrorLogger&;
    
    // 记录异常
    auto log_exception(const FastQException& ex, 
                      LogLevel level = LogLevel::Error) -> void;
    
    // 记录消息
    auto log_message(LogLevel level, 
                   const std::string& category,
                   const std::string& message) -> void;
    
    // 记录带上下文的消息
    auto log_message(LogLevel level,
                   const std::string& category,
                   const std::string& message,
                   const ErrorContext& context) -> void;
    
    // 设置日志级别
    auto set_log_level(LogLevel level) -> void;
    auto get_log_level() const -> LogLevel;
    
    // 添加日志输出器
    auto add_appender(std::unique_ptr<LogAppender> appender) -> void;
    
    // 获取日志历史
    auto get_recent_entries(size_t count = 100) const -> std::vector<LogEntry>;
    auto get_entries_by_category(const std::string& category) const -> std::vector<LogEntry>;
    
    // 清理日志
    auto clear_old_entries(std::chrono::hours age = std::chrono::hours(24)) -> void;
    
    // 错误统计
    auto get_error_statistics() const -> ErrorStatistics;

private:
    ErrorLogger();
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

// 日志输出器接口
class LogAppender {
public:
    virtual ~LogAppender() = default;
    virtual auto append(const LogEntry& entry) -> void = 0;
    virtual auto flush() -> void = 0;
};

// 控制台输出器
class ConsoleAppender : public LogAppender {
public:
    explicit ConsoleAppender(bool colored = true);
    auto append(const LogEntry& entry) -> void override;
    auto flush() -> void override;

private:
    bool m_colored;
};

// 文件输出器
class FileAppender : public LogAppender {
public:
    explicit FileAppender(const std::string& file_path, 
                         bool rotate = true,
                         size_t max_size = 10 * 1024 * 1024);
    auto append(const LogEntry& entry) -> void override;
    auto flush() -> void override;

private:
    std::string m_file_path;
    bool m_rotate;
    size_t m_max_size;
    std::ofstream m_file_stream;
};

// JSON 输出器
class JsonAppender : public LogAppender {
public:
    explicit JsonAppender(const std::string& file_path);
    auto append(const LogEntry& entry) -> void override;
    auto flush() -> void override;

private:
    std::string m_file_path;
    std::ofstream m_file_stream;
};

// 日志宏
#define FQ_LOG_DEBUG(category, message) \
    fq::error::ErrorLogger::get_instance().log_message( \
        fq::error::LogLevel::Debug, category, message)

#define FQ_LOG_INFO(category, message) \
    fq::error::ErrorLogger::get_instance().log_message( \
        fq::error::LogLevel::Info, category, message)

#define FQ_LOG_WARNING(category, message) \
    fq::error::ErrorLogger::get_instance().log_message( \
        fq::error::LogLevel::Warning, category, message)

#define FQ_LOG_ERROR(category, message) \
    fq::error::ErrorLogger::get_instance().log_message( \
        fq::error::LogLevel::Error, category, message)

#define FQ_LOG_CRITICAL(category, message) \
    fq::error::ErrorLogger::get_instance().log_message( \
        fq::error::LogLevel::Critical, category, message)

#define FQ_LOG_EXCEPTION(exception) \
    fq::error::ErrorLogger::get_instance().log_exception(exception)

} // namespace fq::error
```

## 🛠️ 使用示例

### 1. 基本异常处理
```cpp
#include "modules/error/exception_macros.h"

void process_file(const std::string& file_path) {
    try {
        // 检查文件是否存在
        FQ_CHECK(std::filesystem::exists(file_path), 
                "File does not exist: " + file_path);
        
        // 打开文件
        std::ifstream file(file_path);
        FQ_CHECK_IO(file.is_open(), file_path, errno);
        
        // 处理文件内容
        process_file_content(file);
        
    } catch (const fq::error::IoException& ex) {
        FQ_LOG_EXCEPTION(ex);
        FQ_LOG_ERROR("file_processing", "Failed to process file: " + file_path);
        throw; // 重新抛出给上层处理
    } catch (const fq::error::FastQException& ex) {
        FQ_LOG_EXCEPTION(ex);
        throw; // 重新抛出给上层处理
    } catch (const std::exception& ex) {
        FQ_LOG_ERROR("file_processing", 
                   std::string("Unexpected error: ") + ex.what());
        throw fq::error::FastQException("Unexpected error during file processing");
    }
}
```

### 2. 带上下文的异常处理
```cpp
#include "modules/error/error_context.h"

void validate_config(const Config& config) {
    auto context = ErrorContextBuilder()
        .add_operation_info("config_validation", "Validating configuration parameters")
        .build();
    
    try {
        // 验证必需参数
        if (config.input_path.empty()) {
            FQ_THROW_WITH_CONTEXT(
                fq::error::ConfigurationException,
                "Input path is required",
                context.add("config_key", "input_path")
            );
        }
        
        // 验证参数范围
        if (config.batch_size < 1000 || config.batch_size > 1000000) {
            FQ_THROW_WITH_CONTEXT(
                fq::error::ConfigurationException,
                "Batch size out of range",
                context.add("config_key", "batch_size")
                     .add("min_value", 1000)
                     .add("max_value", 1000000)
                     .add("actual_value", config.batch_size)
            );
        }
        
    } catch (const fq::error::FastQException& ex) {
        FQ_LOG_EXCEPTION(ex);
        throw;
    }
}
```

### 3. 错误恢复处理
```cpp
#include "modules/error/error_recovery.h"

void process_records_with_recovery(const std::vector<FqRecord>& records) {
    auto recovery_handler = std::make_unique<ErrorRecoveryHandler>();
    
    // 注册错误恢复策略
    recovery_handler->register_handler(
        ErrorCode::DataCorrupted,
        RecoveryStrategies::record_skip_strategy()
    );
    
    recovery_handler->register_handler(
        ErrorCode::MemoryAllocationFailed,
        RecoveryStrategies::memory_reduce_batch_strategy()
    );
    
    size_t processed_count = 0;
    size_t skipped_count = 0;
    
    for (const auto& record : records) {
        try {
            process_single_record(record);
            processed_count++;
            
        } catch (const fq::error::FastQException& ex) {
            // 尝试恢复
            auto result = recovery_handler->try_recover(ex);
            
            switch (result) {
                case RecoveryResult::Success:
                    FQ_LOG_INFO("recovery", "Successfully recovered from error");
                    break;
                case RecoveryResult::Skipped:
                    skipped_count++;
                    FQ_LOG_WARNING("recovery", "Skipped corrupted record");
                    break;
                case RecoveryResult::Failed:
                    FQ_LOG_ERROR("recovery", "Recovery failed, aborting");
                    throw;
                default:
                    break;
            }
        }
    }
    
    FQ_LOG_INFO("processing", 
               "Processed " + std::to_string(processed_count) + " records, " +
               "skipped " + std::to_string(skipped_count) + " records");
}
```

### 4. Result 类型使用
```cpp
#include "modules/error/result_type.h"

Result<std::unique_ptr<FqRecord>> read_record_safely(std::ifstream& file) {
    try {
        auto record = std::make_unique<FqRecord>();
        
        // 读取记录
        if (!read_record_from_file(file, *record)) {
            return Result<std::unique_ptr<FqRecord>>::error("Failed to read record");
        }
        
        // 验证记录
        if (!record->is_valid()) {
            return Result<std::unique_ptr<FqRecord>>::error("Invalid record format");
        }
        
        return Result<std::unique_ptr<FqRecord>>::success(std::move(record));
        
    } catch (const std::exception& ex) {
        return Result<std::unique_ptr<FqRecord>>::error(ex.what());
    }
}

void process_file_with_result() {
    std::ifstream file("data.fastq");
    
    while (true) {
        auto result = read_record_safely(file);
        
        if (result.is_error()) {
            FQ_LOG_ERROR("file_processing", result.error());
            continue; // 跳过错误记录
        }
        
        auto record = result.value();
        process_record(*record);
    }
}
```

## 📊 实施计划

### 1. 第一阶段：基础架构（1-2周）
- 实现异常层次结构
- 定义错误代码系统
- 实现错误上下文
- 创建异常抛出宏

### 2. 第二阶段：恢复机制（2-3周）
- 实现错误恢复处理器
- 创建预定义恢复策略
- 集成到现有代码中
- 编写单元测试

### 3. 第三阶段：日志系统（1-2周）
- 实现错误日志记录器
- 创建各种日志输出器
- 集成日志宏
- 完善日志功能

### 4. 第四阶段：集成测试（1周）
- 编写异常处理测试
- 测试错误恢复机制
- 验证日志功能
- 性能测试

## 🎯 预期效果

通过实施这个统一的异常处理策略，FastQTools 项目将获得：

1. **一致的错误处理**: 统一的异常类和错误处理模式
2. **清晰的错误信息**: 丰富的错误上下文和建议
3. **可靠的错误恢复**: 系统性的错误恢复机制
4. **完整的日志记录**: 全面的错误日志和统计
5. **更好的用户体验**: 友好的错误信息和恢复提示

这个异常处理策略为 FastQTools 项目提供了企业级的错误处理体系。