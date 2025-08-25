/**
 * @file error_context.cpp
 * @brief 错误上下文系统实现
 * @details 实现错误上下文信息的存储和检索功能
 * 
 * @author FastQTools Team
 * @date 2024
 * @version 1.0
 * 
 * @copyright Copyright (c) 2024 FastQTools
 * @license MIT License
 */

#include "error_context.h"
#include "error_logger.h"
#include <algorithm>
#include <thread>
#include <chrono>

namespace fq::error {

// ErrorContext 实现
auto ErrorContext::add(const std::string& key, const ContextValue& value) -> void {
    m_context[key] = value;
}

auto ErrorContext::add(const std::string& key, const std::string& value) -> void {
    m_context[key] = value;
}

auto ErrorContext::add(const std::string& key, int value) -> void {
    m_context[key] = value;
}

auto ErrorContext::add(const std::string& key, size_t value) -> void {
    m_context[key] = value;
}

auto ErrorContext::add(const std::string& key, double value) -> void {
    m_context[key] = value;
}

auto ErrorContext::add(const std::string& key, bool value) -> void {
    m_context[key] = value;
}

auto ErrorContext::add_time(const std::string& key, std::time_t value) -> void {
    m_context[key] = static_cast<size_t>(value);
}

auto ErrorContext::add_line(const std::string& key, uint32_t value) -> void {
    m_context[key] = static_cast<size_t>(value);
}

auto ErrorContext::get(const std::string& key) const -> std::optional<ContextValue> {
    auto it = m_context.find(key);
    if (it != m_context.end()) {
        return it->second;
    }
    return std::nullopt;
}

auto ErrorContext::get_string(const std::string& key) const -> std::optional<std::string> {
    auto it = m_context.find(key);
    if (it != m_context.end()) {
        if (auto* str_val = std::get_if<std::string>(&it->second)) {
            return *str_val;
        }
    }
    return std::nullopt;
}

auto ErrorContext::get_int(const std::string& key) const -> std::optional<int> {
    auto it = m_context.find(key);
    if (it != m_context.end()) {
        if (auto* int_val = std::get_if<int>(&it->second)) {
            return *int_val;
        }
    }
    return std::nullopt;
}

auto ErrorContext::get_size(const std::string& key) const -> std::optional<size_t> {
    auto it = m_context.find(key);
    if (it != m_context.end()) {
        if (auto* size_val = std::get_if<size_t>(&it->second)) {
            return *size_val;
        }
    }
    return std::nullopt;
}

auto ErrorContext::get_double(const std::string& key) const -> std::optional<double> {
    auto it = m_context.find(key);
    if (it != m_context.end()) {
        if (auto* double_val = std::get_if<double>(&it->second)) {
            return *double_val;
        }
    }
    return std::nullopt;
}

auto ErrorContext::get_bool(const std::string& key) const -> std::optional<bool> {
    auto it = m_context.find(key);
    if (it != m_context.end()) {
        if (auto* bool_val = std::get_if<bool>(&it->second)) {
            return *bool_val;
        }
    }
    return std::nullopt;
}

auto ErrorContext::contains(const std::string& key) const -> bool {
    return m_context.find(key) != m_context.end();
}

auto ErrorContext::get_keys() const -> std::vector<std::string> {
    std::vector<std::string> keys;
    keys.reserve(m_context.size());
    
    for (const auto& [key, value] : m_context) {
        keys.push_back(key);
    }
    
    return keys;
}

auto ErrorContext::clear() -> void {
    m_context.clear();
}

auto ErrorContext::format() const -> std::string {
    if (m_context.empty()) {
        return "";
    }
    
    std::ostringstream oss;
    oss << "{";
    
    bool first = true;
    for (const auto& [key, value] : m_context) {
        if (!first) {
            oss << ", ";
        }
        first = false;
        
        oss << "\"" << key << "\": " << format_value(value);
    }
    
    oss << "}";
    return oss.str();
}

auto ErrorContext::merge(const ErrorContext& other) -> void {
    for (const auto& [key, value] : other.m_context) {
        m_context[key] = value;
    }
}

auto ErrorContext::size() const -> size_t {
    return m_context.size();
}

auto ErrorContext::empty() const -> bool {
    return m_context.empty();
}

auto ErrorContext::format_value(const ContextValue& value) const -> std::string {
    std::ostringstream oss;
    
    std::visit([&oss](const auto& val) {
        using T = std::decay_t<decltype(val)>;
        
        if constexpr (std::is_same_v<T, std::string>) {
            oss << "\"" << val << "\"";
        } else if constexpr (std::is_same_v<T, bool>) {
            oss << (val ? "true" : "false");
        } else {
            oss << val;
        }
    }, value);
    
    return oss.str();
}

// ErrorContextBuilder 实现
auto ErrorContextBuilder::add(const std::string& key, const ErrorContext::ContextValue& value) -> ErrorContextBuilder& {
    m_context.add(key, value);
    return *this;
}

auto ErrorContextBuilder::add_file_info(const std::string& file_path, int line_number) -> ErrorContextBuilder& {
    m_context.add(ContextKeys::FILE_PATH, file_path);
    m_context.add(ContextKeys::LINE_NUMBER, line_number);
    return *this;
}

auto ErrorContextBuilder::add_system_info(int error_code, const std::string& error_message) -> ErrorContextBuilder& {
    m_context.add(ContextKeys::ERROR_CODE, error_code);
    m_context.add(ContextKeys::ERROR_MESSAGE, error_message);
    return *this;
}

auto ErrorContextBuilder::add_operation_info(const std::string& operation, const std::string& details) -> ErrorContextBuilder& {
    m_context.add(ContextKeys::OPERATION, operation);
    m_context.add("operation_details", details);
    return *this;
}

auto ErrorContextBuilder::add_performance_info(size_t processed_count, double elapsed_time) -> ErrorContextBuilder& {
    m_context.add(ContextKeys::PROCESSED_COUNT, processed_count);
    m_context.add(ContextKeys::ELAPSED_TIME, elapsed_time);
    
    if (elapsed_time > 0) {
        double throughput = processed_count / elapsed_time;
        m_context.add(ContextKeys::THROUGHPUT, throughput);
    }
    
    return *this;
}

auto ErrorContextBuilder::build() const -> ErrorContext {
    return m_context;
}

// ContextScopeGuard 实现
ContextScopeGuard::ContextScopeGuard(const std::string& key, const std::string& value)
    : m_key(key) {
    
    // 获取当前线程的上下文
    // 这里简化实现，实际应该使用线程局部存储
    auto& logger = ErrorLogger::get_instance();
    
    // 保存旧值
    auto old_value = logger.get_thread_context(m_key);
    m_had_value = old_value.has_value();
    if (m_had_value) {
        m_old_value = *old_value;
    }
    
    // 设置新值
    logger.set_thread_context(m_key, value);
}

ContextScopeGuard::~ContextScopeGuard() {
    // 恢复旧值
    auto& logger = ErrorLogger::get_instance();
    
    if (m_had_value) {
        logger.set_thread_context(m_key, m_old_value);
    } else {
        logger.remove_thread_context(m_key);
    }
}

} // namespace fq::error