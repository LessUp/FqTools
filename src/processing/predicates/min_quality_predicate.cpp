#include "min_quality_predicate.h"
#include "spdlog/spdlog.h"
#include <algorithm>
#include <numeric>
#include <sstream>
#include <iomanip>

namespace fq::processing {

// MinQualityPredicate 实现
MinQualityPredicate::MinQualityPredicate(double min_quality, int quality_encoding)
    : m_min_quality(min_quality), m_quality_encoding(quality_encoding) {
    
        if (min_quality < 0.0 || min_quality > fq::fastq::MAX_PHRED_SCORE) {
        throw std::invalid_argument("质量分数阈值必须在0-93之间");
    }
    
    if (quality_encoding != fq::fastq::PHRED_OFFSET_SANGER && quality_encoding != fq::fastq::PHRED_OFFSET_ILLUMINA_1_3) {
        throw std::invalid_argument("质量编码必须是33（Sanger）或64（Illumina 1.3+）");
    }
    
    spdlog::debug("MinQualityPredicate: 创建，最小质量={}, 编码偏移={}", 
                 min_quality, quality_encoding);
}

auto MinQualityPredicate::evaluate(const fq::fastq::FqInfo& read) const -> bool {
    m_total_evaluated.fetch_add(1, std::memory_order_relaxed);
    
    if (read.qual.empty()) {
        spdlog::warn("MinQualityPredicate: 读取缺少质量信息");
        return false;
    }
    
    double avg_quality = calculateAverageQuality(read.qual);
    m_total_quality.fetch_add(avg_quality, std::memory_order_relaxed);
    
    bool passed = avg_quality >= m_min_quality;
    if (passed) {
        m_passed_count.fetch_add(1, std::memory_order_relaxed);
    }
    
    return passed;
}

auto MinQualityPredicate::getName() const -> std::string {
    return "最小质量过滤器";
}

auto MinQualityPredicate::getDescription() const -> std::string {
    std::ostringstream oss;
    oss << "过滤平均质量分数低于 " << m_min_quality << " 的读取";
    return oss.str();
}

auto MinQualityPredicate::getStatistics() const -> std::string {
    size_t total = m_total_evaluated.load(std::memory_order_relaxed);
    size_t passed = m_passed_count.load(std::memory_order_relaxed);
    double total_qual = m_total_quality.load(std::memory_order_relaxed);
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "评估: " << total << ", 通过: " << passed;
    
    if (total > 0) {
        double pass_rate = static_cast<double>(passed) / total * 100.0;
        double avg_quality = total_qual / total;
        oss << " (" << pass_rate << "%), 平均质量: " << avg_quality;
    }
    
    return oss.str();
}

auto MinQualityPredicate::calculateAverageQuality(const std::string& quality_string) const -> double {
    if (quality_string.empty()) {
        return 0.0;
    }
    
    double sum = 0.0;
    for (char quality_char : quality_string) {
        sum += static_cast<double>(static_cast<unsigned char>(quality_char) - m_quality_encoding);
    }
    
    return sum / quality_string.length();
}

// MinLengthPredicate 实现
MinLengthPredicate::MinLengthPredicate(size_t min_length)
    : m_min_length(min_length) {
    
    spdlog::debug("MinLengthPredicate: 创建，最小长度={}", min_length);
}

auto MinLengthPredicate::evaluate(const fq::fastq::FqInfo& read) const -> bool {
    m_total_evaluated.fetch_add(1, std::memory_order_relaxed);
    m_total_length.fetch_add(read.base.length(), std::memory_order_relaxed);
    
    bool passed = read.base.length() >= m_min_length;
    if (passed) {
        m_passed_count.fetch_add(1, std::memory_order_relaxed);
    }
    
    return passed;
}

auto MinLengthPredicate::getName() const -> std::string {
    return "最小长度过滤器";
}

auto MinLengthPredicate::getDescription() const -> std::string {
    std::ostringstream oss;
    oss << "过滤长度小于 " << m_min_length << " bp 的读取";
    return oss.str();
}

auto MinLengthPredicate::getStatistics() const -> std::string {
    size_t total = m_total_evaluated.load(std::memory_order_relaxed);
    size_t passed = m_passed_count.load(std::memory_order_relaxed);
    size_t total_len = m_total_length.load(std::memory_order_relaxed);
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "评估: " << total << ", 通过: " << passed;
    
    if (total > 0) {
        double pass_rate = static_cast<double>(passed) / total * 100.0;
        double avg_length = static_cast<double>(total_len) / total;
        oss << " (" << pass_rate << "%), 平均长度: " << avg_length << " bp";
    }
    
    return oss.str();
}

// MaxLengthPredicate 实现
MaxLengthPredicate::MaxLengthPredicate(size_t max_length)
    : m_max_length(max_length) {
    
    spdlog::debug("MaxLengthPredicate: 创建，最大长度={}", max_length);
}

auto MaxLengthPredicate::evaluate(const fq::fastq::FqInfo& read) const -> bool {
    m_total_evaluated.fetch_add(1, std::memory_order_relaxed);
    
    bool passed = read.base.length() <= m_max_length;
    if (passed) {
        m_passed_count.fetch_add(1, std::memory_order_relaxed);
    }
    
    return passed;
}

auto MaxLengthPredicate::getName() const -> std::string {
    return "最大长度过滤器";
}

auto MaxLengthPredicate::getDescription() const -> std::string {
    std::ostringstream oss;
    oss << "过滤长度大于 " << m_max_length << " bp 的读取";
    return oss.str();
}

auto MaxLengthPredicate::getStatistics() const -> std::string {
    size_t total = m_total_evaluated.load(std::memory_order_relaxed);
    size_t passed = m_passed_count.load(std::memory_order_relaxed);
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "评估: " << total << ", 通过: " << passed;
    
    if (total > 0) {
        double pass_rate = static_cast<double>(passed) / total * 100.0;
        oss << " (" << pass_rate << "%)";
    }
    
    return oss.str();
}

// MaxNRatioPredicate 实现
MaxNRatioPredicate::MaxNRatioPredicate(double max_n_ratio)
    : m_max_n_ratio(max_n_ratio) {
    
    if (max_n_ratio < 0.0 || max_n_ratio > 1.0) {
        throw std::invalid_argument("N碱基比例阈值必须在0.0-1.0之间");
    }
    
    spdlog::debug("MaxNRatioPredicate: 创建，最大N比例={}", max_n_ratio);
}

auto MaxNRatioPredicate::evaluate(const fq::fastq::FqInfo& read) const -> bool {
    m_total_evaluated.fetch_add(1, std::memory_order_relaxed);
    
    if (read.base.empty()) {
        return false;
    }
    
    double n_ratio = calculateNRatio(read.base);
    m_total_n_ratio.fetch_add(n_ratio, std::memory_order_relaxed);
    
    bool passed = n_ratio <= m_max_n_ratio;
    if (passed) {
        m_passed_count.fetch_add(1, std::memory_order_relaxed);
    }
    
    return passed;
}

auto MaxNRatioPredicate::getName() const -> std::string {
    return "最大N比例过滤器";
}

auto MaxNRatioPredicate::getDescription() const -> std::string {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "过滤N碱基比例大于 " << (m_max_n_ratio * 100.0) << "% 的读取";
    return oss.str();
}

auto MaxNRatioPredicate::getStatistics() const -> std::string {
    size_t total = m_total_evaluated.load(std::memory_order_relaxed);
    size_t passed = m_passed_count.load(std::memory_order_relaxed);
    double total_n_ratio = m_total_n_ratio.load(std::memory_order_relaxed);
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "评估: " << total << ", 通过: " << passed;
    
    if (total > 0) {
        double pass_rate = static_cast<double>(passed) / total * 100.0;
        double avg_n_ratio = total_n_ratio / total * 100.0;
        oss << " (" << pass_rate << "%), 平均N比例: " << avg_n_ratio << "%";
    }
    
    return oss.str();
}

auto MaxNRatioPredicate::calculateNRatio(const std::string& sequence) const -> double {
    if (sequence.empty()) {
        return 0.0;
    }
    
    size_t n_count = std::count_if(sequence.begin(), sequence.end(), 
                                  [](char base) { return base == 'N' || base == 'n'; });
    
    return static_cast<double>(n_count) / sequence.length();
}

} // namespace fq::processing
