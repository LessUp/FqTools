#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>

#include "modules/fastq/fastq.h"
#include "utils/test_helpers.h"

namespace fq::fastq {

// --- FastQ File Processing Tests ---

class FastQFileTest : public fq::test::FastQToolsTest {
protected:
    void SetUp() override {
        fq::test::FastQToolsTest::SetUp();
        
        // 创建测试数据目录
        test_data_dir_ = temp_dir_ / "test_data";
        std::filesystem::create_directories(test_data_dir_);
    }
    
    void TearDown() override {
        fq::test::FastQToolsTest::TearDown();
    }
    
    // 创建测试FastQ文件
    std::filesystem::path createTestFastQFile(const std::string& content, 
                                              const std::string& suffix = ".fastq") {
        auto file_path = test_data_dir_ / ("test_" + std::to_string(test_file_counter_++) + suffix);
        
        std::ofstream file(file_path);
        file << content;
        file.close();
        
        return file_path;
    }
    
    // 生成标准FastQ记录
    std::string generateFastQRecord(const std::string& id, 
                                   const std::string& sequence, 
                                   const std::string& quality) {
        return "@" + id + "\n" + sequence + "\n+\n" + quality + "\n";
    }
    
    // 生成随机FastQ记录
    std::string generateRandomFastQRecord(size_t length = 100) {
        static const char bases[] = "ATCG";
        static const char qualities[] = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJ";
        
        std::string seq;
        std::string qual;
        
        for (size_t i = 0; i < length; ++i) {
            seq += bases[rand() % 4];
            qual += qualities[rand() % 40];
        }
        
        return generateFastQRecord("read_" + std::to_string(rand()), seq, qual);
    }
    
    std::filesystem::path test_data_dir_;
    int test_file_counter_ = 0;
};

// --- Basic FastQ File Tests ---

TEST_F(FastQFileTest, ValidFastQFile) {
    // 创建有效的FastQ文件
    std::string content = 
        "@test_read_001\n"
        "ATCGATCGATCG\n"
        "+\n"
        "IIIIIIIIIIII\n"
        "@test_read_002\n"
        "GCTAGCTAGCTA\n"
        "+\n"
        "IIIIIIIIIIII\n";
    
    auto file_path = createTestFastQFile(content);
    
    // 使用FileInferrer分析文件
    FileInferrer inferrer(file_path);
    auto attrs = inferrer.infer_attributes(100);
    
    EXPECT_EQ(attrs.read_length, 12);
    EXPECT_FALSE(attrs.is_variable_length);
    EXPECT_GT(attrs.average_quality, 30.0);
    EXPECT_GT(attrs.gc_content, 0.0);
    EXPECT_GT(attrs.estimated_record_count, 0);
    EXPECT_EQ(attrs.q_score_type, fq::core::QScoreType::Sanger);
}

TEST_F(FastQFileTest, EmptyFastQFile) {
    // 创建空的FastQ文件
    auto file_path = createTestFastQFile("");
    
    FileInferrer inferrer(file_path);
    
    EXPECT_THROW(
        inferrer.infer_attributes(100),
        fq::error::FastQException
    );
}

TEST_F(FastQFileTest, SingleRecordFastQFile) {
    // 创建单条记录的FastQ文件
    std::string content = 
        "@single_read\n"
        "ATCG\n"
        "+\n"
        "IIII\n";
    
    auto file_path = createTestFastQFile(content);
    
    FileInferrer inferrer(file_path);
    auto attrs = inferrer.infer_attributes(100);
    
    EXPECT_EQ(attrs.read_length, 4);
    EXPECT_FALSE(attrs.is_variable_length);
    EXPECT_GT(attrs.average_quality, 30.0);
    EXPECT_GT(attrs.gc_content, 0.0);
    EXPECT_GT(attrs.estimated_record_count, 0);
}

// --- Variable Length FastQ File Tests ---

TEST_F(FastQFileTest, VariableLengthRecords) {
    // 创建变长记录的FastQ文件
    std::string content = 
        "@short_read\n"
        "ATCG\n"
        "+\n"
        "IIII\n"
        "@medium_read\n"
        "ATCGATCGATCG\n"
        "+\n"
        "IIIIIIIIIIII\n"
        "@long_read\n"
        "ATCGATCGATCGATCGATCGATCG\n"
        "+\n"
        "IIIIIIIIIIIIIIIIIIIIIIII\n";
    
    auto file_path = createTestFastQFile(content);
    
    FileInferrer inferrer(file_path);
    auto attrs = inferrer.infer_attributes(100);
    
    EXPECT_EQ(attrs.read_length, 24);
    EXPECT_TRUE(attrs.is_variable_length);
    EXPECT_GT(attrs.average_quality, 30.0);
    EXPECT_GT(attrs.gc_content, 0.0);
    EXPECT_GT(attrs.estimated_record_count, 0);
}

// --- Quality Score Type Detection Tests ---

TEST_F(FastQFileTest, SangerQualityScores) {
    // Sanger格式质量分数 (Phred+33, 范围0-40)
    std::string content = 
        "@sanger_read\n"
        "ATCG\n"
        "+\n"
        "!\"#$\n"; // Phred分数 0, 1, 2, 3
    
    auto file_path = createTestFastQFile(content);
    
    FileInferrer inferrer(file_path);
    auto attrs = inferrer.infer_attributes(100);
    
    EXPECT_EQ(attrs.q_score_type, fq::core::QScoreType::Sanger);
    EXPECT_LT(attrs.average_quality, 10.0);
}

TEST_F(FastQFileTest, IlluminaQualityScores) {
    // Illumina格式质量分数 (Phred+64, 范围0-40)
    std::string content = 
        "@illumina_read\n"
        "ATCG\n"
        "+\n"
        "@ABC\n"; // Phred分数 32, 33, 34, 35 (对应64+0, 64+1, 64+2, 64+3)
    
    auto file_path = createTestFastQFile(content);
    
    FileInferrer inferrer(file_path);
    auto attrs = inferrer.infer_attributes(100);
    
    EXPECT_EQ(attrs.q_score_type, fq::core::QScoreType::Illumina13);
    EXPECT_GT(attrs.average_quality, 30.0);
}

// --- Large FastQ File Tests ---

TEST_F(FastQFileTest, LargeFastQFile) {
    // 创建包含大量记录的FastQ文件
    std::stringstream content;
    
    for (int i = 0; i < 1000; ++i) {
        content << generateFastQRecord("read_" + std::to_string(i), 
                                      "ATCGATCGATCG", 
                                      "IIIIIIIIIIII");
    }
    
    auto file_path = createTestFastQFile(content.str());
    
    FileInferrer inferrer(file_path);
    auto attrs = inferrer.infer_attributes(1000);
    
    EXPECT_EQ(attrs.read_length, 12);
    EXPECT_FALSE(attrs.is_variable_length);
    EXPECT_GT(attrs.average_quality, 30.0);
    EXPECT_GT(attrs.gc_content, 0.0);
    EXPECT_GE(attrs.estimated_record_count, 1000);
}

// --- Malformed FastQ File Tests ---

TEST_F(FastQFileTest, MalformedFastQFile_MissingHeader) {
    // 缺少头部的FastQ文件
    std::string content = 
        "ATCGATCGATCG\n"
        "+\n"
        "IIIIIIIIIIII\n";
    
    auto file_path = createTestFastQFile(content);
    
    FileInferrer inferrer(file_path);
    
    // 应该能处理，但记录数可能为0
    auto attrs = inferrer.infer_attributes(100);
    EXPECT_EQ(attrs.estimated_record_count, 0);
}

TEST_F(FastQFileTest, MalformedFastQFile_MissingSeparator) {
    // 缺少分隔符的FastQ文件
    std::string content = 
        "@test_read\n"
        "ATCGATCGATCG\n"
        "IIIIIIIIIIII\n";
    
    auto file_path = createTestFastQFile(content);
    
    FileInferrer inferrer(file_path);
    
    // 应该能处理，但记录数可能为0
    auto attrs = inferrer.infer_attributes(100);
    EXPECT_EQ(attrs.estimated_record_count, 0);
}

TEST_F(FastQFileTest, MalformedFastQFile_TruncatedRecord) {
    // 截断的FastQ记录
    std::string content = 
        "@test_read_001\n"
        "ATCGATCGATCG\n"
        "+\n"
        "IIIIIIIIIIII\n"
        "@test_read_002\n"
        "ATCGATCGATCG\n"
        "+\n";
    
    auto file_path = createTestFastQFile(content);
    
    FileInferrer inferrer(file_path);
    
    // 应该能处理，但只计算完整记录
    auto attrs = inferrer.infer_attributes(100);
    EXPECT_GT(attrs.estimated_record_count, 0);
}

// --- Special Character Tests ---

TEST_F(FastQFileTest, SpecialCharactersInHeaders) {
    // 头部包含特殊字符
    std::string content = 
        "@test/read:001/1\n"
        "ATCGATCGATCG\n"
        "+\n"
        "IIIIIIIIIIII\n"
        "@test-read_002/2\n"
        "GCTAGCTAGCTA\n"
        "+\n"
        "IIIIIIIIIIII\n";
    
    auto file_path = createTestFastQFile(content);
    
    FileInferrer inferrer(file_path);
    auto attrs = inferrer.infer_attributes(100);
    
    EXPECT_EQ(attrs.read_length, 12);
    EXPECT_FALSE(attrs.is_variable_length);
    EXPECT_GT(attrs.average_quality, 30.0);
    EXPECT_GT(attrs.gc_content, 0.0);
    EXPECT_GT(attrs.estimated_record_count, 0);
}

TEST_F(FastQFileTest, LowercaseSequences) {
    // 小写序列
    std::string content = 
        "@test_read\n"
        "atcgatcgatcg\n"
        "+\n"
        "IIIIIIIIIIII\n";
    
    auto file_path = createTestFastQFile(content);
    
    FileInferrer inferrer(file_path);
    auto attrs = inferrer.infer_attributes(100);
    
    EXPECT_EQ(attrs.read_length, 12);
    EXPECT_FALSE(attrs.is_variable_length);
    EXPECT_GT(attrs.average_quality, 30.0);
    EXPECT_GT(attrs.gc_content, 0.0);
    EXPECT_GT(attrs.estimated_record_count, 0);
}

// --- File Attribute Calculation Tests ---

TEST_F(FastQFileTest, FileAttributeCalculations) {
    // 测试文件属性计算
    
    // 创建已知属性的文件
    std::string content = 
        "@read_001\n"
        "AAAA\n"  // 0% GC
        "+\n"
        "IIII\n"
        "@read_002\n"
        "CCCC\n"  // 100% GC
        "+\n"
        "IIII\n"
        "@read_003\n"
        "ATCG\n"  // 50% GC
        "+\n"
        "IIII\n"
        "@read_004\n"
        "AAAA\n"  // 0% GC
        "+\n"
        "!!!!\n";  // 低质量
    
    auto file_path = createTestFastQFile(content);
    
    FileInferrer inferrer(file_path);
    auto attrs = inferrer.infer_attributes(100);
    
    EXPECT_EQ(attrs.read_length, 4);
    EXPECT_FALSE(attrs.is_variable_length);
    
    // GC含量应该是 (0 + 100 + 50 + 0) / 4 = 37.5%
    EXPECT_DOUBLE_EQ(attrs.gc_content, 37.5);
    
    // 平均质量应该是 (40 + 40 + 40 + 0) / 4 = 30
    EXPECT_DOUBLE_EQ(attrs.average_quality, 30.0);
    
    EXPECT_EQ(attrs.estimated_record_count, 4);
}

// --- File Inference Performance Tests ---

TEST_F(FastQFileTest, FileInferencePerformance) {
    // 测试文件推断性能
    
    // 创建大文件
    std::stringstream content;
    for (int i = 0; i < 10000; ++i) {
        content << generateFastQRecord("read_" + std::to_string(i), 
                                      "ATCGATCGATCG", 
                                      "IIIIIIIIIIII");
    }
    
    auto file_path = createTestFastQFile(content.str());
    
    // 测试推断性能
    auto start = std::chrono::high_resolution_clock::now();
    
    FileInferrer inferrer(file_path);
    auto attrs = inferrer.infer_attributes(10000);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_EQ(attrs.read_length, 12);
    EXPECT_FALSE(attrs.is_variable_length);
    EXPECT_GT(attrs.average_quality, 30.0);
    EXPECT_GT(attrs.gc_content, 0.0);
    EXPECT_GE(attrs.estimated_record_count, 10000);
    
    // 应该在合理时间内完成
    EXPECT_LT(duration.count(), 1000);
}

// --- File Format Detection Tests ---

TEST_F(FastQFileTest, FileFormatDetection) {
    // 测试文件格式检测
    
    // 创建不同格式的文件
    std::vector<std::pair<std::string, fq::core::QScoreType>> test_cases = {
        {"!", fq::core::QScoreType::Sanger},    // Phred+33, 0
        {"\"", fq::core::QScoreType::Sanger},   // Phred+33, 1
        {"I", fq::core::QScoreType::Sanger},    // Phred+33, 40
        {"@", fq::core::QScoreType::Illumina13}, // Phred+64, 0
        {"A", fq::core::QScoreType::Illumina13}, // Phred+64, 1
        {"h", fq::core::QScoreType::Illumina13}, // Phred+64, 40
        {"C", fq::core::QScoreType::Illumina15}, // Phred+64, 35
        {"5", fq::core::QScoreType::Illumina18}  // Phred+33, 20
    };
    
    for (const auto& [qual_char, expected_type] : test_cases) {
        std::string content = 
            "@test_read\n"
            "ATCG\n"
            "+\n"
            + std::string(4, qual_char) + "\n";
        
        auto file_path = createTestFastQFile(content);
        
        FileInferrer inferrer(file_path);
        auto attrs = inferrer.infer_attributes(100);
        
        EXPECT_EQ(attrs.q_score_type, expected_type) 
            << "Quality character '" << qual_char << "' should map to " 
            << static_cast<int>(expected_type);
    }
}

// --- Edge Cases Tests ---

TEST_F(FastQFileTest, EdgeCase_VeryLongReads) {
    // 测试超长读长
    std::string long_sequence(10000, 'A');
    std::string long_quality(10000, 'I');
    
    std::string content = 
        "@long_read\n"
        + long_sequence + "\n"
        "+\n"
        + long_quality + "\n";
    
    auto file_path = createTestFastQFile(content);
    
    FileInferrer inferrer(file_path);
    auto attrs = inferrer.infer_attributes(100);
    
    EXPECT_EQ(attrs.read_length, 10000);
    EXPECT_FALSE(attrs.is_variable_length);
    EXPECT_GT(attrs.average_quality, 30.0);
    EXPECT_DOUBLE_EQ(attrs.gc_content, 0.0);
    EXPECT_GT(attrs.estimated_record_count, 0);
}

TEST_F(FastQFileTest, EdgeCase_ZeroLengthReads) {
    // 测试零长度读长
    std::string content = 
        "@empty_read\n"
        "\n"
        "+\n"
        "\n";
    
    auto file_path = createTestFastQFile(content);
    
    FileInferrer inferrer(file_path);
    auto attrs = inferrer.infer_attributes(100);
    
    EXPECT_EQ(attrs.read_length, 0);
    EXPECT_FALSE(attrs.is_variable_length);
    EXPECT_GT(attrs.estimated_record_count, 0);
}

TEST_F(FastQFileTest, EdgeCase_MixedQualityEncodings) {
    // 测试混合质量编码
    std::string content = 
        "@mixed_read\n"
        "ATCG\n"
        "+\n"
        "!I@A\n"; // 混合不同编码的质量字符
    
    auto file_path = createTestFastQFile(content);
    
    FileInferrer inferrer(file_path);
    auto attrs = inferrer.infer_attributes(100);
    
    // 应该能处理混合编码
    EXPECT_EQ(attrs.read_length, 4);
    EXPECT_GT(attrs.average_quality, 0.0);
    EXPECT_GT(attrs.estimated_record_count, 0);
}

} // namespace fq::fastq