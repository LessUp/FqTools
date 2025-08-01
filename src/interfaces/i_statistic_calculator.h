/**
 * @file i_statistic_calculator.h
 * @brief Interface and structures for statistic calculation tasks.
 * 
 * This file provides interfaces and supporting structures for performing
 * various statistics calculations on FASTQ data. It decouples clients from
 * concrete implementations and facilitates flexible, high-level processing.
 * 
 * @author Shane
 * @date 2023-10-05
 * @version 1.0
 * @copyright (c) 2023 Shane. All rights reserved.
 */

#pragma once

#include <memory>
#include <string>
#include <cstdint>

namespace fq::statistic {

/**
 * @brief Configuration options for a statistics calculation task.
 * This struct is defined at the interface level to decouple clients
 * from the concrete implementation.
 */
struct StatisticOptions {
    std::string input_fastq;
    std::string output_stat;
    uint32_t batch_size = 50000;
    uint8_t thread_num = 4;
};

/**
 * @brief Abstract interface for a high-level statistic calculation task.
 * This decouples the command-line layer from the statistics implementation.
 */
class i_statisticCalculator {
public:
    virtual ~i_statisticCalculator() = default;

    /**
     * @brief Executes the entire statistics generation process.
     */
    virtual void run() = 0;
};

/**
 * @brief factory function to create an instance of a statistic calculator.
 * @param options The configuration for the calculation task.
 * @return A unique_ptr to an object implementing the i_statisticCalculator interface.
 */
auto create_statistic_calculator(const StatisticOptions& options) -> std::unique_ptr<i_statisticCalculator>;

} // namespace fq::statistic