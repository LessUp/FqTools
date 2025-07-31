# Error模块CMakeLists.txt

add_library(fq_error
    src/modules/error/error.h
    src/modules/error/error.cpp
)

target_include_directories(fq_error PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/error
)

target_compile_features(fq_error PUBLIC cxx_std_20)

target_link_libraries(fq_error PUBLIC
    fmt::fmt
)

# 设置目标属性
set_target_properties(fq_error PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# 测试可执行文件
add_executable(test_error_exceptions
    tests/unit/modules/error/test_exceptions.cpp
)

target_link_libraries(test_error_exceptions PRIVATE
    fq_error
    gtest_main
    gtest
    gmock
)

target_include_directories(test_error_exceptions PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/error
)

add_test(NAME test_error_exceptions COMMAND test_error_exceptions)

# 错误处理器测试
add_executable(test_error_handler
    tests/unit/modules/error/test_error_handler.cpp
)

target_link_libraries(test_error_handler PRIVATE
    fq_error
    gtest_main
    gtest
    gmock
)

target_include_directories(test_error_handler PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/error
)

add_test(NAME test_error_handler COMMAND test_error_handler)