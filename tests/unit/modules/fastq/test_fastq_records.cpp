#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <filesystem>

#include "modules/fastq/fastq.h"
#include "utils/test_helpers.h"

namespace fq::fastq {

// --- FqRecord Tests ---

TEST(FastQTest, FqRecord_DefaultConstructor) {
    FqRecord record;
    
    EXPECT_TRUE(record.name().empty());
    EXPECT_TRUE(record.sequence().empty());
    EXPECT_TRUE(record.quality().empty());
    EXPECT_EQ(record.length(), 0);
}

TEST(FastQTest, FqRecord_ParameterizedConstructor) {
    // 创建测试数据
    std::string name = "@test_read_001";
    std::string sequence = "ATCGATCGATCG";
    std::string quality = "IIIIIIIIIIII";
    
    // 创建共享缓冲区
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
    
    // 创建FqRecord
    FqRecord record(buffer, name_offset, name_length, 
                   sequence_offset, sequence_length,
                   quality_offset, quality_length);
    
    EXPECT_EQ(record.name(), name);
    EXPECT_EQ(record.sequence(), sequence);
    EXPECT_EQ(record.quality(), quality);
    EXPECT_EQ(record.length(), sequence_length);
    EXPECT_TRUE(record.is_valid());
}

TEST(FastQTest, FqRecord_InvalidOffsets) {
    auto buffer = std::make_shared<fq::io::SharedBuffer>(10);
    
    // 测试无效偏移量
    EXPECT_THROW(
        FqRecord record(buffer, 0, 5, 8, 5, 13, 5), // 超出缓冲区范围
        fq::error::FastQException
    );
}

TEST(FastQTest, FqRecord_QualityCalculations) {
    // 创建测试数据
    std::string name = "@test_read_001";
    std::string sequence = "ATCGATCGATCG";
    std::string quality = "IIIIIIIIIIII"; // 高质量分数
    
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
    
    FqRecord record(buffer, name_offset, name_length, 
                   sequence_offset, sequence_length,
                   quality_offset, quality_length);
    
    EXPECT_DOUBLE_EQ(record.calculate_average_quality(), 40.0);
    EXPECT_DOUBLE_EQ(record.calculate_gc_content(), 50.0);
    EXPECT_GT(record.calculate_complexity(), 0.0);
}

TEST(FastQTest, FqRecord_ToMutable) {
    // 创建FqRecord
    std::string name = "@test_read_001";
    std::string sequence = "ATCGATCGATCG";
    std::string quality = "IIIIIIIIIIII";
    
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
    
    FqRecord record(buffer, name_offset, name_length, 
                   sequence_offset, sequence_length,
                   quality_offset, quality_length);
    
    // 转换为MutableFqRecord
    auto mutable_record = record.to_mutable();
    
    EXPECT_EQ(mutable_record.name(), name);
    EXPECT_EQ(mutable_record.sequence(), sequence);
    EXPECT_EQ(mutable_record.quality(), quality);
    EXPECT_EQ(mutable_record.length(), sequence_length);
    EXPECT_TRUE(mutable_record.is_valid());
}

// --- MutableFqRecord Tests ---

TEST(FastQTest, MutableFqRecord_DefaultConstructor) {
    MutableFqRecord record;
    
    EXPECT_TRUE(record.name().empty());
    EXPECT_TRUE(record.sequence().empty());
    EXPECT_TRUE(record.quality().empty());
    EXPECT_EQ(record.length(), 0);
}

TEST(FastQTest, MutableFqRecord_ParameterizedConstructor) {
    std::string name = "@test_read_001";
    std::string sequence = "ATCGATCGATCG";
    std::string quality = "IIIIIIIIIIII";
    
    MutableFqRecord record(name, sequence, quality);
    
    EXPECT_EQ(record.name(), name);
    EXPECT_EQ(record.sequence(), sequence);
    EXPECT_EQ(record.quality(), quality);
    EXPECT_EQ(record.length(), sequence.length());
    EXPECT_TRUE(record.is_valid());
}

TEST(FastQTest, MutableFqRecord_InvalidData) {
    // 测试序列和质量长度不匹配
    EXPECT_THROW(
        MutableFqRecord("@test", "ATCG", "I"), // 序列长度4，质量长度1
        fq::error::FastQException
    );
    
    // 测试无效DNA序列
    EXPECT_THROW(
        MutableFqRecord("@test", "ATXCG", "IIIII"),
        fq::error::FastQException
    );
    
    // 测试无效质量字符串
    EXPECT_THROW(
        MutableFqRecord("@test", "ATCG", "I!I!I"),
        fq::error::FastQException
    );
}

TEST(FastQTest, MutableFqRecord_TrimOperations) {
    std::string name = "@test_read_001";
    std::string sequence = "ATCGATCGATCG";
    std::string quality = "IIIIIIIIIIII";
    
    MutableFqRecord record(name, sequence, quality);
    
    // 测试左侧修剪
    record.trim_left(3);
    EXPECT_EQ(record.sequence(), "GATCGATCG");
    EXPECT_EQ(record.quality(), "IIIIIIIII");
    
    // 重新创建记录测试右侧修剪
    MutableFqRecord record2(name, sequence, quality);
    record2.trim_right(3);
    EXPECT_EQ(record2.sequence(), "ATCGATCG");
    EXPECT_EQ(record2.quality(), "IIIIIIIII");
    
    // 测试质量修剪
    MutableFqRecord record3(name, sequence, quality);
    record3.trim_quality(35.0); // 修剪低于35的质量
    EXPECT_EQ(record3.sequence(), sequence); // 所有质量都高于35，应该保持不变
    EXPECT_EQ(record3.quality(), quality);
}

TEST(FastQTest, MutableFqRecord_SequenceOperations) {
    std::string name = "@test_read_001";
    std::string sequence = "atcgatcgatcg";
    std::string quality = "IIIIIIIIIIII";
    
    MutableFqRecord record(name, sequence, quality);
    
    // 测试反向互补
    record.reverse_complement();
    EXPECT_EQ(record.sequence(), "CGATCGATCGAT");
    EXPECT_EQ(record.quality(), "IIIIIIIIIIII");
    
    // 重新创建记录测试大小写转换
    MutableFqRecord record2(name, sequence, quality);
    record2.to_uppercase();
    EXPECT_EQ(record2.sequence(), "ATCGATCGATCG");
    
    MutableFqRecord record3(name, sequence, quality);
    record3.to_lowercase();
    EXPECT_EQ(record3.sequence(), "atcgatcgatcg");
}

TEST(FastQTest, MutableFqRecord_ToShared) {
    std::string name = "@test_read_001";
    std::string sequence = "ATCGATCGATCG";
    std::string quality = "IIIIIIIIIIII";
    
    MutableFqRecord mutable_record(name, sequence, quality);
    
    // 转换为FqRecord
    auto shared_record = mutable_record.to_shared();
    
    EXPECT_EQ(shared_record.name(), name);
    EXPECT_EQ(shared_record.sequence(), sequence);
    EXPECT_EQ(shared_record.quality(), quality);
    EXPECT_EQ(shared_record.length(), sequence.length());
    EXPECT_TRUE(shared_record.is_valid());
}

// --- FqBatch Tests ---

TEST(FastQTest, FqBatch_BasicOperations) {
    FqBatch batch;
    
    EXPECT_TRUE(batch.empty());
    EXPECT_EQ(batch.size(), 0);
    
    // 添加记录
    std::string name = "@test_read_001";
    std::string sequence = "ATCGATCGATCG";
    std::string quality = "IIIIIIIIIIII";
    
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
    
    FqRecord record(buffer, name_offset, name_length, 
                   sequence_offset, sequence_length,
                   quality_offset, quality_length);
    
    batch.add_record(std::move(record));
    
    EXPECT_FALSE(batch.empty());
    EXPECT_EQ(batch.size(), 1);
    EXPECT_EQ(batch[0].length(), sequence_length);
}

TEST(FastQTest, FqBatch_Statistics) {
    FqBatch batch;
    
    // 添加多个记录
    for (int i = 0; i < 3; ++i) {
        std::string name = "@test_read_" + std::to_string(i);
        std::string sequence = "ATCGATCGATCG";
        std::string quality = "IIIIIIIIIIII";
        
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
        
        FqRecord record(buffer, name_offset, name_length, 
                       sequence_offset, sequence_length,
                       quality_offset, quality_length);
        
        batch.add_record(std::move(record));
    }
    
    EXPECT_EQ(batch.size(), 3);
    EXPECT_EQ(batch.calculate_total_bases(), 36); // 3 * 12
    EXPECT_DOUBLE_EQ(batch.calculate_average_length(), 12.0);
    EXPECT_DOUBLE_EQ(batch.calculate_average_quality(), 40.0);
}

TEST(FastQTest, FqBatch_RemoveInvalidRecords) {
    MutableFqBatch batch;
    
    // 添加有效记录
    batch.add_record(MutableFqRecord("@valid1", "ATCG", "IIII"));
    batch.add_record(MutableFqRecord("@valid2", "GCTA", "IIII"));
    
    // 添加无效记录
    batch.add_record(MutableFqRecord("@invalid1", "ATXCG", "IIIII")); // 无效DNA
    batch.add_record(MutableFqRecord("@invalid2", "ATCG", "I!I!I")); // 无效质量
    
    EXPECT_EQ(batch.size(), 4);
    EXPECT_EQ(batch.count_valid_records(), 2);
    
    // 移除无效记录
    batch.remove_invalid_records();
    
    EXPECT_EQ(batch.size(), 2);
    EXPECT_EQ(batch.count_valid_records(), 2);
}

// --- FileInferrer Tests ---

TEST(FastQTest, FileInferrer_NonExistentFile) {
    std::filesystem::path non_existent = "/tmp/non_existent.fastq";
    
    EXPECT_THROW(
        FileInferrer inferrer(non_existent),
        fq::error::FastQException
    );
}

TEST(FastQTest, FileInferrer_ValidFile) {
    // 创建测试FastQ文件
    std::string fastq_content = 
        "@test_read_001\n"
        "ATCGATCGATCG\n"
        "+\n"
        "IIIIIIIIIIII\n"
        "@test_read_002\n"
        "GCTAGCTAGCTA\n"
        "+\n"
        "IIIIIIIIIIII\n";
    
    auto temp_file = fq::test::TestHelpers::createTempFile(fastq_content, ".fastq");
    
    FileInferrer inferrer(temp_file);
    auto attrs = inferrer.infer_attributes(10);
    
    EXPECT_EQ(attrs.read_length, 12);
    EXPECT_FALSE(attrs.is_variable_length);
    EXPECT_GT(attrs.average_quality, 30.0);
    EXPECT_GT(attrs.gc_content, 0.0);
    EXPECT_GT(attrs.estimated_record_count, 0);
}

TEST(FastQTest, FileInferrer_VariableLength) {
    // 创建变长FastQ文件
    std::string fastq_content = 
        "@test_read_001\n"
        "ATCG\n"
        "+\n"
        "IIII\n"
        "@test_read_002\n"
        "GCTAGCTAGCTA\n"
        "+\n"
        "IIIIIIIIIIII\n";
    
    auto temp_file = fq::test::TestHelpers::createTempFile(fastq_content, ".fastq");
    
    FileInferrer inferrer(temp_file);
    auto attrs = inferrer.infer_attributes(10);
    
    EXPECT_EQ(attrs.read_length, 12);
    EXPECT_TRUE(attrs.is_variable_length);
}

// --- Edge Cases and Error Handling ---

TEST(FastQTest, EdgeCases_EmptyRecords) {
    FqRecord record;
    EXPECT_TRUE(record.name().empty());
    EXPECT_TRUE(record.sequence().empty());
    EXPECT_TRUE(record.quality().empty());
    EXPECT_EQ(record.length(), 0);
    EXPECT_FALSE(record.is_valid()); // 空记录应该无效
}

TEST(FastQTest, EdgeCases_BufferLifecycle) {
    // 测试缓冲区生命周期管理
    auto buffer = std::make_shared<fq::io::SharedBuffer>(100);
    
    {
        FqRecord record(buffer, 0, 10, 10, 10, 20, 10);
        EXPECT_EQ(record.memory_usage(), buffer->memory_usage());
    }
    
    // 记录销毁后，缓冲区应该仍然存在
    EXPECT_GT(buffer->ref_count(), 0);
}

TEST(FastQTest, EdgeCases_LargeRecords) {
    // 测试大记录处理
    std::string name = "@large_read";
    std::string sequence(10000, 'A');
    std::string quality(10000, 'I');
    
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
    
    FqRecord record(buffer, name_offset, name_length, 
                   sequence_offset, sequence_length,
                   quality_offset, quality_length);
    
    EXPECT_EQ(record.length(), 10000);
    EXPECT_TRUE(record.is_valid());
    EXPECT_DOUBLE_EQ(record.calculate_gc_content(), 0.0); // 纯A序列
}

} // namespace fq::fastq