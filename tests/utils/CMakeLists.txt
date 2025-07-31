add_library(test_utils STATIC
    fixture_loader.cpp
    test_helpers.cpp
)

target_include_directories(test_utils PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(test_utils PUBLIC fq_common gtest::gtest)
