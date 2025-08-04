/**
 * @file fastq.h
 * @brief FastQ记录的类与操作函数定义。
 *
 * 此头文件包含处理FastQ格式记录的类与函数，用于高效的序列数据处理与验证。
 */

#pragma once

// 传统头文件使用，尚未模块化
// export module fq.fastq;  // 当前尚不支持此模块导出

#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <span>
#include <algorithm>

#include "../core/core.h"
#include "../io/io.h"
#include "../error/error.h"

// import fq.core;
// import fq.io;
// import fq.error;

namespace fq::fastq {
    /**
     * @class FqRecord
     * @brief 零拷贝FastQ记录。
     *
     * 使用共享缓冲区减少内存拷贝，提供序列和质量值的高效访问。
     */
    class FqRecord : public fq::core::WithID, public fq::core::MemoryTrackable, public fq::core::Validatable {
    public:
        /**
         * @brief 默认构造函数
         * @details 创建空的FqRecord实例
         * 
         * @post 空的FqRecord实例被创建
         */
        FqRecord() = default;
        
        /**
         * @brief 参数化构造函数
         * @details 使用共享缓冲区和偏移量创建FqRecord实例
         * 
         * @param buffer 共享缓冲区指针
         * @param name_offset 名称偏移量
         * @param name_length 名称长度
         * @param sequence_offset 序列偏移量
         * @param sequence_length 序列长度
         * @param quality_offset 质量偏移量
         * @param quality_length 质量长度
         * @pre buffer必须是有效的共享指针
         * @pre 所有偏移量和长度必须在缓冲区范围内
         * @post FqRecord实例被创建并初始化
         * @throw FastQException 如果偏移量验证失败
         */
        explicit FqRecord(std::shared_ptr<fq::io::SharedBuffer> buffer,
                         std::size_t name_offset, std::size_t name_length,
                         std::size_t sequence_offset, std::size_t sequence_length,
                         std::size_t quality_offset, std::size_t quality_length)
            : m_buffer(std::move(buffer))
            , m_name_offset(name_offset)
            , m_name_length(name_length)
            , m_sequence_offset(sequence_offset)
            , m_sequence_length(sequence_length)
            , m_quality_offset(quality_offset)
            , m_quality_length(quality_length)
        {
            if (!validate_offsets()) {
                FQ_THROW_VALIDATION_ERROR("FqRecord", "invalid buffer offsets");
            }
        }
        
        [[nodiscard]] auto name() const noexcept -> std::string_view {
            if (!m_buffer || m_name_length == 0) {
                return {};
            }
            auto data = reinterpret_cast<const char*>(m_buffer->data() + m_name_offset);
            return std::string_view(data, m_name_length);
        }
        
        [[nodiscard]] auto sequence() const noexcept -> std::string_view {
            if (!m_buffer || m_sequence_length == 0) {
                return {};
            }
            auto data = reinterpret_cast<const char*>(m_buffer->data() + m_sequence_offset);
            return std::string_view(data, m_sequence_length);
        }
        
        [[nodiscard]] auto quality() const noexcept -> std::string_view {
            if (!m_buffer || m_quality_length == 0) {
                return {};
            }
            auto data = reinterpret_cast<const char*>(m_buffer->data() + m_quality_offset);
            return std::string_view(data, m_quality_length);
        }
        
        [[nodiscard]] auto length() const noexcept -> std::size_t {
            return m_sequence_length;
        }
        
        [[nodiscard]] auto is_valid() const noexcept -> bool override {
            return m_buffer && 
                   validate_offsets() && 
                   m_sequence_length == m_quality_length &&
                   fq::core::SequenceUtils::is_valid_dna(sequence()) &&
                   fq::core::QualityScore::is_valid_quality_string(quality());
        }
        
        [[nodiscard]] auto validation_errors() const -> std::vector<std::string> override {
            std::vector<std::string> errors;
            
            if (!m_buffer) {
                errors.emplace_back("No buffer attached");
            }
            
            if (!validate_offsets()) {
                errors.emplace_back("Invalid buffer offsets");
            }
            
            if (m_sequence_length != m_quality_length) {
                errors.emplace_back("Sequence and quality length mismatch");
            }
            
            if (!fq::core::SequenceUtils::is_valid_dna(sequence())) {
                errors.emplace_back("Invalid DNA sequence");
            }
            
            if (!fq::core::QualityScore::is_valid_quality_string(quality())) {
                errors.emplace_back("Invalid quality string");
            }
            
            return errors;
        }
        
        [[nodiscard]] auto memory_usage() const noexcept -> std::size_t override {
            return m_buffer ? m_buffer->memory_usage() / m_buffer->ref_count() : 0;
        }
        
        // 转换为可修改的记录（必要时进行拷贝）
        [[nodiscard]] auto to_mutable() const -> class MutableFqRecord;
        
        // 计算质量统计
        [[nodiscard]] auto calculate_average_quality() const -> double {
            return fq::core::QualityScore::calculate_average_quality(quality());
        }
        
        // 计算GC含量
        [[nodiscard]] auto calculate_gc_content() const -> double {
            return fq::core::SequenceUtils::calculate_gc_content(sequence());
        }
        
        // 计算序列复杂度
        [[nodiscard]] auto calculate_complexity() const -> double {
            return fq::core::SequenceUtils::calculate_complexity(sequence());
        }
        
    private:
        std::shared_ptr<fq::io::SharedBuffer> m_buffer;
        std::size_t m_name_offset = 0;
        std::size_t m_name_length = 0;
        std::size_t m_sequence_offset = 0;
        std::size_t m_sequence_length = 0;
        std::size_t m_quality_offset = 0;
        std::size_t m_quality_length = 0;
        
        [[nodiscard]] auto validate_offsets() const noexcept -> bool {
            if (!m_buffer) return false;
            
            auto buffer_size = m_buffer->size();
            return m_name_offset + m_name_length <= buffer_size &&
                   m_sequence_offset + m_sequence_length <= buffer_size &&
                   m_quality_offset + m_quality_length <= buffer_size;
        }
    };

    /**
     * @class MutableFqRecord
     * @brief 可修改的FastQ记录。
     *
     * 提供对FastQ记录的可修改接口，支持序列和质量的修改操作。
     */
    class MutableFqRecord : public fq::core::WithID, public fq::core::MemoryTrackable, public fq::core::Validatable {
    public:
        /**
         * @brief 默认构造函数
         * @details 创建空的MutableFqRecord实例
         * 
         * @post 空的MutableFqRecord实例被创建
         */
        MutableFqRecord() = default;
        
        /**
         * @brief 参数化构造函数
         * @details 使用名称、序列和质量字符串创建MutableFqRecord实例
         * 
         * @param name 序列名称
         * @param sequence 序列字符串
         * @param quality 质量字符串
         * @pre sequence和quality长度必须相同
         * @pre sequence必须是有效的DNA序列
         * @pre quality必须是有效的质量字符串
         * @post MutableFqRecord实例被创建并初始化
         * @throw FastQException 如果数据验证失败
         */
        MutableFqRecord(std::string name, std::string sequence, std::string quality)
            : m_name(std::move(name))
            , m_sequence(std::move(sequence))
            , m_quality(std::move(quality))
        {
            if (!is_valid()) {
                FQ_THROW_VALIDATION_ERROR("MutableFqRecord", "invalid record data");
            }
        }
        
        [[nodiscard]] auto name() const -> const std::string& { return m_name; }
        [[nodiscard]] auto sequence() const -> const std::string& { return m_sequence; }
        [[nodiscard]] auto quality() const -> const std::string& { return m_quality; }
        
        auto name() -> std::string& { return m_name; }
        auto sequence() -> std::string& { return m_sequence; }
        auto quality() -> std::string& { return m_quality; }
        
        [[nodiscard]] auto length() const noexcept -> std::size_t {
            return m_sequence.length();
        }
        
        [[nodiscard]] auto is_valid() const noexcept -> bool override {
            return m_sequence.length() == m_quality.length() &&
                   fq::core::SequenceUtils::is_valid_dna(m_sequence) &&
                   fq::core::QualityScore::is_valid_quality_string(m_quality);
        }
        
        [[nodiscard]] auto validation_errors() const -> std::vector<std::string> override {
            std::vector<std::string> errors;
            
            if (m_sequence.length() != m_quality.length()) {
                errors.emplace_back("Sequence and quality length mismatch");
            }
            
            if (!fq::core::SequenceUtils::is_valid_dna(m_sequence)) {
                errors.emplace_back("Invalid DNA sequence");
            }
            
            if (!fq::core::QualityScore::is_valid_quality_string(m_quality)) {
                errors.emplace_back("Invalid quality string");
            }
            
            return errors;
        }
        
        [[nodiscard]] auto memory_usage() const noexcept -> std::size_t override {
            return m_name.capacity() + m_sequence.capacity() + m_quality.capacity();
        }
        
        // 转换为零拷贝记录
        [[nodiscard]] auto to_shared() const -> FqRecord;
        
        // 修改操作
        void trim_left(std::size_t count) {
            if (count >= m_sequence.length()) {
                m_sequence.clear();
                m_quality.clear();
            } else {
                m_sequence = m_sequence.substr(count);
                m_quality = m_quality.substr(count);
            }
        }
        
        void trim_right(std::size_t count) {
            if (count >= m_sequence.length()) {
                m_sequence.clear();
                m_quality.clear();
            } else {
                auto new_length = m_sequence.length() - count;
                m_sequence.resize(new_length);
                m_quality.resize(new_length);
            }
        }
        
        void trim_quality(double min_quality) {
            if (m_sequence.empty()) return;
            
            // 从左侧修剪
            std::size_t left_trim = 0;
            for (std::size_t i = 0; i < m_quality.length(); ++i) {
                if (fq::core::QualityScore::sanger_to_quality(m_quality[i]) >= min_quality) {
                    break;
                }
                ++left_trim;
            }
            
            // 从右侧修剪
            std::size_t right_trim = 0;
            for (std::size_t i = m_quality.length(); i > 0; --i) {
                if (fq::core::QualityScore::sanger_to_quality(m_quality[i - 1]) >= min_quality) {
                    break;
                }
                ++right_trim;
            }
            
            if (left_trim + right_trim >= m_sequence.length()) {
                m_sequence.clear();
                m_quality.clear();
            } else {
                auto new_length = m_sequence.length() - left_trim - right_trim;
                m_sequence = m_sequence.substr(left_trim, new_length);
                m_quality = m_quality.substr(left_trim, new_length);
            }
        }
        
        void reverse_complement() {
            m_sequence = fq::core::SequenceUtils::reverse_complement(m_sequence);
            std::reverse(m_quality.begin(), m_quality.end());
        }
        
        void to_uppercase() {
            std::transform(m_sequence.begin(), m_sequence.end(), m_sequence.begin(), ::toupper);
        }
        
        void to_lowercase() {
            std::transform(m_sequence.begin(), m_sequence.end(), m_sequence.begin(), ::tolower);
        }
        
        // 计算质量统计
        [[nodiscard]] auto calculate_average_quality() const -> double {
            return fq::core::QualityScore::calculate_average_quality(m_quality);
        }
        
        // 计算GC含量
        [[nodiscard]] auto calculate_gc_content() const -> double {
            return fq::core::SequenceUtils::calculate_gc_content(m_sequence);
        }
        
        // 计算序列复杂度
        [[nodiscard]] auto calculate_complexity() const -> double {
            return fq::core::SequenceUtils::calculate_complexity(m_sequence);
        }
        
    private:
        std::string m_name;
        std::string m_sequence;
        std::string m_quality;
    };

    // 实现转换函数
    auto FqRecord::to_mutable() const -> MutableFqRecord {
        return MutableFqRecord(std::string(name()), std::string(sequence()), std::string(quality()));
    }
    
    auto MutableFqRecord::to_shared() const -> FqRecord {
        // 创建共享缓冲区
        auto total_size = m_name.size() + m_sequence.size() + m_quality.size();
        auto buffer = std::make_shared<fq::io::SharedBuffer>(total_size);
        
        std::size_t offset = 0;
        
        // 写入名称
        buffer->write_string(m_name, offset);
        auto name_offset = offset;
        auto name_length = m_name.size();
        offset += name_length;
        
        // 写入序列
        buffer->write_string(m_sequence, offset);
        auto sequence_offset = offset;
        auto sequence_length = m_sequence.size();
        offset += sequence_length;
        
        // 写入质量
        buffer->write_string(m_quality, offset);
        auto quality_offset = offset;
        auto quality_length = m_quality.size();
        
        return FqRecord(buffer, name_offset, name_length, 
                       sequence_offset, sequence_length,
                       quality_offset, quality_length);
    }

    /**
     * @class FqBatchT
     * @brief FastQ记录的批处理容器。
     *
     * 提供批量管理和处理FastQ记录的功能，支持迭代和容量管理。
     */
    template<typename RecordType>
    class FqBatchT : public fq::core::WithID, public fq::core::MemoryTrackable {
    public:
        using RecordContainer = std::vector<RecordType>;
        using iterator = typename RecordContainer::iterator;
        using const_iterator = typename RecordContainer::const_iterator;
        
        FqBatchT() = default;
        explicit FqBatchT(std::size_t reserve_size) {
            m_records.reserve(reserve_size);
        }
        
        void add_record(RecordType record) {
            m_records.push_back(std::move(record));
        }
        
        void reserve(std::size_t capacity) {
            m_records.reserve(capacity);
        }
        
        void clear() {
            m_records.clear();
        }
        
        [[nodiscard]] auto size() const noexcept -> std::size_t {
            return m_records.size();
        }
        
        [[nodiscard]] auto empty() const noexcept -> bool {
            return m_records.empty();
        }
        
        [[nodiscard]] auto capacity() const noexcept -> std::size_t {
            return m_records.capacity();
        }
        
        [[nodiscard]] auto memory_usage() const noexcept -> std::size_t override {
            std::size_t total = m_records.capacity() * sizeof(RecordType);
            for (const auto& record : m_records) {
                if constexpr (requires { record.memory_usage(); }) {
                    total += record.memory_usage();
                }
            }
            return total;
        }
        
        // 迭代器支持
        [[nodiscard]] auto begin() -> iterator { return m_records.begin(); }
        [[nodiscard]] auto end() -> iterator { return m_records.end(); }
        [[nodiscard]] auto begin() const -> const_iterator { return m_records.begin(); }
        [[nodiscard]] auto end() const -> const_iterator { return m_records.end(); }
        
        // 索引访问
        [[nodiscard]] auto operator[](std::size_t index) -> RecordType& {
            return m_records[index];
        }
        
        [[nodiscard]] auto operator[](std::size_t index) const -> const RecordType& {
            return m_records[index];
        }
        
        [[nodiscard]] auto at(std::size_t index) -> RecordType& {
            if (index >= m_records.size()) {
                FQ_THROW_VALIDATION_ERROR("batch_index", std::to_string(index));
            }
            return m_records[index];
        }
        
        [[nodiscard]] auto at(std::size_t index) const -> const RecordType& {
            if (index >= m_records.size()) {
                FQ_THROW_VALIDATION_ERROR("batch_index", std::to_string(index));
            }
            return m_records[index];
        }
        
        // 批处理操作
        void remove_invalid_records() {
            if constexpr (requires(RecordType r) { r.is_valid(); }) {
                auto new_end = std::remove_if(m_records.begin(), m_records.end(),
                    [](const RecordType& record) { return !record.is_valid(); });
                m_records.erase(new_end, m_records.end());
            }
        }
        
        [[nodiscard]] auto count_valid_records() const -> std::size_t {
            if constexpr (requires(RecordType r) { r.is_valid(); }) {
                return std::count_if(m_records.begin(), m_records.end(),
                    [](const RecordType& record) { return record.is_valid(); });
            }
            return m_records.size();
        }
        
        // 统计计算
        [[nodiscard]] auto calculate_total_bases() const -> std::size_t {
            std::size_t total = 0;
            for (const auto& record : m_records) {
                total += record.length();
            }
            return total;
        }
        
        [[nodiscard]] auto calculate_average_length() const -> double {
            if (m_records.empty()) return 0.0;
            return static_cast<double>(calculate_total_bases()) / static_cast<double>(m_records.size());
        }
        
        [[nodiscard]] auto calculate_average_quality() const -> double {
            if (m_records.empty()) return 0.0;
            
            double sum = 0.0;
            for (const auto& record : m_records) {
                if constexpr (requires { record.calculate_average_quality(); }) {
                    sum += record.calculate_average_quality();
                }
            }
            return sum / static_cast<double>(m_records.size());
        }
        
    private:
        RecordContainer m_records;
    };

    // 类型别名
    using FqBatch = FqBatchT<FqRecord>;
    using MutableFqBatch = FqBatchT<MutableFqRecord>;

    /**
     * @struct FileAttributes
     * @brief 推断的FastQ文件属性。
     *
     * 包含估算的读长、质量评分类型、GC含量等文件属性信息。
     */
    struct FileAttributes {
        std::uint32_t read_length = 0;
        fq::core::QScoreType q_score_type = fq::core::QScoreType::Unknown;
        bool is_variable_length = false;
        bool is_paired_end = false;
        fq::core::SequencingGeneration generation = fq::core::SequencingGeneration::Second;
        double average_quality = 0.0;
        double gc_content = 0.0;
        std::size_t estimated_record_count = 0;
    };

    class FileInferrer {
    public:
        explicit FileInferrer(const std::filesystem::path& file_path)
            : m_file_path(file_path)
        {
            if (!std::filesystem::exists(file_path)) {
                FQ_THROW_IO_ERROR(file_path.string(), ENOENT);
            }
        }
        
        [[nodiscard]] auto infer_attributes(std::size_t sample_size = 10000) -> FileAttributes {
            FileAttributes attrs;
            
            std::ifstream file(m_file_path);
            if (!file) {
                FQ_THROW_IO_ERROR(m_file_path.string(), errno);
            }
            
            std::string line;
            std::size_t record_count = 0;
            std::vector<std::size_t> lengths;
            std::vector<double> qualities;
            std::vector<double> gc_contents;
            
            while (record_count < sample_size && std::getline(file, line)) {
                if (line.empty() || line[0] != '@') continue;
                
                // 读取序列行
                if (!std::getline(file, line)) break;
                auto sequence = line;
                
                // 跳过+行
                if (!std::getline(file, line)) break;
                
                // 读取质量行
                if (!std::getline(file, line)) break;
                auto quality = line;
                
                if (sequence.length() != quality.length()) continue;
                
                lengths.push_back(sequence.length());
                qualities.push_back(fq::core::QualityScore::calculate_average_quality(quality));
                gc_contents.push_back(fq::core::SequenceUtils::calculate_gc_content(sequence));
                
                ++record_count;
            }
            
            if (record_count == 0) {
                FQ_THROW_FORMAT_ERROR("No valid FastQ records found");
            }
            
            // 计算统计信息
            attrs.read_length = static_cast<std::uint32_t>(*std::max_element(lengths.begin(), lengths.end()));
            attrs.is_variable_length = *std::min_element(lengths.begin(), lengths.end()) != attrs.read_length;
            
            attrs.average_quality = std::accumulate(qualities.begin(), qualities.end(), 0.0) / qualities.size();
            attrs.gc_content = std::accumulate(gc_contents.begin(), gc_contents.end(), 0.0) / gc_contents.size();
            
            // 推断质量评分系统
            attrs.q_score_type = infer_quality_system(qualities);
            
            // 估算总记录数（基于文件大小）
            auto file_size = fq::io::FileUtils::get_file_size(m_file_path);
            auto avg_record_size = 4 * (50 + attrs.read_length); // 估算值
            attrs.estimated_record_count = file_size / avg_record_size;
            
            return attrs;
        }
        
    private:
        std::filesystem::path m_file_path;
        
        auto infer_quality_system(const std::vector<double>& qualities) -> fq::core::QScoreType {
            if (qualities.empty()) return fq::core::QScoreType::Unknown;
            
            auto min_qual = *std::min_element(qualities.begin(), qualities.end());
            auto max_qual = *std::max_element(qualities.begin(), qualities.end());
            
            // 基于质量分数范围推断系统
            if (max_qual <= 40) {
                return fq::core::QScoreType::Sanger;
            } else if (min_qual >= 64 && max_qual <= 104) {
                return fq::core::QScoreType::Illumina13;
            } else if (min_qual >= 67 && max_qual <= 104) {
                return fq::core::QScoreType::Illumina15;
            } else {
                return fq::core::QScoreType::Illumina18;
            }
        }
    };
}