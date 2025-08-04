/**
 * @file test_error_handling.cpp
 * @brief 错误处理系统测试
 * @details 测试异常层次结构、错误恢复机制和日志系统的功能
 * 
 * @author FastQTools Team
 * @date 2024
 * @version 1.0
 * 
 * @copyright Copyright (c) 2024 FastQTools
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "modules/error/exception_hierarchy.h"
#include "modules/error/error_recovery.h"
#include "modules/error/error_logger.h"
#include "modules/error/exception_macros.h"

using namespace ::testing;
using namespace fq::error;

class ErrorHandlingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 在每个测试前设置
        ErrorLogger::get_instance().set_log_level(LogLevel::Debug);
    }

    void TearDown() override {
        // 在每个测试后清理
        ErrorLogger::get_instance().clear_old_entries(std::chrono::hours(0));
    }
};

// 测试基础异常类
TEST_F(ErrorHandlingTest, FastQException_BasicFunctionality) {
    // 测试构造函数
    FastQException ex("Test error message", ErrorCode::Unknown, ErrorSeverity::Error);
    
    EXPECT_STREQ(ex.what(), "Test error message");
    EXPECT_EQ(ex.get_error_code(), ErrorCode::Unknown);
    EXPECT_EQ(ex.get_severity(), ErrorSeverity::Error);
    EXPECT_TRUE(ex.is_recoverable());
    
    // 测试上下文添加
    ex.add_context("test_key", "test_value");
    ex.add_context("test_number", 42);
    
    auto context = ex.get_context();
    EXPECT_TRUE(context.contains("test_key"));
    EXPECT_TRUE(context.contains("test_number"));
    
    auto string_value = context.get_string("test_key");
    ASSERT_TRUE(string_value.has_value());
    EXPECT_EQ(*string_value, "test_value");
    
    auto number_value = context.get_int("test_number");
    ASSERT_TRUE(number_value.has_value());
    EXPECT_EQ(*number_value, 42);
}

// 测试 IO 异常
TEST_F(ErrorHandlingTest, IoException_Functionality) {
    IoException ex("test.txt", ENOENT, "read");
    
    EXPECT_EQ(ex.get_file_path(), "test.txt");
    EXPECT_EQ(ex.get_system_error_code(), ENOENT);
    EXPECT_EQ(ex.get_operation(), "read");
    
    // 测试系统错误消息
    auto system_msg = ex.get_system_error_message();
    EXPECT_FALSE(system_msg.empty());
    
    // 测试克隆
    auto cloned = ex.clone();
    ASSERT_NE(cloned, nullptr);
    
    auto io_cloned = dynamic_cast<IoException*>(cloned.get());
    ASSERT_NE(io_cloned, nullptr);
    EXPECT_EQ(io_cloned->get_file_path(), "test.txt");
}

// 测试配置异常
TEST_F(ErrorHandlingTest, ConfigurationException_Functionality) {
    ConfigurationException ex("timeout", "invalid", "Must be a positive integer");
    
    EXPECT_EQ(ex.get_config_key(), "timeout");
    EXPECT_EQ(ex.get_config_value(), "invalid");
    EXPECT_EQ(ex.get_reason(), "Must be a positive integer");
    
    // 测试克隆
    auto cloned = ex.clone();
    ASSERT_NE(cloned, nullptr);
    
    auto config_cloned = dynamic_cast<ConfigurationException*>(cloned.get());
    ASSERT_NE(config_cloned, nullptr);
    EXPECT_EQ(config_cloned->get_config_key(), "timeout");
}

// 测试验证异常
TEST_F(ErrorHandlingTest, ValidationException_Functionality) {
    ValidationException ex("quality", "abc", "Must be a numeric value");
    
    EXPECT_EQ(ex.get_field_name(), "quality");
    EXPECT_EQ(ex.get_field_value(), "abc");
    EXPECT_EQ(ex.get_validation_rule(), "Must be a numeric value");
    
    // 测试带期望值的验证异常
    ValidationException ex2("length", "150", "200", "Must be between 100 and 200");
    EXPECT_EQ(ex2.get_expected_value(), "200");
    
    // 测试克隆
    auto cloned = ex.clone();
    ASSERT_NE(cloned, nullptr);
    
    auto validation_cloned = dynamic_cast<ValidationException*>(cloned.get());
    ASSERT_NE(validation_cloned, nullptr);
    EXPECT_EQ(validation_cloned->get_field_name(), "quality");
}

// 测试处理异常
TEST_F(ErrorHandlingTest, ProcessingException_Functionality) {
    ProcessingException ex("filtering", 1000, 5, "Quality threshold not met");
    
    EXPECT_EQ(ex.get_operation(), "filtering");
    EXPECT_EQ(ex.get_processed_count(), 1000);
    EXPECT_EQ(ex.get_failed_count(), 5);
    EXPECT_EQ(ex.get_success_rate(), 1000.0 / 1005.0);
    EXPECT_EQ(ex.get_details(), "Quality threshold not met");
    
    // 测试克隆
    auto cloned = ex.clone();
    ASSERT_NE(cloned, nullptr);
    
    auto processing_cloned = dynamic_cast<ProcessingException*>(cloned.get());
    ASSERT_NE(processing_cloned, nullptr);
    EXPECT_EQ(processing_cloned->get_operation(), "filtering");
}

// 测试内存异常
TEST_F(ErrorHandlingTest, MemoryException_Functionality) {
    MemoryException ex(1024 * 1024, 512 * 1024, "buffer allocation");
    
    EXPECT_EQ(ex.get_requested_size(), 1024 * 1024);
    EXPECT_EQ(ex.get_available_size(), 512 * 1024);
    EXPECT_EQ(ex.get_allocation_type(), "buffer allocation");
    EXPECT_TRUE(ex.is_allocation_error());
    EXPECT_FALSE(ex.is_access_error());
    
    // 测试内存访问异常
    MemoryException ex2(0xdeadbeef, "read", "Invalid address");
    EXPECT_EQ(ex2.get_access_address(), 0xdeadbeef);
    EXPECT_EQ(ex2.get_access_type(), "read");
    EXPECT_FALSE(ex2.is_allocation_error());
    EXPECT_TRUE(ex2.is_access_error());
    
    // 测试克隆
    auto cloned = ex.clone();
    ASSERT_NE(cloned, nullptr);
    
    auto memory_cloned = dynamic_cast<MemoryException*>(cloned.get());
    ASSERT_NE(memory_cloned, nullptr);
    EXPECT_EQ(memory_cloned->get_requested_size(), 1024 * 1024);
}

// 测试并发异常
TEST_F(ErrorHandlingTest, ConcurrencyException_Functionality) {
    ConcurrencyException ex("thread_creation", "thread_pool", 8);
    
    EXPECT_EQ(ex.get_operation(), "thread_creation");
    EXPECT_EQ(ex.get_resource_name(), "thread_pool");
    EXPECT_EQ(ex.get_thread_count(), 8);
    EXPECT_FALSE(ex.is_deadlock());
    
    // 测试死锁异常
    std::vector<std::string> locks = {"lock1", "lock2", "lock3"};
    std::vector<std::string> threads = {"thread1", "thread2"};
    ConcurrencyException ex2(locks, threads);
    
    EXPECT_TRUE(ex2.is_deadlock());
    EXPECT_EQ(ex2.get_lock_sequence(), locks);
    EXPECT_EQ(ex2.get_thread_ids(), threads);
    
    // 测试克隆
    auto cloned = ex.clone();
    ASSERT_NE(cloned, nullptr);
    
    auto concurrency_cloned = dynamic_cast<ConcurrencyException*>(cloned.get());
    ASSERT_NE(concurrency_cloned, nullptr);
    EXPECT_EQ(concurrency_cloned->get_operation(), "thread_creation");
}

// 测试网络异常
TEST_F(ErrorHandlingTest, NetworkException_Functionality) {
    NetworkException ex("example.com", 8080, "connect", ECONNREFUSED);
    
    EXPECT_EQ(ex.get_host(), "example.com");
    EXPECT_EQ(ex.get_port(), 8080);
    EXPECT_EQ(ex.get_operation(), "connect");
    EXPECT_EQ(ex.get_network_error_code(), ECONNREFUSED);
    
    // 测试克隆
    auto cloned = ex.clone();
    ASSERT_NE(cloned, nullptr);
    
    auto network_cloned = dynamic_cast<NetworkException*>(cloned.get());
    ASSERT_NE(network_cloned, nullptr);
    EXPECT_EQ(network_cloned->get_host(), "example.com");
}

// 测试错误恢复机制
TEST_F(ErrorHandlingTest, ErrorRecovery_Functionality) {
    ErrorRecoveryHandler handler;
    
    // 测试重试策略
    handler.set_retry_policy(ErrorCode::FileNotFound, 3, std::chrono::milliseconds(100));
    
    // 测试恢复处理器注册
    auto recovery_func = [](const FastQException& ex) -> RecoveryResult {
        if (ex.get_error_code() == ErrorCode::FileNotFound) {
            return RecoveryResult::Skipped;
        }
        return RecoveryResult::Failed;
    };
    
    handler.register_handler(ErrorCode::FileNotFound, recovery_func);
    
    // 测试恢复
    FastQException ex("File not found", ErrorCode::FileNotFound, ErrorSeverity::Error);
    auto result = handler.try_recover(ex);
    EXPECT_EQ(result, RecoveryResult::Skipped);
    
    // 测试统计
    auto stats = handler.get_recovery_stats();
    EXPECT_EQ(stats.total_attempts, 1);
    EXPECT_EQ(stats.skipped_operations, 1);
}

// 测试错误日志系统
TEST_F(ErrorHandlingTest, ErrorLogger_Functionality) {
    auto& logger = ErrorLogger::get_instance();
    
    // 测试日志记录
    logger.log_message(LogLevel::Error, "test", "Test error message");
    
    // 测试异常记录
    FastQException ex("Test exception", ErrorCode::Unknown, ErrorSeverity::Error);
    logger.log_exception(ex);
    
    // 测试带上下文的日志记录
    ErrorContext context;
    context.add("key1", "value1");
    context.add("key2", 42);
    logger.log_message(LogLevel::Warning, "test", "Warning with context", context);
    
    // 测试获取最近的日志条目
    auto entries = logger.get_recent_entries(10);
    EXPECT_GE(entries.size(), 3);
    
    // 测试按类别获取日志条目
    auto test_entries = logger.get_entries_by_category("test");
    EXPECT_GE(test_entries.size(), 1);
    
    // 测试错误统计
    auto stats = logger.get_error_statistics();
    EXPECT_GE(stats.total_errors, 2);
}

// 测试异常宏
TEST_F(ErrorHandlingTest, ExceptionMacros_Functionality) {
    // 测试基本异常抛出
    EXPECT_THROW(
        FQ_THROW(FastQException, "Test exception"),
        FastQException
    );
    
    // 测试 IO 异常宏
    EXPECT_THROW(
        FQ_THROW_FILE_NOT_FOUND("nonexistent.txt"),
        IoException
    );
    
    // 测试配置异常宏
    EXPECT_THROW(
        FQ_THROW_MISSING_CONFIG("required_key"),
        ConfigurationException
    );
    
    // 测试验证异常宏
    EXPECT_THROW(
        FQ_THROW_VALIDATION_ERROR("field", "value", "rule"),
        ValidationException
    );
    
    // 测试检查宏
    EXPECT_THROW(
        FQ_CHECK(false, "Condition failed"),
        FastQException
    );
    
    // 测试范围检查宏
    EXPECT_THROW(
        FQ_CHECK_RANGE(150, 0, 100, "test_value"),
        ValidationException
    );
    
    // 测试空指针检查宏
    EXPECT_THROW(
        FQ_CHECK_NOT_NULL(nullptr, "test_ptr"),
        ValidationException
    );
}

// 测试错误代码系统
TEST_F(ErrorHandlingTest, ErrorCodes_Functionality) {
    // 测试错误代码枚举值
    EXPECT_EQ(static_cast<int>(ErrorCode::Unknown), 1000);
    EXPECT_EQ(static_cast<int>(ErrorCode::FileNotFound), 2001);
    EXPECT_EQ(static_cast<int>(ErrorCode::InvalidConfig), 3001);
    EXPECT_EQ(static_cast<int>(ErrorCode::ValidationFailed), 4005);
    EXPECT_EQ(static_cast<int>(ErrorCode::DataCorrupted), 5001);
    EXPECT_EQ(static_cast<int>(ErrorCode::ProcessingFailed), 6001);
    EXPECT_EQ(static_cast<int>(ErrorCode::MemoryAllocationFailed), 7001);
    EXPECT_EQ(static_cast<int>(ErrorCode::DeadlockDetected), 8001);
    
    // 测试错误严重程度
    EXPECT_EQ(static_cast<int>(ErrorSeverity::Info), 0);
    EXPECT_EQ(static_cast<int>(ErrorSeverity::Warning), 1);
    EXPECT_EQ(static_cast<int>(ErrorSeverity::Error), 2);
    EXPECT_EQ(static_cast<int>(ErrorSeverity::Critical), 3);
    EXPECT_EQ(static_cast<int>(ErrorSeverity::Fatal), 4);
}

// 测试错误上下文
TEST_F(ErrorHandlingTest, ErrorContext_Functionality) {
    ErrorContext context;
    
    // 测试添加各种类型的值
    context.add("string_value", "test_string");
    context.add("int_value", 42);
    context.add("size_value", size_t(1024));
    context.add("double_value", 3.14);
    context.add("bool_value", true);
    
    // 测试获取值
    auto string_val = context.get_string("string_value");
    ASSERT_TRUE(string_val.has_value());
    EXPECT_EQ(*string_val, "test_string");
    
    auto int_val = context.get_int("int_value");
    ASSERT_TRUE(int_val.has_value());
    EXPECT_EQ(*int_val, 42);
    
    auto size_val = context.get_size("size_value");
    ASSERT_TRUE(size_val.has_value());
    EXPECT_EQ(*size_val, 1024);
    
    auto double_val = context.get_double("double_value");
    ASSERT_TRUE(double_val.has_value());
    EXPECT_DOUBLE_EQ(*double_val, 3.14);
    
    auto bool_val = context.get_bool("bool_value");
    ASSERT_TRUE(bool_val.has_value());
    EXPECT_TRUE(*bool_val);
    
    // 测试键的获取
    auto keys = context.get_keys();
    EXPECT_EQ(keys.size(), 5);
    
    // 测试格式化
    auto formatted = context.format();
    EXPECT_FALSE(formatted.empty());
    EXPECT_THAT(formatted, HasSubstr("string_value"));
    EXPECT_THAT(formatted, HasSubstr("test_string"));
}

// 测试错误恢复策略构建器
TEST_F(ErrorHandlingTest, RecoveryStrategyBuilder_Functionality) {
    // 测试重试策略
    auto retry_strategy = RecoveryStrategyBuilder()
        .on_error(ErrorCode::FileNotFound)
        .retry(3, std::chrono::milliseconds(100))
        .build();
    
    FastQException ex("File not found", ErrorCode::FileNotFound, ErrorSeverity::Error);
    auto result = retry_strategy(ex);
    EXPECT_EQ(result, RecoveryResult::Retrying);
    
    // 测试跳过策略
    auto skip_strategy = RecoveryStrategyBuilder()
        .on_error(ErrorCode::DataCorrupted)
        .skip()
        .build();
    
    FastQException ex2("Data corrupted", ErrorCode::DataCorrupted, ErrorSeverity::Error);
    result = skip_strategy(ex2);
    EXPECT_EQ(result, RecoveryResult::Skipped);
    
    // 测试使用默认值策略
    auto default_strategy = RecoveryStrategyBuilder()
        .on_error(ErrorCode::MissingConfig)
        .use_default_value()
        .build();
    
    FastQException ex3("Missing config", ErrorCode::MissingConfig, ErrorSeverity::Error);
    result = default_strategy(ex3);
    EXPECT_EQ(result, RecoveryResult::Success);
    
    // 测试中止策略
    auto abort_strategy = RecoveryStrategyBuilder()
        .on_severity(ErrorSeverity::Fatal)
        .abort()
        .build();
    
    FastQException ex4("Fatal error", ErrorCode::Unknown, ErrorSeverity::Fatal);
    result = abort_strategy(ex4);
    EXPECT_EQ(result, RecoveryResult::Aborted);
}

// 测试预定义恢复策略
TEST_F(ErrorHandlingTest, PredefinedRecoveryStrategies_Functionality) {
    // 测试文件读取重试策略
    auto file_retry_strategy = RecoveryStrategies::file_read_retry_strategy();
    FastQException ex("File not found", ErrorCode::FileNotFound, ErrorSeverity::Error);
    auto result = file_retry_strategy(ex);
    EXPECT_EQ(result, RecoveryResult::Retrying);
    
    // 测试记录跳过策略
    auto record_skip_strategy = RecoveryStrategies::record_skip_strategy();
    FastQException ex2("Data corrupted", ErrorCode::DataCorrupted, ErrorSeverity::Error);
    result = record_skip_strategy(ex2);
    EXPECT_EQ(result, RecoveryResult::Skipped);
    
    // 测试配置使用默认值策略
    auto config_default_strategy = RecoveryStrategies::config_use_default_strategy();
    FastQException ex3("Missing config", ErrorCode::MissingConfig, ErrorSeverity::Error);
    result = config_default_strategy(ex3);
    EXPECT_EQ(result, RecoveryResult::Success);
    
    // 测试配置中止策略
    auto config_abort_strategy = RecoveryStrategies::config_abort_strategy();
    FastQException ex4("Invalid config", ErrorCode::InvalidConfig, ErrorSeverity::Error);
    result = config_abort_strategy(ex4);
    EXPECT_EQ(result, RecoveryResult::Aborted);
}

// 测试全局恢复处理器
TEST_F(ErrorHandlingTest, GlobalRecoveryHandler_Functionality) {
    auto& global_handler = get_global_recovery_handler();
    
    // 测试全局恢复
    FastQException ex("File not found", ErrorCode::FileNotFound, ErrorSeverity::Error);
    auto result = try_recover_from_error(ex);
    // 应该返回重试结果，因为文件读取重试策略是默认注册的
    EXPECT_EQ(result, RecoveryResult::Retrying);
    
    // 测试获取全局统计
    auto stats = get_recovery_statistics();
    EXPECT_GE(stats.total_attempts, 1);
}

// 测试异常工厂函数
TEST_F(ErrorHandlingTest, ExceptionFactory_Functionality) {
    // 测试 IO 异常工厂
    auto io_ex = create_io_exception("test.txt", ENOENT, "read");
    ASSERT_NE(io_ex, nullptr);
    EXPECT_EQ(io_ex->get_error_code(), ErrorCode::FileNotFound);
    
    auto io_ptr = dynamic_cast<IoException*>(io_ex.get());
    ASSERT_NE(io_ptr, nullptr);
    EXPECT_EQ(io_ptr->get_file_path(), "test.txt");
    
    // 测试配置异常工厂
    auto config_ex = create_config_exception("timeout", "invalid", "Must be numeric");
    ASSERT_NE(config_ex, nullptr);
    EXPECT_EQ(config_ex->get_error_code(), ErrorCode::InvalidConfig);
    
    auto config_ptr = dynamic_cast<ConfigurationException*>(config_ex.get());
    ASSERT_NE(config_ptr, nullptr);
    EXPECT_EQ(config_ptr->get_config_key(), "timeout");
    
    // 测试验证异常工厂
    auto validation_ex = create_validation_exception("quality", "abc", "Must be numeric");
    ASSERT_NE(validation_ex, nullptr);
    EXPECT_EQ(validation_ex->get_error_code(), ErrorCode::ValidationFailed);
    
    auto validation_ptr = dynamic_cast<ValidationException*>(validation_ex.get());
    ASSERT_NE(validation_ptr, nullptr);
    EXPECT_EQ(validation_ptr->get_field_name(), "quality");
    
    // 测试处理异常工厂
    auto processing_ex = create_processing_exception("filter", 1000, 5, "Quality check failed");
    ASSERT_NE(processing_ex, nullptr);
    EXPECT_EQ(processing_ex->get_error_code(), ErrorCode::ProcessingFailed);
    
    auto processing_ptr = dynamic_cast<ProcessingException*>(processing_ex.get());
    ASSERT_NE(processing_ptr, nullptr);
    EXPECT_EQ(processing_ptr->get_operation(), "filter");
    
    // 测试内存异常工厂
    auto memory_ex = create_memory_exception(1024, 512, "buffer");
    ASSERT_NE(memory_ex, nullptr);
    EXPECT_EQ(memory_ex->get_error_code(), ErrorCode::MemoryAllocationFailed);
    
    auto memory_ptr = dynamic_cast<MemoryException*>(memory_ex.get());
    ASSERT_NE(memory_ptr, nullptr);
    EXPECT_EQ(memory_ptr->get_requested_size(), 1024);
    
    // 测试并发异常工厂
    auto concurrency_ex = create_concurrency_exception("thread_creation", "pool", 8);
    ASSERT_NE(concurrency_ex, nullptr);
    EXPECT_EQ(concurrency_ex->get_error_code(), ErrorCode::ResourceBusy);
    
    auto concurrency_ptr = dynamic_cast<ConcurrencyException*>(concurrency_ex.get());
    ASSERT_NE(concurrency_ptr, nullptr);
    EXPECT_EQ(concurrency_ptr->get_thread_count(), 8);
    
    // 测试网络异常工厂
    auto network_ex = create_network_exception("example.com", 8080, "connect", ECONNREFUSED);
    ASSERT_NE(network_ex, nullptr);
    EXPECT_EQ(network_ex->get_error_code(), ErrorCode::NetworkError);
    
    auto network_ptr = dynamic_cast<NetworkException*>(network_ex.get());
    ASSERT_NE(network_ptr, nullptr);
    EXPECT_EQ(network_ptr->get_host(), "example.com");
}

// 测试异常转换
TEST_F(ErrorHandlingTest, ExceptionConversion_Functionality) {
    // 测试标准异常转换
    std::runtime_error std_ex("Standard exception");
    auto converted = convert_std_exception(std_ex);
    
    ASSERT_NE(converted, nullptr);
    EXPECT_EQ(converted->get_error_code(), ErrorCode::InternalError);
    EXPECT_STREQ(converted->what(), "Standard exception");
}

// 测试日志级别转换
TEST_F(ErrorHandlingTest, LogLevelConversion_Functionality) {
    // 测试日志级别到字符串的转换
    EXPECT_EQ(log_level_to_string(LogLevel::Debug), "DEBUG");
    EXPECT_EQ(log_level_to_string(LogLevel::Info), "INFO");
    EXPECT_EQ(log_level_to_string(LogLevel::Warning), "WARNING");
    EXPECT_EQ(log_level_to_string(LogLevel::Error), "ERROR");
    EXPECT_EQ(log_level_to_string(LogLevel::Critical), "CRITICAL");
    
    // 测试字符串到日志级别的转换
    EXPECT_EQ(string_to_log_level("DEBUG"), LogLevel::Debug);
    EXPECT_EQ(string_to_log_level("INFO"), LogLevel::Info);
    EXPECT_EQ(string_to_log_level("WARNING"), LogLevel::Warning);
    EXPECT_EQ(string_to_log_level("ERROR"), LogLevel::Error);
    EXPECT_EQ(string_to_log_level("CRITICAL"), LogLevel::Critical);
    EXPECT_EQ(string_to_log_level("UNKNOWN"), LogLevel::Info); // 默认值
}

// 测试用户友好的错误消息
TEST_F(ErrorHandlingTest, UserFriendlyMessages_Functionality) {
    // 测试基础异常的用户消息
    FastQException ex("Test error", ErrorCode::Unknown, ErrorSeverity::Error);
    auto user_msg = ex.get_user_message();
    
    EXPECT_THAT(user_msg, HasSubstr("错误: Test error"));
    EXPECT_THAT(user_msg, HasSubstr("(错误)"));
    
    // 测试 IO 异常的用户消息
    IoException io_ex("test.txt", ENOENT, "read");
    auto io_user_msg = io_ex.get_user_message();
    
    EXPECT_THAT(io_user_msg, HasSubstr("test.txt"));
    
    // 测试配置异常的用户消息
    ConfigurationException config_ex("timeout", "invalid", "Must be numeric");
    auto config_user_msg = config_ex.get_user_message();
    
    EXPECT_THAT(config_user_msg, HasSubstr("timeout"));
}

// 测试日志消息格式
TEST_F(ErrorHandlingTest, LogMessageFormat_Functionality) {
    // 测试基础异常的日志消息
    FastQException ex("Test error", ErrorCode::Unknown, ErrorSeverity::Error);
    auto log_msg = ex.get_log_message();
    
    EXPECT_THAT(log_msg, HasSubstr("Test error"));
    EXPECT_THAT(log_msg, HasSubstr("1000")); // 错误代码
    EXPECT_THAT(log_msg, HasSubstr("2"));   // 严重程度
    
    // 测试带上下文的日志消息
    ex.add_context("key1", "value1");
    ex.add_context("key2", 42);
    auto context_log_msg = ex.get_log_message();
    
    EXPECT_THAT(context_log_msg, HasSubstr("Context:"));
    EXPECT_THAT(context_log_msg, HasSubstr("key1"));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}