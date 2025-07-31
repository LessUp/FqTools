#include "interfaces/IStatisticCalculator.h"
#include "stats/FqStatistic.h"
#include "interfaces/IProcessingPipeline.h"
#include "Processing/ProcessingPipeline.h"
#include "Processing/TbbProcessingPipeline.h"

namespace fq::statistic {

/**
 * @brief Factory function implementation.
 * This is the only place in the code (outside of the module itself)
 * that knows about the concrete FqStatistic class.
 */
auto create_statistic_calculator(const StatisticOptions& options) -> std::unique_ptr<IStatisticCalculator> {
    return std::make_unique<FqStatistic>(options);
}

} // namespace fq::statistic

namespace fq::processing {

/**
 * @brief Factory function for the processing pipeline.
 */
auto create_processing_pipeline() -> std::unique_ptr<IProcessingPipeline> {
    return std::make_unique<ProcessingPipeline>();
}

/**
 * @brief Factory function for the high-performance TBB processing pipeline.
 */
auto create_tbb_processing_pipeline() -> std::unique_ptr<IProcessingPipeline> {
    return std::make_unique<fq::processing::TbbProcessingPipeline>();
}

} // namespace fq::processing