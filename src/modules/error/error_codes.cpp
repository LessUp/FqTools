/**
 * @file error_codes.cpp
 * @brief 错误代码系统实现
 * @details 实现错误代码的注册、查询和管理功能
 * 
 * @author FastQTools Team
 * @date 2024
 * @version 1.0
 * 
 * @copyright Copyright (c) 2024 FastQTools
 * @license MIT License
 */

#include "error_codes.h"
#include <algorithm>
#include <mutex>
#include <memory>
#include <sstream>

namespace fq::error {

namespace {
    // 全局错误注册表
    std::unique_ptr<ErrorCodeRegistry> g_registry;
    std::mutex g_registry_mutex;
}

// ErrorInfo 实现
auto ErrorInfo::get_user_message() const -> std::string {
    std::ostringstream oss;
    oss << "错误: " << description;
    
    switch (default_severity) {
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
    
    return oss.str();
}

auto ErrorInfo::get_log_message() const -> std::string {
    std::ostringstream oss;
    oss << "[" << static_cast<int>(code) << ":" << static_cast<int>(default_severity) << "] " << description;
    return oss.str();
}

// ErrorCodeRegistry 实现
ErrorCodeRegistry::ErrorCodeRegistry() {
    initialize_default_errors();
}

auto ErrorCodeRegistry::get_instance() -> ErrorCodeRegistry& {
    std::lock_guard<std::mutex> lock(g_registry_mutex);
    
    if (!g_registry) {
        g_registry = std::make_unique<ErrorCodeRegistry>();
    }
    
    return *g_registry;
}

auto ErrorCodeRegistry::register_error(const ErrorInfo& info) -> void {
    m_error_registry[info.code] = info;
}

auto ErrorCodeRegistry::get_error_info(ErrorCode code) const -> const ErrorInfo& {
    auto it = m_error_registry.find(code);
    if (it != m_error_registry.end()) {
        return it->second;
    }
    
    // 返回未知错误信息
    static ErrorInfo unknown_error{
        ErrorCode::Unknown,
        "Unknown",
        "未知错误",
        ErrorSeverity::Error,
        {"请检查错误日志获取更多信息"}
    };
    
    return unknown_error;
}

auto ErrorCodeRegistry::get_all_errors() const -> std::vector<ErrorInfo> {
    std::vector<ErrorInfo> errors;
    errors.reserve(m_error_registry.size());
    
    for (const auto& [code, info] : m_error_registry) {
        errors.push_back(info);
    }
    
    return errors;
}

auto ErrorCodeRegistry::get_user_message(ErrorCode code) const -> std::string {
    return get_error_info(code).get_user_message();
}

auto ErrorCodeRegistry::get_suggestions(ErrorCode code) const -> std::vector<std::string> {
    return get_error_info(code).suggestions;
}

auto ErrorCodeRegistry::is_registered(ErrorCode code) const -> bool {
    return m_error_registry.find(code) != m_error_registry.end();
}

auto ErrorCodeRegistry::initialize_default_errors() -> void {
    // 通用错误
    register_error({
        ErrorCode::Unknown,
        "Unknown",
        "未知错误",
        ErrorSeverity::Error,
        {"请检查错误日志获取更多信息", "联系技术支持"}
    });
    
    register_error({
        ErrorCode::InternalError,
        "InternalError",
        "内部错误",
        ErrorSeverity::Critical,
        {"检查系统资源", "重启应用程序", "联系技术支持"}
    });
    
    register_error({
        ErrorCode::NotImplemented,
        "NotImplemented",
        "功能未实现",
        ErrorSeverity::Error,
        {"检查功能是否可用", "更新到最新版本"}
    });
    
    register_error({
        ErrorCode::Timeout,
        "Timeout",
        "操作超时",
        ErrorSeverity::Error,
        {"增加超时时间", "检查网络连接", "减少数据量"}
    });
    
    // IO 错误
    register_error({
        ErrorCode::FileNotFound,
        "FileNotFound",
        "文件未找到",
        ErrorSeverity::Error,
        {"检查文件路径", "确认文件存在", "检查文件权限"}
    });
    
    register_error({
        ErrorCode::PermissionDenied,
        "PermissionDenied",
        "权限被拒绝",
        ErrorSeverity::Error,
        {"检查文件权限", "使用管理员权限运行", "联系系统管理员"}
    });
    
    register_error({
        ErrorCode::InvalidFormat,
        "InvalidFormat",
        "无效格式",
        ErrorSeverity::Error,
        {"检查文件格式", "使用正确的文件格式", "验证文件完整性"}
    });
    
    register_error({
        ErrorCode::FileCorrupted,
        "FileCorrupted",
        "文件损坏",
        ErrorSeverity::Error,
        {"检查文件完整性", "使用备份文件", "重新生成文件"}
    });
    
    register_error({
        ErrorCode::DiskFull,
        "DiskFull",
        "磁盘空间不足",
        ErrorSeverity::Critical,
        {"清理磁盘空间", "使用更大的存储设备", "减少数据量"}
    });
    
    register_error({
        ErrorCode::NetworkError,
        "NetworkError",
        "网络错误",
        ErrorSeverity::Error,
        {"检查网络连接", "重试操作", "检查防火墙设置"}
    });
    
    // 配置错误
    register_error({
        ErrorCode::InvalidConfig,
        "InvalidConfig",
        "无效配置",
        ErrorSeverity::Error,
        {"检查配置文件格式", "使用默认配置", "参考配置文档"}
    });
    
    register_error({
        ErrorCode::MissingConfig,
        "MissingConfig",
        "缺少配置",
        ErrorSeverity::Error,
        {"添加必需的配置项", "使用默认配置", "检查配置文件"}
    });
    
    register_error({
        ErrorCode::ConfigOutOfRange,
        "ConfigOutOfRange",
        "配置超出范围",
        ErrorSeverity::Error,
        {"调整配置值到有效范围", "使用默认值", "参考配置文档"}
    });
    
    register_error({
        ErrorCode::ConfigTypeMismatch,
        "ConfigTypeMismatch",
        "配置类型不匹配",
        ErrorSeverity::Error,
        {"检查配置值类型", "使用正确的数据类型", "参考配置文档"}
    });
    
    // 验证错误
    register_error({
        ErrorCode::InvalidParameter,
        "InvalidParameter",
        "无效参数",
        ErrorSeverity::Error,
        {"检查参数值", "使用有效的参数范围", "参考参数文档"}
    });
    
    register_error({
        ErrorCode::InvalidRange,
        "InvalidRange",
        "无效范围",
        ErrorSeverity::Error,
        {"检查范围值", "使用有效的范围", "参考范围文档"}
    });
    
    register_error({
        ErrorCode::InvalidFormat,
        "InvalidFormat",
        "无效格式",
        ErrorSeverity::Error,
        {"检查数据格式", "使用正确的格式", "验证数据完整性"}
    });
    
    register_error({
        ErrorCode::MissingRequiredField,
        "MissingRequiredField",
        "缺少必需字段",
        ErrorSeverity::Error,
        {"添加必需字段", "检查数据完整性", "参考数据格式文档"}
    });
    
    register_error({
        ErrorCode::ValidationFailed,
        "ValidationFailed",
        "验证失败",
        ErrorSeverity::Error,
        {"检查数据有效性", "修正数据错误", "参考验证规则"}
    });
    
    // 数据错误
    register_error({
        ErrorCode::DataCorrupted,
        "DataCorrupted",
        "数据损坏",
        ErrorSeverity::Error,
        {"检查数据完整性", "使用备份数据", "重新生成数据"}
    });
    
    register_error({
        ErrorCode::DataInconsistent,
        "DataInconsistent",
        "数据不一致",
        ErrorSeverity::Error,
        {"检查数据一致性", "同步数据", "验证数据完整性"}
    });
    
    register_error({
        ErrorCode::DataTooLarge,
        "DataTooLarge",
        "数据过大",
        ErrorSeverity::Error,
        {"减少数据量", "分批处理", "增加系统资源"}
    });
    
    register_error({
        ErrorCode::DataEmpty,
        "DataEmpty",
        "数据为空",
        ErrorSeverity::Warning,
        {"检查数据源", "提供有效数据", "跳过空数据"}
    });
    
    // 处理错误
    register_error({
        ErrorCode::ProcessingFailed,
        "ProcessingFailed",
        "处理失败",
        ErrorSeverity::Error,
        {"检查输入数据", "重新处理", "检查处理逻辑"}
    });
    
    register_error({
        ErrorCode::ProcessingTimeout,
        "ProcessingTimeout",
        "处理超时",
        ErrorSeverity::Error,
        {"增加超时时间", "减少数据量", "优化处理算法"}
    });
    
    register_error({
        ErrorCode::ProcessingInterrupted,
        "ProcessingInterrupted",
        "处理中断",
        ErrorSeverity::Error,
        {"重新启动处理", "检查中断原因", "恢复处理状态"}
    });
    
    register_error({
        ErrorCode::ResourceBusy,
        "ResourceBusy",
        "资源忙",
        ErrorSeverity::Error,
        {"等待资源释放", "减少并发", "增加资源"}
    });
    
    // 内存错误
    register_error({
        ErrorCode::MemoryAllocationFailed,
        "MemoryAllocationFailed",
        "内存分配失败",
        ErrorSeverity::Critical,
        {"增加系统内存", "减少内存使用", "重启应用程序"}
    });
    
    register_error({
        ErrorCode::MemoryAccessViolation,
        "MemoryAccessViolation",
        "内存访问违规",
        ErrorSeverity::Critical,
        {"检查内存访问", "修复内存错误", "重启应用程序"}
    });
    
    register_error({
        ErrorCode::MemoryLeakDetected,
        "MemoryLeakDetected",
        "内存泄漏检测",
        ErrorSeverity::Critical,
        {"修复内存泄漏", "优化内存管理", "重启应用程序"}
    });
    
    register_error({
        ErrorCode::MemoryLimitExceeded,
        "MemoryLimitExceeded",
        "内存限制超出",
        ErrorSeverity::Critical,
        {"增加内存限制", "减少内存使用", "优化内存管理"}
    });
    
    // 并发错误
    register_error({
        ErrorCode::DeadlockDetected,
        "DeadlockDetected",
        "死锁检测",
        ErrorSeverity::Critical,
        {"解决死锁问题", "优化锁策略", "重启应用程序"}
    });
    
    register_error({
        ErrorCode::RaceCondition,
        "RaceCondition",
        "竞争条件",
        ErrorSeverity::Critical,
        {"解决竞争条件", "使用同步机制", "优化并发逻辑"}
    });
    
    register_error({
        ErrorCode::ThreadCreationFailed,
        "ThreadCreationFailed",
        "线程创建失败",
        ErrorSeverity::Error,
        {"增加系统资源", "减少线程数量", "检查线程配置"}
    });
    
    register_error({
        ErrorCode::SynchronizationError,
        "SynchronizationError",
        "同步错误",
        ErrorSeverity::Error,
        {"检查同步逻辑", "修复同步错误", "优化并发处理"}
    });
}

// 全局函数实现
auto get_error_info(ErrorCode code) -> const ErrorInfo& {
    return ErrorCodeRegistry::get_instance().get_error_info(code);
}

auto get_error_message(ErrorCode code) -> std::string {
    return ErrorCodeRegistry::get_instance().get_user_message(code);
}

auto get_error_severity(ErrorCode code) -> ErrorSeverity {
    return ErrorCodeRegistry::get_instance().get_error_info(code).default_severity;
}

auto get_error_suggestions(ErrorCode code) -> std::vector<std::string> {
    return ErrorCodeRegistry::get_instance().get_suggestions(code);
}

} // namespace fq::error