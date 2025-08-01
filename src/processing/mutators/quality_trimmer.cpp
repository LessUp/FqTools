#include "quality_trimmer.h"
#include "spdlog/spdlog.h"
#include <algorithm>
#include <sstream>

namespace fq::processing {

// quality_trimmer 实现
quality_trimmer::quality_trimmer(double quality_threshold, 
                               size_t min_length,
                               TrimMode mode,
                               int quality_encoding)
    : m_quality_threshold(quality_threshold)
    , m_min_length(min_length)
    , m_trim_mode(mode)
    , m_quality_encoding(quality_encoding) {
    
        if (quality_threshold < 0.0 || quality_threshold > fq::fastq::MAX_PHRED_SCORE) {
        throw std::invalid_argument("质量阈值必须在0-93之间");
    }
    
    if (quality_encoding != fq::fastq::PHRED_OFFSET_SANGER && quality_encoding != fq::fastq::PHRED_OFFSET_ILLUMINA_1_3) {
        throw std::invalid_argument("质量编码必须是33（Sanger）或64（Illumina 1.3+）");
    }
    
    spdlog::debug("quality_trimmer: 创建，质量阈值={}, 最小长度={}, 模式={}", 
                 quality_threshold, min_length, static_cast<int>(mode));
}

auto quality_trimmer::process(fq::fastq::FqInfo& read) -> bool {
    m_total_processed.fetch_add(1, std::memory_order_relaxed);
    
    if (read.base.empty() || read.qual.empty()) {
        return false;
    }
    
    if (read.base.length() != read.qual.length()) {
        spdlog::warn("quality_trimmer: 序列和质量长度不匹配");
        return false;
    }
    
    size_t original_length = read.base.length();
    size_t start_pos = 0;
    size_t end_pos = original_length;
    
    // 根据模式进行修剪
    if (m_trim_mode == TrimMode::Both || m_trim_mode == TrimMode::FivePrime) {
        start_pos = trimFivePrime(read.base, read.qual);
    }
    
    if (m_trim_mode == TrimMode::Both || m_trim_mode == TrimMode::ThreePrime) {
        end_pos = trimThreePrime(read.base, read.qual);
    }
    
    // 确保修剪后长度满足最小要求
    if (end_pos <= start_pos || (end_pos - start_pos) < m_min_length) {
        // 如果修剪后长度不足，清空读取
        read.base.clear();
        read.qual.clear();
        return true;
    }
    
    // 应用修剪
    if (start_pos > 0 || end_pos < original_length) {
        read.base = read.base.substr(start_pos, end_pos - start_pos);
        read.qual = read.qual.substr(start_pos, end_pos - start_pos);
        
        m_trimmed_count.fetch_add(1, std::memory_order_relaxed);
        size_t bases_removed = original_length - (end_pos - start_pos);
        m_total_bases_removed.fetch_add(bases_removed, std::memory_order_relaxed);
    }
    
    return true;
}

auto quality_trimmer::getName() const -> std::string {
    return "质量修剪器";
}

auto quality_trimmer::getDescription() const -> std::string {
    std::ostringstream oss;
    oss << "修剪质量低于 " << m_quality_threshold << " 的碱基";
    
    switch (m_trim_mode) {
        case TrimMode::Both:
            oss << "（两端）";
            break;
        case TrimMode::FivePrime:
            oss << "（5'端）";
            break;
        case TrimMode::ThreePrime:
            oss << "（3'端）";
            break;
    }
    
    return oss.str();
}

void quality_trimmer::reset() {
    m_total_processed.store(0, std::memory_order_relaxed);
    m_trimmed_count.store(0, std::memory_order_relaxed);
    m_total_bases_removed.store(0, std::memory_order_relaxed);
}

auto quality_trimmer::trimFivePrime(const std::string& /*sequence*/, const std::string& quality) const -> size_t {
    for (size_t i = 0; i < quality.length(); ++i) {
        if (isHighQuality(quality[i])) {
            return i;
        }
    }
    return quality.length(); // 全部都是低质量
}

auto quality_trimmer::trimThreePrime(const std::string& /*sequence*/, const std::string& quality) const -> size_t {
    for (size_t i = quality.length(); i > 0; --i) {
        if (isHighQuality(quality[i - 1])) {
            return i;
        }
    }
    return 0; // 全部都是低质量
}

auto quality_trimmer::isHighQuality(char quality_char) const -> bool {
    double quality_score = static_cast<double>(static_cast<unsigned char>(quality_char) - m_quality_encoding);
    return quality_score >= m_quality_threshold;
}

// LengthTrimmer 实现
LengthTrimmer::LengthTrimmer(size_t target_length, TrimStrategy strategy)
    : m_target_length(target_length), m_strategy(strategy) {
    
    if (target_length == 0) {
        throw std::invalid_argument("目标长度不能为0");
    }
    
    spdlog::debug("LengthTrimmer: 创建，目标长度={}, 策略={}", 
                 target_length, static_cast<int>(strategy));
}

auto LengthTrimmer::process(fq::fastq::FqInfo& read) -> bool {
    m_total_processed.fetch_add(1, std::memory_order_relaxed);
    
    if (read.base.empty()) {
        return false;
    }
    
    size_t original_length = read.base.length();
    size_t new_length = original_length;
    size_t start_pos = 0;
    
    switch (m_strategy) {
        case TrimStrategy::FixedLength:
            if (original_length > m_target_length) {
                new_length = m_target_length;
            }
            break;
            
        case TrimStrategy::MaxLength:
            if (original_length > m_target_length) {
                new_length = m_target_length;
            }
            break;
            
        case TrimStrategy::FromStart:
            if (original_length > m_target_length) {
                start_pos = m_target_length;
                new_length = original_length - m_target_length;
            }
            break;
            
        case TrimStrategy::FromEnd:
            if (original_length > m_target_length) {
                new_length = original_length - m_target_length;
            }
            break;
    }
    
    // 应用修剪
    if (new_length != original_length || start_pos > 0) {
        read.base = read.base.substr(start_pos, new_length);
        if (!read.qual.empty()) {
            read.qual = read.qual.substr(start_pos, new_length);
        }
        
        m_trimmed_count.fetch_add(1, std::memory_order_relaxed);
        size_t bases_removed = original_length - new_length;
        m_total_bases_removed.fetch_add(bases_removed, std::memory_order_relaxed);
    }
    
    return true;
}

auto LengthTrimmer::getName() const -> std::string {
    return "长度修剪器";
}

auto LengthTrimmer::getDescription() const -> std::string {
    std::ostringstream oss;
    oss << "修剪到目标长度 " << m_target_length << " bp";
    
    switch (m_strategy) {
        case TrimStrategy::FixedLength:
            oss << "（固定长度）";
            break;
        case TrimStrategy::MaxLength:
            oss << "（最大长度限制）";
            break;
        case TrimStrategy::FromStart:
            oss << "（从起始修剪）";
            break;
        case TrimStrategy::FromEnd:
            oss << "（从末端修剪）";
            break;
    }
    
    return oss.str();
}

void LengthTrimmer::reset() {
    m_total_processed.store(0, std::memory_order_relaxed);
    m_trimmed_count.store(0, std::memory_order_relaxed);
    m_total_bases_removed.store(0, std::memory_order_relaxed);
}

// AdapterTrimmer 实现
AdapterTrimmer::AdapterTrimmer(const std::vector<std::string>& adapter_sequences,
                               size_t min_overlap,
                               size_t max_mismatches)
    : m_adapters(adapter_sequences)
    , m_min_overlap(min_overlap)
    , m_max_mismatches(max_mismatches) {
    
    if (adapter_sequences.empty()) {
        throw std::invalid_argument("适配器序列列表不能为空");
    }
    
    if (min_overlap == 0) {
        throw std::invalid_argument("最小重叠长度不能为0");
    }
    
    spdlog::debug("AdapterTrimmer: 创建，适配器数量={}, 最小重叠={}, 最大错配={}", 
                 adapter_sequences.size(), min_overlap, max_mismatches);
}

auto AdapterTrimmer::process(fq::fastq::FqInfo& read) -> bool {
    m_total_processed.fetch_add(1, std::memory_order_relaxed);
    
    if (read.base.empty()) {
        return false;
    }
    
    size_t original_length = read.base.length();
    size_t trim_position = std::string::npos;
    
    // 查找所有适配器中最早出现的位置
    for (const auto& adapter : m_adapters) {
        size_t pos = findAdapter(read.base, adapter);
        if (pos != std::string::npos) {
            if (trim_position == std::string::npos || pos < trim_position) {
                trim_position = pos;
            }
        }
    }
    
    // 如果找到适配器，进行修剪
    if (trim_position != std::string::npos) {
        read.base = read.base.substr(0, trim_position);
        if (!read.qual.empty()) {
            read.qual = read.qual.substr(0, trim_position);
        }
        
        m_adapter_found.fetch_add(1, std::memory_order_relaxed);
        size_t bases_removed = original_length - trim_position;
        m_total_bases_removed.fetch_add(bases_removed, std::memory_order_relaxed);
    }
    
    return true;
}

auto AdapterTrimmer::getName() const -> std::string {
    return "适配器修剪器";
}

auto AdapterTrimmer::getDescription() const -> std::string {
    std::ostringstream oss;
    oss << "移除 " << m_adapters.size() << " 种适配器序列";
    oss << "（最小重叠=" << m_min_overlap << ", 最大错配=" << m_max_mismatches << "）";
    return oss.str();
}

void AdapterTrimmer::reset() {
    m_total_processed.store(0, std::memory_order_relaxed);
    m_adapter_found.store(0, std::memory_order_relaxed);
    m_total_bases_removed.store(0, std::memory_order_relaxed);
}

auto AdapterTrimmer::findAdapter(const std::string& sequence, const std::string& adapter) const -> size_t {
    if (sequence.length() < m_min_overlap || adapter.length() < m_min_overlap) {
        return std::string::npos;
    }
    
    // 从序列的每个位置开始查找适配器
    for (size_t i = 0; i <= sequence.length() - m_min_overlap; ++i) {
        size_t max_compare_length = std::min(sequence.length() - i, adapter.length());
        
        // 确保至少有最小重叠长度
        if (max_compare_length < m_min_overlap) {
            break;
        }
        
        // 计算错配数
        size_t mismatches = countMismatches(sequence, adapter, i, 0, max_compare_length);
        
        // 如果错配数在允许范围内，返回位置
        if (mismatches <= m_max_mismatches) {
            return i;
        }
    }
    
    return std::string::npos;
}

auto AdapterTrimmer::countMismatches(const std::string& seq1, const std::string& seq2,
                                      size_t start1, size_t start2, size_t length) const -> size_t {
    size_t mismatches = 0;
    
    for (size_t i = 0; i < length; ++i) {
        if (start1 + i >= seq1.length() || start2 + i >= seq2.length()) {
            break;
        }
        
        char c1 = std::toupper(seq1[start1 + i]);
        char c2 = std::toupper(seq2[start2 + i]);
        
        if (c1 != c2) {
            mismatches++;
        }
    }
    
    return mismatches;
}

} // namespace fq::processing
