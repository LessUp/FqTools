#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace fq::test {

/**
 * @brief 测试数据加载器
 */
class FixtureLoader {
public:
    /**
     * @brief 加载文本文件内容
     * @param path 文件路径
     * @return 文件内容
     */
    static std::string loadTextFile(const std::filesystem::path& path);

    /**
     * @brief 加载文件的所有行
     * @param path 文件路径
     * @return 行数组
     */
    static std::vector<std::string> loadLines(const std::filesystem::path& path);

    /**
     * @brief 获取测试数据文件路径
     * @param filename 文件名
     * @return 完整路径
     */
    static std::filesystem::path getFixturePath(const std::string& filename);

    /**
     * @brief 检查测试数据文件是否存在
     * @param filename 文件名
     * @return 是否存在
     */
    static bool fixtureExists(const std::string& filename);
};

} // namespace fq::test
