#include "fqtools/statistics/statistic_calculator_interface.h"
#include "statistics/fq_statistic.h"
#include "fqtools/pipeline/processing_pipeline_interface.h"
#include "processing/processing_pipeline.h"
#include "processing/tbb_processing_pipeline.h"

namespace fq::statistic {

/**
 * @brief factory function implementation.
 * This is the only place in the code (outside of the module itself)
 * that knows about the concrete FqStatistic class.
 */
auto make_statistic_calculator(const StatisticOptions& options) -> std::unique_ptr<StatisticCalculatorInterface> {
    return std::make_unique<FastqStatisticCalculator>(options);
}

// Backward-compatible wrapper
auto create_statistic_calculator(const StatisticOptions& options) -> std::unique_ptr<StatisticCalculatorInterface> {
    return make_statistic_calculator(options);
}

} // namespace fq::statistic

namespace fq::processing {

/**
 * @brief factory function for the processing pipeline.
 */
auto make_processing_pipeline() -> std::unique_ptr<ProcessingPipelineInterface> {
    return std::make_unique<SequentialProcessingPipeline>();
}

// Backward-compatible wrapper
auto create_processing_pipeline() -> std::unique_ptr<ProcessingPipelineInterface> {
    return make_processing_pipeline();
}

/**
 * @brief factory function for the high-performance TBB processing pipeline.
 */
auto create_tbb_processing_pipeline() -> std::unique_ptr<ProcessingPipelineInterface> {
    // Delegate to unified factory with default config and memory manager
    return create_tbb_pipeline();
}

/**
 * @brief factory overload that accepts Config to match example usage.
 */
auto create_tbb_processing_pipeline(const TbbProcessingPipeline::Config& config) -> std::unique_ptr<ProcessingPipelineInterface> {
    return create_tbb_pipeline(config);
}

} // namespace fq::processing