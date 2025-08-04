/**
 * @file exception_hierarchy.h
 * @brief 异常层次结构定义
 * @details 定义完整的异常类层次结构，包括基础异常类和特定领域的异常类
 * 
 * @author FastQTools Team
 * @date 2024
 * @version 1.0
 * 
 * @copyright Copyright (c) 2024 FastQTools
 * @license MIT License
 */

#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include <sstream>
#include <source_location>

#include "error_codes.h"
#include "error_context.h"

namespace fq::error {

// 前向声明
class FastQException;

/**
 * @class FastQException
 * @brief FastQTools 基础异常类
 * @details 所有 FastQTools 异常的基类，提供丰富的错误信息和上下文
 */
class FastQException : public std::runtime_error {
public:
    /**
     * @brief 构造函数
     * @param message 错误消息
     * @param code 错误代码
     * @param severity 错误严重程度
     */
    FastQException(const std::string& message, 
                  ErrorCode code = ErrorCode::Unknown,
                  ErrorSeverity severity = ErrorSeverity::Error);
    
    /**
     * @brief 带位置的构造函数
     * @param message 错误消息
     * @param code 错误代码
     * @param severity 错误严重程度
     * @param location 源代码位置
     */
    FastQException(const std::string& message, 
                  ErrorCode code,
                  ErrorSeverity severity,
                  const std::source_location& location);
    
    virtual ~FastQException() = default;
    
    // 禁止拷贝和赋值
    FastQException(const FastQException&) = default;
    FastQException& operator=(const FastQException&) = default;
    
    // 获取错误代码
    [[nodiscard]] auto get_error_code() const noexcept -> ErrorCode;
    
    // 获取错误严重程度
    [[nodiscard]] auto get_severity() const noexcept -> ErrorSeverity;
    
    // 获取错误上下文
    [[nodiscard]] auto get_context() const noexcept -> const ErrorContext&;
    
    // 获取错误时间戳
    [[nodiscard]] auto get_timestamp() const noexcept -> std::chrono::system_clock::time_point;
    
    // 获取堆栈跟踪
    [[nodiscard]] auto get_stack_trace() const noexcept -> const std::string&;
    
    // 获取错误建议
    [[nodiscard]] auto get_suggestions() const noexcept -> const std::vector<std::string>&;
    
    // 添加上下文信息
    auto add_context(const std::string& key, const std::string& value) -> void;
    auto add_context(const std::string& key, int value) -> void;
    auto add_context(const std::string& key, size_t value) -> void;
    auto add_context(const std::string& key, double value) -> void;
    auto add_context(const std::string& key, bool value) -> void;
    
    // 添加修复建议
    auto add_suggestion(const std::string& suggestion) -> void;
    
    // 转换为用户友好的消息
    [[nodiscard]] auto get_user_message() const -> std::string;
    
    // 转换为日志消息
    [[nodiscard]] auto get_log_message() const -> std::string;
    
    // 检查是否可以恢复
    [[nodiscard]] virtual auto is_recoverable() const noexcept -> bool;
    
    // 获取恢复策略
    [[nodiscard]] virtual auto get_recovery_strategy() const -> std::string;
    
    // 克隆异常
    [[nodiscard]] virtual auto clone() const -> std::unique_ptr<FastQException>;

protected:
    ErrorCode m_code;
    ErrorSeverity m_severity;
    ErrorContext m_context;
    std::chrono::system_clock::time_point m_timestamp;
    std::string m_stack_trace;
    std::vector<std::string> m_suggestions;
    std::source_location m_location;
    
    // 生成堆栈跟踪
    auto generate_stack_trace() const -> std::string;
    
    // 生成默认建议
    auto generate_default_suggestions() const -> std::vector<std::string>;
};

/**
 * @class IoException
 * @brief IO 相关异常
 * @details 处理文件操作、网络操作等 IO 相关的错误
 */
class IoException : public FastQException {
public:
    /**
     * @brief 构造函数
     * @param file_path 文件路径
     * @param system_error_code 系统错误代码
     * @param operation 操作类型
     */
    IoException(const std::string& file_path, 
               int system_error_code,
               const std::string& operation = "file operation");
    
    /**
     * @brief 带消息的构造函数
     * @param message 错误消息
     * @param file_path 文件路径
     * @param system_error_code 系统错误代码
     * @param operation 操作类型
     */
    IoException(const std::string& message,
               const std::string& file_path,
               int system_error_code,
               const std::string& operation = "file operation");
    
    // 获取文件路径
    [[nodiscard]] auto get_file_path() const noexcept -> const std::string&;
    
    // 获取系统错误代码
    [[nodiscard]] auto get_system_error_code() const noexcept -> int;
    
    // 获取操作类型
    [[nodiscard]] auto get_operation() const noexcept -> const std::string&;
    
    // 获取系统错误消息
    [[nodiscard]] auto get_system_error_message() const -> std::string;
    
    // 克隆异常
    [[nodiscard]] auto clone() const -> std::unique_ptr<FastQException> override;

private:
    std::string m_file_path;
    int m_system_error_code;
    std::string m_operation;
};

/**
 * @class ConfigurationException
 * @brief 配置相关异常
 * @details 处理配置文件、参数验证等配置相关的错误
 */
class ConfigurationException : public FastQException {
public:
    /**
     * @brief 构造函数
     * @param config_key 配置键
     * @param config_value 配置值
     * @param reason 错误原因
     */
    ConfigurationException(const std::string& config_key,
                           const std::string& config_value,
                           const std::string& reason);
    
    // 获取配置键
    [[nodiscard]] auto get_config_key() const noexcept -> const std::string&;
    
    // 获取配置值
    [[nodiscard]] auto get_config_value() const noexcept -> const std::string&;
    
    // 获取错误原因
    [[nodiscard]] auto get_reason() const noexcept -> const std::string&;
    
    // 克隆异常
    [[nodiscard]] auto clone() const -> std::unique_ptr<FastQException> override;

private:
    std::string m_config_key;
    std::string m_config_value;
    std::string m_reason;
};

/**
 * @class ValidationException
 * @brief 数据验证异常
 * @details 处理数据格式、范围、类型验证等验证相关的错误
 */
class ValidationException : public FastQException {
public:
    /**
     * @brief 构造函数
     * @param field_name 字段名称
     * @param field_value 字段值
     * @param validation_rule 验证规则
     */
    ValidationException(const std::string& field_name,
                        const std::string& field_value,
                        const std::string& validation_rule);
    
    /**
     * @brief 带期望值的构造函数
     * @param field_name 字段名称
     * @param field_value 字段值
     * @param expected_value 期望值
     * @param validation_rule 验证规则
     */
    ValidationException(const std::string& field_name,
                        const std::string& field_value,
                        const std::string& expected_value,
                        const std::string& validation_rule);
    
    // 获取字段名称
    [[nodiscard]] auto get_field_name() const noexcept -> const std::string&;
    
    // 获取字段值
    [[nodiscard]] auto get_field_value() const noexcept -> const std::string&;
    
    // 获取验证规则
    [[nodiscard]] auto get_validation_rule() const noexcept -> const std::string&;
    
    // 获取期望值
    [[nodiscard]] auto get_expected_value() const noexcept -> const std::string&;
    
    // 克隆异常
    [[nodiscard]] auto clone() const -> std::unique_ptr<FastQException> override;

private:
    std::string m_field_name;
    std::string m_field_value;
    std::string m_validation_rule;
    std::string m_expected_value;
};

/**
 * @class ProcessingException
 * @brief 处理流程异常
 * @details 处理数据处理、计算流程等处理相关的错误
 */
class ProcessingException : public FastQException {
public:
    /**
     * @brief 构造函数
     * @param operation 操作名称
     * @param processed_count 已处理数量
     * @param failed_count 失败数量
     * @param details 详细信息
     */
    ProcessingException(const std::string& operation,
                        size_t processed_count,
                        size_t failed_count,
                        const std::string& details);
    
    // 获取操作名称
    [[nodiscard]] auto get_operation() const noexcept -> const std::string&;
    
    // 获取已处理数量
    [[nodiscard]] auto get_processed_count() const noexcept -> size_t;
    
    // 获取失败数量
    [[nodiscard]] auto get_failed_count() const noexcept -> size_t;
    
    // 获取成功率
    [[nodiscard]] auto get_success_rate() const noexcept -> double;
    
    // 获取详细信息
    [[nodiscard]] auto get_details() const noexcept -> const std::string&;
    
    // 克隆异常
    [[nodiscard]] auto clone() const -> std::unique_ptr<FastQException> override;

private:
    std::string m_operation;
    size_t m_processed_count;
    size_t m_failed_count;
    std::string m_details;
};

/**
 * @class MemoryException
 * @brief 内存管理异常
 * @details 处理内存分配、访问、泄漏等内存相关的错误
 */
class MemoryException : public FastQException {
public:
    /**
     * @brief 构造函数
     * @param requested_size 请求大小
     * @param available_size 可用大小
     * @param allocation_type 分配类型
     */
    MemoryException(size_t requested_size,
                   size_t available_size,
                   const std::string& allocation_type);
    
    /**
     * @brief 内存访问异常构造函数
     * @param address 访问地址
     * @param access_type 访问类型
     * @param reason 错误原因
     */
    MemoryException(uintptr_t address,
                   const std::string& access_type,
                   const std::string& reason);
    
    // 获取请求大小
    [[nodiscard]] auto get_requested_size() const noexcept -> size_t;
    
    // 获取可用大小
    [[nodiscard]] auto get_available_size() const noexcept -> size_t;
    
    // 获取分配类型
    [[nodiscard]] auto get_allocation_type() const noexcept -> const std::string&;
    
    // 获取访问地址
    [[nodiscard]] auto get_access_address() const noexcept -> uintptr_t;
    
    // 获取访问类型
    [[nodiscard]] auto get_access_type() const noexcept -> const std::string&;
    
    // 是否为内存分配异常
    [[nodiscard]] auto is_allocation_error() const noexcept -> bool;
    
    // 是否为内存访问异常
    [[nodiscard]] auto is_access_error() const noexcept -> bool;
    
    // 克隆异常
    [[nodiscard]] auto clone() const -> std::unique_ptr<FastQException> override;

private:
    size_t m_requested_size{0};
    size_t m_available_size{0};
    std::string m_allocation_type;
    uintptr_t m_access_address{0};
    std::string m_access_type;
    bool m_is_allocation_error{true};
};

/**
 * @class ConcurrencyException
 * @brief 并发处理异常
 * @details 处理线程、锁、同步等并发相关的错误
 */
class ConcurrencyException : public FastQException {
public:
    /**
     * @brief 构造函数
     * @param operation 操作名称
     * @param resource_name 资源名称
     * @param thread_count 线程数量
     */
    ConcurrencyException(const std::string& operation,
                        const std::string& resource_name,
                        int thread_count);
    
    /**
     * @brief 死锁异常构造函数
     * @param lock_sequence 锁序列
     * @param thread_ids 涉及的线程ID
     */
    ConcurrencyException(const std::vector<std::string>& lock_sequence,
                        const std::vector<std::string>& thread_ids);
    
    // 获取操作名称
    [[nodiscard]] auto get_operation() const noexcept -> const std::string&;
    
    // 获取资源名称
    [[nodiscard]] auto get_resource_name() const noexcept -> const std::string&;
    
    // 获取线程数量
    [[nodiscard]] auto get_thread_count() const noexcept -> int;
    
    // 获取锁序列
    [[nodiscard]] auto get_lock_sequence() const noexcept -> const std::vector<std::string>&;
    
    // 获取涉及的线程ID
    [[nodiscard]] auto get_thread_ids() const noexcept -> const std::vector<std::string>&;
    
    // 是否为死锁异常
    [[nodiscard]] auto is_deadlock() const noexcept -> bool;
    
    // 克隆异常
    [[nodiscard]] auto clone() const -> std::unique_ptr<FastQException> override;

private:
    std::string m_operation;
    std::string m_resource_name;
    int m_thread_count{0};
    std::vector<std::string> m_lock_sequence;
    std::vector<std::string> m_thread_ids;
    bool m_is_deadlock{false};
};

/**
 * @class NetworkException
 * @brief 网络相关异常
 * @details 处理网络连接、通信等网络相关的错误
 */
class NetworkException : public FastQException {
public:
    /**
     * @brief 构造函数
     * @param host 主机名
     * @param port 端口
     * @param operation 操作类型
     * @param error_code 错误代码
     */
    NetworkException(const std::string& host,
                    int port,
                    const std::string& operation,
                    int error_code);
    
    // 获取主机名
    [[nodiscard]] auto get_host() const noexcept -> const std::string&;
    
    // 获取端口
    [[nodiscard]] auto get_port() const noexcept -> int;
    
    // 获取操作类型
    [[nodiscard]] auto get_operation() const noexcept -> const std::string&;
    
    // 获取错误代码
    [[nodiscard]] auto get_network_error_code() const noexcept -> int;
    
    // 克隆异常
    [[nodiscard]] auto clone() const -> std::unique_ptr<FastQException> override;

private:
    std::string m_host;
    int m_port{0};
    std::string m_operation;
    int m_error_code{0};
};

// 异常工厂函数
auto create_io_exception(const std::string& file_path, 
                        int system_error_code,
                        const std::string& operation) -> std::unique_ptr<FastQException>;

auto create_config_exception(const std::string& config_key,
                             const std::string& config_value,
                             const std::string& reason) -> std::unique_ptr<FastQException>;

auto create_validation_exception(const std::string& field_name,
                                const std::string& field_value,
                                const std::string& validation_rule) -> std::unique_ptr<FastQException>;

auto create_processing_exception(const std::string& operation,
                                 size_t processed_count,
                                 size_t failed_count,
                                 const std::string& details) -> std::unique_ptr<FastQException>;

auto create_memory_exception(size_t requested_size,
                            size_t available_size,
                            const std::string& allocation_type) -> std::unique_ptr<FastQException>;

auto create_concurrency_exception(const std::string& operation,
                                 const std::string& resource_name,
                                 int thread_count) -> std::unique_ptr<FastQException>;

auto create_network_exception(const std::string& host,
                             int port,
                             const std::string& operation,
                             int error_code) -> std::unique_ptr<FastQException>;

// 异常转换函数
auto convert_std_exception(const std::exception& ex) -> std::unique_ptr<FastQException>;

} // namespace fq::error