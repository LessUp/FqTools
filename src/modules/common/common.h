/**
 * @file common.h
 * @brief 提供项目中通用的工具和类，例如计时器、日志记录器和字符串处理函数。
 * @details 该文件是传统头文件实现方式下的通用模块，定义了多个在项目中广泛使用的基础组件。
 * 
 * @author shane
 * @date 2025-07-31
 * @version 1.0
 * 
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
 * @details 此类提供了 RAII 风格的计时功能，在对象构造时开始计时，
 *          可以随时报告已经过的时间。
 * @note 此计时器不是线程安全的。
 */
class Timer {
  public:
    /**
     * @brief 构造一个新的计时器并开始计时。
     * @param name 计时器的名称，用于在报告中标识。
     */
    explicit Timer(std::string_view name);

    /**
     * @brief 报告从计时器创建到当前的总耗时。
     * @details 将格式化的耗时信息（毫秒）输出到标准输出。
     */
    void report() const;

    /**
     * @brief 获取从计时器创建到当前的总耗时。
     * @return 返回一个 `std::chrono::nanoseconds` 类型的值，表示经过的纳秒数。
     * @threadsafe 线程安全。
     */
    [[nodiscard]] auto elapsed() const -> std::chrono::nanoseconds;

  private:
    std::string m_name; ///< 计时器的名称。
    std::chrono::high_resolution_clock::time_point m_start; ///< 计时开始的时间点。
};

/**
 * @brief 按指定分隔符分割字符串。
 * @param input 要分割的输入字符串视图。
 * @param delimiter 用于分割的分隔符。
 * @return 返回一个包含所有非空子字符串的 `std::vector<std::string>`。
 */
auto split(std::string_view input, char delimiter) -> std::vector<std::string>;

/**
 * @brief 移除字符串首尾的空白字符。
 * @param input 要修剪的输入字符串视图。
 * @return 返回一个移除了首尾空白（空格, \t, \n, \r）的新字符串。
 */
auto trim(std::string_view input) -> std::string;

/**
 * @brief 使用指定分隔符连接一个字符串向量。
 * @param parts 要连接的字符串向量。
 * @param delimiter 用于连接各部分的分隔符。
 * @return 返回连接后的完整字符串。
 */
auto join(const std::vector<std::string>& parts, std::string_view delimiter) -> std::string;

/**
 * @brief 一个简单的单例日志记录器。
 * @details 提供分级别的日志记录功能，并支持使用 `fmt` 库进行格式化。
 * @note 这是一个线程安全的单例，但日志消息的输出顺序不完全保证。
 */
class Logger {
  public:
    /**
     * @brief 日志级别枚举。
     */
    enum class Level {
        Debug = 0, ///< 调试信息，用于开发和诊断。
        Info = 1,  ///< 常规信息，报告程序的运行状态。
        Warn = 2,  ///< 警告信息，表示可能存在的问题。
        Error = 3, ///< 错误信息，表示发生了可恢复的错误。
        Critical = 4 ///< 严重错误，可能导致程序终止。
    };

    /**
     * @brief 获取日志记录器的唯一实例。
     * @return 返回对 Logger 实例的引用。
     * @threadsafe 线程安全。
     */
    static auto instance() -> Logger&;

    /**
     * @brief 记录一条日志消息。
     * @param level 日志级别。
     * @param message 要记录的日志消息。
     * @pre 消息级别必须高于或等于当前设置的日志级别才会被记录。
     */
    void log(Level level, std::string_view message);

    /**
     * @brief 设置日志记录器的最低级别。
     * @param level 要设置的最低日志级别。低于此级别的消息将被忽略。
     */
    void set_level(Level level);

    /**
     * @brief 记录一条 DEBUG 级别的格式化日志。
     * @tparam Args 格式化参数的类型。
     * @param fmt `fmt` 格式化字符串。
     * @param args 传递给格式化字符串的参数。
     */
    template <typename... Args>
    void debug(fmt::format_string<Args...> fmt, Args&&... args) {
        log(Level::Debug, fmt::format(fmt, std::forward<Args>(args)...));
    }

    /**
     * @brief 记录一条 INFO 级别的格式化日志。
     * @tparam Args 格式化参数的类型。
     * @param fmt `fmt` 格式化字符串。
     * @param args 传递给格式化字符串的参数。
     */
    template <typename... Args>
    void info(fmt::format_string<Args...> fmt, Args&&... args) {
        log(Level::Info, fmt::format(fmt, std::forward<Args>(args)...));
    }

    /**
     * @brief 记录一条 WARN 级别的格式化日志。
     * @tparam Args 格式化参数的类型。
     * @param fmt `fmt` 格式化字符串。
     * @param args 传递给格式化字符串的参数。
     */
    template <typename... Args>
    void warn(fmt::format_string<Args...> fmt, Args&&... args) {
        log(Level::Warn, fmt::format(fmt, std::forward<Args>(args)...));
    }

    /**
     * @brief 记录一条 ERROR 级别的格式化日志。
     * @tparam Args 格式化参数的类型。
     * @param fmt `fmt` 格式化字符串。
     * @param args 传递给格式化字符串的参数。
     */
    template <typename... Args>
    void error(fmt::format_string<Args...> fmt, Args&&... args) {
        log(Level::Error, fmt::format(fmt, std::forward<Args>(args)...));
    }

  private:
    Logger() = default;
    Level m_level = Level::Info; ///< 当前的日志记录级别。
    auto get_level_string(Level level) -> std::string_view;
    auto get_current_time() -> std::string;
};

/**
 * @brief 一个简单的线程安全的唯一ID生成器。
 * @details 使用原子计数器来保证在多线程环境下每次调用 `next_id` 都返回一个唯一的ID。
 * @note ID 从 1 开始计数。
 */
class IDGenerator {
  public:
    using ID = std::uint64_t; ///< 定义ID的类型为64位无符号整数。

    /**
     * @brief 获取下一个唯一的ID。
     * @return 返回一个新的唯一ID。
     * @threadsafe 线程安全。
     */
    static auto next_id() -> ID;

    /**
     * @brief 重置ID生成器。
     * @details 将内部计数器重置为初始值 1。
     * @warning 这个函数不是线程安全的，只应在测试或单线程初始化阶段使用。
     */
    static auto reset() -> void;
};

/**
 * @brief 打印软件基本信息
 * @details 在控制台输出软件名称、版本号和版权信息
 * 
 * @post 软件信息被输出到标准输出
 */
void print_software_info();

/**
 * @brief 打印软件的ASCII艺术Logo
 * @details 在控制台输出一个由ASCII字符组成的Logo，用于启动时展示
 * 
 * @post Logo被输出到标准输出
 */
void print_logo();
}  // namespace fq::common
