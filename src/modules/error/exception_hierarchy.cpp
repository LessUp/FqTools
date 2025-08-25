/**
 * @file exception_hierarchy.cpp
 * @brief 异常层次结构实现
 * @details 实现完整的异常类层次结构，包括基础异常类和特定领域的异常类
 * 
 * @author FastQTools Team
 * @date 2024
 * @version 1.0
 * 
 * @copyright Copyright (c) 2024 FastQTools
 * @license MIT License
 */

#include "exception_hierarchy.h"
#include "error_codes.h"
#include "error_context.h"
#include <sstream>
#include <algorithm>
#include <system_error>
#include <cstring>
#include <iomanip>
#include <ctime>

namespace fq::error {

// FastQException 实现
FastQException::FastQException(const std::string& message, 
                               ErrorCode code,
                               ErrorSeverity severity)
    : std::runtime_error(message)
    , m_code(code)
    , m_severity(severity)
    , m_timestamp(std::chrono::system_clock::now())
    , m_location(std::source_location::current())
{
    m_stack_trace = generate_stack_trace();
    m_suggestions = generate_default_suggestions();
    
    // 添加基本上下文信息
    m_context.add("error_code", static_cast<int>(code));
    m_context.add("severity", static_cast<int>(severity));
    m_context.add_time("timestamp", std::chrono::system_clock::to_time_t(m_timestamp));
    m_context.add("file", m_location.file_name());
    m_context.add_line("line", m_location.line());
    m_context.add("function", m_location.function_name());
}

FastQException::FastQException(const std::string& message, 
                               ErrorCode code,
                               ErrorSeverity severity,
                               const std::source_location& location)
    : std::runtime_error(message)
    , m_code(code)
    , m_severity(severity)
    , m_timestamp(std::chrono::system_clock::now())
    , m_location(location)
{
    m_stack_trace = generate_stack_trace();
    m_suggestions = generate_default_suggestions();
    
    // 添加基本上下文信息
    m_context.add("error_code", static_cast<int>(code));
    m_context.add("severity", static_cast<int>(severity));
    m_context.add_time("timestamp", std::chrono::system_clock::to_time_t(m_timestamp));
    m_context.add("file", m_location.file_name());
    m_context.add_line("line", m_location.line());
    m_context.add("function", m_location.function_name());
}

auto FastQException::get_error_code() const noexcept -> ErrorCode {
    return m_code;
}

auto FastQException::get_severity() const noexcept -> ErrorSeverity {
    return m_severity;
}

auto FastQException::get_context() const noexcept -> const ErrorContext& {
    return m_context;
}

auto FastQException::get_timestamp() const noexcept -> std::chrono::system_clock::time_point {
    return m_timestamp;
}

auto FastQException::get_stack_trace() const noexcept -> const std::string& {
    return m_stack_trace;
}

auto FastQException::get_suggestions() const noexcept -> const std::vector<std::string>& {
    return m_suggestions;
}

auto FastQException::add_context(const std::string& key, const std::string& value) -> void {
    m_context.add(key, value);
}

auto FastQException::add_context(const std::string& key, int value) -> void {
    m_context.add(key, value);
}

auto FastQException::add_context(const std::string& key, size_t value) -> void {
    m_context.add(key, value);
}

auto FastQException::add_context(const std::string& key, double value) -> void {
    m_context.add(key, value);
}

auto FastQException::add_context(const std::string& key, bool value) -> void {
    m_context.add(key, value);
}

auto FastQException::add_suggestion(const std::string& suggestion) -> void {
    m_suggestions.push_back(suggestion);
}

auto FastQException::get_user_message() const -> std::string {
    std::ostringstream oss;
    
    // 基础错误信息
    oss << "错误: " << what();
    
    // 根据严重程度添加不同的前缀
    switch (m_severity) {
        case ErrorSeverity::Info:
            oss << " (信息)";
            break;
        case ErrorSeverity::Warning:
            oss << " (警告)";
            break;
        case ErrorSeverity::Error:
            oss << " (错误)";
            break;
        case ErrorSeverity::Critical:
            oss << " (严重错误)";
            break;
        case ErrorSeverity::Fatal:
            oss << " (致命错误)";
            break;
    }
    
    // 添加位置信息
    if (m_location.line() > 0) {
        oss << "\n位置: " << m_location.file_name() << ":" << m_location.line();
        if (m_location.function_name()) {
            oss << " (" << m_location.function_name() << ")";
        }
    }
    
    return oss.str();
}

auto FastQException::get_log_message() const -> std::string {
    std::ostringstream oss;
    
    // 时间戳
    auto time_t = std::chrono::system_clock::to_time_t(m_timestamp);
    oss << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "]";
    
    // 错误级别和代码
    oss << "[" << static_cast<int>(m_code) << ":" << static_cast<int>(m_severity) << "]";
    
    // 错误消息
    oss << " " << what();
    
    // 上下文信息
    if (!m_context.get_keys().empty()) {
        oss << " Context: " << m_context.format();
    }
    
    return oss.str();
}

auto FastQException::is_recoverable() const noexcept -> bool {
    switch (m_severity) {
        case ErrorSeverity::Info:
        case ErrorSeverity::Warning:
        case ErrorSeverity::Error:
            return true;
        case ErrorSeverity::Critical:
        case ErrorSeverity::Fatal:
            return false;
        default:
            return false;
    }
}

auto FastQException::get_recovery_strategy() const -> std::string {
    switch (m_code) {
        case ErrorCode::FileNotFound:
            return "检查文件路径是否正确，或使用默认文件";
        case ErrorCode::PermissionDenied:
            return "检查文件权限，或使用管理员权限运行";
        case ErrorCode::DataCorrupted:
            return "跳过损坏的数据，或尝试数据修复";
        case ErrorCode::MemoryAllocationFailed:
            return "减少批处理大小，或增加系统内存";
        case ErrorCode::NetworkError:
            return "检查网络连接，或重试操作";
        case ErrorCode::InvalidConfig:
            return "检查配置文件格式，或使用默认配置";
        default:
            return "请联系技术支持";
    }
}

auto FastQException::clone() const -> std::unique_ptr<FastQException> {
    return std::make_unique<FastQException>(*this);
}

auto FastQException::generate_stack_trace() const -> std::string {
    // 简化的堆栈跟踪实现
    // 在实际实现中，可以使用更高级的堆栈跟踪库
    std::ostringstream oss;
    oss << "Stack trace:\n";
    oss << "  at " << m_location.function_name() << "\n";
    oss << "  in " << m_location.file_name() << ":" << m_location.line() << "\n";
    return oss.str();
}

auto FastQException::generate_default_suggestions() const -> std::vector<std::string> {
    std::vector<std::string> suggestions;
    
    // 根据错误代码生成建议
    switch (m_code) {
        case ErrorCode::FileNotFound:
            suggestions.push_back("检查文件路径是否正确");
            suggestions.push_back("确认文件存在于指定位置");
            suggestions.push_back("检查文件权限");
            break;
            
        case ErrorCode::PermissionDenied:
            suggestions.push_back("检查文件权限设置");
            suggestions.push_back("使用适当的用户权限运行程序");
            suggestions.push_back("联系系统管理员");
            break;
            
        case ErrorCode::DataCorrupted:
            suggestions.push_back("检查数据完整性");
            suggestions.push_back("尝试使用备份数据");
            suggestions.push_back("考虑跳过损坏的数据");
            break;
            
        case ErrorCode::MemoryAllocationFailed:
            suggestions.push_back("减少批处理大小");
            suggestions.push_back("增加系统内存");
            suggestions.push_back("关闭其他内存密集型应用");
            break;
            
        case ErrorCode::NetworkError:
            suggestions.push_back("检查网络连接");
            suggestions.push_back("重试操作");
            suggestions.push_back("检查防火墙设置");
            break;
            
        default:
            suggestions.push_back("请查看错误日志获取更多信息");
            suggestions.push_back("联系技术支持");
            break;
    }
    
    return suggestions;
}

// IoException 实现
IoException::IoException(const std::string& file_path, 
                         int system_error_code,
                         const std::string& operation)
    : FastQException("IO Error: " + operation + " failed for file: " + file_path,
                     ErrorCode::FileNotFound, ErrorSeverity::Error)
    , m_file_path(file_path)
    , m_system_error_code(system_error_code)
    , m_operation(operation)
{
    add_context("file_path", file_path);
    add_context("system_error_code", system_error_code);
    add_context("operation", operation);
    add_context("system_error_message", get_system_error_message());
}

IoException::IoException(const std::string& message,
                         const std::string& file_path,
                         int system_error_code,
                         const std::string& operation)
    : FastQException(message, ErrorCode::FileNotFound, ErrorSeverity::Error)
    , m_file_path(file_path)
    , m_system_error_code(system_error_code)
    , m_operation(operation)
{
    add_context("file_path", file_path);
    add_context("system_error_code", system_error_code);
    add_context("operation", operation);
    add_context("system_error_message", get_system_error_message());
}

auto IoException::get_file_path() const noexcept -> const std::string& {
    return m_file_path;
}

auto IoException::get_system_error_code() const noexcept -> int {
    return m_system_error_code;
}

auto IoException::get_operation() const noexcept -> const std::string& {
    return m_operation;
}

auto IoException::get_system_error_message() const -> std::string {
    std::error_code ec(m_system_error_code, std::system_category());
    return ec.message();
}

auto IoException::clone() const -> std::unique_ptr<FastQException> {
    return std::make_unique<IoException>(*this);
}

// ConfigurationException 实现
ConfigurationException::ConfigurationException(const std::string& config_key,
                                                   const std::string& config_value,
                                                   const std::string& reason)
    : FastQException("Configuration Error: " + reason + " for key: " + config_key,
                     ErrorCode::InvalidConfig, ErrorSeverity::Error)
    , m_config_key(config_key)
    , m_config_value(config_value)
    , m_reason(reason)
{
    add_context("config_key", config_key);
    add_context("config_value", config_value);
    add_context("reason", reason);
}

auto ConfigurationException::get_config_key() const noexcept -> const std::string& {
    return m_config_key;
}

auto ConfigurationException::get_config_value() const noexcept -> const std::string& {
    return m_config_value;
}

auto ConfigurationException::get_reason() const noexcept -> const std::string& {
    return m_reason;
}

auto ConfigurationException::clone() const -> std::unique_ptr<FastQException> {
    return std::make_unique<ConfigurationException>(*this);
}

// ValidationException 实现
ValidationException::ValidationException(const std::string& field_name,
                                        const std::string& field_value,
                                        const std::string& validation_rule)
    : FastQException("Validation Error: " + field_name + " failed validation",
                     ErrorCode::ValidationFailed, ErrorSeverity::Error)
    , m_field_name(field_name)
    , m_field_value(field_value)
    , m_validation_rule(validation_rule)
{
    add_context("field_name", field_name);
    add_context("field_value", field_value);
    add_context("validation_rule", validation_rule);
}

ValidationException::ValidationException(const std::string& field_name,
                                        const std::string& field_value,
                                        const std::string& expected_value,
                                        const std::string& validation_rule)
    : FastQException("Validation Error: " + field_name + " failed validation",
                     ErrorCode::ValidationFailed, ErrorSeverity::Error)
    , m_field_name(field_name)
    , m_field_value(field_value)
    , m_validation_rule(validation_rule)
    , m_expected_value(expected_value)
{
    add_context("field_name", field_name);
    add_context("field_value", field_value);
    add_context("expected_value", expected_value);
    add_context("validation_rule", validation_rule);
}

auto ValidationException::get_field_name() const noexcept -> const std::string& {
    return m_field_name;
}

auto ValidationException::get_field_value() const noexcept -> const std::string& {
    return m_field_value;
}

auto ValidationException::get_validation_rule() const noexcept -> const std::string& {
    return m_validation_rule;
}

auto ValidationException::get_expected_value() const noexcept -> const std::string& {
    return m_expected_value;
}

auto ValidationException::clone() const -> std::unique_ptr<FastQException> {
    return std::make_unique<ValidationException>(*this);
}

// ProcessingException 实现
ProcessingException::ProcessingException(const std::string& operation,
                                        size_t processed_count,
                                        size_t failed_count,
                                        const std::string& details)
    : FastQException("Processing Error: " + operation + " failed",
                     ErrorCode::ProcessingFailed, ErrorSeverity::Error)
    , m_operation(operation)
    , m_processed_count(processed_count)
    , m_failed_count(failed_count)
    , m_details(details)
{
    add_context("operation", operation);
    add_context("processed_count", processed_count);
    add_context("failed_count", failed_count);
    add_context("success_rate", get_success_rate());
    add_context("details", details);
}

auto ProcessingException::get_operation() const noexcept -> const std::string& {
    return m_operation;
}

auto ProcessingException::get_processed_count() const noexcept -> size_t {
    return m_processed_count;
}

auto ProcessingException::get_failed_count() const noexcept -> size_t {
    return m_failed_count;
}

auto ProcessingException::get_success_rate() const noexcept -> double {
    size_t total = m_processed_count + m_failed_count;
    return total > 0 ? static_cast<double>(m_processed_count) / total : 0.0;
}

auto ProcessingException::get_details() const noexcept -> const std::string& {
    return m_details;
}

auto ProcessingException::clone() const -> std::unique_ptr<FastQException> {
    return std::make_unique<ProcessingException>(*this);
}

// MemoryException 实现
MemoryException::MemoryException(size_t requested_size,
                               size_t available_size,
                               const std::string& allocation_type)
    : FastQException("Memory Error: Failed to allocate " + std::to_string(requested_size) + " bytes",
                     ErrorCode::MemoryAllocationFailed, ErrorSeverity::Critical)
    , m_requested_size(requested_size)
    , m_available_size(available_size)
    , m_allocation_type(allocation_type)
    , m_is_allocation_error(true)
{
    add_context("requested_size", requested_size);
    add_context("available_size", available_size);
    add_context("allocation_type", allocation_type);
    add_context("is_allocation_error", true);
}

MemoryException::MemoryException(uintptr_t address,
                                 const std::string& access_type,
                                 const std::string& reason)
    : FastQException("Memory Error: " + access_type + " access violation at address " + std::to_string(address),
                     ErrorCode::MemoryAccessViolation, ErrorSeverity::Critical)
    , m_access_address(address)
    , m_access_type(access_type)
    , m_is_allocation_error(false)
{
    add_context("access_address", static_cast<size_t>(address));
    add_context("access_type", access_type);
    add_context("reason", reason);
    add_context("is_allocation_error", false);
}

auto MemoryException::get_requested_size() const noexcept -> size_t {
    return m_requested_size;
}

auto MemoryException::get_available_size() const noexcept -> size_t {
    return m_available_size;
}

auto MemoryException::get_allocation_type() const noexcept -> const std::string& {
    return m_allocation_type;
}

auto MemoryException::get_access_address() const noexcept -> uintptr_t {
    return m_access_address;
}

auto MemoryException::get_access_type() const noexcept -> const std::string& {
    return m_access_type;
}

auto MemoryException::is_allocation_error() const noexcept -> bool {
    return m_is_allocation_error;
}

auto MemoryException::is_access_error() const noexcept -> bool {
    return !m_is_allocation_error;
}

auto MemoryException::clone() const -> std::unique_ptr<FastQException> {
    return std::make_unique<MemoryException>(*this);
}

// ConcurrencyException 实现
ConcurrencyException::ConcurrencyException(const std::string& operation,
                                          const std::string& resource_name,
                                          int thread_count)
    : FastQException("Concurrency Error: " + operation + " failed on resource: " + resource_name,
                     ErrorCode::ResourceBusy, ErrorSeverity::Error)
    , m_operation(operation)
    , m_resource_name(resource_name)
    , m_thread_count(thread_count)
    , m_is_deadlock(false)
{
    add_context("operation", operation);
    add_context("resource_name", resource_name);
    add_context("thread_count", thread_count);
    add_context("is_deadlock", false);
}

ConcurrencyException::ConcurrencyException(const std::vector<std::string>& lock_sequence,
                                          const std::vector<std::string>& thread_ids)
    : FastQException("Concurrency Error: Deadlock detected",
                     ErrorCode::DeadlockDetected, ErrorSeverity::Critical)
    , m_lock_sequence(lock_sequence)
    , m_thread_ids(thread_ids)
    , m_is_deadlock(true)
{
    add_context("is_deadlock", true);
    add_context("lock_count", lock_sequence.size());
    add_context("thread_count", thread_ids.size());
}

auto ConcurrencyException::get_operation() const noexcept -> const std::string& {
    return m_operation;
}

auto ConcurrencyException::get_resource_name() const noexcept -> const std::string& {
    return m_resource_name;
}

auto ConcurrencyException::get_thread_count() const noexcept -> int {
    return m_thread_count;
}

auto ConcurrencyException::get_lock_sequence() const noexcept -> const std::vector<std::string>& {
    return m_lock_sequence;
}

auto ConcurrencyException::get_thread_ids() const noexcept -> const std::vector<std::string>& {
    return m_thread_ids;
}

auto ConcurrencyException::is_deadlock() const noexcept -> bool {
    return m_is_deadlock;
}

auto ConcurrencyException::clone() const -> std::unique_ptr<FastQException> {
    return std::make_unique<ConcurrencyException>(*this);
}

// NetworkException 实现
NetworkException::NetworkException(const std::string& host,
                                  int port,
                                  const std::string& operation,
                                  int error_code)
    : FastQException("Network Error: " + operation + " failed for " + host + ":" + std::to_string(port),
                     ErrorCode::NetworkError, ErrorSeverity::Error)
    , m_host(host)
    , m_port(port)
    , m_operation(operation)
    , m_error_code(error_code)
{
    add_context("host", host);
    add_context("port", port);
    add_context("operation", operation);
    add_context("network_error_code", error_code);
}

auto NetworkException::get_host() const noexcept -> const std::string& {
    return m_host;
}

auto NetworkException::get_port() const noexcept -> int {
    return m_port;
}

auto NetworkException::get_operation() const noexcept -> const std::string& {
    return m_operation;
}

auto NetworkException::get_network_error_code() const noexcept -> int {
    return m_error_code;
}

auto NetworkException::clone() const -> std::unique_ptr<FastQException> {
    return std::make_unique<NetworkException>(*this);
}

// 异常工厂函数实现
auto create_io_exception(const std::string& file_path, 
                        int system_error_code,
                        const std::string& operation) -> std::unique_ptr<FastQException> {
    return std::make_unique<IoException>(file_path, system_error_code, operation);
}

auto create_config_exception(const std::string& config_key,
                             const std::string& config_value,
                             const std::string& reason) -> std::unique_ptr<FastQException> {
    return std::make_unique<ConfigurationException>(config_key, config_value, reason);
}

auto create_validation_exception(const std::string& field_name,
                                const std::string& field_value,
                                const std::string& validation_rule) -> std::unique_ptr<FastQException> {
    return std::make_unique<ValidationException>(field_name, field_value, validation_rule);
}

auto create_processing_exception(const std::string& operation,
                                 size_t processed_count,
                                 size_t failed_count,
                                 const std::string& details) -> std::unique_ptr<FastQException> {
    return std::make_unique<ProcessingException>(operation, processed_count, failed_count, details);
}

auto create_memory_exception(size_t requested_size,
                            size_t available_size,
                            const std::string& allocation_type) -> std::unique_ptr<FastQException> {
    return std::make_unique<MemoryException>(requested_size, available_size, allocation_type);
}

auto create_concurrency_exception(const std::string& operation,
                                 const std::string& resource_name,
                                 int thread_count) -> std::unique_ptr<FastQException> {
    return std::make_unique<ConcurrencyException>(operation, resource_name, thread_count);
}

auto create_network_exception(const std::string& host,
                             int port,
                             const std::string& operation,
                             int error_code) -> std::unique_ptr<FastQException> {
    return std::make_unique<NetworkException>(host, port, operation, error_code);
}

auto convert_std_exception(const std::exception& ex) -> std::unique_ptr<FastQException> {
    return std::make_unique<FastQException>(ex.what(), ErrorCode::InternalError, ErrorSeverity::Error);
}

} // namespace fq::error