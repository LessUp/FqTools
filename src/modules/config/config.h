/**
 * @file config.h
 * @brief 提供了一个灵活的配置管理系统。
 * @author BGI-Research
 * @version 1.0
 * @date 2025-07-31
 * @copyright Copyright (c) 2025 BGI-Research
 */

#pragma once

#include <cstdint>
#include <filesystem>
#include <format>
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include "error.h"

namespace fq::config {

/// @brief 配置项的值类型，可以是 bool, int64, double, 或 string。
using ConfigValue = std::variant<bool, std::int64_t, double, std::string>;

/**
 * @brief 负责加载、管理和验证项目配置。
 * @details 可以从文件、命令行参数和环境变量中加载配置。
 */
class Configuration {
  public:
    Configuration() = default;
    /**
     * @brief 从配置文件加载配置
     * @details 从指定的配置文件中读取并解析配置参数
     * 
     * @param config_file 配置文件路径
     * @pre config_file 必须指向有效的配置文件
     * @post 配置参数被加载到实例中
     * @throw ConfigurationError 如果文件不存在或格式错误
     */
    void load_from_file(const std::filesystem::path& config_file);

    /**
     * @brief 从命令行参数加载配置
     * @details 解析命令行参数并加载相应的配置项
     * 
     * @param argc 参数个数
     * @param argv 参数数组
     * @pre argv 必须包含有效的命令行参数
     * @post 配置参数被加载到实例中
     * @throw ConfigurationError 如果参数格式错误
     */
    void load_from_args(int argc, char* argv[]);

    /**
     * @brief 从环境变量加载配置
     * @details 从系统环境变量中读取并加载配置参数
     * 
     * @post 配置参数被加载到实例中
     * @note 只会加载已定义的环境变量
     */
    void load_from_env();
    
    template <typename T>
    [[nodiscard]] auto get(std::string_view key) const -> T;
    
    template <typename T>
    [[nodiscard]] auto get_or(std::string_view key, T default_value) const -> T;

    template <typename T>
    void set(std::string_view key, T value);

    /**
     * @brief 检查配置键是否存在
     * @details 检查指定的配置键是否存在于当前配置中
     * 
     * @param key 配置键名
     * @return 存在返回 true，不存在返回 false
     */
    [[nodiscard]] auto has_key(std::string_view key) const -> bool;

    /**
     * @brief 验证配置参数
     * @details 验证所有配置参数的有效性，检查必需的配置项和参数范围
     * 
     * @post 配置参数被验证
     * @throw ConfigurationError 如果配置参数无效
     */
    void validate() const;

    /**
     * @brief 打印配置信息
     * @details 将当前配置信息格式化输出到指定的输出流
     * 
     * @param out 输出流
     * @post 配置信息被输出到指定流
     */
    void print_config(std::ostream& out) const;

  private:
    std::unordered_map<std::string, ConfigValue> m_values;
    std::unordered_map<char, std::string> m_short_to_long = {
        {'i', "input"}, {'o', "output"}, {'t', "threads"}, {'m', "memory_limit_mb"}, {'v', "verbose"}, {'h', "help"}};
    void validate_key(std::string_view key) const;
    void parse_config_stream(std::istream& stream);
    void set_from_string(const std::string& key, const std::string& value);
    auto get_long_name_for_short(char short_name) const -> std::string;
};

/**
 * @brief 获取全局配置实例
 * @details 获取全局配置管理器的单例实例
 * 
 * @return Configuration& 全局配置实例的引用
 * @note 如果未初始化，会自动创建默认配置的实例
 */
auto global_config() -> Configuration&;

template <typename T>
auto get_config(std::string_view key) -> T {
    return global_config().get<T>(key);
}
template <typename T>
auto get_config_or(std::string_view key, T default_value) -> T {
    return global_config().get_or<T>(key, default_value);
}
template <typename T>
void set_config(std::string_view key, T value) {
    global_config().set(key, value);
}
}  // namespace fq::config
