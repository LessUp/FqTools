#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "config.h"

namespace fq::config {

TEST(ConfigHelperFunctionsTest, ParseBool) {
    EXPECT_TRUE(parse_bool("true"));
    EXPECT_TRUE(parse_bool("TRUE"));
    EXPECT_TRUE(parse_bool("yes"));
    EXPECT_TRUE(parse_bool("YES"));
    EXPECT_TRUE(parse_bool("on"));
    EXPECT_TRUE(parse_bool("ON"));
    EXPECT_TRUE(parse_bool("1"));
    
    EXPECT_FALSE(parse_bool("false"));
    EXPECT_FALSE(parse_bool("FALSE"));
    EXPECT_FALSE(parse_bool("no"));
    EXPECT_FALSE(parse_bool("NO"));
    EXPECT_FALSE(parse_bool("off"));
    EXPECT_FALSE(parse_bool("OFF"));
    EXPECT_FALSE(parse_bool("0"));
}

TEST(ConfigHelperFunctionsTest, ParseInt) {
    EXPECT_EQ(parse_int("42"), 42);
    EXPECT_EQ(parse_int("0"), 0);
    EXPECT_EQ(parse_int("-123"), -123);
    EXPECT_EQ(parse_int("9223372036854775807"), std::numeric_limits<int64_t>::max());
    
    EXPECT_THROW(parse_int("not_a_number"), ConfigError);
    EXPECT_THROW(parse_int("123.45"), ConfigError);
    EXPECT_THROW(parse_int(""), ConfigError);
}

TEST(ConfigHelperFunctionsTest, ParseDouble) {
    EXPECT_DOUBLE_EQ(parse_double("3.14"), 3.14);
    EXPECT_DOUBLE_EQ(parse_double("0.0"), 0.0);
    EXPECT_DOUBLE_EQ(parse_double("-2.5"), -2.5);
    EXPECT_DOUBLE_EQ(parse_double("1e10"), 1e10);
    
    EXPECT_THROW(parse_double("not_a_number"), ConfigError);
    EXPECT_THROW(parse_double(""), ConfigError);
}

TEST(ConfigValueParsingTest, AutoParseBool) {
    Configuration config;
    
    config.set("bool_true", "true");
    config.set("bool_false", "false");
    config.set("bool_yes", "yes");
    config.set("bool_no", "no");
    config.set("bool_on", "on");
    config.set("bool_off", "off");
    config.set("bool_1", "1");
    config.set("bool_0", "0");
    
    EXPECT_TRUE(config.get<bool>("bool_true"));
    EXPECT_FALSE(config.get<bool>("bool_false"));
    EXPECT_TRUE(config.get<bool>("bool_yes"));
    EXPECT_FALSE(config.get<bool>("bool_no"));
    EXPECT_TRUE(config.get<bool>("bool_on"));
    EXPECT_FALSE(config.get<bool>("bool_off"));
    EXPECT_TRUE(config.get<bool>("bool_1"));
    EXPECT_FALSE(config.get<bool>("bool_0"));
}

TEST(ConfigValueParsingTest, AutoParseNumber) {
    Configuration config;
    
    config.set("positive_int", "123");
    config.set("negative_int", "-456");
    config.set("zero", "0");
    config.set("double", "3.14159");
    config.set("scientific", "1.5e10");
    config.set("negative_double", "-2.718");
    
    EXPECT_EQ(config.get<int64_t>("positive_int"), 123);
    EXPECT_EQ(config.get<int64_t>("negative_int"), -456);
    EXPECT_EQ(config.get<int64_t>("zero"), 0);
    EXPECT_DOUBLE_EQ(config.get<double>("double"), 3.14159);
    EXPECT_DOUBLE_EQ(config.get<double>("scientific"), 1.5e10);
    EXPECT_DOUBLE_EQ(config.get<double>("negative_double"), -2.718);
}

TEST(ConfigValueParsingTest, AutoParseString) {
    Configuration config;
    
    config.set("simple_string", "hello");
    config.set("string_with_spaces", "hello world");
    config.set("string_with_numbers", "test123");
    config.set("string_like_number", "123abc"); // 不是纯数字，作为字符串处理
    
    EXPECT_EQ(config.get<std::string>("simple_string"), "hello");
    EXPECT_EQ(config.get<std::string>("string_with_spaces"), "hello world");
    EXPECT_EQ(config.get<std::string>("string_with_numbers"), "test123");
    EXPECT_EQ(config.get<std::string>("string_like_number"), "123abc");
}

TEST(ConfigFileLoadingTest, EmptyLinesAndComments) {
    Configuration config;
    
    // 创建包含空行和注释的配置文件
    std::ofstream file("test_comments.txt");
    file << "# This is a comment\n";
    file << "\n";
    file << "# Another comment\n";
    file << "key1 = value1\n";
    file << "\n";
    file << "key2 = value2\n";
    file << "# End comment\n";
    file.close();
    
    config.load_from_file("test_comments.txt");
    
    EXPECT_EQ(config.get<std::string>("key1"), "value1");
    EXPECT_EQ(config.get<std::string>("key2"), "value2");
    EXPECT_EQ(config.size(), 2);
    
    std::filesystem::remove("test_comments.txt");
}

TEST(ConfigFileLoadingTest, MalformedLines) {
    Configuration config;
    
    // 创建包含错误格式的配置文件
    std::ofstream file("test_malformed.txt");
    file << "valid_key = valid_value\n";
    file << "invalid_line_without_equals\n";
    file << "another_valid = another_value\n";
    file.close();
    
    EXPECT_THROW(config.load_from_file("test_malformed.txt"), ConfigError);
    
    std::filesystem::remove("test_malformed.txt");
}

TEST(ConfigArgsLoadingTest, ShortArgs) {
    Configuration config;
    
    // 测试短参数格式（虽然我们的实现主要处理长参数）
    const char* args[] = {
        "program_name",
        "--key1=value1",
        "--key2", "value2",
        "--flag"
    };
    
    config.load_from_args(4, const_cast<char**>(args));
    
    EXPECT_EQ(config.get<std::string>("key1"), "value1");
    EXPECT_EQ(config.get<std::string>("key2"), "value2");
    EXPECT_TRUE(config.get<bool>("flag"));
}

TEST(ConfigEnvironmentTest, EnvVarConversion) {
    // 设置环境变量
    setenv("FQ_TEST_VALUE", "123", 1);
    setenv("FQ_ANOTHER_VALUE", "hello", 1);
    setenv("FQ_WITH_UNDERSCORES", "world", 1);
    
    Configuration config;
    config.load_from_env();
    
    EXPECT_EQ(config.get<int64_t>("test.value"), 123);
    EXPECT_EQ(config.get<std::string>("another.value"), "hello");
    EXPECT_EQ(config.get<std::string>("with.underscores"), "world");
    
    // 清理环境变量
    unsetenv("FQ_TEST_VALUE");
    unsetenv("FQ_ANOTHER_VALUE");
    unsetenv("FQ_WITH_UNDERSCORES");
}

TEST(ConfigErrorTest, FileNotFound) {
    Configuration config;
    
    EXPECT_THROW(config.load_from_file("nonexistent_file.txt"), ConfigError);
}

TEST(ConfigErrorTest, InvalidKeyFormat) {
    Configuration config;
    
    EXPECT_THROW(config.set("key with spaces", "value"), ConfigError);
    EXPECT_THROW(config.set("key@special#chars", "value"), ConfigError);
    EXPECT_THROW(config.set("", "value"), ConfigError);
}

TEST(ConfigErrorTest, TypeMismatch) {
    Configuration config;
    
    config.set("number", "42");
    
    EXPECT_THROW(config.get<bool>("number"), ConfigError);
}

TEST(ConfigPrintTest, PrintConfiguration) {
    Configuration config;
    
    config.set("bool_val", true);
    config.set("int_val", int64_t(42));
    config.set("double_val", 3.14);
    config.set("string_val", "hello world");
    
    // 测试打印功能（不应该崩溃）
    EXPECT_NO_THROW(config.print());
}

} // namespace fq::config