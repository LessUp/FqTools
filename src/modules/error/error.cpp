/**
 * @file error.cpp
 * @brief 错误处理模块实现文件
 * @details 实现了 FastQTools 的错误处理与异常类，包括错误分类、严重性和消息生成。
 * @author FastQTools Team
 * @date 2025-08-01
 * @version 1.0
 * @copyright Copyright (c) 2025 FastQTools
 */

#include "error.h"

#include <fmt/format.h>

#include <cstring>
#include <utility>

namespace fq::error {

/**
 * @brief 获取 ErrorHandler 单例实例
 * @return ErrorHandler 实例的引用
 */
auto ErrorHandler::instance() -> ErrorHandler& {
    static ErrorHandler handler;
    return handler;
}

/**
 * @brief 获取异常的错误类别
 * @return 错误类别枚举值
 */
auto FastQException::category() const noexcept -> ErrorCategory {
    return m_category;
}

/**
 * @brief 获取异常的严重性等级
 * @return 错误严重性枚举值
 */
auto FastQException::severity() const noexcept -> ErrorSeverity {
    return m_severity;
}

/**
 * @brief 获取异常的详细信息
 * @return 异常消息字符串
 */
auto FastQException::message() const noexcept -> const std::string& {
    return m_message;
}

/**
 * @brief 获取异常的 what 消息（标准异常接口）
 * @return C 风格字符串
 */
auto FastQException::what() const noexcept -> const char* {
    return m_what_message.c_str();
}

/**
 * @brief 判断异常是否可恢复
 * @return 可恢复返回 true，不可恢复返回 false
 */
auto FastQException::is_recoverable() const noexcept -> bool {
    return m_severity != ErrorSeverity::Critical;
}

/**
 * @brief 错误类别枚举转字符串
 * @param cat 错误类别
 * @return 字符串视图
 */
auto FastQException::category_string(ErrorCategory cat) const -> std::string_view {
    switch (cat) {
        case ErrorCategory::IO:
            return "IO";
        case ErrorCategory::Format:
            return "FORMAT";
        case ErrorCategory::Validation:
            return "VALIDATION";
        case ErrorCategory::Processing:
            return "PROCESSING";
        case ErrorCategory::Resource:
            return "RESOURCE";
        case ErrorCategory::Configuration:
            return "CONFIG";
        default:
            return "UNKNOWN";
    }
}

/**
 * @brief 错误严重性枚举转字符串
 * @param sev 错误严重性
 * @return 字符串视图
 */
auto FastQException::severity_string(ErrorSeverity sev) const -> std::string_view {
    switch (sev) {
        case ErrorSeverity::Info:
            return "INFO";
        case ErrorSeverity::Warning:
            return "WARN";
        case ErrorSeverity::Error:
            return "ERROR";
        case ErrorSeverity::Critical:
            return "CRITICAL";
        default:
            return "UNKNOWN";
    }
}

void FastQException::format_what_message() {
    m_what_message = fmt::format("[{}:{}] {}", category_string(m_category), severity_string(m_severity), m_message);
}

auto ErrorHandler::handle_error(const FastQException& error) -> bool {
    std::lock_guard lock(m_mutex);
    auto it = m_handlers.find(error.category());
    if (it != m_handlers.end()) {
        for (auto& handler : it->second) {
            if (handler(error))
                return true;
        }
    }
    fq::common::Logger::instance().error("Unhandled exception: {}", error.what());
    return false;
}


}  // namespace fq::error