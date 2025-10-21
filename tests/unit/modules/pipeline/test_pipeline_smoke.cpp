#include <gtest/gtest.h>
#include <fqtools/processing_pipeline.h>

TEST(PipelineSmokeTest, CanCreatePipelineFromFactory) {
    auto pipeline = fq::processing::create_processing_pipeline();
    ASSERT_TRUE(static_cast<bool>(pipeline));
}
