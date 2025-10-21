/**
 * @file exception_macros.h
 * @brief 异常处理宏定义
 * @details 提供便利的异常处理宏，简化异常抛出和处理代码
 * 
 * @author FastQTools Team
 * @date 2024
 * @version 1.0
 * 
 * @copyright Copyright (c) 2024 FastQTools
 * @license MIT License
 */

#pragma once

#include "exception_hierarchy.h"
#include "error_context.h"
#include <sstream>

// 基础异常抛出宏
#define FQ_THROW(exception_type, message) \
    throw exception_type(message, fq::error::ErrorCode::exception_type##Error)

// 带上下文的异常抛出宏
#define FQ_THROW_WITH_CONTEXT(exception_type, message, context) \
    do { \
        auto ex = exception_type(message, fq::error::ErrorCode::exception_type##Error); \
        ex.add_context(context); \
        throw ex; \
    } while(0)

// 带位置的异常抛出宏
#define FQ_THROW_AT(exception_type, message, code, severity) \
    throw exception_type(message, code, severity, std::source_location::current())

// IO 异常宏
#define FQ_THROW_IO_ERROR(file_path, system_error) \
    throw fq::error::IoException(file_path, system_error)

#define FQ_THROW_FILE_NOT_FOUND(file_path) \
    FQ_THROW_IO_ERROR(file_path, ENOENT)

#define FQ_THROW_PERMISSION_DENIED(file_path) \
    FQ_THROW_IO_ERROR(file_path, EACCES)

#define FQ_THROW_DISK_FULL(file_path) \
    FQ_THROW_IO_ERROR(file_path, ENOSPC)

// 配置异常宏
#define FQ_THROW_CONFIG_ERROR(key, value, reason) \
    throw fq::error::ConfigurationException(key, value, reason)

#define FQ_THROW_MISSING_CONFIG(key) \
    FQ_THROW_CONFIG_ERROR(key, "", "Missing required configuration")

#define FQ_THROW_INVALID_CONFIG(key, value, reason) \
    FQ_THROW_CONFIG_ERROR(key, value, reason)

#define FQ_THROW_CONFIG_OUT_OF_RANGE(key, value, min, max) \
    do { \
        std::ostringstream oss; \
        oss << "Value " << value << " is out of range [" << min << ", " << max << "]"; \
        FQ_THROW_CONFIG_ERROR(key, std::to_string(value), oss.str()); \
    } while(0)

// 验证异常宏
#define FQ__FQ_VA_SELECT(_1,_2,_3,NAME,...) NAME
#define FQ_THROW_VALIDATION_ERROR2(field, value) \
    throw fq::error::ValidationException(field, value, "Validation error")
#define FQ_THROW_VALIDATION_ERROR3(field, value, rule) \
    throw fq::error::ValidationException(field, value, rule)
#define FQ_THROW_VALIDATION_ERROR(...) \
    FQ__FQ_VA_SELECT(__VA_ARGS__, FQ_THROW_VALIDATION_ERROR3, FQ_THROW_VALIDATION_ERROR2)(__VA_ARGS__)

#define FQ_THROW_VALIDATION_ERROR_EXPECTED(field, value, expected, rule) \
    throw fq::error::ValidationException(field, value, expected, rule)

#define FQ_THROW_INVALID_PARAMETER(name, value) \
    FQ_THROW_VALIDATION_ERROR(name, value, "Invalid parameter")

#define FQ_THROW_MISSING_REQUIRED_FIELD(name) \
    FQ_THROW_VALIDATION_ERROR(name, "", "Missing required field")

#define FQ_THROW_DATA_CORRUPTED(field, value) \
    FQ_THROW_VALIDATION_ERROR(field, value, "Data corrupted")

// 处理异常宏
#define FQ_THROW_PROCESSING_ERROR(operation, processed, failed, details) \
    throw fq::error::ProcessingException(operation, processed, failed, details)

#define FQ_THROW_PROCESSING_TIMEOUT(operation, timeout_ms) \
    do { \
        std::ostringstream oss; \
        oss << "Operation timed out after " << timeout_ms << "ms"; \
        FQ_THROW_PROCESSING_ERROR(operation, 0, 0, oss.str()); \
    } while(0)

#define FQ_THROW_PROCESSING_INTERRUPTED(operation, reason) \
    FQ_THROW_PROCESSING_ERROR(operation, 0, 0, reason)

// 内存异常宏
#define FQ_THROW_MEMORY_ERROR(requested, available, type) \
    throw fq::error::MemoryException(requested, available, type)

#define FQ_THROW_MEMORY_ALLOCATION_FAILED(size, type) \
    FQ_THROW_MEMORY_ERROR(size, 0, type)

#define FQ_THROW_MEMORY_OUT_OF_MEMORY(size, type) \
    FQ_THROW_MEMORY_ERROR(size, 0, type)

#define FQ_THROW_MEMORY_ACCESS_VIOLATION(address, access_type, reason) \
    throw fq::error::MemoryException(address, access_type, reason)

// 并发异常宏
#define FQ_THROW_CONCURRENCY_ERROR(operation, resource, threads) \
    throw fq::error::ConcurrencyException(operation, resource, threads)

#define FQ_THROW_DEADLOCK_DETECTED(lock_sequence, thread_ids) \
    throw fq::error::ConcurrencyException(lock_sequence, thread_ids)

#define FQ_THROW_RESOURCE_BUSY(resource, operation) \
    FQ_THROW_CONCURRENCY_ERROR(operation, resource, 0)

#define FQ_THROW_THREAD_CREATION_FAILED(count, reason) \
    do { \
        std::ostringstream oss; \
        oss << "Failed to create " << count << " threads: " << reason; \
        FQ_THROW_CONCURRENCY_ERROR("thread_creation", "thread_pool", count); \
    } while(0)

// 网络异常宏
#define FQ_THROW_NETWORK_ERROR(host, port, operation, error_code) \
    throw fq::error::NetworkException(host, port, operation, error_code)

#define FQ_THROW_CONNECTION_FAILED(host, port, error_code) \
    FQ_THROW_NETWORK_ERROR(host, port, "connect", error_code)

#define FQ_THROW_NETWORK_TIMEOUT(host, port, operation, timeout_ms) \
    do { \
        std::ostringstream oss; \
        oss << "Network operation " << operation << " timed out after " << timeout_ms << "ms"; \
        FQ_THROW_NETWORK_ERROR(host, port, operation, ETIMEDOUT); \
    } while(0)

// 错误检查宏
#define FQ_CHECK(condition, message) \
    do { \
        if (!(condition)) { \
            FQ_THROW(fq::error::FastQException, message); \
        } \
    } while(0)

#define FQ_CHECK_IO(condition, file_path, error_code) \
    do { \
        if (!(condition)) { \
            FQ_THROW_IO_ERROR(file_path, error_code); \
        } \
    } while(0)

#define FQ_CHECK_VALID(condition, field, value, rule) \
    do { \
        if (!(condition)) { \
            FQ_THROW_VALIDATION_ERROR(field, value, rule); \
        } \
    } while(0)

#define FQ_CHECK_RANGE(value, min, max, name) \
    do { \
        if ((value) < (min) || (value) > (max)) { \
            std::ostringstream oss; \
            oss << "Value " << (value) << " is out of range [" << (min) << ", " << (max) << "]"; \
            FQ_THROW_VALIDATION_ERROR(name, std::to_string(value), oss.str()); \
        } \
    } while(0)

#define FQ_CHECK_NOT_NULL(ptr, name) \
    do { \
        if ((ptr) == nullptr) { \
            FQ_THROW_VALIDATION_ERROR(name, "null", "Pointer cannot be null"); \
        } \
    } while(0)

#define FQ_CHECK_NOT_EMPTY(str, name) \
    do { \
        if ((str).empty()) { \
            FQ_THROW_VALIDATION_ERROR(name, "", "String cannot be empty"); \
        } \
    } while(0)

// 带错误代码的检查宏
#define FQ_CHECK_CODE(condition, code, message) \
    do { \
        if (!(condition)) { \
            throw fq::error::FastQException(message, code); \
        } \
    } while(0)

#define FQ_CHECK_SEVERITY(condition, code, severity, message) \
    do { \
        if (!(condition)) { \
            throw fq::error::FastQException(message, code, severity); \
        } \
    } while(0)

// 函数异常处理宏
#define FQ_TRY_BEGIN \
    try {

#define FQ_TRY_END \
    } catch (const fq::error::FastQException& ex) { \
        FQ_LOG_EXCEPTION(ex); \
        throw; \
    } catch (const std::exception& ex) { \
        FQ_LOG_ERROR("exception", std::string("Standard exception: ") + ex.what()); \
        throw fq::error::FastQException(ex.what()); \
    } catch (...) { \
        FQ_LOG_ERROR("exception", "Unknown exception occurred"); \
        throw fq::error::FastQException("Unknown exception"); \
    }

#define FQ_TRY(function_call) \
    FQ_TRY_BEGIN \
        function_call; \
    FQ_TRY_END

// 带异常处理的函数调用
#define FQ_TRY_OR_DEFAULT(function_call, default_value) \
    ([&]() -> decltype(function_call) { \
        FQ_TRY_BEGIN \
            return function_call; \
        FQ_TRY_END \
        return default_value; \
    })()

#define FQ_TRY_OR_LOG(function_call, log_message) \
    ([&]() -> decltype(function_call) { \
        FQ_TRY_BEGIN \
            return function_call; \
        FQ_TRY_END \
        FQ_LOG_ERROR("exception", log_message); \
        if constexpr (!std::is_void_v<decltype(function_call)>) { \
            return decltype(function_call){}; \
        } \
    })()

// 返回值包装宏
#define FQ_RETURN_IF_ERROR(expression) \
    do { \
        auto result = expression; \
        if (!result.is_success()) { \
            return result.error(); \
        } \
    } while(0)

#define FQ_RETURN_OR_THROW(expression) \
    do { \
        auto result = expression; \
        if (!result.is_success()) { \
            throw fq::error::FastQException(result.error().message()); \
        } \
        return result.value(); \
    } while(0)

// 断言宏（调试模式）
#ifdef NDEBUG
#define FQ_ASSERT(condition, message) ((void)0)
#define FQ_ASSERT_NOT_NULL(ptr, name) ((void)0)
#define FQ_ASSERT_RANGE(value, min, max, name) ((void)0)
#else
#define FQ_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            std::ostringstream oss; \
            oss << "Assertion failed: " << message << " at " << __FILE__ << ":" << __LINE__; \
            FQ_THROW(fq::error::FastQException, oss.str()); \
        } \
    } while(0)

#define FQ_ASSERT_NOT_NULL(ptr, name) \
    FQ_ASSERT(ptr != nullptr, std::string("Null pointer assertion failed: ") + name)

#define FQ_ASSERT_RANGE(value, min, max, name) \
    FQ_ASSERT((value) >= (min) && (value) <= (max), \
              std::string("Range assertion failed: ") + name + " value " + std::to_string(value) + \
              " not in range [" + std::to_string(min) + ", " + std::to_string(max) + "]")
#endif

// 日志记录宏（带异常）
#define FQ_LOG_IF_EXCEPTION(operation, category) \
    do { \
        try { \
            operation; \
        } catch (const fq::error::FastQException& ex) { \
            FQ_LOG_EXCEPTION(ex); \
            FQ_LOG_ERROR(category, std::string("Operation failed: ") + ex.what()); \
            throw; \
        } catch (const std::exception& ex) { \
            FQ_LOG_ERROR(category, std::string("Operation failed: ") + ex.what()); \
            throw; \
        } catch (...) { \
            FQ_LOG_ERROR(category, "Operation failed with unknown exception"); \
            throw; \
        } \
    } while(0)

// 性能监控宏
#define FQ_TIME_OPERATION(operation, name) \
    ([&]() -> auto { \
        auto start = std::chrono::high_resolution_clock::now(); \
        auto result = operation; \
        auto end = std::chrono::high_resolution_clock::now(); \
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start); \
        FQ_LOG_INFO("performance", name + " took " + std::to_string(duration.count()) + "ms"); \
        return result; \
    })()

// 资源管理宏
#define FQ_GUARD(resource, cleanup) \
    std::unique_ptr<void, std::function<void()>> resource##_guard( \
        reinterpret_cast<void*>(0x1), \
        [&](void*) { cleanup; } \
    ); (void)resource##_guard

// 上下文作用域宏
#define FQ_CONTEXT_SCOPE(key, value) \
    fq::error::ContextScopeGuard context_scope_##key(key, value)

// 条件编译宏
#if defined(__GNUC__) || defined(__clang__)
#define FQ_LIKELY(x) __builtin_expect((x), 1)
#define FQ_UNLIKELY(x) __builtin_expect((x), 0)
#else
#define FQ_LIKELY(x) (x)
#define FQ_UNLIKELY(x) (x)
#endif

// 分支预测宏
#define FQ_IF_LIKELY(condition) if (FQ_LIKELY(condition))
#define FQ_IF_UNLIKELY(condition) if (FQ_UNLIKELY(condition))

// 编译时检查宏
#define FQ_STATIC_ASSERT(condition, message) \
    static_assert(condition, message)

#define FQ_STATIC_ASSERT_MSG(condition, message) \
    static_assert(condition, message)

// 类型特征检查宏
#define FQ_IS_SAME(T1, T2) std::is_same_v<T1, T2>
#define FQ_IS_BASE_OF(Base, Derived) std::is_base_of_v<Base, Derived>
#define FQ_IS_CONVERTIBLE(From, To) std::is_convertible_v<From, To>

// 容器操作宏
#define FQ_FOR_EACH(container, element) \
    for (auto& element : container)

#define FQ_FOR_EACH_CONST(container, element) \
    for (const auto& element : container)

#define FQ_FOR_EACH_INDEXED(container, element, index) \
    for (size_t index = 0; index < (container).size(); ++index) \
        for (auto& element = (container)[index]; index < (container).size(); ++index)

// 字符串操作宏
#define FQ_TO_STRING(value) std::to_string(value)
#define FQ_STRING_JOIN(values, delimiter) \
    ([&]() -> std::string { \
        std::ostringstream oss; \
        bool first = true; \
        for (const auto& value : values) { \
            if (!first) oss << delimiter; \
            oss << value; \
            first = false; \
        } \
        return oss.str(); \
    })()

// 调试宏
#ifdef DEBUG
#define FQ_DEBUG_LOG(category, message) \
    FQ_LOG_DEBUG(category, message)
#define FQ_DEBUG_ASSERT(condition, message) \
    FQ_ASSERT(condition, message)
#else
#define FQ_DEBUG_LOG(category, message) ((void)0)
#define FQ_DEBUG_ASSERT(condition, message) ((void)0)
#endif

// 内存管理宏
#define FQ_MAKE_UNIQUE(type, ...) \
    std::make_unique<type>(__VA_ARGS__)

#define FQ_MAKE_SHARED(type, ...) \
    std::make_shared<type>(__VA_ARGS__)

// 线程安全宏
#define FQ_LOCK_GUARD(mutex) \
    std::lock_guard<std::mutex> lock(mutex)

#define FQ_UNIQUE_LOCK(mutex) \
    std::unique_lock<std::mutex> lock(mutex)

#define FQ_SHARED_LOCK(mutex) \
    std::shared_lock<std::shared_mutex> lock(mutex)

// 文件操作宏
#define FQ_FILE_OPEN(file, path, mode) \
    std::ifstream file(path, mode); \
    FQ_CHECK_IO(file.is_open(), path, errno)

#define FQ_FILE_OPEN_W(file, path, mode) \
    std::ofstream file(path, mode); \
    FQ_CHECK_IO(file.is_open(), path, errno)

// 日期时间宏
#define FQ_GET_CURRENT_TIME() \
    std::chrono::system_clock::now()

#define FQ_TIME_TO_STRING(time_point) \
    ([&]() -> std::string { \
        auto time_t = std::chrono::system_clock::to_time_t(time_point); \
        std::tm tm = *std::localtime(&time_t); \
        std::ostringstream oss; \
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S"); \
        return oss.str(); \
    })()