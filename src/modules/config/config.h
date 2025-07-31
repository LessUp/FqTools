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
    void load_from_file(const std::filesystem::path& config_file);
    void load_from_args(int argc, char* argv[]);
    void load_from_env();
    
    template <typename T>
    [[nodiscard]] auto get(std::string_view key) const -> T;
    
    template <typename T>
    [[nodiscard]] auto get_or(std::string_view key, T default_value) const -> T;

    template <typename T>
    void set(std::string_view key, T value);

    [[nodiscard]] auto has_key(std::string_view key) const -> bool;
    void validate() const;
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
