/**
 * @file error_codes.h
 * @brief 错误代码系统定义
 * @details 定义错误代码枚举和错误严重程度
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
#include <map>
#include <vector>

namespace fq::error {

// 错误代码枚举
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
    InvalidDataFormat = 4003,
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

// 错误严重程度
enum class ErrorSeverity {
    Info = 0,      // 信息性消息
    Warning = 1,   // 警告，程序可以继续
    Error = 2,     // 错误，当前操作失败
    Critical = 3,  // 严重错误，程序可能需要终止
    Fatal = 4      // 致命错误，程序必须终止
};

// 错误信息结构
struct ErrorInfo {
    ErrorCode code;
    std::string name;
    std::string description;
    ErrorSeverity default_severity;
    std::vector<std::string> suggestions;
    
    auto get_user_message() const -> std::string;
    auto get_log_message() const -> std::string;
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
    
    auto is_registered(ErrorCode code) const -> bool;

public:
    ErrorCodeRegistry();
    std::map<ErrorCode, ErrorInfo> m_error_registry;
    
    auto initialize_default_errors() -> void;
};

// 全局函数
auto get_error_info(ErrorCode code) -> const ErrorInfo&;
auto get_error_message(ErrorCode code) -> std::string;
auto get_error_severity(ErrorCode code) -> ErrorSeverity;
auto get_error_suggestions(ErrorCode code) -> std::vector<std::string>;

} // namespace fq::error