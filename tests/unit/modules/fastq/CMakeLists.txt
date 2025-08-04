# FastQ模块测试
# 包含FastQ记录处理、验证和文件处理的测试

# 添加FastQ记录测试
add_executable(test_fastq_records
    test_fastq_records.cpp
)

target_link_libraries(test_fastq_records
    PRIVATE
    fq_fastq
    fq_test_utils
    gtest
    gtest_main
)

# 添加FastQ验证测试
add_executable(test_fastq_validation
    test_fastq_validation.cpp
)

target_link_libraries(test_fastq_validation
    PRIVATE
    fq_fastq
    fq_test_utils
    gtest
    gtest_main
)

# 添加FastQ文件处理测试
add_executable(test_fastq_files
    test_fastq_files.cpp
)

target_link_libraries(test_fastq_files
    PRIVATE
    fq_fastq
    fq_test_utils
    gtest
    gtest_main
)

# 将测试添加到测试套件
gtest_discover_tests(test_fastq_records)
gtest_discover_tests(test_fastq_validation)
gtest_discover_tests(test_fastq_files)

# 包含目录
target_include_directories(test_fastq_records
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../..
)

target_include_directories(test_fastq_validation
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../..
)

target_include_directories(test_fastq_files
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../..
)