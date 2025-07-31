#include "test_helpers.h"
#include <random>
#include <sstream>
#include <algorithm>

namespace fq::test {

std::vector<std::filesystem::path> TestHelpers::temp_paths_;

std::filesystem::path TestHelpers::createTempFile(
    const std::string& content, 
    const std::string& suffix
) {
    auto temp_dir = std::filesystem::temp_directory_path();
    auto temp_file = temp_dir / ("fastqtools_test_" + std::to_string(std::random_device{}()) + suffix);
    
    std::ofstream file(temp_file);
    file << content;
    file.close();
    
    temp_paths_.push_back(temp_file);
    return temp_file;
}

std::filesystem::path TestHelpers::createTempDir() {
    auto temp_dir = std::filesystem::temp_directory_path();
    auto test_dir = temp_dir / ("fastqtools_test_dir_" + std::to_string(std::random_device{}()));
    
    std::filesystem::create_directories(test_dir);
    temp_paths_.push_back(test_dir);
    return test_dir;
}

std::string TestHelpers::generateFastQRecords(size_t count, size_t read_length) {
    std::ostringstream oss;
    
    for (size_t i = 0; i < count; ++i) {
        oss << "@read_" << i << "\n";
        oss << generateRandomDNA(read_length) << "\n";
        oss << "+\n";
        oss << generateRandomQuality(read_length) << "\n";
    }
    
    return oss.str();
}

std::string TestHelpers::generateRandomDNA(size_t length) {
    static const char bases[] = "ATGC";
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 3);
    
    std::string sequence;
    sequence.reserve(length);
    
    for (size_t i = 0; i < length; ++i) {
        sequence += bases[dis(gen)];
    }
    
    return sequence;
}

std::string TestHelpers::generateRandomQuality(size_t length, int min_quality, int max_quality) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min_quality, max_quality);
    
    std::string quality;
    quality.reserve(length);
    
    for (size_t i = 0; i < length; ++i) {
        quality += static_cast<char>(dis(gen) + 33); // Phred+33 encoding
    }
    
    return quality;
}

bool TestHelpers::compareFiles(
    const std::filesystem::path& file1,
    const std::filesystem::path& file2
) {
    std::ifstream f1(file1, std::ios::binary);
    std::ifstream f2(file2, std::ios::binary);
    
    if (!f1.is_open() || !f2.is_open()) {
        return false;
    }
    
    return std::equal(
        std::istreambuf_iterator<char>(f1.rdbuf()),
        std::istreambuf_iterator<char>(),
        std::istreambuf_iterator<char>(f2.rdbuf())
    );
}

void TestHelpers::cleanup() {
    for (const auto& path : temp_paths_) {
        std::error_code ec;
        if (std::filesystem::is_directory(path)) {
            std::filesystem::remove_all(path, ec);
        } else {
            std::filesystem::remove(path, ec);
        }
    }
    temp_paths_.clear();
}

void FastQToolsTest::SetUp() {
    temp_dir_ = TestHelpers::createTempDir();
    test_data_dir_ = std::filesystem::current_path() / "tests" / "fixtures";
}

void FastQToolsTest::TearDown() {
    TestHelpers::cleanup();
}

} // namespace fq::test
