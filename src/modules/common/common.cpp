#include "common.h"

#include <fmt/format.h>

#include <atomic>
#include <ctime>
#include <iostream>
#include <sstream>
#include <utility>

namespace fq::common {

Timer::Timer(std::string_view name) : m_name(name), m_start(std::chrono::high_resolution_clock::now()) {}

void Timer::report() const {
    auto elapsed = std::chrono::high_resolution_clock::now() - m_start;
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
    std::cout << fmt::format("[{}] Elapsed: {}ms\n", m_name, milliseconds);
}

auto Timer::elapsed() const -> std::chrono::nanoseconds {
    return std::chrono::high_resolution_clock::now() - m_start;
}

auto split(std::string_view input, char delimiter) -> std::vector<std::string> {
    std::vector<std::string> result;
    std::istringstream stream{std::string(input)};
    std::string token;
    while (std::getline(stream, token, delimiter)) {
        if (!token.empty()) {
            result.push_back(token);
        }
    }
    return result;
}

auto trim(std::string_view input) -> std::string {
    auto start = input.find_first_not_of(" \t\n\r");
    if (start == std::string_view::npos)
        return "";
    auto end = input.find_last_not_of(" \t\n\r");
    return std::string(input.substr(start, end - start + 1));
}

auto join(const std::vector<std::string>& parts, std::string_view delimiter) -> std::string {
    if (parts.empty())
        return "";
    std::ostringstream result;
    result << parts[0];
    for (size_t i = 1; i < parts.size(); ++i) {
        result << delimiter << parts[i];
    }
    return result.str();
}

auto Logger::instance() -> Logger& {
    static Logger logger;
    return logger;
}

void Logger::log(Level level, std::string_view message) {
    if (level >= m_level) {
        std::cout << fmt::format("[{}] {}: {}\n", get_level_string(level), get_current_time(), message);
    }
}

void Logger::set_level(Level level) {
    m_level = level;
}

auto Logger::get_level_string(Level level) -> std::string_view {
    switch (level) {
        case Level::Debug:
            return "DEBUG";
        case Level::Info:
            return "INFO";
        case Level::Warn:
            return "WARN";
        case Level::Error:
            return "ERROR";
        case Level::Critical:
            return "CRITICAL";
        default:
            return "UNKNOWN";
    }
}

auto Logger::get_current_time() -> std::string {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    struct tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &time_t_now);
#else
    localtime_r(&time_t_now, &tm_buf);
#endif

    return fmt::format("{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}", tm_buf.tm_year + 1900, tm_buf.tm_mon + 1,
                       tm_buf.tm_mday, tm_buf.tm_hour, tm_buf.tm_min, tm_buf.tm_sec);
}

auto IDGenerator::next_id() -> ID {
    static std::atomic<ID> counter{1};
    return counter.fetch_add(1, std::memory_order_relaxed);
}

auto IDGenerator::reset() -> void {
    static std::atomic<ID> counter{1};
    counter.store(1, std::memory_order_relaxed);
}

void print_software_info() {
    std::cout << "FastQTools v3.0.0 - Modern C++20 FastQ Processing Tool\n";
    std::cout << "Copyright (c) 2025 BGI-Research\n";
    std::cout << "Built with modern C++ modules and high-performance parallel processing\n";
}

void print_logo() {
    std::cout << R"(
    ███████╗ █████╗ ███████╗████████╗ ██████╗ ████████╗ ██████╗  ██████╗ ██╗     ███████╗
    ██╔════╝██╔══██╗██╔════╝╚══██╔══╝██╔═══██╗╚══██╔══╝██╔═══██╗██╔═══██╗██║     ██╔════╝
    █████╗  ███████║███████╗   ██║   ██║   ██║   ██║   ██║   ██║██║   ██║██║     ███████╗
    ██╔══╝  ██╔══██║╚════██║   ██║   ██║▄▄ ██║   ██║   ██║   ██║██║   ██║██║     ╚════██║
    ██║     ██║  ██║███████║   ██║   ╚██████╔╝   ██║   ╚██████╔╝╚██████╔╝███████╗███████║
    ╚═╝     ╚═╝  ╚═╝╚══════╝   ╚═╝    ╚══▀▀═╝    ╚═╝    ╚═════╝  ╚═════╝ ╚══════╝╚══════╝
        )" << std::endl;
}
}  // namespace fq::common