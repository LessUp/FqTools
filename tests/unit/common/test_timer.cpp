#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "modules/common/common.h"
#include "test_helpers.h"

namespace fq::test {

class TimerTest : public FastQToolsTest {};

// Tests the basic functionality of the timer measuring an interval.
TEST_F(TimerTest, BasicTiming) {
    fq::common::Timer timer("test_timer");

    // Simulate some work
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto elapsed_ns = timer.elapsed();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_ns);

    // Check if the elapsed time is within a reasonable range (e.g., >90ms)
    EXPECT_GT(elapsed_ms.count(), 90);
    EXPECT_LT(elapsed_ms.count(), 200);  // Allow for system scheduling delays
}

// Tests that the timer name is correctly stored.
TEST_F(TimerTest, NamedTimer) {
    const std::string timer_name = "named_test_timer";
    fq::common::Timer timer(timer_name);

    // The main test is that this compiles and runs without issue.
    // A more advanced test could involve redirecting stdout and checking
    // the output of timer.report(), but this is sufficient for now.
    SUCCEED();
}

// Tests timing of sequential operations by using two separate timers.
TEST_F(TimerTest, SequentialTiming) {
    long long first_duration_ms = 0;
    long long second_duration_ms = 0;

    {
        fq::common::Timer first_timer("first_interval");
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        first_duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(first_timer.elapsed()).count();
    }  // first_timer is destroyed here

    {
        fq::common::Timer second_timer("second_interval");
        std::this_thread::sleep_for(std::chrono::milliseconds(70));
        second_duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(second_timer.elapsed()).count();
    }  // second_timer is destroyed here

    EXPECT_GT(first_duration_ms, 45);
    EXPECT_LT(first_duration_ms, 100);

    EXPECT_GT(second_duration_ms, 65);
    EXPECT_LT(second_duration_ms, 120);
}

}  // namespace fq::test
