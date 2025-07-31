# Config模块CMakeLists.txt

add_library(fq_config
    src/modules/config/config.h
    src/modules/config/config.cpp
)

target_include_directories(fq_config PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/config
)

target_compile_features(fq_config PUBLIC cxx_std_20)

target_link_libraries(fq_config PUBLIC
    fq_error
    fmt::fmt
)

# 设置目标属性
set_target_properties(fq_config PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# 测试可执行文件
add_executable(test_configuration
    tests/unit/modules/config/test_configuration.cpp
)

target_link_libraries(test_configuration PRIVATE
    fq_config
    gtest_main
    gtest
    gmock
)

target_include_directories(test_configuration PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/config
)

add_test(NAME test_configuration COMMAND test_configuration)

# 配置解析测试
add_executable(test_config_parsing
    tests/unit/modules/config/test_config_parsing.cpp
)

target_link_libraries(test_config_parsing PRIVATE
    fq_config
    gtest_main
    gtest
    gmock
)

target_include_directories(test_config_parsing PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/config
)

add_test(NAME test_config_parsing COMMAND test_config_parsing)