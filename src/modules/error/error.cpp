#include "error.h"

#include <fmt/format.h>

#include <cstring>
#include <utility>

namespace fq::error {

auto ErrorHandler::instance() -> ErrorHandler& {
    static ErrorHandler handler;
    return handler;
}

auto FastQException::category() const noexcept -> ErrorCategory {
    return m_category;
}

auto FastQException::severity() const noexcept -> ErrorSeverity {
    return m_severity;
}

auto FastQException::message() const noexcept -> const std::string& {
    return m_message;
}

auto FastQException::what() const noexcept -> const char* {
    return m_what_message.c_str();
}

auto FastQException::is_recoverable() const noexcept -> bool {
    return m_severity != ErrorSeverity::Critical;
}

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