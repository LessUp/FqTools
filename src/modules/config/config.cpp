/**
 * @file config.cpp
 * @brief 实现了 Configuration 类的功能。
 * @author BGI-Research
 * @version 1.0
 * @date 2025-07-31
 * @copyright Copyright (c) 2025 BGI-Research
 */

#include "config.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <format>
#include <fstream>
#include <istream>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>

#include "common.h"
#include "error.h"

extern char** environ;

namespace fq::config {

void Configuration::load_from_file(const std::filesystem::path& config_file) {
    if (!std::filesystem::exists(config_file)) {
        FQ_THROW_CONFIG_ERROR(std::format("Configuration file '{}' does not exist", config_file.string()));
    }
    std::ifstream file(config_file);
    if (!file) {
        FQ_THROW_CONFIG_ERROR(std::format("Cannot open configuration file '{}'", config_file.string()));
    }
    parse_config_stream(file);
}

void Configuration::load_from_args(int argc, char* argv[]) {
    // 解析命令行参数。
    // 支持三种格式:
    // 1. --key=value
    // 2. --key (布尔值 true)
    // 3. -k value (短名称)
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg.starts_with("--")) {
            auto eq_pos = arg.find('=');
            if (eq_pos != std::string::npos) {
                auto key = arg.substr(2, eq_pos - 2);
                auto value = arg.substr(eq_pos + 1);
                set_from_string(key, value);
            } else {
                auto key = arg.substr(2);
                set(key, true);
            }
        } else if (arg.starts_with("-") && arg.length() == 2) {
            char short_name = arg[1];
            auto key = get_long_name_for_short(short_name);
            if (!key.empty()) {
                if (i + 1 < argc && !std::string(argv[i + 1]).starts_with("-")) {
                    set_from_string(key, argv[++i]);
                } else {
                    set(key, true);
                }
            }
        }
    }
}

void Configuration::load_from_env() {
    // 从环境变量加载配置。
    // 只加载以 "FASTQTOOLS_" 为前缀的变量，并将其余部分转换为小写作为键。
    const char* env_prefix = "FASTQTOOLS_";
    for (char** env = environ; *env != nullptr; ++env) {
        std::string env_var(*env);
        auto eq_pos = env_var.find('=');
        if (eq_pos != std::string::npos) {
            auto key = env_var.substr(0, eq_pos);
            auto value = env_var.substr(eq_pos + 1);
            if (key.starts_with(env_prefix)) {
                auto config_key = key.substr(std::strlen(env_prefix));
                std::transform(config_key.begin(), config_key.end(), config_key.begin(), ::tolower);
                set_from_string(config_key, value);
            }
        }
    }
}



void Configuration::validate() const {
    static const std::vector<std::string> required_keys = {"input", "output"};
    for (const auto& key : required_keys) {
        if (!has_key(key)) {
            FQ_THROW_CONFIG_ERROR(std::format("Required configuration key '{}' is missing", key));
        }
    }
    if (has_key("threads")) {
        auto threads = get_or<std::int64_t>("threads", 1);
        if (threads <= 0 || threads > 256) {
            FQ_THROW_CONFIG_ERROR("threads must be between 1 and 256");
        }
    }
    if (has_key("memory_limit_mb")) {
        auto memory = get_or<std::int64_t>("memory_limit_mb", 1024);
        if (memory <= 0 || memory > 1024 * 1024) {
            FQ_THROW_CONFIG_ERROR("memory_limit_mb must be between 1 and 1048576 MB");
        }
    }
}

void Configuration::print_config(std::ostream& out) const {
    out << "Current Configuration:\n=====================\n";
    for (const auto& [key, value] : m_values) {
        out << std::format("{:20}: ", key);
        std::visit(
            [&out](const auto& v) {
                using T = std::decay_t<decltype(v)>;
                if constexpr (std::is_same_v<T, bool>) {
                    out << (v ? "true" : "false");
                } else {
                    out << v;
                }
            },
            value);
        out << '\n';
    }
}

auto Configuration::has_key(std::string_view key) const -> bool {
    return m_values.find(std::string(key)) != m_values.end();
}

void Configuration::validate_key(std::string_view key) const {
    if (key.empty()) {
        FQ_THROW_CONFIG_ERROR("Configuration key cannot be empty");
    }
    for (char character : key) {
        if (!std::isalnum(character) && character != '_' && character != '-') {
            FQ_THROW_CONFIG_ERROR(std::format("Invalid character '{}' in configuration key '{}'", character, key));
        }
    }
}

void Configuration::parse_config_stream(std::istream& stream) {
    std::string line;
    int line_number = 0;
    while (std::getline(stream, line)) {
        ++line_number;
        // 忽略 '#' 之后的注释
        auto comment_pos = line.find('#');
        if (comment_pos != std::string::npos) {
            line = line.substr(0, comment_pos);
        }
        auto trimmed = fq::common::trim(line);
        if (trimmed.empty())
            continue;
        
        // 按 '=' 分割键值对
        auto eq_pos = trimmed.find('=');
        if (eq_pos == std::string::npos) {
            FQ_THROW_CONFIG_ERROR(std::format("Invalid configuration line {} (missing '='): {}", line_number, line));
        }
        auto key = fq::common::trim(trimmed.substr(0, eq_pos));
        auto value = fq::common::trim(trimmed.substr(eq_pos + 1));
        if (key.empty()) {
            FQ_THROW_CONFIG_ERROR(std::format("Empty key on line {}: {}", line_number, line));
        }
        set_from_string(key, value);
    }
}

void Configuration::set_from_string(const std::string& key, const std::string& value) {
    // 尝试根据字符串内容自动推断值的类型。
    // 推断顺序: 布尔值 -> 浮点数 -> 整数 -> 字符串。
    if (value == "true" || value == "1" || value == "yes" || value == "on") {
        set(key, true);
    } else if (value == "false" || value == "0" || value == "no" || value == "off") {
        set(key, false);
    } else if (value.find('.') != std::string::npos) {
        try {
            set(key, std::stod(value));
        } catch (const std::exception&) {
            set(key, value); // 转换失败，则视为字符串
        }
    } else {
        try {
            set(key, std::stoll(value));
        } catch (const std::exception&) {
            set(key, value); // 转换失败，则视为字符串
        }
    }
}

auto Configuration::get_long_name_for_short(char short_name) const -> std::string {
    auto iterator = m_short_to_long.find(short_name);
    return iterator != m_short_to_long.end() ? iterator->second : "";
}

auto global_config() -> Configuration& {
    static Configuration config;
    return config;
}

}  // namespace fq::config
