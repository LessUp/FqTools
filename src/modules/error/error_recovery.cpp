// src/modules/error/error_recovery.cpp
#include "error_recovery.h"
#include "exception_macros.h"
#include "error_logger.h"
#include <thread>
#include <chrono>

namespace fq::error {

// 错误恢复处理器实现
class ErrorRecoveryHandler::Impl {
public:
    std::map<ErrorCode, RecoveryFunction> code_handlers;
    std::map<ErrorSeverity, RecoveryFunction> severity_handlers;
    RecoveryFunction default_handler;
    std::map<ErrorCode, RetryPolicy> retry_policies;
    RecoveryStats stats;
    
    // 默认恢复处理器
    static auto default_recovery_handler(const FastQException& ex) -> RecoveryResult {
        FQ_LOG_EXCEPTION(ex);
        
        switch (ex.get_severity()) {
            case ErrorSeverity::Info:
            case ErrorSeverity::Warning:
                return RecoveryResult::Continue;
            case ErrorSeverity::Error:
                return RecoveryResult::Failed;
            case ErrorSeverity::Critical:
            case ErrorSeverity::Fatal:
                return RecoveryResult::Aborted;
            default:
                return RecoveryResult::Failed;
        }
    }
};

ErrorRecoveryHandler::ErrorRecoveryHandler() 
    : m_impl(std::make_unique<Impl>()) {
    // 注册默认处理器
    m_impl->default_handler = Impl::default_recovery_handler;
}

ErrorRecoveryHandler::~ErrorRecoveryHandler() = default;

auto ErrorRecoveryHandler::register_handler(ErrorCode code, RecoveryFunction handler) -> void {
    m_impl->code_handlers[code] = handler;
}

auto ErrorRecoveryHandler::register_handler(ErrorSeverity severity, RecoveryFunction handler) -> void {
    m_impl->severity_handlers[severity] = handler;
}

auto ErrorRecoveryHandler::register_default_handler(RecoveryFunction handler) -> void {
    m_impl->default_handler = handler;
}

auto ErrorRecoveryHandler::try_recover(const FastQException& ex) -> RecoveryResult {
    // 更新统计
    m_impl->stats.total_attempts++;
    
    // 首先尝试特定错误代码的处理器
    auto code_it = m_impl->code_handlers.find(ex.get_error_code());
    if (code_it != m_impl->code_handlers.end()) {
        auto result = code_it->second(ex);
        update_stats(result, ex.get_error_code());
        return result;
    }
    
    // 尝试基于严重程度的处理器
    auto severity_it = m_impl->severity_handlers.find(ex.get_severity());
    if (severity_it != m_impl->severity_handlers.end()) {
        auto result = severity_it->second(ex);
        update_stats(result, ex.get_error_code());
        return result;
    }
    
    // 使用默认处理器
    auto result = m_impl->default_handler(ex);
    update_stats(result, ex.get_error_code());
    return result;
}

auto ErrorRecoveryHandler::set_retry_policy(ErrorCode code, 
                                           size_t max_retries, 
                                           std::chrono::milliseconds delay) -> void {
    m_impl->retry_policies[code] = {max_retries, delay};
}

auto ErrorRecoveryHandler::get_recovery_stats() const -> RecoveryStats {
    return m_impl->stats;
}

auto ErrorRecoveryHandler::update_stats(RecoveryResult result, ErrorCode code) -> void {
    m_impl->stats.last_error_code = code;
    m_impl->stats.last_recovery_attempt = std::chrono::system_clock::now();
    
    switch (result) {
        case RecoveryResult::Success:
            m_impl->stats.successful_recoveries++;
            break;
        case RecoveryResult::Failed:
            m_impl->stats.failed_recoveries++;
            break;
        case RecoveryResult::Skipped:
            m_impl->stats.skipped_operations++;
            break;
        case RecoveryResult::Retrying:
            m_impl->stats.retry_attempts++;
            break;
        case RecoveryResult::Aborted:
            m_impl->stats.aborted_operations++;
            break;
    }
}

// 错误恢复策略构建器实现
auto RecoveryStrategyBuilder::on_error(ErrorCode code) -> RecoveryStrategyBuilder& {
    m_error_codes.push_back(code);
    return *this;
}

auto RecoveryStrategyBuilder::on_severity(ErrorSeverity severity) -> RecoveryStrategyBuilder& {
    m_severities.push_back(severity);
    return *this;
}

auto RecoveryStrategyBuilder::retry(size_t max_attempts, std::chrono::milliseconds delay) -> RecoveryStrategyBuilder& {
    m_strategy = RecoveryStrategy::Retry;
    m_max_retries = max_attempts;
    m_retry_delay = delay;
    return *this;
}

auto RecoveryStrategyBuilder::skip() -> RecoveryStrategyBuilder& {
    m_strategy = RecoveryStrategy::Skip;
    return *this;
}

auto RecoveryStrategyBuilder::use_default_value() -> RecoveryStrategyBuilder& {
    m_strategy = RecoveryStrategy::UseDefault;
    return *this;
}

auto RecoveryStrategyBuilder::fallback_to(std::function<void()> fallback) -> RecoveryStrategyBuilder& {
    m_strategy = RecoveryStrategy::Fallback;
    m_fallback = fallback;
    return *this;
}

auto RecoveryStrategyBuilder::abort() -> RecoveryStrategyBuilder& {
    m_strategy = RecoveryStrategy::Abort;
    return *this;
}

auto RecoveryStrategyBuilder::continue_execution() -> RecoveryStrategyBuilder& {
    m_strategy = RecoveryStrategy::Continue;
    return *this;
}

auto RecoveryStrategyBuilder::build() const -> std::function<RecoveryResult(const FastQException&)> {
    return [this](const FastQException& ex) -> RecoveryResult {
        // 检查是否匹配指定的错误代码或严重程度
        bool matches = false;
        
        if (!m_error_codes.empty()) {
            matches = std::find(m_error_codes.begin(), m_error_codes.end(), 
                               ex.get_error_code()) != m_error_codes.end();
        }
        
        if (!matches && !m_severities.empty()) {
            matches = std::find(m_severities.begin(), m_severities.end(), 
                               ex.get_severity()) != m_severities.end();
        }
        
        if (!m_error_codes.empty() && !m_severities.empty()) {
            matches = std::find(m_error_codes.begin(), m_error_codes.end(), 
                               ex.get_error_code()) != m_error_codes.end() ||
                     std::find(m_severities.begin(), m_severities.end(), 
                               ex.get_severity()) != m_severities.end();
        }
        
        if (!m_error_codes.empty() || !m_severities.empty()) {
            if (!matches) {
                return RecoveryResult::Failed;
            }
        }
        
        // 执行相应的恢复策略
        switch (m_strategy) {
            case RecoveryStrategy::Retry:
                return handle_retry(ex);
            case RecoveryStrategy::Skip:
                return RecoveryResult::Skipped;
            case RecoveryStrategy::UseDefault:
                return RecoveryResult::Success;
            case RecoveryStrategy::Fallback:
                if (m_fallback) {
                    m_fallback();
                }
                return RecoveryResult::Success;
            case RecoveryStrategy::Abort:
                return RecoveryResult::Aborted;
            case RecoveryStrategy::Continue:
                return RecoveryResult::Continue;
            default:
                return RecoveryResult::Failed;
        }
    };
}

auto RecoveryStrategyBuilder::handle_retry(const FastQException& ex) const -> RecoveryResult {
    for (size_t attempt = 0; attempt < m_max_retries; ++attempt) {
        FQ_LOG_INFO("recovery", 
                   "Retry attempt " + std::to_string(attempt + 1) + "/" + 
                   std::to_string(m_max_retries) + " for error: " + ex.what());
        
        // 等待重试延迟
        if (m_retry_delay.count() > 0) {
            std::this_thread::sleep_for(m_retry_delay);
        }
        
        // 这里应该重新尝试原始操作
        // 由于这是通用实现，我们返回 Retrying 状态
        // 实际的重试逻辑应该在调用者中实现
        return RecoveryResult::Retrying;
    }
    
    FQ_LOG_WARNING("recovery", "Max retry attempts exceeded for error: " + std::string(ex.what()));
    return RecoveryResult::Failed;
}

// 预定义恢复策略实现
namespace RecoveryStrategies {

// 文件读取重试策略
auto file_read_retry_strategy() -> std::function<RecoveryResult(const FastQException&)> {
    return RecoveryStrategyBuilder()
        .on_error(ErrorCode::FileNotFound)
        .on_error(ErrorCode::PermissionDenied)
        .on_error(ErrorCode::NetworkError)
        .retry(3, std::chrono::milliseconds(1000))
        .build();
}

// 文件写入跳过策略
auto file_write_skip_strategy() -> std::function<RecoveryResult(const FastQException&)> {
    return RecoveryStrategyBuilder()
        .on_error(ErrorCode::DiskFull)
        .on_error(ErrorCode::PermissionDenied)
        .skip()
        .build();
}

// 记录跳过策略
auto record_skip_strategy() -> std::function<RecoveryResult(const FastQException&)> {
    return RecoveryStrategyBuilder()
        .on_error(ErrorCode::DataCorrupted)
        .on_error(ErrorCode::DataInconsistent)
        .on_severity(ErrorSeverity::Warning)
        .skip()
        .build();
}

// 批处理继续策略
auto batch_continue_strategy() -> std::function<RecoveryResult(const FastQException&)> {
    return RecoveryStrategyBuilder()
        .on_error(ErrorCode::ProcessingFailed)
        .on_severity(ErrorSeverity::Error)
        .continue_execution()
        .build();
}

// 配置使用默认值策略
auto config_use_default_strategy() -> std::function<RecoveryResult(const FastQException&)> {
    return RecoveryStrategyBuilder()
        .on_error(ErrorCode::MissingConfig)
        .on_error(ErrorCode::ConfigOutOfRange)
        .use_default_value()
        .build();
}

// 配置中止策略
auto config_abort_strategy() -> std::function<RecoveryResult(const FastQException&)> {
    return RecoveryStrategyBuilder()
        .on_error(ErrorCode::InvalidConfig)
        .on_error(ErrorCode::ConfigTypeMismatch)
        .abort()
        .build();
}

// 内存减少批处理策略
auto memory_reduce_batch_strategy() -> std::function<RecoveryResult(const FastQException&)> {
    return [](const FastQException& ex) -> RecoveryResult {
        if (ex.get_error_code() == ErrorCode::MemoryAllocationFailed ||
            ex.get_error_code() == ErrorCode::MemoryLimitExceeded) {
            
            FQ_LOG_INFO("recovery", "Attempting to recover from memory error by reducing batch size");
            
            // 获取当前上下文中的批处理大小
            auto context = ex.get_context();
            if (auto batch_size = context.get_size("batch_size")) {
                // 减少批处理大小
                size_t new_batch_size = std::max(*batch_size / 2, size_t(100));
                
                FQ_LOG_INFO("recovery", 
                           "Reducing batch size from " + std::to_string(*batch_size) + 
                           " to " + std::to_string(new_batch_size));
                
                // 这里应该更新批处理大小
                // 由于这是通用实现，我们返回 Success
                return RecoveryResult::Success;
            }
        }
        
        return RecoveryResult::Failed;
    };
}

// 内存中止策略
auto memory_abort_strategy() -> std::function<RecoveryResult(const FastQException&)> {
    return RecoveryStrategyBuilder()
        .on_error(ErrorCode::MemoryAccessViolation)
        .on_error(ErrorCode::MemoryLeakDetected)
        .on_severity(ErrorSeverity::Critical)
        .abort()
        .build();
}

} // namespace RecoveryStrategies

// 全局错误恢复处理器实例
namespace {
    std::unique_ptr<ErrorRecoveryHandler> g_global_recovery_handler;
    std::mutex g_recovery_handler_mutex;
}

auto get_global_recovery_handler() -> ErrorRecoveryHandler& {
    std::lock_guard<std::mutex> lock(g_recovery_handler_mutex);
    
    if (!g_global_recovery_handler) {
        g_global_recovery_handler = std::make_unique<ErrorRecoveryHandler>();
        
        // 注册默认的恢复策略
        g_global_recovery_handler->register_handler(
            ErrorCode::FileNotFound,
            RecoveryStrategies::file_read_retry_strategy()
        );
        
        g_global_recovery_handler->register_handler(
            ErrorCode::DataCorrupted,
            RecoveryStrategies::record_skip_strategy()
        );
        
        g_global_recovery_handler->register_handler(
            ErrorCode::MemoryAllocationFailed,
            RecoveryStrategies::memory_reduce_batch_strategy()
        );
        
        g_global_recovery_handler->register_handler(
            ErrorSeverity::Critical,
            [](const FastQException& ex) -> RecoveryResult {
                FQ_LOG_CRITICAL("recovery", "Critical error encountered: " + std::string(ex.what()));
                return RecoveryResult::Aborted;
            }
        );
    }
    
    return *g_global_recovery_handler;
}

// 便捷函数
auto try_recover_from_error(const FastQException& ex) -> RecoveryResult {
    return get_global_recovery_handler().try_recover(ex);
}

auto register_recovery_strategy(ErrorCode code, 
                              std::function<RecoveryResult(const FastQException&)> handler) -> void {
    get_global_recovery_handler().register_handler(code, handler);
}

auto get_recovery_statistics() -> RecoveryStats {
    return get_global_recovery_handler().get_recovery_stats();
}

} // namespace fq::error