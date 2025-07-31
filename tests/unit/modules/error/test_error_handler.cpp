#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include "error.h"

namespace fq::error {

class ErrorHandlerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 每个测试前重置全局处理器状态
        // 注意：在实际项目中，可能需要更好的状态管理
    }
    
    void TearDown() override {
        // 清理
    }
};

TEST_F(ErrorHandlerTest, RegisterSingleHandler) {
    ErrorHandler handler;
    
    bool was_called = false;
    ErrorHandler::HandlerFunc test_handler = [&was_called](const FastQException& ex) {
        was_called = true;
        return true;
    };
    
    handler.register_handler(ErrorCategory::IO, test_handler);
    
    IOError ex("test.txt", 42);
    bool result = handler.handle_error(ex);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(was_called);
}

TEST_F(ErrorHandlerTest, HandlerNotCalledForWrongCategory) {
    ErrorHandler handler;
    
    bool was_called = false;
    ErrorHandler::HandlerFunc test_handler = [&was_called](const FastQException& ex) {
        was_called = true;
        return true;
    };
    
    handler.register_handler(ErrorCategory::IO, test_handler);
    
    // 创建不同类别的异常
    FormatError ex("test format error");
    bool result = handler.handle_error(ex);
    
    EXPECT_FALSE(result);
    EXPECT_FALSE(was_called);
}

TEST_F(ErrorHandlerTest, MultipleHandlersSameCategory) {
    ErrorHandler handler;
    
    int call_count = 0;
    ErrorHandler::HandlerFunc handler1 = [&call_count](const FastQException& ex) {
        call_count++;
        return false; // 继续下一个处理器
    };
    
    ErrorHandler::HandlerFunc handler2 = [&call_count](const FastQException& ex) {
        call_count++;
        return true; // 处理并停止
    };
    
    ErrorHandler::HandlerFunc handler3 = [&call_count](const FastQException& ex) {
        call_count++;
        return true;
    };
    
    handler.register_handler(ErrorCategory::Processing, handler1);
    handler.register_handler(ErrorCategory::Processing, handler2);
    handler.register_handler(ErrorCategory::Processing, handler3);
    
    ProcessingError ex("test processing error");
    bool result = handler.handle_error(ex);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(call_count, 2); // handler3不应该被调用
}

TEST_F(ErrorHandlerTest, HandlerChainStopsOnFirstTrue) {
    ErrorHandler handler;
    
    int call_count = 0;
    ErrorHandler::HandlerFunc handler1 = [&call_count](const FastQException& ex) {
        call_count++;
        return true; // 处理并停止
    };
    
    ErrorHandler::HandlerFunc handler2 = [&call_count](const FastQException& ex) {
        call_count++;
        return true;
    };
    
    handler.register_handler(ErrorCategory::Validation, handler1);
    handler.register_handler(ErrorCategory::Validation, handler2);
    
    ValidationError ex("test validation error");
    bool result = handler.handle_error(ex);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(call_count, 1); // 只有handler1被调用
}

TEST_F(ErrorHandlerTest, AllHandlersReturnFalse) {
    ErrorHandler handler;
    
    int call_count = 0;
    ErrorHandler::HandlerFunc handler1 = [&call_count](const FastQException& ex) {
        call_count++;
        return false;
    };
    
    ErrorHandler::HandlerFunc handler2 = [&call_count](const FastQException& ex) {
        call_count++;
        return false;
    };
    
    handler.register_handler(ErrorCategory::Resource, handler1);
    handler.register_handler(ErrorCategory::Resource, handler2);
    
    ResourceError ex("test resource error");
    bool result = handler.handle_error(ex);
    
    EXPECT_FALSE(result);
    EXPECT_EQ(call_count, 2); // 两个处理器都被调用
}

TEST_F(ErrorHandlerTest, NoHandlersRegistered) {
    ErrorHandler handler;
    
    IOError ex("test.txt", 42);
    bool result = handler.handle_error(ex);
    
    EXPECT_FALSE(result);
}

TEST_F(ErrorHandlerTest, HandlerReceivesCorrectException) {
    ErrorHandler handler;
    
    FastQException* captured_exception = nullptr;
    ErrorHandler::HandlerFunc test_handler = [&captured_exception](const FastQException& ex) {
        captured_exception = const_cast<FastQException*>(&ex);
        return true;
    };
    
    handler.register_handler(ErrorCategory::Configuration, test_handler);
    
    ConfigurationError ex("test config error", std::source_location::current());
    bool result = handler.handle_error(ex);
    
    EXPECT_TRUE(result);
    ASSERT_NE(captured_exception, nullptr);
    EXPECT_EQ(captured_exception->category(), ErrorCategory::Configuration);
    EXPECT_EQ(captured_exception->severity(), ErrorSeverity::Error);
    EXPECT_THAT(captured_exception->message(), testing::HasSubstr("test config error"));
}

TEST_F(ErrorHandlerTest, HandlerWithExceptionDetails) {
    ErrorHandler handler;
    
    std::string captured_message;
    ErrorCategory captured_category;
    ErrorSeverity captured_severity;
    
    ErrorHandler::HandlerFunc detailed_handler = [&captured_message, &captured_category, &captured_severity](const FastQException& ex) {
        captured_message = ex.message();
        captured_category = ex.category();
        captured_severity = ex.severity();
        return true;
    };
    
    handler.register_handler(ErrorCategory::Format, detailed_handler);
    
    FormatError ex("format error details");
    bool result = handler.handle_error(ex);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(captured_category, ErrorCategory::Format);
    EXPECT_EQ(captured_severity, ErrorSeverity::Error);
    EXPECT_EQ(captured_message, "Format Error: format error details");
}

TEST_F(ErrorHandlerTest, MultipleCategoriesIndependent) {
    ErrorHandler handler;
    
    bool io_called = false;
    bool format_called = false;
    
    ErrorHandler::HandlerFunc io_handler = [&io_called](const FastQException& ex) {
        io_called = true;
        return true;
    };
    
    ErrorHandler::HandlerFunc format_handler = [&format_called](const FastQException& ex) {
        format_called = true;
        return true;
    };
    
    handler.register_handler(ErrorCategory::IO, io_handler);
    handler.register_handler(ErrorCategory::Format, format_handler);
    
    // 测试IO异常
    IOError io_ex("io_test.txt", 123);
    bool io_result = handler.handle_error(io_ex);
    EXPECT_TRUE(io_result);
    EXPECT_TRUE(io_called);
    EXPECT_FALSE(format_called);
    
    // 重置标志
    io_called = false;
    
    // 测试Format异常
    FormatError format_ex("format_test");
    bool format_result = handler.handle_error(format_ex);
    EXPECT_TRUE(format_result);
    EXPECT_FALSE(io_called);
    EXPECT_TRUE(format_called);
}

} // namespace fq::error