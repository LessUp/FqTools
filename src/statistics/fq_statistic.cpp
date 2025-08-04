/**
 * @file FqStatistic.cpp
 * @brief FASTQ 统计功能实现文件
 * @details 包含 FqStatistic 类及相关统计方法的实现，支持 TBB 并行统计。
 * @author FastQTools Team
 * @date 2025-08-01
 * @version 1.0
 * @copyright Copyright (c) 2025 FastQTools
 */

#include "statistics/FqStatistic.h"

#include "statistics/FqStatistic_worker.h"

namespace fq::statistic {

/**
 * @brief 统计结果累加操作符重载
 * @details 将另一个 FqStatisticResult 的统计数据累加到当前对象
 * @param other 另一个统计结果
 * @return 当前对象的引用
 */
auto FqStatisticResult::operator+=(const FqStatisticResult& other) -> FqStatisticResult& {
    this->n_read += other.n_read;
    // Add other members here when the struct is expanded
    return *this;
}
}  // namespace fq::statistic
#include <tbb/parallel_pipeline.h>

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <memory>
#include <numeric>
#include <vector>

#include "core_legacy/core.h"
#include "spdlog/spdlog.h"

namespace fq::statistic {

// Helper function to calculate error rate from quality scores remains the same
/**
 * @brief 计算每个位点的错误率
 * @details 根据质量分数统计，计算每个位点的平均错误率
 * @param n_pos_qual 每个位点各质量分数的计数
 * @param n_read 总读取数
 * @return 平均错误率
 */
static auto calErrPerPos(const std::vector<uint64_t>& n_pos_qual, uint64_t n_read) -> double {
    if (n_read == 0)
        return 0.0;
    
    double err_per_pos = 0.0;
    for (int i = 0; i < fq::fastq::MAX_QUAL; ++i) {
        err_per_pos += static_cast<double>(n_pos_qual[i]) * pow(10, -0.1 * i);
    }
    return err_per_pos / static_cast<double>(n_read);
}

/**
 * @brief FqStatistic 构造函数
 * @details 初始化统计参数和输入文件推断对象
 * @param options 统计选项
 */
FqStatistic::FqStatistic(const StatisticOptions& options) : m_options(options) {
    m_fq_infer = std::make_shared<fq::fastq::FastQInfer>(m_options.input_fastq);
}

/**
 * @brief 执行 FASTQ 统计主流程（TBB 并行）
 * @details 包括输入校验、并行流水线构建及最终结果聚合
 */
void FqStatistic::run() {
    spdlog::info("Starting FASTQ statistics generation for '{}' using TBB pipeline.", m_options.input_fastq);

    const auto& attrib = m_fq_infer->getFqFileAttribution();
    if (attrib.is_mutable_read_length || attrib.read_length == 0) {
        throw fq::exception("Statistics generation requires a fixed read length.");
    }

    // The final result, captured by the aggregation stage
    FqStatisticResult final_result;

    // The pipeline will manage its own concurrency.
    // The number of "tokens" controls the level of parallelism.
    const size_t max_live_tokens = m_options.thread_num;

    auto reader = std::make_shared<fq::fastq::FastQReader>(m_options.input_fastq, m_fq_infer);

    tbb::parallel_pipeline(
        max_live_tokens,
        // Stage 1: Input Filter (Serial)
        // Reads batches of FASTQ data from the file.
        tbb::make_filter<void, std::shared_ptr<fq::fastq::FqInfoBatch>>(
            tbb::filter_mode::serial_in_order,
            [this, reader](tbb::flow_control& fc) -> std::shared_ptr<fq::fastq::FqInfoBatch> {
                auto batch = std::make_shared<fq::fastq::FqInfoBatch>();
                                if (m_options.batch_size > std::numeric_limits<int>::max()) {
                    throw fq::exception("Batch size exceeds the maximum value for an integer.");
                }
                if (reader->read(*batch, static_cast<int>(m_options.batch_size))) {
                    return batch;
                } else {
                    // No more batches, stop the pipeline
                    fc.stop();
                    return nullptr;
                }
            }) &
            // Stage 2: Processing Filter (Parallel)
            // Takes a batch, processes it, and returns a partial result.
            tbb::make_filter<std::shared_ptr<fq::fastq::FqInfoBatch>, FqStatisticResult>(
                tbb::filter_mode::parallel,
                [this](std::shared_ptr<fq::fastq::FqInfoBatch> batch) -> FqStatisticResult {
                    if (!batch)
                        return FqStatisticResult();
                    FqStatistic_worker worker(m_fq_infer);
                    return worker.stat(*batch);
                }) &
            // Stage 3: Aggregation Filter (Serial)
            // Takes partial results and accumulates them into a final result.
            tbb::make_filter<FqStatisticResult, void>(
                tbb::filter_mode::serial_in_order,
                [&final_result](const FqStatisticResult& partial_result) { final_result += partial_result; }));

    spdlog::info("TBB pipeline finished. Aggregated results from all batches.");

    writeResult(final_result);
    spdlog::info("Statistics report saved to '{}'", m_options.output_stat);
}

void FqStatistic::writeResult(const FqStatisticResult& result) {
    std::ofstream writer(m_options.output_stat);
    if (!writer) {
        throw fq::exception("Failed to open output statistics file: " + m_options.output_stat);
    }

    writer << std::fixed << std::setprecision(2);

    const uint64_t n_base = result.n_read * result.read_length;
    if (n_base == 0) {
        spdlog::warn("No data to write for fqStat file.");
        return;
    }

    const auto& attrib = m_fq_infer->getFqFileAttribution();
    std::string fq_name = std::filesystem::path(m_options.input_fastq).filename().string();

    writer << "#Name\t" << fq_name << "\n";
        writer << "#PhredQual\t" << static_cast<int>(attrib.q_score_type == fq::fastq::QScoreType::Sanger ? fq::fastq::PHRED_OFFSET_SANGER : fq::fastq::PHRED_OFFSET_ILLUMINA_1_3)
           << "\n";
    writer << "#ReadNum\t" << result.n_read << "\n";
    writer << "#ReadLength\t" << result.read_length << "\n";
    writer << "#BaseCount\t" << n_base << "\n";

        constexpr int Q20_THRESHOLD = 20;
    constexpr int Q30_THRESHOLD = 30;
    uint64_t n_q20 = 0, n_q30 = 0;
    uint64_t n_a = 0, n_c = 0, n_g = 0, n_t = 0, n_n = 0;

    for (uint32_t i = 0; i < result.read_length; ++i) {
        for (int j = Q20_THRESHOLD; j < fq::fastq::MAX_QUAL; ++j)
            n_q20 += result.n_pos_qual[i][j];
        for (int j = Q30_THRESHOLD; j < fq::fastq::MAX_QUAL; ++j)
            n_q30 += result.n_pos_qual[i][j];
        n_a += result.n_pos_base[i][0];
        n_c += result.n_pos_base[i][1];
        n_g += result.n_pos_base[i][2];
        n_t += result.n_pos_base[i][3];
        n_n += result.n_pos_base[i][4];
    }

        writer << "#Q20(>=20)\t" << n_q20 << "\t" << 100.0 * static_cast<double>(n_q20) / static_cast<double>(n_base) << "%\n";
    writer << "#Q30(>=30)\t" << n_q30 << "\t" << 100.0 * static_cast<double>(n_q30) / static_cast<double>(n_base) << "%\n";
    writer << "#A\t" << n_a << "\t" << 100.0 * static_cast<double>(n_a) / static_cast<double>(n_base) << "%\n";
    writer << "#C\t" << n_c << "\t" << 100.0 * static_cast<double>(n_c) / static_cast<double>(n_base) << "%\n";
    writer << "#G\t" << n_g << "\t" << 100.0 * static_cast<double>(n_g) / static_cast<double>(n_base) << "%\n";
    writer << "#T\t" << n_t << "\t" << 100.0 * static_cast<double>(n_t) / static_cast<double>(n_base) << "%\n";
    writer << "#N\t" << n_n << "\t" << 100.0 * static_cast<double>(n_n) / static_cast<double>(n_base) << "%\n";
    writer << "#GC\t" << n_g + n_c << "\t" << 100.0 * static_cast<double>(n_g + n_c) / static_cast<double>(n_base) << "%\n";

    writer << "#Pos\tA\tC\tG\tT\tN\tAvgQual\tErrRate\n";
    for (uint32_t i = 0; i < result.read_length; ++i) {
        writer << i + 1 << "\t";
        writer << result.n_pos_base[i][0] << "\t" << result.n_pos_base[i][1] << "\t" << result.n_pos_base[i][2] << "\t"
               << result.n_pos_base[i][3] << "\t" << result.n_pos_base[i][4] << "\t";

        uint64_t sum_qual = 0;
        for (int j = 0; j < fq::fastq::MAX_QUAL; ++j) {
            sum_qual += result.n_pos_qual[i][j] * j;
        }
                writer << static_cast<double>(sum_qual) / static_cast<double>(result.n_read) << "\t";
        writer << calErrPerPos(result.n_pos_qual[i], result.n_read) << "\n";
    }
}

}  // namespace fq::statistic