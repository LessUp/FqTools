#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#include "Core/Core.h"
#include "IStatistic.h"

namespace fq::statistic {

// Replaced macros with constexpr for type safety and scoping
constexpr int MAX_QUAL = 42;
constexpr int MAX_BASE_NUM = 5;

/**
 * @brief Processes batches of FASTQ records to generate statistics.
 * This modernized class is a standalone utility that operates on a batch of data.
 * It relies on FastQInfer for contextual information like quality score type.
 */
class FqStatisticWorker final : public IStatistic {
public:
    /**
     * @brief Constructs a statistic worker.
     * @param fq_infer A shared pointer to a FastQInfer object containing file attributes.
     */
    explicit FqStatisticWorker(std::shared_ptr<fq::fastq::FastQInfer> fq_infer);

    /**
     * @brief Processes a single batch of FASTQ records and returns the result.
     * @param batch The batch of records to process.
     * @return A statistic result for the given batch.
     */
    auto stat(const Batch& batch) -> Result override;

private:
    std::shared_ptr<fq::fastq::FastQInfer> m_fq_infer;
    uint8_t m_qual_offset = 33;
};

} // namespace fq::statistic