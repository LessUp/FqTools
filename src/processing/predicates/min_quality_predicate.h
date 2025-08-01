#pragma once

#include "../i_read_processor.h"
#include <atomic>
#include <string>

namespace fq::processing {

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