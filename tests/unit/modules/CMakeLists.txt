# 模块测试CMakeLists.txt

# 通用测试设置函数
function(add_module_test test_name module_name source_files)
    add_executable(${test_name} ${source_files})
    
    target_link_libraries(${test_name} PRIVATE
        GTest::gtest_main
        GTest::gtest
        fq_lib # Link against the main interface library to get all dependencies
        test_utils
    )
    
    target_include_directories(${test_name} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../utils
    )
    
    # 添加到CTest
    add_test(NAME ${test_name} COMMAND ${test_name})
    
    # 设置测试属性
    set_tests_properties(${test_name} PROPERTIES
        TIMEOUT 60
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endfunction()

# Common模块测试 - 使用新的名称避免冲突
add_module_test(test_common_module_new common
    common/test_common.cpp
)

# Pipeline 模块Smoke测试
add_module_test(test_pipeline_smoke pipeline
    pipeline/test_pipeline_smoke.cpp
)

# Error模块测试 (暂时注释掉，等待实现)
# add_module_test(test_error_system error
#     error/test_exceptions.cpp
#     error/test_error_handler.cpp
# )

# Config模块测试 (暂时注释掉，等待实现)
# add_module_test(test_config_system config
#     config/test_configuration.cpp
#     config/test_config_parsing.cpp
# )

# Core模块测试 (暂时注释掉，等待实现)
# add_module_test(test_core_types core
#     core/test_core_types.cpp
#     core/test_interfaces.cpp
# )

# IO模块测试 (暂时注释掉，等待实现)
# add_module_test(test_io_system io
#     io/test_buffer.cpp
#     io/test_async_io.cpp
# )

# FastQ模块测试 (暂时注释掉，等待实现)
# add_module_test(test_fastq_models fastq
#     fastq/test_fastq_record.cpp
#     fastq/test_fastq_batch.cpp
#     fastq/test_file_inferrer.cpp
# )

# 创建测试组
add_custom_target(module_tests
    DEPENDS 
        test_common_module_new
        test_pipeline_smoke
        # test_error_system
        # test_config_system
        # test_core_types
        # test_io_system
        # test_fastq_models
    COMMENT "Building all module tests"
)
