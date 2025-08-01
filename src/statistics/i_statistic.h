#pragma once

#include "core_legacy/core.h"

namespace fq::statistic {

// Forward declaration
struct fq_statisticResult;

class i_statistic : public fq::common::WithID {
public:
    using Batch = fq::fastq::FqInfoBatch;
    using Result = fq_statisticResult;
    ~i_statistic() override = default;
    virtual auto stat(const Batch& batch) -> Result = 0;
};

} // namespace fq::statistic