# 单元测试配置

# 通用测试设置函数
function(add_unit_test test_name source_files)
    add_executable(${test_name} ${source_files})
    
    target_link_libraries(${test_name} PRIVATE
        gtest_main
        fq_lib
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

# Common模块测试
add_unit_test(test_common_timer
    common/test_timer.cpp
)

# 模块测试
add_subdirectory(modules)

# 创建测试组
add_custom_target(unit_tests
    DEPENDS 
        test_common_timer
        module_tests
    COMMENT "Building all unit tests"
)
