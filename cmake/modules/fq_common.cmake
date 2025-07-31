# Common模块CMakeLists.txt

add_library(fq_common
    src/modules/common/common.h
    src/modules/common/common.cpp
)

target_include_directories(fq_common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/common
)

target_compile_features(fq_common PUBLIC cxx_std_20)

target_link_libraries(fq_common PUBLIC
    fmt::fmt
)

# 设置目标属性
set_target_properties(fq_common PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# 测试可执行文件
add_executable(test_common_module
    tests/unit/modules/common/test_common.cpp
)

target_link_libraries(test_common_module PRIVATE
    fq_common
    gtest_main
    gtest
)

target_include_directories(test_common_module PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/common
)

add_test(NAME test_common_module COMMAND test_common_module)