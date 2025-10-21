/**
 * @file fq_statistic_worker.h
 * @brief FASTQ 统计信息工作器类定义
 * @details 该文件定义了 FASTQ 统计信息工作器类，用于处理 FASTQ 记录批次并生成统计信息
 * 
 * @author FastQTools Team
 * @date 2024
 * @version 1.0
 * 
 * @copyright Copyright (c) 2024 FastQTools
 * @license MIT License
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#include "core_legacy/core.h"
#include "fqtools/statistics/statistic_interface.h"

namespace fq::statistic {

// Replaced macros with constexpr for type safety and scoping
constexpr int MAX_QUAL = 42;          ///< 最大质量分数值
constexpr int MAX_BASE_NUM = 5;       ///< 最大碱基数量

/**
 * @brief FASTQ 统计信息工作器
 * @details 该类用于处理 FASTQ 记录批次并生成统计信息，是一个独立的工具类，
 *          依赖于 FastQInfer 对象提供的上下文信息（如质量分数类型）
 * 
 * @note 该类被标记为 final，不允许被继承
 * @warning 统计信息的准确性依赖于 FastQInfer 的正确配置
 */
class FqStatisticWorker final : public IStatistic {
public:
    /**
     * @brief 构造函数
     * @details 创建 FASTQ 统计信息工作器实例
     * 
     * @param fq_infer 指向 FastQInfer 对象的共享指针，包含文件属性信息
     * @pre fq_infer 必须是有效的指针
     * @post 工作器被初始化并准备使用
     */
    explicit FqStatisticWorker(std::shared_ptr<fq::fastq::FastQInfer> fq_infer);

    /**
     * @brief 处理单个 FASTQ 记录批次并返回统计结果
     * @details 对给定的 FASTQ 记录批次进行统计分析，生成统计信息
     * 
     * @param batch 要处理的记录批次
     * @return 给定批次的统计结果
     * @threadsafe 线程安全操作
     */
    auto stat(const Batch& batch) -> Result override;

private:
    std::shared_ptr<fq::fastq::FastQInfer> m_fq_infer;   ///< FASTQ 推断器指针
    uint8_t m_qual_offset = 33;                            ///< 质量分数偏移量
};

} // namespace fq::statistic