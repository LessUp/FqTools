/**
 * @file error_logger.h
 * @brief 错误日志记录器定义
 * @details 提供统一的错误日志记录功能，支持多种输出格式和目标
 * 
 * @author FastQTools Team
 * @date 2024
 * @version 1.0
 * 
 * @copyright Copyright (c) 2024 FastQTools
 * @license MIT License
 */

#pragma once

#include <string>
#include <memory>
#include <vector>
#include <chrono>
#include <fstream>
#include <mutex>
#include <map>
#include <sstream>
#include <iomanip>

#include "error_context.h"
#include "error_codes.h"
#include "exception_hierarchy.h"

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

// 错误统计信息
struct ErrorStatistics {
    size_t total_errors = 0;
    size_t critical_errors = 0;
    size_t error_errors = 0;
    size_t warning_warnings = 0;
    size_t info_messages = 0;
    std::map<std::string, size_t> error_by_category;
    std::map<ErrorCode, size_t> error_by_code;
    std::chrono::system_clock::time_point first_error;
    std::chrono::system_clock::time_point last_error;
    
    void record_error(const LogEntry& entry) {
        total_errors++;
        
        switch (entry.level) {
            case LogLevel::Critical:
                critical_errors++;
                break;
            case LogLevel::Error:
                error_errors++;
                break;
            case LogLevel::Warning:
                warning_warnings++;
                break;
            case LogLevel::Info:
                info_messages++;
                break;
            case LogLevel::Debug:
                break;
        }
        
        error_by_category[entry.category]++;
        if (auto code_value = entry.context.get_int("error_code")) {
            error_by_code[static_cast<ErrorCode>(*code_value)]++;
        }
        
        if (first_error == std::chrono::system_clock::time_point{}) {
            first_error = entry.timestamp;
        }
        last_error = entry.timestamp;
    }
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
    auto get_color_code(LogLevel level) const -> std::string;
    auto reset_color() const -> std::string;
    auto format_timestamp(const std::chrono::system_clock::time_point& timestamp) const -> std::string;
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
    std::mutex m_mutex;
    
    auto check_rotation() -> void;
    auto rotate_file() -> void;
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
    std::mutex m_mutex;
    
    auto format_context(const ErrorContext& context) const -> std::string;
};

// 错误日志记录器
class ErrorLogger {
public:
    static auto get_instance() -> ErrorLogger&;
    
    // 禁止拷贝和赋值
    ErrorLogger(const ErrorLogger&) = delete;
    ErrorLogger& operator=(const ErrorLogger&) = delete;
    
    // 记录异常
    auto log_exception(const FastQException& ex, 
                      LogLevel level = LogLevel::Error,
                      const std::string& file_path = "",
                      int line_number = 0,
                      const std::string& function_name = "") -> void;
    
    // 记录消息
    auto log_message(LogLevel level, 
                   const std::string& category,
                   const std::string& message,
                   const std::string& file_path = "",
                   int line_number = 0,
                   const std::string& function_name = "") -> void;
    
    // 记录带上下文的消息
    auto log_message(LogLevel level,
                   const std::string& category,
                   const std::string& message,
                   const ErrorContext& context,
                   const std::string& file_path = "",
                   int line_number = 0,
                   const std::string& function_name = "") -> void;
    
    // 设置日志级别
    auto set_log_level(LogLevel level) -> void;
    auto get_log_level() const -> LogLevel;
    
    // 添加日志输出器
    auto add_appender(std::unique_ptr<LogAppender> appender) -> void;
    
    // 获取日志历史
    auto get_recent_entries(size_t count = 100) const -> std::vector<LogEntry>;
    auto get_entries_by_category(const std::string& category) const -> std::vector<LogEntry>;
    auto get_entries_by_level(LogLevel level) const -> std::vector<LogEntry>;
    
    // 清理日志
    auto clear_old_entries(std::chrono::hours age = std::chrono::hours(24)) -> void;
    
    // 错误统计
    auto get_error_statistics() const -> ErrorStatistics;
    
    // 设置上下文信息
    auto set_thread_context(const std::string& key, const std::string& value) -> void;
    auto remove_thread_context(const std::string& key) -> void;
    
    // 刷新所有输出器
    auto flush_all() -> void;

private:
    ErrorLogger();
    ~ErrorLogger() = default;
    
    struct Impl;
    std::unique_ptr<Impl> m_impl;
    
    auto create_log_entry(LogLevel level,
                         const std::string& category,
                         const std::string& message,
                         const ErrorContext& context,
                         const std::string& file_path,
                         int line_number,
                         const std::string& function_name) -> LogEntry;
    
    auto should_log(LogLevel level) const -> bool;
    auto append_to_all_appenders(const LogEntry& entry) -> void;
};

// 日志宏
#define FQ_LOG_DEBUG(category, message) \
    fq::error::ErrorLogger::get_instance().log_message( \
        fq::error::LogLevel::Debug, category, message, __FILE__, __LINE__, __FUNCTION__)

#define FQ_LOG_INFO(category, message) \
    fq::error::ErrorLogger::get_instance().log_message( \
        fq::error::LogLevel::Info, category, message, __FILE__, __LINE__, __FUNCTION__)

#define FQ_LOG_WARNING(category, message) \
    fq::error::ErrorLogger::get_instance().log_message( \
        fq::error::LogLevel::Warning, category, message, __FILE__, __LINE__, __FUNCTION__)

#define FQ_LOG_ERROR(category, message) \
    fq::error::ErrorLogger::get_instance().log_message( \
        fq::error::LogLevel::Error, category, message, __FILE__, __LINE__, __FUNCTION__)

#define FQ_LOG_CRITICAL(category, message) \
    fq::error::ErrorLogger::get_instance().log_message( \
        fq::error::LogLevel::Critical, category, message, __FILE__, __LINE__, __FUNCTION__)

#define FQ_LOG_EXCEPTION(exception) \
    fq::error::ErrorLogger::get_instance().log_exception( \
        exception, fq::error::LogLevel::Error, __FILE__, __LINE__, __FUNCTION__)

// 带上下文的日志宏
#define FQ_LOG_DEBUG_WITH_CONTEXT(category, message, context) \
    fq::error::ErrorLogger::get_instance().log_message( \
        fq::error::LogLevel::Debug, category, message, context, __FILE__, __LINE__, __FUNCTION__)

#define FQ_LOG_INFO_WITH_CONTEXT(category, message, context) \
    fq::error::ErrorLogger::get_instance().log_message( \
        fq::error::LogLevel::Info, category, message, context, __FILE__, __LINE__, __FUNCTION__)

#define FQ_LOG_WARNING_WITH_CONTEXT(category, message, context) \
    fq::error::ErrorLogger::get_instance().log_message( \
        fq::error::LogLevel::Warning, category, message, context, __FILE__, __LINE__, __FUNCTION__)

#define FQ_LOG_ERROR_WITH_CONTEXT(category, message, context) \
    fq::error::ErrorLogger::get_instance().log_message( \
        fq::error::LogLevel::Error, category, message, context, __FILE__, __LINE__, __FUNCTION__)

#define FQ_LOG_CRITICAL_WITH_CONTEXT(category, message, context) \
    fq::error::ErrorLogger::get_instance().log_message( \
        fq::error::LogLevel::Critical, category, message, context, __FILE__, __LINE__, __FUNCTION__)

// 便利函数
auto get_thread_id() -> std::string;
auto log_level_to_string(LogLevel level) -> std::string;
auto string_to_log_level(const std::string& str) -> LogLevel;

} // namespace fq::error