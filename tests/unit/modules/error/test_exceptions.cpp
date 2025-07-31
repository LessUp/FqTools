#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include "error.h"

namespace fq::error {

TEST(ErrorExceptionTest, BasicFastQException) {
    FastQException ex(ErrorCategory::Processing, ErrorSeverity::Error, "Test error");
    
    EXPECT_EQ(ex.category(), ErrorCategory::Processing);
    EXPECT_EQ(ex.severity(), ErrorSeverity::Error);
    EXPECT_EQ(ex.message(), "Test error");
    EXPECT_STRNE(ex.what(), "");
    EXPECT_THAT(ex.what(), testing::HasSubstr("Processing"));
    EXPECT_THAT(ex.what(), testing::HasSubstr("Error"));
    EXPECT_THAT(ex.what(), testing::HasSubstr("Test error"));
}

TEST(ErrorExceptionTest, IOErrorCreation) {
    IOError ex("test.txt", 42);
    
    EXPECT_EQ(ex.category(), ErrorCategory::IO);
    EXPECT_EQ(ex.severity(), ErrorSeverity::Error);
    EXPECT_THAT(ex.message(), testing::HasSubstr("test.txt"));
    EXPECT_THAT(ex.message(), testing::HasSubstr("42"));
    EXPECT_THAT(ex.what(), testing::HasSubstr("IO Error"));
}

TEST(ErrorExceptionTest, FormatErrorCreation) {
    FormatError ex("Invalid FASTQ format");
    
    EXPECT_EQ(ex.category(), ErrorCategory::Format);
    EXPECT_EQ(ex.severity(), ErrorSeverity::Error);
    EXPECT_THAT(ex.message(), testing::HasSubstr("Invalid FASTQ format"));
    EXPECT_THAT(ex.what(), testing::HasSubstr("Format Error"));
}

TEST(ErrorExceptionTest, ValidationErrorCreation) {
    ValidationError ex("Sequence validation failed");
    
    EXPECT_EQ(ex.category(), ErrorCategory::Validation);
    EXPECT_EQ(ex.severity(), ErrorSeverity::Error);
    EXPECT_THAT(ex.message(), testing::HasSubstr("Sequence validation failed"));
    EXPECT_THAT(ex.what(), testing::HasSubstr("Validation Error"));
}

TEST(ErrorExceptionTest, ProcessingErrorCreation) {
    ProcessingError ex("Pipeline processing failed");
    
    EXPECT_EQ(ex.category(), ErrorCategory::Processing);
    EXPECT_EQ(ex.severity(), ErrorSeverity::Error);
    EXPECT_THAT(ex.message(), testing::HasSubstr("Pipeline processing failed"));
    EXPECT_THAT(ex.what(), testing::HasSubstr("Processing Error"));
}

TEST(ErrorExceptionTest, ResourceErrorCreation) {
    ResourceError ex("Memory allocation failed");
    
    EXPECT_EQ(ex.category(), ErrorCategory::Resource);
    EXPECT_EQ(ex.severity(), ErrorSeverity::Error);
    EXPECT_THAT(ex.message(), testing::HasSubstr("Memory allocation failed"));
    EXPECT_THAT(ex.what(), testing::HasSubstr("Resource Error"));
}

TEST(ErrorExceptionTest, ConfigurationErrorCreation) {
    ConfigurationError ex("Invalid configuration parameter");
    
    EXPECT_EQ(ex.category(), ErrorCategory::Configuration);
    EXPECT_EQ(ex.severity(), ErrorSeverity::Error);
    EXPECT_THAT(ex.message(), testing::HasSubstr("Invalid configuration parameter"));
    EXPECT_THAT(ex.what(), testing::HasSubstr("Configuration Error"));
}

TEST(ErrorExceptionTest, SourceLocation) {
    // 测试源位置信息
    auto current_loc = std::source_location::current();
    FastQException ex(ErrorCategory::Processing, ErrorSeverity::Error, "Location test");
    
    auto loc = ex.location();
    EXPECT_STRNE(loc.file_name(), "");
    EXPECT_GT(loc.line(), 0);
    EXPECT_STRNE(loc.function_name(), "");
}

TEST(ErrorHandlerTest, SingletonInstance) {
    auto& handler1 = global_error_handler();
    auto& handler2 = global_error_handler();
    
    EXPECT_EQ(&handler1, &handler2);
}

TEST(ErrorHandlerTest, RegisterAndHandle) {
    auto& handler = global_error_handler();
    
    bool handler_called = false;
    ErrorHandler::HandlerFunc test_handler = [&handler_called](const FastQException& ex) {
        handler_called = true;
        EXPECT_EQ(ex.category(), ErrorCategory::IO);
        return true;
    };
    
    // 注册处理器
    handler.register_handler(ErrorCategory::IO, test_handler);
    
    // 创建异常并处理
    IOError ex("test.txt", 42);
    bool handled = handler.handle_error(ex);
    
    EXPECT_TRUE(handled);
    EXPECT_TRUE(handler_called);
}

TEST(ErrorHandlerTest, MultipleHandlers) {
    auto& handler = global_error_handler();
    
    int call_count = 0;
    ErrorHandler::HandlerFunc handler1 = [&call_count](const FastQException& ex) {
        call_count++;
        return false; // 不处理，继续下一个
    };
    
    ErrorHandler::HandlerFunc handler2 = [&call_count](const FastQException& ex) {
        call_count++;
        return true; // 处理
    };
    
    // 注册多个处理器
    handler.register_handler(ErrorCategory::Format, handler1);
    handler.register_handler(ErrorCategory::Format, handler2);
    
    FormatError ex("Test format error");
    bool handled = handler.handle_error(ex);
    
    EXPECT_TRUE(handled);
    EXPECT_EQ(call_count, 2);
}

TEST(ErrorHandlerTest, NoHandlerForCategory) {
    auto& handler = global_error_handler();
    
    // 创建一个没有注册处理器的异常类型
    ResourceError ex("Test resource error");
    bool handled = handler.handle_error(ex);
    
    EXPECT_FALSE(handled);
}

TEST(ErrorHandlerTest, HandlerReturnsFalse) {
    auto& handler = global_error_handler();
    
    ErrorHandler::HandlerFunc false_handler = [](const FastQException& ex) {
        return false; // 不处理
    };
    
    handler.register_handler(ErrorCategory::Validation, false_handler);
    
    ValidationError ex("Test validation error");
    bool handled = handler.handle_error(ex);
    
    EXPECT_FALSE(handled);
}

TEST(ErrorHelperFunctionsTest, CategoryToString) {
    EXPECT_EQ(category_to_string(ErrorCategory::IO), "IO");
    EXPECT_EQ(category_to_string(ErrorCategory::Format), "Format");
    EXPECT_EQ(category_to_string(ErrorCategory::Validation), "Validation");
    EXPECT_EQ(category_to_string(ErrorCategory::Processing), "Processing");
    EXPECT_EQ(category_to_string(ErrorCategory::Resource), "Resource");
    EXPECT_EQ(category_to_string(ErrorCategory::Configuration), "Configuration");
}

TEST(ErrorHelperFunctionsTest, SeverityToString) {
    EXPECT_EQ(severity_to_string(ErrorSeverity::Info), "Info");
    EXPECT_EQ(severity_to_string(ErrorSeverity::Warning), "Warning");
    EXPECT_EQ(severity_to_string(ErrorSeverity::Error), "Error");
    EXPECT_EQ(severity_to_string(ErrorSeverity::Critical), "Critical");
}

TEST(ErrorExceptionTest, ExceptionAsStdException) {
    try {
        throw FastQException(ErrorCategory::Processing, ErrorSeverity::Error, "Test std::exception");
    } catch (const std::exception& e) {
        EXPECT_THAT(e.what(), testing::HasSubstr("Processing"));
        EXPECT_THAT(e.what(), testing::HasSubstr("Error"));
        EXPECT_THAT(e.what(), testing::HasSubstr("Test std::exception"));
    }
}

} // namespace fq::error