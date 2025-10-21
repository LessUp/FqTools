#pragma once

// Keep legacy include until legacy directory is reorganized in later batch
#include "core_legacy/core.h"

namespace fq::processing {

class ReadMutatorInterface {
public:
    virtual ~ReadMutatorInterface() = default;
    virtual auto process(fq::fastq::FqInfo &read) -> bool = 0;
};

} // namespace fq::processing
