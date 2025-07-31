add_library(fq_cli STATIC
    commands/FilterCommand.cpp
    commands/FilterCommand.h
    commands/StatCommand.cpp
    commands/StatCommand.h
    commands/ICommand.h
)

target_link_libraries(fq_cli
    PUBLIC
        fq_lib
        cxxopts::cxxopts
)

target_include_directories(fq_cli
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
