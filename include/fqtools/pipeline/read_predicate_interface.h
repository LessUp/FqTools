#pragma once

// Keep legacy include until legacy directory is reorganized in later batch
#include "core_legacy/core.h"

namespace fq::processing {

class ReadPredicateInterface {
public:
    virtual ~ReadPredicateInterface() = default;
    virtual auto evaluate(const fq::fastq::FqInfo &read) const -> bool = 0;
};

} // namespace fq::processing
