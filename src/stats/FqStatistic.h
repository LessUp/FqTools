#pragma once

#include "Core/Core.h"
#include "IStatistic.h"
#include "interfaces/IStatisticCalculator.h" // Include the new interface
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace fq::statistic
{

struct FqStatisticResult
{
    uint64_t n_read = 0;
    uint32_t read_length = 0;
    std::vector<std::vector<uint64_t>> n_pos_qual;
    std::vector<std::vector<uint64_t>> n_pos_base;

    auto operator+=(const FqStatisticResult &other) -> FqStatisticResult &;
};

/**
 * @brief 使用 TBB 管道管理整体 FASTQ 统计信息生成过程。
 * 这是 IStatisticCalculator 接口的具体实现。
 */
class FqStatistic : public IStatisticCalculator
{ // Inherit from the interface
public:
    /**
     * @brief 使用给定选项构造 FqStatistic 实例。
     * @param options 统计运行的配置。
     */
    explicit FqStatistic(const StatisticOptions &options);

    /**
     * @brief 使用 TBB 并行管道执行统计信息生成过程。
     */
    void run() override; // Mark as override

private:
    /**
     * @brief 将最终聚合的统计信息写入输出文件。
     * @param result 要写入的最终结果。
     */
    void writeResult(const FqStatisticResult &result);

    StatisticOptions m_options; // Use the options from the interface
    std::shared_ptr<fq::fastq::FastQInfer> m_fq_infer;
};

} // namespace fq::statistic