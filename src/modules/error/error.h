/**
 * @file error.h
 * @brief 定义了项目统一的异常处理框架。
 * @author BGI-Research
 * @version 1.0
 * @date 2025-07-31
 * @copyright Copyright (c) 2025 BGI-Research
 */

#pragma once

#include <fmt/format.h>

#include <cstring>
#include <exception>
#include <functional>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "common.h"

namespace fq::error {

/// @brief 错误类别枚举。
enum class ErrorCategory { IO = 1, Format = 2, Validation = 3, Processing = 4, Resource = 5, Configuration = 6 };
/// @brief 错误严重性枚举。
enum class ErrorSeverity { Info = 1, Warning = 2, Error = 3, Critical = 4 };

/**
 * @brief 项目所有异常的基类。
 */
class FastQException : public std::exception {
  public:
    FastQException(ErrorCategory category, ErrorSeverity severity, std::string message);

    [[nodiscard]] auto category() const noexcept -> ErrorCategory;
    [[nodiscard]] auto severity() const noexcept -> ErrorSeverity;
    [[nodiscard]] auto message() const noexcept -> const std::string&;
    [[nodiscard]] auto what() const noexcept -> const char* override;
    [[nodiscard]] auto is_recoverable() const noexcept -> bool;

  private:
    ErrorCategory m_category;
    ErrorSeverity m_severity;
    std::string m_message;
    mutable std::string m_what_message;

    auto category_string(ErrorCategory cat) const -> std::string_view;
    auto severity_string(ErrorSeverity sev) const -> std::string_view;
    void format_what_message();
};

/// @brief 表示 I/O 错误的异常。
class IOError : public FastQException {
  public:
    explicit IOError(std::string_view file_path, int error_code = 0);
};

/// @brief 表示文件格式错误的异常。
class FormatError : public FastQException {
  public:
    explicit FormatError(std::string_view message);
};

/// @brief 表示配置错误的异常。
class ConfigurationError : public FastQException {
  public:
    explicit ConfigurationError(std::string_view message);
};

/**
 * @brief 一个单例的错误处理器，用于分发和处理异常。
 */
class ErrorHandler {
  public:
    using HandlerFunc = std::function<bool(const FastQException&)>;

    static auto instance() -> ErrorHandler&;

    void register_handler(ErrorCategory category, HandlerFunc handler);
    auto handle_error(const FastQException& error) -> bool;

  private:
    mutable std::mutex m_mutex;
    std::unordered_map<ErrorCategory, std::vector<HandlerFunc>> m_handlers;
};

// Convenience Macros
#define FQ_THROW_IO_ERROR(file_path, error_code) throw fq::error::IOError(file_path, error_code)
#define FQ_THROW_FORMAT_ERROR(message) throw fq::error::FormatError(message)
#define FQ_THROW_CONFIG_ERROR(message) throw fq::error::ConfigurationError(message)

}  // namespace fq::error
