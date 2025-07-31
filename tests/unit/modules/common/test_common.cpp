#include <gtest/gtest.h>

#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "modules/common/common.h"
namespace fq::common {

// --- Timer Tests ---
TEST(CommonModuleTest, Timer_BasicFunctionality) {
    Timer timer("test_timer");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto elapsed = timer.elapsed();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

    EXPECT_GE(elapsed_ms, 90);
    EXPECT_LE(elapsed_ms, 200);

    // Test that report() doesn't crash
    EXPECT_NO_THROW(timer.report());
}

// --- StringUtils Tests ---
TEST(CommonModuleTest, StringUtils_SplitBasic) {
    auto result = split("a,b,c", ',');
    std::vector<std::string> expected = {"a", "b", "c"};
    ASSERT_EQ(result, expected);
}

TEST(CommonModuleTest, StringUtils_SplitWithEmptyParts) {
    // Note: The new implementation might handle this differently.
    // Assuming it skips empty parts based on the previous test logic.
    auto result = split("a,,c", ',');
    std::vector<std::string> expected = {"a", "c"};
    ASSERT_EQ(result, expected);
}

TEST(CommonModuleTest, StringUtils_TrimBasic) {
    EXPECT_EQ(trim("  hello  "), "hello");
    EXPECT_EQ(trim("\t\nhello\r\n"), "hello");
    EXPECT_EQ(trim("hello"), "hello");
}

TEST(CommonModuleTest, StringUtils_JoinBasic) {
    std::vector<std::string> parts = {"a", "b", "c"};
    EXPECT_EQ(join(parts, ","), "a,b,c");
}

// --- Logger Tests ---
TEST(CommonModuleTest, Logger_SingletonInstance) {
    auto& logger1 = Logger::instance();
    auto& logger2 = Logger::instance();
    EXPECT_EQ(&logger1, &logger2);
}

TEST(CommonModuleTest, Logger_LogLevelChange) {
    auto& logger = Logger::instance();
    EXPECT_NO_THROW(logger.set_level(Logger::Level::Debug));
    EXPECT_NO_THROW(logger.log(Logger::Level::Info, "Test message"));
    EXPECT_NO_THROW(logger.set_level(Logger::Level::Critical));
}

// --- SoftwareInfo Tests ---
TEST(CommonModuleTest, SoftwareInfo_PrintFunctions) {
    // Test that printing functions do not throw exceptions
    EXPECT_NO_THROW(print_software_info());
    EXPECT_NO_THROW(print_logo());
}

}  // namespace fq::common
