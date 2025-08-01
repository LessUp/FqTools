/**
 * @file fq_statistic.h
 * @brief Implements the fq_statistic class for handling FASTQ statistics.
 * 
 * This file contains the implementation of the fq_statistic class, which
 * manages the complete process of generating FASTQ statistic outputs,
 * utilizing the TBB pipeline to enhance performance.
 * 
 * @author Shane
 * @date 2023-10-05
 * @version 1.0
 * @copyright (c) 2023 Shane. All rights reserved.
 */

#pragma once

#include "core_legacy/core.h"
#include "i_statistic.h"
#include "interfaces/i_statistic_calculator.h" // Include the new interface
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace fq::statistic
{

struct fq_statisticResult
{
    uint64_t n_read = 0;
    uint32_t read_length = 0;
    std::vector<std::vector<uint64_t>> n_pos_qual;
    std::vector<std::vector<uint64_t>> n_pos_base;

    auto operator+=(const fq_statisticResult &other) -> fq_statisticResult &;
};

/**
 * @brief 使用 TBB 管道管理整体 FASTQ 统计信息生成过程。
 * 这是 i_statisticCalculator 接口的具体实现。
 */
class fq_statistic : public i_statisticCalculator
{ // Inherit from the interface
public:
    /**
     * @brief 使用给定选项构造 fq_statistic 实例。
     * @param options 统计运行的配置。
     */
    explicit fq_statistic(const StatisticOptions &options);

    /**
     * @brief 使用 TBB 并行管道执行统计信息生成过程。
     */
    void run() override; // Mark as override

private:
    /**
     * @brief 将最终聚合的统计信息写入输出文件。
     * @param result 要写入的最终结果。
     */
    void writeResult(const fq_statisticResult &result);

    StatisticOptions m_options; // Use the options from the interface
    std::shared_ptr<fq::fastq::FastQInfer> m_fq_infer;
};

} // namespace fq::statistic