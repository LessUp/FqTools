#pragma once

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>

namespace fq::test {

/**
 * @brief 测试辅助工具类
 */
class TestHelpers {
public:
    /**
     * @brief 创建临时测试文件
     * @param content 文件内容
     * @param suffix 文件后缀
     * @return 临时文件路径
     */
    static std::filesystem::path createTempFile(
        const std::string& content, 
        const std::string& suffix = ".tmp"
    );

    /**
     * @brief 创建临时目录
     * @return 临时目录路径
     */
    static std::filesystem::path createTempDir();

    /**
     * @brief 生成测试用的FastQ记录
     * @param count 记录数量
     * @param read_length 读长
     * @return FastQ格式字符串
     */
    static std::string generateFastQRecords(size_t count, size_t read_length = 100);

    /**
     * @brief 生成随机DNA序列
     * @param length 序列长度
     * @return DNA序列字符串
     */
    static std::string generateRandomDNA(size_t length);

    /**
     * @brief 生成随机质量分数
     * @param length 质量分数长度
     * @param min_quality 最小质量值
     * @param max_quality 最大质量值
     * @return 质量分数字符串
     */
    static std::string generateRandomQuality(
        size_t length, 
        int min_quality = 20, 
        int max_quality = 40
    );

    /**
     * @brief 比较两个文件内容是否相同
     * @param file1 文件1路径
     * @param file2 文件2路径
     * @return 是否相同
     */
    static bool compareFiles(
        const std::filesystem::path& file1,
        const std::filesystem::path& file2
    );

    /**
     * @brief 清理临时文件和目录
     */
    static void cleanup();

private:
    static std::vector<std::filesystem::path> temp_paths_;
};

/**
 * @brief 测试基类，提供通用的设置和清理
 */
class FastQToolsTest : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;

    std::filesystem::path temp_dir_;
    std::filesystem::path test_data_dir_;
};

/**
 * @brief 参数化测试基类
 */
template<typename T>
class FastQToolsParameterizedTest : public FastQToolsTest, 
                                   public ::testing::WithParamInterface<T> {
};

} // namespace fq::test
