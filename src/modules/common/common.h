/**
 * @file common.h
 * @brief 提供项目中通用的工具和类。
 * @author BGI-Research
 * @version 1.0
 * @date 2025-07-31
 * @copyright Copyright (c) 2025 BGI-Research
 */

#pragma once

#include <fmt/format.h>

#include <chrono>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace fq::common {

/**
 * @brief 一个简单的计时器类，用于测量代码块的执行时间。
 */
class Timer {
  public:
    explicit Timer(std::string_view name);
    void report() const;
    [[nodiscard]] auto elapsed() const -> std::chrono::nanoseconds;

  private:
    std::string m_name; ///< 计时器的名称。
    std::chrono::high_resolution_clock::time_point m_start; ///< 计时开始的时间点。
};

auto split(std::string_view input, char delimiter) -> std::vector<std::string>;
auto trim(std::string_view input) -> std::string;
auto join(const std::vector<std::string>& parts, std::string_view delimiter) -> std::string;

/**
 * @brief 一个简单的单例日志记录器。
 */
class Logger {
  public:
    enum class Level { Debug = 0, Info = 1, Warn = 2, Error = 3, Critical = 4 };
    static auto instance() -> Logger&;
    void log(Level level, std::string_view message);
    void set_level(Level level);

    template <typename... Args>
    void debug(fmt::format_string<Args...> fmt, Args&&... args) {
        log(Level::Debug, fmt::format(fmt, std::forward<Args>(args)...));
    }
    template <typename... Args>
    void info(fmt::format_string<Args...> fmt, Args&&... args) {
        log(Level::Info, fmt::format(fmt, std::forward<Args>(args)...));
    }
    template <typename... Args>
    void warn(fmt::format_string<Args...> fmt, Args&&... args) {
        log(Level::Warn, fmt::format(fmt, std::forward<Args>(args)...));
    }
    template <typename... Args>
    void error(fmt::format_string<Args...> fmt, Args&&... args) {
        log(Level::Error, fmt::format(fmt, std::forward<Args>(args)...));
    }

  private:
    Logger() = default;
    Level m_level = Level::Info;
    auto get_level_string(Level level) -> std::string_view;
    auto get_current_time() -> std::string;
};

/**
 * @brief 一个简单的线程安全的唯一ID生成器。
 */
class IDGenerator {
  public:
    using ID = std::uint64_t;
    static auto next_id() -> ID;
    static auto reset() -> void;
};

void print_software_info();
void print_logo();
}  // namespace fq::common
