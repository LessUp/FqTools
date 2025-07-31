#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fstream>
#include <filesystem>
#include "config.h"

namespace fq::config {

class ConfigurationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 每个测试前清理全局配置
        global_config().clear();
    }
    
    void TearDown() override {
        // 清理测试文件
        if (std::filesystem::exists("test_config.txt")) {
            std::filesystem::remove("test_config.txt");
        }
    }
    
    // 创建测试配置文件
    void create_test_config_file(const std::string& content) {
        std::ofstream file("test_config.txt");
        file << content;
        file.close();
    }
};

TEST_F(ConfigurationTest, EmptyConfiguration) {
    Configuration config;
    EXPECT_TRUE(config.empty());
    EXPECT_EQ(config.size(), 0);
}

TEST_F(ConfigurationTest, SetAndGetBool) {
    Configuration config;
    
    config.set("debug_mode", true);
    config.set("verbose", false);
    
    EXPECT_TRUE(config.get<bool>("debug_mode"));
    EXPECT_FALSE(config.get<bool>("verbose"));
}

TEST_F(ConfigurationTest, SetAndGetInt) {
    Configuration config;
    
    config.set("thread_count", int64_t(8));
    config.set("batch_size", int64_t(1000));
    
    EXPECT_EQ(config.get<int64_t>("thread_count"), 8);
    EXPECT_EQ(config.get<int64_t>("batch_size"), 1000);
}

TEST_F(ConfigurationTest, SetAndGetDouble) {
    Configuration config;
    
    config.set("quality_threshold", 20.5);
    config.set("memory_limit", 1024.0);
    
    EXPECT_DOUBLE_EQ(config.get<double>("quality_threshold"), 20.5);
    EXPECT_DOUBLE_EQ(config.get<double>("memory_limit"), 1024.0);
}

TEST_F(ConfigurationTest, SetAndGetString) {
    Configuration config;
    
    config.set("input_file", "test.fastq");
    config.set("output_dir", "/tmp/output");
    
    EXPECT_EQ(config.get<std::string>("input_file"), "test.fastq");
    EXPECT_EQ(config.get<std::string>("output_dir"), "/tmp/output");
}

TEST_F(ConfigurationTest, GetOrDefault) {
    Configuration config;
    
    config.set("existing_key", "value");
    
    EXPECT_EQ(config.get_or<std::string>("existing_key", "default"), "value");
    EXPECT_EQ(config.get_or<std::string>("non_existing_key", "default"), "default");
    EXPECT_EQ(config.get_or<int64_t>("non_existing_int", 42), 42);
}

TEST_F(ConfigurationTest, HasKey) {
    Configuration config;
    
    config.set("existing_key", "value");
    
    EXPECT_TRUE(config.has("existing_key"));
    EXPECT_FALSE(config.has("non_existing_key"));
}

TEST_F(ConfigurationTest, GetKeys) {
    Configuration config;
    
    config.set("key1", "value1");
    config.set("key2", "value2");
    config.set("key3", "value3");
    
    auto keys = config.keys();
    EXPECT_EQ(keys.size(), 3);
    EXPECT_THAT(keys, testing::UnorderedElementsAre("key1", "key2", "key3"));
}

TEST_F(ConfigurationTest, LoadFromFile) {
    std::string config_content = R"(
# This is a comment
thread_count = 8
batch_size = 1000
debug_mode = true
input_file = "test.fastq"
)";
    
    create_test_config_file(config_content);
    
    Configuration config;
    config.load_from_file("test_config.txt");
    
    EXPECT_EQ(config.get<int64_t>("thread_count"), 8);
    EXPECT_EQ(config.get<int64_t>("batch_size"), 1000);
    EXPECT_TRUE(config.get<bool>("debug_mode"));
    EXPECT_EQ(config.get<std::string>("input_file"), "test.fastq");
}

TEST_F(ConfigurationTest, LoadFromFileWithQuotes) {
    std::string config_content = R"(
input_file = 'test.fastq'
output_dir = "/tmp/output"
description = "This is a test"
)";
    
    create_test_config_file(config_content);
    
    Configuration config;
    config.load_from_file("test_config.txt");
    
    EXPECT_EQ(config.get<std::string>("input_file"), "test.fastq");
    EXPECT_EQ(config.get<std::string>("output_dir"), "/tmp/output");
    EXPECT_EQ(config.get<std::string>("description"), "This is a test");
}

TEST_F(ConfigurationTest, LoadFromArgs) {
    const char* args[] = {
        "program_name",
        "--thread_count=8",
        "--batch_size", "1000",
        "--debug_mode",
        "--input_file", "test.fastq"
    };
    
    Configuration config;
    config.load_from_args(6, const_cast<char**>(args));
    
    EXPECT_EQ(config.get<int64_t>("thread_count"), 8);
    EXPECT_EQ(config.get<int64_t>("batch_size"), 1000);
    EXPECT_TRUE(config.get<bool>("debug_mode"));
    // input_file 会被解析为字符串，不是int64_t
    EXPECT_EQ(config.get<std::string>("input_file"), "test.fastq");
}

TEST_F(ConfigurationTest, LoadFromEnv) {
    // 设置环境变量
    setenv("FQ_THREAD_COUNT", "4", 1);
    setenv("FQ_BATCH_SIZE", "500", 1);
    setenv("FQ_DEBUG_MODE", "true", 1);
    
    Configuration config;
    config.load_from_env();
    
    EXPECT_EQ(config.get<int64_t>("thread.count"), 4);
    EXPECT_EQ(config.get<int64_t>("batch.size"), 500);
    EXPECT_TRUE(config.get<bool>("debug.mode"));
    
    // 清理环境变量
    unsetenv("FQ_THREAD_COUNT");
    unsetenv("FQ_BATCH_SIZE");
    unsetenv("FQ_DEBUG_MODE");
}

TEST_F(ConfigurationTest, ParseValueTypes) {
    Configuration config;
    
    // 测试自动类型解析
    config.set<bool>("bool_true", "true");
    config.set<bool>("bool_false", "false");
    config.set<int64_t>("integer", "42");
    config.set<int64_t>("negative_int", "-10");
    config.set<double>("floating", "3.14");
    config.set<std::string>("string", "hello world");
    
    EXPECT_TRUE(config.get<bool>("bool_true"));
    EXPECT_FALSE(config.get<bool>("bool_false"));
    EXPECT_EQ(config.get<int64_t>("integer"), 42);
    EXPECT_EQ(config.get<int64_t>("negative_int"), -10);
    EXPECT_DOUBLE_EQ(config.get<double>("floating"), 3.14);
    EXPECT_EQ(config.get<std::string>("string"), "hello world");
}

TEST_F(ConfigurationTest, Validation) {
    Configuration config;
    
    // 设置有效配置
    config.set("thread_count", int64_t(8));
    config.set("batch_size", int64_t(1000));
    config.set("max_memory_mb", int64_t(1024));
    
    // 验证应该通过
    EXPECT_NO_THROW(config.validate());
    
    // 测试无效配置
    config.set("thread_count", int64_t(0));
    EXPECT_THROW(config.validate(), ConfigError);
    
    config.set("thread_count", int64_t(8));
    config.set("batch_size", int64_t(0));
    EXPECT_THROW(config.validate(), ConfigError);
    
    config.set("batch_size", int64_t(1000));
    config.set("max_memory_mb", int64_t(-1));
    EXPECT_THROW(config.validate(), ConfigError);
}

TEST_F(ConfigurationTest, MissingRequiredKey) {
    Configuration config;
    
    // 只设置部分必需的配置
    config.set("thread_count", int64_t(8));
    config.set("batch_size", int64_t(1000));
    // 缺少 max_memory_mb
    
    EXPECT_THROW(config.validate(), ConfigError);
}

TEST_F(ConfigurationTest, Clear) {
    Configuration config;
    
    config.set("key1", "value1");
    config.set("key2", "value2");
    
    EXPECT_EQ(config.size(), 2);
    EXPECT_FALSE(config.empty());
    
    config.clear();
    
    EXPECT_EQ(config.size(), 0);
    EXPECT_TRUE(config.empty());
}

TEST_F(ConfigurationTest, InvalidKeyFormat) {
    Configuration config;
    
    // 测试无效的键名
    EXPECT_THROW(config.set("invalid key", "value"), ConfigError);
    EXPECT_THROW(config.set("key@with#symbols", "value"), ConfigError);
    EXPECT_THROW(config.set("", "value"), ConfigError);
}

TEST_F(ConfigurationTest, WrongTypeAccess) {
    Configuration config;
    
    config.set("number", "42");
    
    // 字符串不能作为bool访问
    EXPECT_THROW(config.get<bool>("number"), ConfigError);
}

TEST_F(ConfigurationTest, GlobalConfig) {
    auto& config1 = global_config();
    auto& config2 = global_config();
    
    EXPECT_EQ(&config1, &config2); // 应该是同一个实例
    
    config1.set("global_key", "global_value");
    EXPECT_TRUE(config2.has("global_key"));
    EXPECT_EQ(config2.get<std::string>("global_key"), "global_value");
}

} // namespace fq::config