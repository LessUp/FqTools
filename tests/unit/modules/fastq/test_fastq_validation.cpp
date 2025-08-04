#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <filesystem>

#include "modules/fastq/fastq.h"
#include "utils/test_helpers.h"

namespace fq::fastq {

// --- FastQ Record Validation Tests ---

class FastQValidationTest : public fq::test::FastQToolsTest {
protected:
    void SetUp() override {
        fq::test::FastQToolsTest::SetUp();
    }
    
    void TearDown() override {
        fq::test::FastQToolsTest::TearDown();
    }
    
    // 创建测试用的FastQ记录
    FqRecord createTestRecord(const std::string& name, 
                             const std::string& sequence, 
                             const std::string& quality) {
        auto total_size = name.size() + sequence.size() + quality.size();
        auto buffer = std::make_shared<fq::io::SharedBuffer>(total_size);
        
        std::size_t offset = 0;
        buffer->write_string(name, offset);
        auto name_offset = offset;
        auto name_length = name.size();
        offset += name_length;
        
        buffer->write_string(sequence, offset);
        auto sequence_offset = offset;
        auto sequence_length = sequence.size();
        offset += sequence_length;
        
        buffer->write_string(quality, offset);
        auto quality_offset = offset;
        auto quality_length = quality.size();
        
        return FqRecord(buffer, name_offset, name_length, 
                       sequence_offset, sequence_length,
                       quality_offset, quality_length);
    }
    
    MutableFqRecord createMutableTestRecord(const std::string& name, 
                                           const std::string& sequence, 
                                           const std::string& quality) {
        return MutableFqRecord(name, sequence, quality);
    }
};

// --- DNA Sequence Validation Tests ---

TEST_F(FastQValidationTest, ValidDNASequences) {
    // 有效的DNA序列
    auto valid_sequences = {
        "ATCG", "AAAA", "CCCC", "GGGG", "TTTT",
        "ATCGATCGATCG", "A", "C", "G", "T",
        "atcg", "AtCg", "aTcG" // 大小写混合
    };
    
    for (const auto& seq : valid_sequences) {
        auto record = createMutableTestRecord("@test", seq, std::string(seq.length(), 'I'));
        EXPECT_TRUE(record.is_valid()) << "Sequence '" << seq << "' should be valid";
    }
}

TEST_F(FastQValidationTest, InvalidDNASequences) {
    // 无效的DNA序列
    auto invalid_sequences = {
        "ATXG", // X不是有效碱基
        "ATCG-", // -不是有效碱基
        "AT CG", // 空格不是有效碱基
        "AT\nCG", // 换行符不是有效碱基
        "ATCG123", // 数字不是有效碱基
        "", // 空序列
        "N", // N碱基需要特殊处理
        "ATCGN" // 包含N碱基
    };
    
    for (const auto& seq : invalid_sequences) {
        auto record = createMutableTestRecord("@test", seq, std::string(seq.length(), 'I'));
        EXPECT_FALSE(record.is_valid()) << "Sequence '" << seq << "' should be invalid";
    }
}

// --- Quality String Validation Tests ---

TEST_F(FastQValidationTest, ValidQualityStrings) {
    // 有效的质量字符串
    auto valid_qualities = {
        "IIII", // Phred+33, 高质量
        "!!!!", // Phred+33, 低质量
        "JJJJ", // Phred+33, 高质量
        "\"\"\"\"", // Phred+33, 低质量
        "!!!!!!!!!!!!!!!!!!!!", // 长质量字符串
        "IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII" // 50个I
    };
    
    for (const auto& qual : valid_qualities) {
        auto record = createMutableTestRecord("@test", "ATCG", qual);
        EXPECT_TRUE(record.is_valid()) << "Quality string '" << qual << "' should be valid";
    }
}

TEST_F(FastQValidationTest, InvalidQualityStrings) {
    // 无效的质量字符串
    auto invalid_qualities = {
        "III", // 质量字符串长度与序列不匹配
        "IIIII", // 质量字符串长度与序列不匹配
        "I!I!", // 包含非ASCII字符
        "IIII@", // @是无效的质量字符
        "IIII[", // [是无效的质量字符
        "IIII ", // 空格是无效的质量字符
        "", // 空质量字符串
        "IIII\n", // 包含换行符
        "IIII\t", // 包含制表符
        "IIII\r", // 包含回车符
        "IIII\x00", // 包含空字符
        "IIII\x7F", // 包含DEL字符
        "IIII\x80" // 包含非ASCII字符
    };
    
    for (const auto& qual : invalid_qualities) {
        auto record = createMutableTestRecord("@test", "ATCG", qual);
        EXPECT_FALSE(record.is_valid()) << "Quality string '" << qual << "' should be invalid";
    }
}

// --- Sequence-Quality Length Matching Tests ---

TEST_F(FastQValidationTest, SequenceQualityLengthMatch) {
    // 序列和质量长度匹配
    auto valid_pairs = {
        std::make_pair("A", "I"),
        std::make_pair("AT", "II"),
        std::make_pair("ATC", "III"),
        std::make_pair("ATCG", "IIII"),
        std::make_pair("ATCGATCGATCG", "IIIIIIIIIIII"),
        std::make_pair("", "") // 空序列和质量
    };
    
    for (const auto& [seq, qual] : valid_pairs) {
        auto record = createMutableTestRecord("@test", seq, qual);
        EXPECT_TRUE(record.is_valid()) << "Sequence '" << seq << "' and quality '" << qual << "' should match";
    }
}

TEST_F(FastQValidationTest, SequenceQualityLengthMismatch) {
    // 序列和质量长度不匹配
    auto invalid_pairs = {
        std::make_pair("A", ""), // 序列长度1，质量长度0
        std::make_pair("A", "II"), // 序列长度1，质量长度2
        std::make_pair("AT", "I"), // 序列长度2，质量长度1
        std::make_pair("AT", "III"), // 序列长度2，质量长度3
        std::make_pair("ATCG", "II"), // 序列长度4，质量长度2
        std::make_pair("ATCG", "IIIII"), // 序列长度4，质量长度5
        std::make_pair("", "I"), // 空序列，非空质量
        std::make_pair("ATCG", "") // 非空序列，空质量
    };
    
    for (const auto& [seq, qual] : invalid_pairs) {
        auto record = createMutableTestRecord("@test", seq, qual);
        EXPECT_FALSE(record.is_valid()) << "Sequence '" << seq << "' and quality '" << qual << "' should not match";
    }
}

// --- Record Validation Error Messages Tests ---

TEST_F(FastQValidationTest, ValidationErrorMessages) {
    // 测试验证错误消息
    
    // 空记录
    MutableFqRecord empty_record;
    auto errors = empty_record.validation_errors();
    EXPECT_FALSE(errors.empty());
    EXPECT_TRUE(std::find(errors.begin(), errors.end(), "Sequence and quality length mismatch") != errors.end());
    
    // 无效DNA序列
    MutableFqRecord invalid_dna("@test", "ATXG", "IIII");
    errors = invalid_dna.validation_errors();
    EXPECT_FALSE(errors.empty());
    EXPECT_TRUE(std::find(errors.begin(), errors.end(), "Invalid DNA sequence") != errors.end());
    
    // 无效质量字符串
    MutableFqRecord invalid_quality("@test", "ATCG", "I!I!");
    errors = invalid_quality.validation_errors();
    EXPECT_FALSE(errors.empty());
    EXPECT_TRUE(std::find(errors.begin(), errors.end(), "Invalid quality string") != errors.end());
    
    // 长度不匹配
    MutableFqRecord length_mismatch("@test", "ATCG", "II");
    errors = length_mismatch.validation_errors();
    EXPECT_FALSE(errors.empty());
    EXPECT_TRUE(std::find(errors.begin(), errors.end(), "Sequence and quality length mismatch") != errors.end());
}

// --- Record Transformation Tests ---

TEST_F(FastQValidationTest, TransformationPreservesValidity) {
    // 创建有效记录
    auto record = createMutableTestRecord("@test", "ATCGATCG", "IIIIIIII");
    EXPECT_TRUE(record.is_valid());
    
    // 测试修剪操作
    record.trim_left(2);
    EXPECT_TRUE(record.is_valid());
    
    // 测试右侧修剪
    record.trim_right(1);
    EXPECT_TRUE(record.is_valid());
    
    // 测试反向互补
    record.reverse_complement();
    EXPECT_TRUE(record.is_valid());
    
    // 测试大小写转换
    record.to_uppercase();
    EXPECT_TRUE(record.is_valid());
    
    record.to_lowercase();
    EXPECT_TRUE(record.is_valid());
}

TEST_F(FastQValidationTest, TransformationEdgeCases) {
    // 测试边界情况的变换
    
    auto record = createMutableTestRecord("@test", "ATCG", "IIII");
    
    // 修剪整个序列
    record.trim_left(4);
    EXPECT_TRUE(record.is_valid());
    EXPECT_TRUE(record.sequence().empty());
    EXPECT_TRUE(record.quality().empty());
    
    // 修剪超过序列长度
    auto record2 = createMutableTestRecord("@test", "ATCG", "IIII");
    record2.trim_left(10);
    EXPECT_TRUE(record2.is_valid());
    EXPECT_TRUE(record2.sequence().empty());
    EXPECT_TRUE(record2.quality().empty());
}

// --- Quality Score Calculations Tests ---

TEST_F(FastQValidationTest, QualityScoreCalculations) {
    // 测试质量分数计算
    
    // 高质量分数
    auto high_quality = createMutableTestRecord("@test", "ATCG", "IIII");
    EXPECT_DOUBLE_EQ(high_quality.calculate_average_quality(), 40.0);
    
    // 低质量分数
    auto low_quality = createMutableTestRecord("@test", "ATCG", "!!!!");
    EXPECT_DOUBLE_EQ(low_quality.calculate_average_quality(), 0.0);
    
    // 混合质量分数
    auto mixed_quality = createMutableTestRecord("@test", "ATCG", "!I!I");
    double expected = (0.0 + 40.0 + 0.0 + 40.0) / 4.0;
    EXPECT_DOUBLE_EQ(mixed_quality.calculate_average_quality(), expected);
}

// --- GC Content Calculations Tests ---

TEST_F(FastQValidationTest, GCContentCalculations) {
    // 测试GC含量计算
    
    // 纯A/T序列
    auto at_only = createMutableTestRecord("@test", "AAAA", "IIII");
    EXPECT_DOUBLE_EQ(at_only.calculate_gc_content(), 0.0);
    
    // 纯G/C序列
    auto gc_only = createMutableTestRecord("@test", "GGGG", "IIII");
    EXPECT_DOUBLE_EQ(gc_only.calculate_gc_content(), 100.0);
    
    // 混合序列
    auto mixed = createMutableTestRecord("@test", "ATCG", "IIII");
    EXPECT_DOUBLE_EQ(mixed.calculate_gc_content(), 50.0);
    
    // 更复杂的序列
    auto complex = createMutableTestRecord("@test", "ATGCGCTA", "IIIIIIII");
    EXPECT_DOUBLE_EQ(complex.calculate_gc_content(), 50.0);
}

// --- Sequence Complexity Tests ---

TEST_F(FastQValidationTest, SequenceComplexity) {
    // 测试序列复杂度计算
    
    // 单碱基重复序列
    auto simple = createMutableTestRecord("@test", "AAAAAAAA", "IIIIIIII");
    double simple_complexity = simple.calculate_complexity();
    EXPECT_DOUBLE_EQ(simple_complexity, 0.0);
    
    // 完全随机序列
    auto complex = createMutableTestRecord("@test", "ATCGATCG", "IIIIIIII");
    double complex_complexity = complex.calculate_complexity();
    EXPECT_GT(complex_complexity, simple_complexity);
    
    // 部分重复序列
    auto partial = createMutableTestRecord("@test", "ATATATAT", "IIIIIIII");
    double partial_complexity = partial.calculate_complexity();
    EXPECT_GT(partial_complexity, simple_complexity);
    EXPECT_LT(partial_complexity, complex_complexity);
}

// --- Buffer Management Tests ---

TEST_F(FastQValidationTest, BufferManagement) {
    // 测试缓冲区管理
    
    // 创建共享缓冲区
    auto buffer = std::make_shared<fq::io::SharedBuffer>(100);
    
    // 创建记录
    auto record = createTestRecord("@test", "ATCG", "IIII");
    
    // 测试内存使用
    EXPECT_GT(record.memory_usage(), 0);
    
    // 测试缓冲区生命周期
    std::weak_ptr<fq::io::SharedBuffer> weak_buffer = buffer;
    
    {
        auto record2 = createTestRecord("@test2", "GCTA", "IIII");
        EXPECT_FALSE(weak_buffer.expired());
    }
    
    // 记录销毁后，缓冲区应该仍然存在
    EXPECT_FALSE(weak_buffer.expired());
}

// --- Performance Tests ---

TEST_F(FastQValidationTest, ValidationPerformance) {
    // 测试验证性能
    
    // 创建大量记录
    std::vector<MutableFqRecord> records;
    records.reserve(1000);
    
    for (int i = 0; i < 1000; ++i) {
        records.emplace_back("@test_" + std::to_string(i), "ATCGATCGATCG", "IIIIIIIIIIII");
    }
    
    // 验证所有记录
    for (const auto& record : records) {
        EXPECT_TRUE(record.is_valid());
    }
    
    // 测试批量验证
    auto start = std::chrono::high_resolution_clock::now();
    
    size_t valid_count = 0;
    for (const auto& record : records) {
        if (record.is_valid()) {
            valid_count++;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_EQ(valid_count, 1000);
    EXPECT_LT(duration.count(), 100); // 应该在100ms内完成
}

// --- Error Handling Tests ---

TEST_F(FastQValidationTest, ErrorHandling) {
    // 测试错误处理
    
    // 测试构造函数异常
    EXPECT_THROW(
        createMutableTestRecord("@test", "ATXG", "IIII"),
        fq::error::FastQException
    );
    
    // 测试无效偏移量
    auto buffer = std::make_shared<fq::io::SharedBuffer>(10);
    EXPECT_THROW(
        FqRecord record(buffer, 0, 5, 8, 5, 13, 5),
        fq::error::FastQException
    );
    
    // 测试空缓冲区
    FqRecord empty_buffer_record;
    EXPECT_FALSE(empty_buffer_record.is_valid());
    
    auto errors = empty_buffer_record.validation_errors();
    EXPECT_FALSE(errors.empty());
    EXPECT_TRUE(std::find(errors.begin(), errors.end(), "No buffer attached") != errors.end());
}

} // namespace fq::fastq