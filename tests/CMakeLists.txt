enable_testing()

find_package(GTest REQUIRED)

add_subdirectory(unit)
add_subdirectory(utils)

add_executable(fastq_tests
    unit/common/test_timer.cpp
)

target_link_libraries(fastq_tests
    PRIVATE
    gtest_main
    fq_lib      # Link against the main interface library
    test_utils
)

add_test(NAME fastq_tests COMMAND fastq_tests)
