// src/modules/error/error_recovery.h
#pragma once

#include <functional>
#include <memory>
#include <vector>
#include <chrono>
#include <map>

namespace fq::error {

// 前向声明
class FastQException;

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
    Aborted,        // 操作中止
    Continue        // 继续执行
};

// 重试策略
struct RetryPolicy {
    size_t max_retries;
    std::chrono::milliseconds delay;
};

// 恢复统计信息
struct RecoveryStats {
    size_t total_attempts = 0;
    size_t successful_recoveries = 0;
    size_t failed_recoveries = 0;
    size_t skipped_operations = 0;
    size_t retry_attempts = 0;
    size_t aborted_operations = 0;
    ErrorCode last_error_code = ErrorCode::Unknown;
    std::chrono::system_clock::time_point last_recovery_attempt;
};

// 错误恢复处理器
class ErrorRecoveryHandler {
public:
    using RecoveryFunction = std::function<RecoveryResult(const FastQException&)>;
    
    ErrorRecoveryHandler();
    ~ErrorRecoveryHandler();
    
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
    class Impl;
    std::unique_ptr<Impl> m_impl;
    
    auto update_stats(RecoveryResult result, ErrorCode code) -> void;
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
    
    auto handle_retry(const FastQException& ex) const -> RecoveryResult;
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

// 便捷函数
auto get_global_recovery_handler() -> ErrorRecoveryHandler&;
auto try_recover_from_error(const FastQException& ex) -> RecoveryResult;
auto register_recovery_strategy(ErrorCode code, 
                              std::function<RecoveryResult(const FastQException&)> handler) -> void;
auto get_recovery_statistics() -> RecoveryStats;

} // namespace fq::error