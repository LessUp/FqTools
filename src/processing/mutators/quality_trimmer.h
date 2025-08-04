/**
 * @file quality_trimmer.h
 * @brief 各种序列修剪工具的类定义。
 *
 * 提供根据质量分数、长度、适配器等进行修剪的功能。
 */

#pragma once

#include "../i_read_processor.h"
#include <atomic>

namespace fq::processing {

/**
 * @brief FastQ读取的质量修剪器。
 *
 * 提供基于质量分数的修剪功能，从5'和3'端去除低质量碱基。
 * 可根据配置选择修剪模式和质量编码。
 */
class QualityTrimmer : public IReadMutator {
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
    QualityTrimmer(double quality_threshold, 
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
/**
 * @brief 长度修剪器
 * @details 该类实现了基于长度的 FASTQ 读取修剪功能，
 *          可以根据不同的策略将读取修剪到指定的长度
 * 
 * @note 该修剪器适用于标准化读取长度，便于后续分析
 * @warning 修剪会永久删除数据，请谨慎设置目标长度
 */
class LengthTrimmer : public IReadMutator {
public:
    /**
     * @brief 修剪策略枚举
     * @details 定义了不同的长度修剪策略
     */
    enum class TrimStrategy {
        FixedLength,    ///< 固定长度修剪，将读取修剪到指定长度
        MaxLength,      ///< 最大长度限制，超过指定长度的部分被修剪
        FromStart,      ///< 从起始位置修剪，保留从起始位置开始的指定长度
        FromEnd         ///< 从结束位置修剪，保留结束位置之前的指定长度
    };
    
    /**
     * @brief 构造函数
     * @details 创建长度修剪器实例
     * 
     * @param target_length 目标长度
     * @param strategy 修剪策略，默认为 FixedLength
     * @pre target_length 必须大于 0
     * @post 修剪器被初始化并准备使用
     */
    LengthTrimmer(size_t target_length, TrimStrategy strategy = TrimStrategy::FixedLength);
    
    /**
     * @brief 处理 FASTQ 读取记录
     * @details 根据设定的策略对读取记录进行长度修剪
     * 
     * @param read 要处理的 FASTQ 读取记录
     * @return 处理成功返回 true，失败返回 false
     * @threadsafe 线程安全操作
     */
    auto process(fq::fastq::FqInfo& read) -> bool override;
    
    /**
     * @brief 获取修剪器名称
     * @return 修剪器的名称字符串
     */
    auto getName() const -> std::string;
    
    /**
     * @brief 获取修剪器描述
     * @return 修剪器的功能描述字符串
     */
    auto getDescription() const -> std::string;
    
    /**
     * @brief 重置统计信息
     * @details 将所有统计计数器重置为零
     * @threadsafe 线程安全操作
     */
    void reset();

private:
    size_t m_target_length;                                 ///< 目标长度
    TrimStrategy m_strategy;                                ///< 修剪策略
    
    // 统计信息
    std::atomic<size_t> m_total_processed{0};              ///< 总处理数量
    std::atomic<size_t> m_trimmed_count{0};                 ///< 修剪数量
    std::atomic<size_t> m_total_bases_removed{0};           ///< 总碱基移除数量
};

/**
 * @brief 适配器修剪器
 * @details 该类实现了基于适配器序列的 FASTQ 读取修剪功能，
 *          能够检测并移除读取中的适配器序列，支持多个适配器序列和模糊匹配
 * 
 * @note 适配器序列是测序过程中添加的已知序列，需要去除以便后续分析
 * @warning 适配器检测的准确性取决于设置的参数，需要根据具体数据调整
 */
class AdapterTrimmer : public IReadMutator {
public:
    /**
     * @brief 构造函数
     * @details 创建适配器修剪器实例
     * 
     * @param adapter_sequences 适配器序列列表
     * @param min_overlap 最小重叠长度，默认为 3
     * @param max_mismatches 最大错配数，默认为 1
     * @pre adapter_sequences 不能为空
     * @pre min_overlap 必须大于 0
     * @post 修剪器被初始化并准备使用
     */
    AdapterTrimmer(const std::vector<std::string>& adapter_sequences,
                   size_t min_overlap = 3,
                   size_t max_mismatches = 1);
    
    /**
     * @brief 处理 FASTQ 读取记录
     * @details 检测并移除读取中的适配器序列
     * 
     * @param read 要处理的 FASTQ 读取记录
     * @return 处理成功返回 true，失败返回 false
     * @threadsafe 线程安全操作
     */
    auto process(fq::fastq::FqInfo& read) -> bool override;
    
    /**
     * @brief 获取修剪器名称
     * @return 修剪器的名称字符串
     */
    auto getName() const -> std::string;
    
    /**
     * @brief 获取修剪器描述
     * @return 修剪器的功能描述字符串
     */
    auto getDescription() const -> std::string;
    
    /**
     * @brief 重置统计信息
     * @details 将所有统计计数器重置为零
     * @threadsafe 线程安全操作
     */
    void reset();

private:
    std::vector<std::string> m_adapters;                    ///< 适配器序列列表
    size_t m_min_overlap;                                   ///< 最小重叠长度
    size_t m_max_mismatches;                               ///< 最大错配数
    
    // 统计信息
    std::atomic<size_t> m_total_processed{0};              ///< 总处理数量
    std::atomic<size_t> m_adapter_found{0};                 ///< 适配器发现数量
    std::atomic<size_t> m_total_bases_removed{0};           ///< 总碱基移除数量
    
    /**
     * @brief 在序列中查找适配器
     * @details 在目标序列中搜索适配器序列，支持模糊匹配
     * 
     * @param sequence 目标序列
     * @param adapter 适配器序列
     * @return 适配器起始位置，如果未找到返回 string::npos
     */
    auto findAdapter(const std::string& sequence, const std::string& adapter) const -> size_t;
    
    /**
     * @brief 计算两个序列片段的错配数
     * @details 计算两个序列片段在指定位置和长度上的错配数量
     * 
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
