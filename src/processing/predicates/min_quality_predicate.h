/**
 * @file min_quality_predicate.h
 * @brief Defines predicates for quality filtering of FASTQ reads.
 * 
 * This file declares several predicates used in the FASTQ data processing
 * to filter reads based on quality metrics. Each predicate is designed to
 * check specific criteria like minimum quality, length restrictions, and
 * maximum allowable N ratios.
 * 
 * @author Shane
 * @date 2023-10-05
 * @version 1.0
 * @copyright (c) 2023 Shane. All rights reserved.
 */

#pragma once

#include "../i_read_processor.h"
#include <atomic>
#include <string>

namespace fq::processing {

/**
 * @brief Predicate for evaluating minimum quality in FASTQ reads.
 * 
 * The MinQualityPredicate class evaluates whether a FASTQ read meets
 * the defined minimum quality threshold.
 */
class MinQualityPredicate : public IReadPredicate {
public:
    /**
     * @brief 构造函数
     * @details 创建最小质量过滤器实例
     * 
     * @param min_quality 最小质量阈值
     * @param quality_encoding 质量编码偏移，默认为33（Sanger编码）
     * @pre min_quality 必须在有效范围内
     * @post 过滤器被初始化并准备使用
     */
    explicit MinQualityPredicate(double min_quality, int quality_encoding = 33);
    /**
     * @brief 评估读取记录
     * @details 评估给定的FASTQ读取记录是否满足最小质量条件
     * 
     * @param read 要评估的FASTQ读取记录
     * @return 满足条件返回true，不满足返回false
     * @threadsafe 线程安全操作
     */
    auto evaluate(const fq::fastq::FqInfo& read) const -> bool override;

    /**
     * @brief 获取过滤器名称
     * @return 过滤器的名称字符串
     */
    auto getName() const -> std::string;

    /**
     * @brief 获取过滤器描述
     * @return 过滤器的功能描述字符串
     */
    auto getDescription() const -> std::string;

    /**
     * @brief 获取统计信息
     * @return 格式化的统计信息字符串
     * @threadsafe 线程安全操作
     */
    auto getStatistics() const -> std::string;
private:
    double m_min_quality;
    int m_quality_encoding;
    mutable std::atomic<size_t> m_total_evaluated{0};
    mutable std::atomic<size_t> m_passed_count{0};
    mutable std::atomic<double> m_total_quality{0.0};
    auto calculateAverageQuality(const std::string& quality_string) const -> double;
};

/**
 * @brief 最小长度过滤器
 * @details 该类实现了基于最小长度的 FASTQ 读取过滤功能，
 *          只保留长度大于等于指定阈值的读取记录
 * 
 * @note 该过滤器适用于质量控制，去除过短的序列
 * @warning 长度过短的序列可能影响后续分析结果的准确性
 */
class MinLengthPredicate : public IReadPredicate {
public:
    /**
     * @brief 构造函数
     * @details 创建最小长度过滤器实例
     * 
     * @param min_length 最小长度阈值
     * @pre min_length 必须大于 0
     * @post 过滤器被初始化并准备使用
     */
    explicit MinLengthPredicate(size_t min_length);
    
    /**
     * @brief 评估读取记录是否满足最小长度条件
     * @details 判断给定的 FASTQ 读取记录长度是否大于等于最小长度阈值
     * 
     * @param read 要评估的 FASTQ 读取记录
     * @return 满足条件返回 true，不满足返回 false
     * @threadsafe 线程安全操作
     */
    auto evaluate(const fq::fastq::FqInfo& read) const -> bool override;
    
    /**
     * @brief 获取过滤器名称
     * @return 过滤器的名称字符串
     */
    auto getName() const -> std::string;
    
    /**
     * @brief 获取过滤器描述
     * @return 过滤器的功能描述字符串
     */
    auto getDescription() const -> std::string;
    
    /**
     * @brief 获取统计信息
     * @return 格式化的统计信息字符串
     * @threadsafe 线程安全操作
     */
    auto getStatistics() const -> std::string;
    
private:
    size_t m_min_length;                                    ///< 最小长度阈值
    mutable std::atomic<size_t> m_total_evaluated{0};       ///< 总评估次数
    mutable std::atomic<size_t> m_passed_count{0};          ///< 通过次数
    mutable std::atomic<size_t> m_total_length{0};          ///< 总长度统计
};

/**
 * @brief 最大长度过滤器
 * @details 该类实现了基于最大长度的 FASTQ 读取过滤功能，
 *          只保留长度小于等于指定阈值的读取记录
 * 
 * @note 该过滤器适用于质量控制，去除过长的序列
 * @warning 过长的序列可能是测序错误或异常数据
 */
class MaxLengthPredicate : public IReadPredicate {
public:
    /**
     * @brief 构造函数
     * @details 创建最大长度过滤器实例
     * 
     * @param max_length 最大长度阈值
     * @pre max_length 必须大于 0
     * @post 过滤器被初始化并准备使用
     */
    explicit MaxLengthPredicate(size_t max_length);
    
    /**
     * @brief 评估读取记录是否满足最大长度条件
     * @details 判断给定的 FASTQ 读取记录长度是否小于等于最大长度阈值
     * 
     * @param read 要评估的 FASTQ 读取记录
     * @return 满足条件返回 true，不满足返回 false
     * @threadsafe 线程安全操作
     */
    auto evaluate(const fq::fastq::FqInfo& read) const -> bool override;
    
    /**
     * @brief 获取过滤器名称
     * @return 过滤器的名称字符串
     */
    auto getName() const -> std::string;
    
    /**
     * @brief 获取过滤器描述
     * @return 过滤器的功能描述字符串
     */
    auto getDescription() const -> std::string;
    
    /**
     * @brief 获取统计信息
     * @return 格式化的统计信息字符串
     * @threadsafe 线程安全操作
     */
    auto getStatistics() const -> std::string;
    
private:
    size_t m_max_length;                                    ///< 最大长度阈值
    mutable std::atomic<size_t> m_total_evaluated{0};       ///< 总评估次数
    mutable std::atomic<size_t> m_passed_count{0};          ///< 通过次数
};

/**
 * @brief 最大 N 比例过滤器
 * @details 该类实现了基于 N 碱基比例的 FASTQ 读取过滤功能，
 *          只保留 N 碱基比例小于等于指定阈值的读取记录
 * 
 * @note N 碱基代表未知或无法确定的碱基，高比例的 N 碱基可能影响测序质量
 * @warning 通常建议 N 碱基比例不超过 5-10%
 */
class MaxNRatioPredicate : public IReadPredicate {
public:
    /**
     * @brief 构造函数
     * @details 创建最大 N 比例过滤器实例
     * 
     * @param max_n_ratio 最大 N 比例阈值（0.0-1.0）
     * @pre max_n_ratio 必须在 0.0 到 1.0 之间
     * @post 过滤器被初始化并准备使用
     */
    explicit MaxNRatioPredicate(double max_n_ratio);
    
    /**
     * @brief 评估读取记录是否满足最大 N 比例条件
     * @details 判断给定的 FASTQ 读取记录中 N 碱基比例是否小于等于最大 N 比例阈值
     * 
     * @param read 要评估的 FASTQ 读取记录
     * @return 满足条件返回 true，不满足返回 false
     * @threadsafe 线程安全操作
     */
    auto evaluate(const fq::fastq::FqInfo& read) const -> bool override;
    
    /**
     * @brief 获取过滤器名称
     * @return 过滤器的名称字符串
     */
    auto getName() const -> std::string;
    
    /**
     * @brief 获取过滤器描述
     * @return 过滤器的功能描述字符串
     */
    auto getDescription() const -> std::string;
    
    /**
     * @brief 获取统计信息
     * @return 格式化的统计信息字符串
     * @threadsafe 线程安全操作
     */
    auto getStatistics() const -> std::string;
    
private:
    double m_max_n_ratio;                                  ///< 最大 N 比例阈值
    mutable std::atomic<size_t> m_total_evaluated{0};     ///< 总评估次数
    mutable std::atomic<size_t> m_passed_count{0};        ///< 通过次数
    mutable std::atomic<double> m_total_n_ratio{0.0};     ///< 总 N 比例统计
    
    /**
     * @brief 计算 N 碱基比例
     * @details 计算给定序列中 N 碱基的比例
     * 
     * @param sequence 要分析的序列
     * @return N 碱基比例（0.0-1.0）
     */
    auto calculateNRatio(const std::string& sequence) const -> double;
};

}