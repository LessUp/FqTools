#include "fixture_loader.h"
#include <fstream>
#include <sstream>

namespace fq::test {

std::string FixtureLoader::loadTextFile(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open fixture file: " + path.string());
    }
    
    std::ostringstream content;
    content << file.rdbuf();
    return content.str();
}

std::vector<std::string> FixtureLoader::loadLines(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open fixture file: " + path.string());
    }
    
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    
    return lines;
}

std::filesystem::path FixtureLoader::getFixturePath(const std::string& filename) {
    auto current_path = std::filesystem::current_path();
    auto fixture_path = current_path / "tests" / "fixtures" / filename;
    
    if (!std::filesystem::exists(fixture_path)) {
        // 尝试从构建目录查找
        fixture_path = current_path / "fixtures" / filename;
    }
    
    return fixture_path;
}

bool FixtureLoader::fixtureExists(const std::string& filename) {
    return std::filesystem::exists(getFixturePath(filename));
}

} // namespace fq::test
