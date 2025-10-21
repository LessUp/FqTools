/**
 * @file i_statistic.h
 * @brief Defines the i_statistic interface for statistic operations.
 * 
 * This file contains the declaration of the i_statistic interface, which is used
 * for defining various statistical operations on fastq data. The interface provides
 * an abstract method for processing batches of fastq information by computing
 * specific statistics.
 * 
 * @author Shane
 * @date 2023-10-05
 * @version 1.0
 * @copyright (c) 2023 Shane. All rights reserved.
 */

#pragma once

#include "core_legacy/core.h"

namespace fq::statistic {

// Forward declaration
struct FqStatisticResult;

/**
 * @brief Abstract interface for statistic computation.
 * 
 * The IStatistic class provides an interface for implementing various
 * statistical analyses over batches of fastq data. Users must implement
 * the stat() method to define the specific statistics to be computed.
 * 
 * @note Users should extend this interface to perform custom statistical
 *       analysis relevant to their use case.
 */
class StatisticInterface : public fq::common::WithID {
public:
    using Batch = fq::fastq::FqInfoBatch;
    using Result = FqStatisticResult;
    ~StatisticInterface() override = default;
    /**
     * @brief Compute statistics for a given batch of fastq data.
     * @param batch The batch of fastq information to process.
     * @return The computed statistics.
     */
    virtual auto stat(const Batch& batch) -> Result = 0;
};

using IStatistic = StatisticInterface;

} // namespace fq::statistic