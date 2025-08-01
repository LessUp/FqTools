/**
 * @file min_quality_predicate.h
 * @brief Defines predicates for quality filtering of FASTQ reads.
 * 
 * This file declares several predicates used in the FASTQ data processing
 * to filter reads based on quality metrics. Each predicate is designed to
 * check specific criteria like minimum quality, length restrictions, and
 * maximum allowable N ratios.
 * 
 * @author Shane
 * @date 2023-10-05
 * @version 1.0
 * @copyright (c) 2023 Shane. All rights reserved.
 */

#pragma once

#include "../i_read_processor.h"
#include <atomic>
#include <string>

namespace fq::processing {

/**
 * @brief Predicate for evaluating minimum quality in FASTQ reads.
 * 
 * The min_quality_predicate class evaluates whether a FASTQ read meets
 * the defined minimum quality threshold.
 */
class min_quality_predicate : public IReadPredicate {
public:
    explicit min_quality_predicate(double min_quality, int quality_encoding = 33);
    auto evaluate(const fq::fastq::FqInfo& read) const -> bool override;
    auto getName() const -> std::string;
    auto getDescription() const -> std::string;
    auto getStatistics() const -> std::string;
private:
    double m_min_quality;
    int m_quality_encoding;
    mutable std::atomic<size_t> m_total_evaluated{0};
    mutable std::atomic<size_t> m_passed_count{0};
    mutable std::atomic<double> m_total_quality{0.0};
    auto calculateAverageQuality(const std::string& quality_string) const -> double;
};

class MinLengthPredicate : public IReadPredicate {
public:
    explicit MinLengthPredicate(size_t min_length);
    auto evaluate(const fq::fastq::FqInfo& read) const -> bool override;
    auto getName() const -> std::string;
    auto getDescription() const -> std::string;
    auto getStatistics() const -> std::string;
private:
    size_t m_min_length;
    mutable std::atomic<size_t> m_total_evaluated{0};
    mutable std::atomic<size_t> m_passed_count{0};
    mutable std::atomic<size_t> m_total_length{0};
};

class MaxLengthPredicate : public IReadPredicate {
public:
    explicit MaxLengthPredicate(size_t max_length);
    auto evaluate(const fq::fastq::FqInfo& read) const -> bool override;
    auto getName() const -> std::string;
    auto getDescription() const -> std::string;
    auto getStatistics() const -> std::string;
private:
    size_t m_max_length;
    mutable std::atomic<size_t> m_total_evaluated{0};
    mutable std::atomic<size_t> m_passed_count{0};
};

class MaxNRatioPredicate : public IReadPredicate {
public:
    explicit MaxNRatioPredicate(double max_n_ratio);
    auto evaluate(const fq::fastq::FqInfo& read) const -> bool override;
    auto getName() const -> std::string;
    auto getDescription() const -> std::string;
    auto getStatistics() const -> std::string;
private:
    double m_max_n_ratio;
    mutable std::atomic<size_t> m_total_evaluated{0};
    mutable std::atomic<size_t> m_passed_count{0};
    mutable std::atomic<double> m_total_n_ratio{0.0};
    auto calculateNRatio(const std::string& sequence) const -> double;
};

}