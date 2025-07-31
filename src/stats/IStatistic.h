#pragma once

#include "Core/Core.h"

namespace fq::statistic {

// Forward declaration
struct FqStatisticResult;

class IStatistic : public fq::common::WithID {
public:
    using Batch = fq::fastq::FqInfoBatch;
    using Result = FqStatisticResult;
    ~IStatistic() override = default;
    virtual auto stat(const Batch& batch) -> Result = 0;
};

} // namespace fq::statistic