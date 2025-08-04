/**
 * @file fq_statistic.h
 * @brief FASTQ 统计信息管理器类定义
 * @details 该文件定义了 FASTQ 统计信息管理器类，用于管理完整的 FASTQ 统计信息生成过程，
 *          利用 TBB 管道来提高性能
 * 
 * @author FastQTools Team
 * @date 2024
 * @version 1.0
 * 
 * @copyright Copyright (c) 2024 FastQTools
 * @license MIT License
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

/**
 * @brief FASTQ 统计信息结果结构体
 * @details 存储 FASTQ 文件统计分析的结果数据，包括读取数量、长度分布、
 *          位置质量分数分布和位置碱基分布等信息
 */
struct FqStatisticResult
{
    uint64_t n_read = 0;                                          ///< 总读取数量
    uint32_t read_length = 0;                                     ///< 读取长度
    std::vector<std::vector<uint64_t>> n_pos_qual;                ///< 位置质量分数分布
    std::vector<std::vector<uint64_t>> n_pos_base;                 ///< 位置碱基分布

    /**
     * @brief 重载 += 运算符，用于合并统计结果
     * @details 将另一个统计结果合并到当前结果中
     * 
     * @param other 要合并的另一个统计结果
     * @return 合并后的统计结果引用
     */
    auto operator+=(const FqStatisticResult &other) -> FqStatisticResult &;
};

/**
 * @brief FASTQ 统计信息管理器
 * @details 该类使用 TBB 管道管理完整的 FASTQ 统计信息生成过程，
 *          是 IStatisticCalculator 接口的具体实现
 * 
 * @note 该类利用并行处理提高大文件的处理效率
 * @warning 处理过程中需要足够的内存空间
 */
class FqStatistic : public IStatisticCalculator
{
public:
    /**
     * @brief 构造函数
     * @details 使用给定选项创建 FqStatistic 实例
     * 
     * @param options 统计运行的配置选项
     * @pre options 必须包含有效的配置参数
     * @post 统计信息管理器被初始化并准备使用
     */
    explicit FqStatistic(const StatisticOptions &options);

    /**
     * @brief 执行统计信息生成过程
     * @details 使用 TBB 并行管道执行完整的 FASTQ 统计信息生成过程
     * @post 统计结果被写入到指定的输出文件中
     */
    void run() override;

private:
    /**
     * @brief 将最终聚合的统计信息写入输出文件
     * @details 将统计结果以适当的格式写入到输出文件中
     * 
     * @param result 要写入的最终结果
     * @pre result 必须包含有效的统计数据
     * @post 统计信息被写入到配置指定的输出文件中
     */
    void writeResult(const FqStatisticResult &result);

    StatisticOptions m_options;                                  ///< 统计配置选项
    std::shared_ptr<fq::fastq::FastQInfer> m_fq_infer;          ///< FASTQ 推断器指针
};

} // namespace fq::statistic