/**
 * @file error_context.h
 * @brief 错误上下文系统定义
 * @details 提供错误上下文信息的存储和检索功能
 * 
 * @author FastQTools Team
 * @date 2024
 * @version 1.0
 * 
 * @copyright Copyright (c) 2024 FastQTools
 * @license MIT License
 */

#pragma once

#include <string>
#include <map>
#include <variant>
#include <vector>
#include <optional>
#include <sstream>
#include <iomanip>

namespace fq::error {

// 错误上下文信息
class ErrorContext {
public:
    using ContextValue = std::variant<
        std::string,
        int,
        size_t,
        double,
        bool
    >;
    
    // 添加上下文信息
    auto add(const std::string& key, const ContextValue& value) -> void;
    auto add(const std::string& key, const std::string& value) -> void;
    auto add(const std::string& key, int value) -> void;
    auto add(const std::string& key, size_t value) -> void;
    auto add(const std::string& key, double value) -> void;
    auto add(const std::string& key, bool value) -> void;
    
    // 特殊版本，用于解决类型转换歧义
    auto add_time(const std::string& key, std::time_t value) -> void;
    auto add_line(const std::string& key, uint32_t value) -> void;
    
    // 获取上下文信息
    auto get(const std::string& key) const -> std::optional<ContextValue>;
    auto get_string(const std::string& key) const -> std::optional<std::string>;
    auto get_int(const std::string& key) const -> std::optional<int>;
    auto get_size(const std::string& key) const -> std::optional<size_t>;
    auto get_double(const std::string& key) const -> std::optional<double>;
    auto get_bool(const std::string& key) const -> std::optional<bool>;
    
    // 检查是否存在
    auto contains(const std::string& key) const -> bool;
    
    // 获取所有键
    auto get_keys() const -> std::vector<std::string>;
    
    // 清空上下文
    auto clear() -> void;
    
    // 格式化为字符串
    auto format() const -> std::string;
    
    // 合并另一个上下文
    auto merge(const ErrorContext& other) -> void;
    
    // 获取上下文大小
    auto size() const -> size_t;
    
    // 检查是否为空
    auto empty() const -> bool;

private:
    std::map<std::string, ContextValue> m_context;
    
    // 格式化单个值
    auto format_value(const ContextValue& value) const -> std::string;
};

// 错误上下文构建器
class ErrorContextBuilder {
public:
    auto add(const std::string& key, const ErrorContext::ContextValue& value) -> ErrorContextBuilder&;
    auto add_file_info(const std::string& file_path, int line_number) -> ErrorContextBuilder&;
    auto add_system_info(int error_code, const std::string& error_message) -> ErrorContextBuilder&;
    auto add_operation_info(const std::string& operation, const std::string& details) -> ErrorContextBuilder&;
    auto add_performance_info(size_t processed_count, double elapsed_time) -> ErrorContextBuilder&;
    
    auto build() const -> ErrorContext;

private:
    ErrorContext m_context;
};

// 常用上下文键
namespace ContextKeys {
    constexpr const char* FILE_PATH = "file_path";
    constexpr const char* LINE_NUMBER = "line_number";
    constexpr const char* FUNCTION_NAME = "function_name";
    constexpr const char* ERROR_CODE = "error_code";
    constexpr const char* ERROR_MESSAGE = "error_message";
    constexpr const char* OPERATION = "operation";
    constexpr const char* PROCESSED_COUNT = "processed_count";
    constexpr const char* FAILED_COUNT = "failed_count";
    constexpr const char* ELAPSED_TIME = "elapsed_time";
    constexpr const char* MEMORY_USAGE = "memory_usage";
    constexpr const char* THREAD_COUNT = "thread_count";
    constexpr const char* CONFIG_KEY = "config_key";
    constexpr const char* CONFIG_VALUE = "config_value";
    constexpr const char* BATCH_SIZE = "batch_size";
    constexpr const char* TOTAL_SIZE = "total_size";
    constexpr const char* CURRENT_SIZE = "current_size";
    constexpr const char* REMAINING_SIZE = "remaining_size";
    constexpr const char* PROGRESS_PERCENTAGE = "progress_percentage";
    constexpr const char* ESTIMATED_TIME = "estimated_time";
    constexpr const char* START_TIME = "start_time";
    constexpr const char* END_TIME = "end_time";
    constexpr const char* DURATION = "duration";
    constexpr const char* THROUGHPUT = "throughput";
    constexpr const char* ERROR_RATE = "error_rate";
    constexpr const char* SUCCESS_RATE = "success_rate";
    constexpr const char* AVERAGE_TIME = "average_time";
    constexpr const char* MIN_TIME = "min_time";
    constexpr const char* MAX_TIME = "max_time";
    constexpr const char* MEDIAN_TIME = "median_time";
    constexpr const char* STANDARD_DEVIATION = "standard_deviation";
    constexpr const char* PERCENTILE_95 = "percentile_95";
    constexpr const char* PERCENTILE_99 = "percentile_99";
    constexpr const char* PERCENTILE_999 = "percentile_999";
    constexpr const char* HOSTNAME = "hostname";
    constexpr const char* USERNAME = "username";
    constexpr const char* PROCESS_ID = "process_id";
    constexpr const char* THREAD_ID = "thread_id";
    constexpr const char* PARENT_PROCESS_ID = "parent_process_id";
    constexpr const char* WORKING_DIRECTORY = "working_directory";
    constexpr const char* COMMAND_LINE = "command_line";
    constexpr const char* EXECUTABLE_PATH = "executable_path";
    constexpr const char* LIBRARY_VERSION = "library_version";
    constexpr const char* COMPILER_VERSION = "compiler_version";
    constexpr const char* BUILD_TYPE = "build_type";
    constexpr const char* BUILD_DATE = "build_date";
    constexpr const char* BUILD_TIME = "build_time";
    constexpr const char* GIT_COMMIT = "git_commit";
    constexpr const char* GIT_BRANCH = "git_branch";
    constexpr const char* GIT_TAG = "git_tag";
    constexpr const char* SYSTEM_NAME = "system_name";
    constexpr const char* SYSTEM_VERSION = "system_version";
    constexpr const char* SYSTEM_ARCHITECTURE = "system_architecture";
    constexpr const char* CPU_MODEL = "cpu_model";
    constexpr const char* CPU_CORES = "cpu_cores";
    constexpr const char* MEMORY_TOTAL = "memory_total";
    constexpr const char* MEMORY_AVAILABLE = "memory_available";
    constexpr const char* MEMORY_USED = "memory_used";
    constexpr const char* MEMORY_FREE = "memory_free";
    constexpr const char* DISK_TOTAL = "disk_total";
    constexpr const char* DISK_AVAILABLE = "disk_available";
    constexpr const char* DISK_USED = "disk_used";
    constexpr const char* DISK_FREE = "disk_free";
    constexpr const char* NETWORK_UP = "network_up";
    constexpr const char* NETWORK_DOWN = "network_down";
    constexpr const char* NETWORK_LATENCY = "network_latency";
    constexpr const char* NETWORK_PACKET_LOSS = "network_packet_loss";
}

} // namespace fq::error