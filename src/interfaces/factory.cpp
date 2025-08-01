#include "interfaces/i_statistic_calculator.h"
#include "statistics/fq_statistic.h"
#include "interfaces/i_processing_pipeline.h"
#include "processing/processing_pipeline.h"
#include "processing/tbb_processing_pipeline.h"

namespace fq::statistic {

/**
 * @brief factory function implementation.
 * This is the only place in the code (outside of the module itself)
 * that knows about the concrete fq_statistic class.
 */
auto create_statistic_calculator(const StatisticOptions& options) -> std::unique_ptr<i_statisticCalculator> {
    return std::make_unique<fq_statistic>(options);
}

} // namespace fq::statistic

namespace fq::processing {

/**
 * @brief factory function for the processing pipeline.
 */
auto create_processing_pipeline() -> std::unique_ptr<i_processingPipeline> {
    return std::make_unique<processing_pipeline>();
}

/**
 * @brief factory function for the high-performance TBB processing pipeline.
 */
auto create_tbb_processing_pipeline() -> std::unique_ptr<i_processingPipeline> {
    return std::make_unique<fq::processing::tbb_processing_pipeline>();
}

} // namespace fq::processing