# Define the library target for this module
add_library(fq_common)

# Scan for C++20 module files (.cppm)
target_sources(fq_common PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    Common.cppm
)

# Add public header files for IDE support
target_sources(fq_common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Exceptions.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Global.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Logger.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Timer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/UI.h
    ${CMAKE_CURRENT_SOURCE_DIR}/UtilsString.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Conveyor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SimpleServer.h
)

# Add regular source files
target_sources(fq_common PRIVATE
    Timer.cpp
    Logger.cpp
    UtilsString.cpp
    UI.cpp
    Conveyor.cpp
    SimpleServer.cpp
)

# Expose the headers to other targets
target_include_directories(fq_common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link against third-party libraries
target_link_libraries(fq_common PUBLIC
    spdlog::spdlog
    TBB::tbb
)
