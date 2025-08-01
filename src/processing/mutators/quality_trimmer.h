#ifndef FASTQTOOLS_QUALITYTRIMMER_H
#define FASTQTOOLS_QUALITYTRIMMER_H

#include "../i_read_processor.h"
#include <atomic>

namespace fq::processing {

/**
 * @brief 质量修剪器
 * 
 * 根据质量分数修剪FastQ读取的两端。
 * 从5'端和3'端移除低质量的碱基，直到遇到高质量碱基。
 */
class quality_trimmer : public IReadMutator {
public:
    /**
     * @brief 修剪模式
     */
    enum class TrimMode {
        Both,       // 修剪两端
        FivePrime,  // 只修剪5'端
        ThreePrime  // 只修剪3'端
    };
    
    /**
     * @brief 构造函数
     * @param quality_threshold 质量阈值
     * @param min_length 修剪后的最小长度
     * @param mode 修剪模式
     * @param quality_encoding 质量编码偏移
     */
    quality_trimmer(double quality_threshold, 
                   size_t min_length = 1,
                   TrimMode mode = TrimMode::Both,
                   int quality_encoding = 33);
    
    auto process(fq::fastq::FqInfo& read) -> bool override;
    auto getName() const -> std::string;
    auto getDescription() const -> std::string;
    void reset();

private:
    double m_quality_threshold;     // 质量阈值
    size_t m_min_length;           // 最小长度
    TrimMode m_trim_mode;          // 修剪模式
    int m_quality_encoding;        // 质量编码偏移
    
    // 统计信息
    std::atomic<size_t> m_total_processed{0};
    std::atomic<size_t> m_trimmed_count{0};
    std::atomic<size_t> m_total_bases_removed{0};
    
    /**
     * @brief 从5'端修剪低质量碱基
     * @param sequence 序列
     * @param quality 质量字符串
     * @return 修剪的起始位置
     */
    auto trimFivePrime(const std::string& sequence, const std::string& quality) const -> size_t;
    
    /**
     * @brief 从3'端修剪低质量碱基
     * @param sequence 序列
     * @param quality 质量字符串
     * @return 修剪的结束位置
     */
    auto trimThreePrime(const std::string& sequence, const std::string& quality) const -> size_t;
    
    /**
     * @brief 检查质量字符是否达到阈值
     * @param quality_char 质量字符
     * @return true 如果质量达到阈值
     */
    auto isHighQuality(char quality_char) const -> bool;
};

/**
 * @brief 长度修剪器
 * 
 * 将FastQ读取修剪到指定长度。
 * 可以从5'端、3'端或两端进行修剪。
 */
class LengthTrimmer : public IReadMutator {
public:
    /**
     * @brief 修剪策略
     */
    enum class TrimStrategy {
        FixedLength,    // 固定长度修剪
        MaxLength,      // 最大长度限制
        FromStart,      // 从起始位置修剪
        FromEnd         // 从结束位置修剪
    };
    
    /**
     * @brief 构造函数
     * @param target_length 目标长度
     * @param strategy 修剪策略
     */
    LengthTrimmer(size_t target_length, TrimStrategy strategy = TrimStrategy::FixedLength);
    
    auto process(fq::fastq::FqInfo& read) -> bool override;
    auto getName() const -> std::string;
    auto getDescription() const -> std::string;
    void reset();

private:
    size_t m_target_length;         // 目标长度
    TrimStrategy m_strategy;        // 修剪策略
    
    // 统计信息
    std::atomic<size_t> m_total_processed{0};
    std::atomic<size_t> m_trimmed_count{0};
    std::atomic<size_t> m_total_bases_removed{0};
};

/**
 * @brief 适配器修剪器
 * 
 * 检测并移除FastQ读取中的适配器序列。
 * 支持多个适配器序列和模糊匹配。
 */
class AdapterTrimmer : public IReadMutator {
public:
    /**
     * @brief 构造函数
     * @param adapter_sequences 适配器序列列表
     * @param min_overlap 最小重叠长度
     * @param max_mismatches 最大错配数
     */
    AdapterTrimmer(const std::vector<std::string>& adapter_sequences,
                   size_t min_overlap = 3,
                   size_t max_mismatches = 1);
    
    auto process(fq::fastq::FqInfo& read) -> bool override;
    auto getName() const -> std::string;
    auto getDescription() const -> std::string;
    void reset();

private:
    std::vector<std::string> m_adapters;    // 适配器序列
    size_t m_min_overlap;                   // 最小重叠长度
    size_t m_max_mismatches;               // 最大错配数
    
    // 统计信息
    std::atomic<size_t> m_total_processed{0};
    std::atomic<size_t> m_adapter_found{0};
    std::atomic<size_t> m_total_bases_removed{0};
    
    /**
     * @brief 在序列中查找适配器
     * @param sequence 目标序列
     * @param adapter 适配器序列
     * @return 适配器起始位置，如果未找到返回string::npos
     */
    auto findAdapter(const std::string& sequence, const std::string& adapter) const -> size_t;
    
    /**
     * @brief 计算两个序列片段的错配数
     * @param seq1 序列1
     * @param seq2 序列2
     * @param start1 序列1起始位置
     * @param start2 序列2起始位置
     * @param length 比较长度
     * @return 错配数
     */
    auto countMismatches(const std::string& seq1, const std::string& seq2,
                          size_t start1, size_t start2, size_t length) const -> size_t;
};

} // namespace fq::processing

#endif // FASTQTOOLS_QUALITYTRIMMER_H
