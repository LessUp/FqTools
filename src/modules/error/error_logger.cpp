/**
 * @file error_logger.cpp
 * @brief 错误日志记录器实现
 * @details 实现统一的错误日志记录功能，支持多种输出格式和目标
 * 
 * @author FastQTools Team
 * @date 2024
 * @version 1.0
 * 
 * @copyright Copyright (c) 2024 FastQTools
 * @license MIT License
 */

#include "error_logger.h"
#include "exception_macros.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <thread>
#include <unordered_map>
#include <algorithm>

namespace fq::error {

namespace {
    // 线程局部存储的上下文信息
    thread_local std::unordered_map<std::string, std::string> g_thread_context;
    
    // 获取线程ID字符串
    auto detail_get_thread_id() -> std::string {
        std::ostringstream oss;
        oss << std::this_thread::get_id();
        return oss.str();
    }
    
    // 日志级别转换为字符串
    auto detail_log_level_to_string(LogLevel level) -> std::string {
        switch (level) {
            case LogLevel::Debug:    return "DEBUG";
            case LogLevel::Info:     return "INFO";
            case LogLevel::Warning:  return "WARNING";
            case LogLevel::Error:    return "ERROR";
            case LogLevel::Critical:  return "CRITICAL";
            default:                 return "UNKNOWN";
        }
    }
    
    // 字符串转换为日志级别
    auto detail_string_to_log_level(const std::string& str) -> LogLevel {
        if (str == "DEBUG")    return LogLevel::Debug;
        if (str == "INFO")     return LogLevel::Info;
        if (str == "WARNING")  return LogLevel::Warning;
        if (str == "ERROR")    return LogLevel::Error;
        if (str == "CRITICAL") return LogLevel::Critical;
        return LogLevel::Info; // 默认级别
    }
}

// 与头文件声明匹配的公开函数实现
auto get_thread_id() -> std::string { return detail_get_thread_id(); }
auto log_level_to_string(LogLevel level) -> std::string { return detail_log_level_to_string(level); }
auto string_to_log_level(const std::string& str) -> LogLevel { return detail_string_to_log_level(str); }

// ErrorLogger 私有实现
struct ErrorLogger::Impl {
    LogLevel log_level = LogLevel::Info;
    std::vector<std::unique_ptr<LogAppender>> appenders;
    std::vector<LogEntry> recent_entries;
    size_t max_recent_entries = 1000;
    std::mutex mutex;
    ErrorStatistics stats;
    
    // 清理过期条目
    auto cleanup_old_entries(std::chrono::hours age) -> void {
        auto now = std::chrono::system_clock::now();
        auto cutoff = now - age;
        
        std::lock_guard<std::mutex> lock(mutex);
        auto it = std::remove_if(recent_entries.begin(), recent_entries.end(),
                                [cutoff](const LogEntry& entry) {
                                    return entry.timestamp < cutoff;
                                });
        recent_entries.erase(it, recent_entries.end());
    }
    
    // 添加条目到历史记录
    auto add_to_history(const LogEntry& entry) -> void {
        std::lock_guard<std::mutex> lock(mutex);
        
        // 限制历史记录大小
        if (recent_entries.size() >= max_recent_entries) {
            recent_entries.erase(recent_entries.begin());
        }
        
        recent_entries.push_back(entry);
        stats.record_error(entry);
    }
};

// ErrorLogger 实现
ErrorLogger::ErrorLogger() 
    : m_impl(std::make_unique<Impl>()) {
    
    // 默认添加控制台输出器
    add_appender(std::make_unique<ConsoleAppender>());
}

auto ErrorLogger::get_instance() -> ErrorLogger& {
    static ErrorLogger instance;
    return instance;
}

auto ErrorLogger::log_exception(const FastQException& ex,
                               LogLevel level,
                               const std::string& file_path,
                               int line_number,
                               const std::string& function_name) -> void {
    
    if (!should_log(level)) {
        return;
    }
    
    auto context = ex.get_context();
    context.add("exception_type", std::string(typeid(ex).name()));
    context.add("error_code", static_cast<int>(ex.get_error_code()));
    context.add("severity", static_cast<int>(ex.get_severity()));
    
    auto entry = create_log_entry(level, "exception", ex.what(), context,
                                 file_path, line_number, function_name);
    
    m_impl->add_to_history(entry);
    append_to_all_appenders(entry);
}

auto ErrorLogger::log_message(LogLevel level,
                            const std::string& category,
                            const std::string& message,
                            const std::string& file_path,
                            int line_number,
                            const std::string& function_name) -> void {
    
    if (!should_log(level)) {
        return;
    }
    
    ErrorContext context;
    
    // 添加线程上下文
    for (const auto& [key, value] : g_thread_context) {
        context.add(key, value);
    }
    
    auto entry = create_log_entry(level, category, message, context,
                                 file_path, line_number, function_name);
    
    m_impl->add_to_history(entry);
    append_to_all_appenders(entry);
}

auto ErrorLogger::log_message(LogLevel level,
                            const std::string& category,
                            const std::string& message,
                            const ErrorContext& context,
                            const std::string& file_path,
                            int line_number,
                            const std::string& function_name) -> void {
    
    if (!should_log(level)) {
        return;
    }
    
    auto entry = create_log_entry(level, category, message, context,
                                 file_path, line_number, function_name);
    
    m_impl->add_to_history(entry);
    append_to_all_appenders(entry);
}

auto ErrorLogger::set_log_level(LogLevel level) -> void {
    m_impl->log_level = level;
}

auto ErrorLogger::get_log_level() const -> LogLevel {
    return m_impl->log_level;
}

auto ErrorLogger::add_appender(std::unique_ptr<LogAppender> appender) -> void {
    m_impl->appenders.push_back(std::move(appender));
}

auto ErrorLogger::get_recent_entries(size_t count) const -> std::vector<LogEntry> {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    
    count = std::min(count, m_impl->recent_entries.size());
    auto start_it = m_impl->recent_entries.end() - count;
    
    return std::vector<LogEntry>(start_it, m_impl->recent_entries.end());
}

auto ErrorLogger::get_entries_by_category(const std::string& category) const -> std::vector<LogEntry> {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    
    std::vector<LogEntry> result;
    std::copy_if(m_impl->recent_entries.begin(), m_impl->recent_entries.end(),
                std::back_inserter(result),
                [&category](const LogEntry& entry) {
                    return entry.category == category;
                });
    
    return result;
}

auto ErrorLogger::get_entries_by_level(LogLevel level) const -> std::vector<LogEntry> {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    
    std::vector<LogEntry> result;
    std::copy_if(m_impl->recent_entries.begin(), m_impl->recent_entries.end(),
                std::back_inserter(result),
                [level](const LogEntry& entry) {
                    return entry.level == level;
                });
    
    return result;
}

auto ErrorLogger::clear_old_entries(std::chrono::hours age) -> void {
    m_impl->cleanup_old_entries(age);
}

auto ErrorLogger::get_error_statistics() const -> ErrorStatistics {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    return m_impl->stats;
}

auto ErrorLogger::set_thread_context(const std::string& key, const std::string& value) -> void {
    g_thread_context[key] = value;
}

auto ErrorLogger::remove_thread_context(const std::string& key) -> void {
    g_thread_context.erase(key);
}

auto ErrorLogger::get_thread_context(const std::string& key) const -> std::optional<std::string> {
    auto it = g_thread_context.find(key);
    if (it != g_thread_context.end()) {
        return it->second;
    }
    return std::nullopt;
}

auto ErrorLogger::flush_all() -> void {
    for (auto& appender : m_impl->appenders) {
        appender->flush();
    }
}

auto ErrorLogger::create_log_entry(LogLevel level,
                                  const std::string& category,
                                  const std::string& message,
                                  const ErrorContext& context,
                                  const std::string& file_path,
                                  int line_number,
                                  const std::string& function_name) -> LogEntry {
    
    LogEntry entry;
    entry.timestamp = std::chrono::system_clock::now();
    entry.level = level;
    entry.category = category;
    entry.message = message;
    entry.context = context;
    entry.thread_id = get_thread_id();
    entry.file_path = file_path;
    entry.line_number = line_number;
    entry.function_name = function_name;
    
    return entry;
}

auto ErrorLogger::should_log(LogLevel level) const -> bool {
    return level >= m_impl->log_level;
}

auto ErrorLogger::append_to_all_appenders(const LogEntry& entry) -> void {
    for (auto& appender : m_impl->appenders) {
        appender->append(entry);
    }
}

// ConsoleAppender 实现
ConsoleAppender::ConsoleAppender(bool colored) : m_colored(colored) {}

auto ConsoleAppender::get_color_code(LogLevel level) const -> std::string {
    if (!m_colored) return "";
    
    switch (level) {
        case LogLevel::Debug:    return "\033[36m"; // Cyan
        case LogLevel::Info:     return "\033[32m"; // Green
        case LogLevel::Warning:  return "\033[33m"; // Yellow
        case LogLevel::Error:    return "\033[31m"; // Red
        case LogLevel::Critical:  return "\033[35m"; // Magenta
        default:                 return "";
    }
}

auto ConsoleAppender::reset_color() const -> std::string {
    return m_colored ? "\033[0m" : "";
}

auto ConsoleAppender::format_timestamp(const std::chrono::system_clock::time_point& timestamp) const -> std::string {
    auto time_t = std::chrono::system_clock::to_time_t(timestamp);
    std::tm tm = *std::localtime(&time_t);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    
    // 添加毫秒
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        timestamp.time_since_epoch()) % 1000;
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return oss.str();
}

auto ConsoleAppender::append(const LogEntry& entry) -> void {
    std::ostringstream oss;
    
    // 时间戳
    oss << "[" << format_timestamp(entry.timestamp) << "] ";
    
    // 日志级别
    oss << "[" << get_color_code(entry.level) 
        << log_level_to_string(entry.level) 
        << reset_color() << "] ";
    
    // 分类
    oss << "[" << entry.category << "] ";
    
    // 线程ID
    oss << "[" << entry.thread_id << "] ";
    
    // 消息
    oss << entry.message;
    
    // 上下文信息
    if (!entry.context.get_keys().empty()) {
        oss << " - Context: " << entry.context.format();
    }
    
    // 位置信息
    if (!entry.file_path.empty()) {
        oss << " - " << entry.file_path;
        if (entry.line_number > 0) {
            oss << ":" << entry.line_number;
        }
        if (!entry.function_name.empty()) {
            oss << " (" << entry.function_name << ")";
        }
    }
    
    oss << std::endl;
    
    // 根据级别选择输出流
    if (entry.level >= LogLevel::Error) {
        std::cerr << oss.str();
    } else {
        std::cout << oss.str();
    }
}

auto ConsoleAppender::flush() -> void {
    std::cout.flush();
    std::cerr.flush();
}

// FileAppender 实现
FileAppender::FileAppender(const std::string& file_path, bool rotate, size_t max_size)
    : m_file_path(file_path), m_rotate(rotate), m_max_size(max_size) {
    
    // 确保目录存在
    auto dir = std::filesystem::path(file_path).parent_path();
    if (!dir.empty()) {
        std::filesystem::create_directories(dir);
    }
    
    // 打开文件
    m_file_stream.open(file_path, std::ios::app);
    if (!m_file_stream.is_open()) {
        throw std::runtime_error("Failed to open log file: " + file_path);
    }
}

auto FileAppender::check_rotation() -> void {
    if (!m_rotate) return;
    
    m_file_stream.flush();
    auto current_size = m_file_stream.tellp();
    
    if (current_size >= static_cast<std::streampos>(m_max_size)) {
        rotate_file();
    }
}

auto FileAppender::rotate_file() -> void {
    m_file_stream.close();
    
    // 生成备份文件名
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time_t);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d_%H%M%S");
    std::string backup_path = m_file_path + "." + oss.str();
    
    // 重命名当前文件
    std::filesystem::rename(m_file_path, backup_path);
    
    // 重新打开文件
    m_file_stream.open(m_file_path, std::ios::app);
}

auto FileAppender::append(const LogEntry& entry) -> void {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    check_rotation();
    
    // 格式化日志条目
    auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp);
    std::tm tm = *std::localtime(&time_t);
    
    m_file_stream << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "] "
                 << "[" << log_level_to_string(entry.level) << "] "
                 << "[" << entry.category << "] "
                 << "[" << entry.thread_id << "] "
                 << entry.message;
    
    if (!entry.context.get_keys().empty()) {
        m_file_stream << " - Context: " << entry.context.format();
    }
    
    m_file_stream << std::endl;
}

auto FileAppender::flush() -> void {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_file_stream.flush();
}

// JsonAppender 实现
JsonAppender::JsonAppender(const std::string& file_path) : m_file_path(file_path) {
    // 确保目录存在
    auto dir = std::filesystem::path(file_path).parent_path();
    if (!dir.empty()) {
        std::filesystem::create_directories(dir);
    }
    
    // 打开文件
    m_file_stream.open(file_path, std::ios::app);
    if (!m_file_stream.is_open()) {
        throw std::runtime_error("Failed to open JSON log file: " + file_path);
    }
}

auto JsonAppender::format_context(const ErrorContext& context) const -> std::string {
    std::ostringstream oss;
    oss << "{";
    
    bool first = true;
    for (const auto& key : context.get_keys()) {
        if (!first) oss << ",";
        first = false;
        
        oss << "\"" << key << "\":";
        
        if (auto value = context.get_string(key)) {
            oss << "\"" << *value << "\"";
        } else if (auto value = context.get_int(key)) {
            oss << *value;
        } else if (auto value = context.get_size(key)) {
            oss << *value;
        } else if (auto value = context.get_double(key)) {
            oss << *value;
        } else if (auto value = context.get_bool(key)) {
            oss << (*value ? "true" : "false");
        }
    }
    
    oss << "}";
    return oss.str();
}

auto JsonAppender::append(const LogEntry& entry) -> void {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp);
    std::tm tm = *std::localtime(&time_t);
    
    m_file_stream << "{"
                 << "\"timestamp\": \"" << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S") << "\","
                 << "\"level\": \"" << log_level_to_string(entry.level) << "\","
                 << "\"category\": \"" << entry.category << "\","
                 << "\"thread_id\": \"" << entry.thread_id << "\","
                 << "\"message\": \"" << entry.message << "\","
                 << "\"context\": " << format_context(entry.context) << ","
                 << "\"file\": \"" << entry.file_path << "\","
                 << "\"line\": " << entry.line_number << ","
                 << "\"function\": \"" << entry.function_name << "\""
                 << "}" << std::endl;
}

auto JsonAppender::flush() -> void {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_file_stream.flush();
}

} // namespace fq::error