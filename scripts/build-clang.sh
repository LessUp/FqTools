#!/bin/bash
# scripts/build-clang.sh
#
# A specialized build script for FastQTools using Clang with advanced features.
#
# Usage:
# ./scripts/build-clang.sh [BUILD_TYPE] [OPTIONS]
#
# Parameters:
#   BUILD_TYPE: 'Debug', 'Release', 'RelWithDebInfo', 'Coverage', 'Sanitize'. Defaults to 'Release'.
#
# Options:
#   --asan:      Enable AddressSanitizer
#   --usan:      Enable UndefinedBehaviorSanitizer
#   --tsan:      Enable ThreadSanitizer
#   --coverage:  Enable code coverage
#   --static:    Enable static analysis
#   --no-lto:    Disable Link Time Optimization
#   --verbose:   Enable verbose output
#   --help:      Show this help message
#
# Examples:
#   ./scripts/build-clang.sh                    # Builds with Clang in Release mode
#   ./scripts/build-clang.sh Debug --asan        # Builds with Clang in Debug mode + ASAN
#   ./scripts/build-clang.sh Sanitize           # Builds with Clang in Sanitize mode

set -e

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Parse arguments properly
ASAN=false
USAN=false
TSAN=false
COVERAGE=false
STATIC_ANALYSIS=false
LTO=true
VERBOSE=false

# Handle --help first
for arg in "$@"; do
    if [[ "$arg" == "--help" ]]; then
        echo "Usage: $0 [BUILD_TYPE] [OPTIONS]"
        echo ""
        echo "BUILD_TYPE: Debug, Release, RelWithDebInfo, Coverage, Sanitize (default: Release)"
        echo ""
        echo "Options:"
        echo "  --asan      Enable AddressSanitizer"
        echo "  --usan      Enable UndefinedBehaviorSanitizer"
        echo "  --tsan      Enable ThreadSanitizer"
        echo "  --coverage  Enable code coverage"
        echo "  --static    Enable static analysis"
        echo "  --no-lto    Disable Link Time Optimization"
        echo "  --verbose   Enable verbose output"
        echo "  --help      Show this help message"
        exit 0
    fi
done

# Process positional arguments
if [[ $# -gt 0 && "$1" != "--"* ]]; then
    BUILD_TYPE="$1"
    shift
else
    BUILD_TYPE="Release"
fi

BUILD_DIR="build-clang-${BUILD_TYPE,,}"

# Process options
while [[ $# -gt 0 ]]; do
    case $1 in
        --asan)
            ASAN=true
            shift
            ;;
        --usan)
            USAN=true
            shift
            ;;
        --tsan)
            TSAN=true
            shift
            ;;
        --coverage)
            COVERAGE=true
            BUILD_TYPE="Coverage"
            shift
            ;;
        --static)
            STATIC_ANALYSIS=true
            shift
            ;;
        --no-lto)
            LTO=false
            shift
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        --help)
            echo "Usage: $0 [BUILD_TYPE] [OPTIONS]"
            echo ""
            echo "BUILD_TYPE: Debug, Release, RelWithDebInfo, Coverage, Sanitize (default: Release)"
            echo ""
            echo "Options:"
            echo "  --asan      Enable AddressSanitizer"
            echo "  --usan      Enable UndefinedBehaviorSanitizer"
            echo "  --tsan      Enable ThreadSanitizer"
            echo "  --coverage  Enable code coverage"
            echo "  --static    Enable static analysis"
            echo "  --no-lto    Disable Link Time Optimization"
            echo "  --verbose   Enable verbose output"
            echo "  --help      Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Validate build type
if [[ "$BUILD_TYPE" != "Debug" && "$BUILD_TYPE" != "Release" && "$BUILD_TYPE" != "RelWithDebInfo" && "$BUILD_TYPE" != "Coverage" && "$BUILD_TYPE" != "Sanitize" ]]; then
    echo -e "${RED}Error: Invalid build type specified. Choose 'Debug', 'Release', 'RelWithDebInfo', 'Coverage', or 'Sanitize'.${NC}"
    exit 1
fi

# Check for incompatible sanitizer combinations
if [[ "$ASAN" == true && "$TSAN" == true ]]; then
    echo -e "${RED}Error: Cannot enable both AddressSanitizer and ThreadSanitizer simultaneously${NC}"
    exit 1
fi

echo -e "${BLUE}FastQTools Clang Build Script${NC}"
echo -e "${BLUE}==========================${NC}"

# Check prerequisites
if ! command -v clang++ &> /dev/null; then
    echo -e "${RED}Error: clang++ is not installed${NC}"
    exit 1
fi

if ! command -v conan &> /dev/null; then
    echo -e "${RED}Error: Conan is not installed. Please run scripts/install_dependencies.sh${NC}"
    exit 1
fi

if ! command -v cmake &> /dev/null; then
    echo -e "${RED}Error: CMake is not installed. Please run scripts/install_dependencies.sh${NC}"
    exit 1
fi

# Show Clang version
echo -e "${GREEN}>>> Using Clang: $(clang++ --version | head -n1)${NC}"

# Set environment variables to ensure Clang is used
export CC=clang
export CXX=clang++
export CONAN_CMAKE_TOOLCHAIN_FILE=generators/conan_toolchain.cmake

# Build CMake options
CMAKE_OPTIONS=()
CMAKE_OPTIONS+=(-DCMAKE_C_COMPILER=clang)
CMAKE_OPTIONS+=(-DCMAKE_CXX_COMPILER=clang++)
CMAKE_OPTIONS+=(-DCMAKE_BUILD_TYPE="${BUILD_TYPE}")
CMAKE_OPTIONS+=(-DCMAKE_EXPORT_COMPILE_COMMANDS=ON)
CMAKE_OPTIONS+=(-G "Ninja")

# Add Clang-specific flags
CMAKE_OPTIONS+=(-DCMAKE_CXX_STANDARD=20)
CMAKE_OPTIONS+=(-DCMAKE_CXX_STANDARD_REQUIRED=ON)
CMAKE_OPTIONS+=(-DCMAKE_CXX_EXTENSIONS=OFF)

# Add sanitizer options
SANITIZER_FLAGS=""
LINKER_FLAGS=""

if [[ "$ASAN" == true ]]; then
    SANITIZER_FLAGS="${SANITIZER_FLAGS} -fsanitize=address -g"
    LINKER_FLAGS="${LINKER_FLAGS} -fsanitize=address"
fi

if [[ "$USAN" == true ]]; then
    SANITIZER_FLAGS="${SANITIZER_FLAGS} -fsanitize=undefined -g"
    LINKER_FLAGS="${LINKER_FLAGS} -fsanitize=undefined"
fi

if [[ "$TSAN" == true ]]; then
    SANITIZER_FLAGS="${SANITIZER_FLAGS} -fsanitize=thread -g"
    LINKER_FLAGS="${LINKER_FLAGS} -fsanitize=thread"
fi

if [[ "$COVERAGE" == true ]]; then
    CMAKE_OPTIONS+=(-DENABLE_COVERAGE=ON)
    SANITIZER_FLAGS="${SANITIZER_FLAGS} --coverage"
    LINKER_FLAGS="${LINKER_FLAGS} --coverage"
fi

# Add build type specific flags
case $BUILD_TYPE in
    "Debug")
        CMAKE_OPTIONS+=(-DCMAKE_CXX_FLAGS_DEBUG="-g -O0 -DDEBUG ${SANITIZER_FLAGS}")
        CMAKE_OPTIONS+=(-DCMAKE_EXE_LINKER_FLAGS_DEBUG="${LINKER_FLAGS}")
        ;;
    "Release")
        if [[ "$LTO" == true ]]; then
            CMAKE_OPTIONS+=(-DCMAKE_CXX_FLAGS_RELEASE="-O3 -DNDEBUG -march=native -flto=thin")
            CMAKE_OPTIONS+=(-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON)
        else
            CMAKE_OPTIONS+=(-DCMAKE_CXX_FLAGS_RELEASE="-O3 -DNDEBUG -march=native")
        fi
        ;;
    "RelWithDebInfo")
        CMAKE_OPTIONS+=(-DCMAKE_CXX_FLAGS_RELWITHDEBINFO="-O2 -g -DNDEBUG ${SANITIZER_FLAGS}")
        CMAKE_OPTIONS+=(-DCMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO="${LINKER_FLAGS}")
        ;;
    "Coverage")
        CMAKE_OPTIONS+=(-DCMAKE_CXX_FLAGS_DEBUG="-g -O0 -DDEBUG --coverage")
        CMAKE_OPTIONS+=(-DCMAKE_EXE_LINKER_FLAGS_DEBUG="--coverage")
        ;;
    "Sanitize")
        CMAKE_OPTIONS+=(-DCMAKE_CXX_FLAGS_DEBUG="-g -O0 -DDEBUG ${SANITIZER_FLAGS}")
        CMAKE_OPTIONS+=(-DCMAKE_EXE_LINKER_FLAGS_DEBUG="${LINKER_FLAGS}")
        ;;
esac

if [[ "$STATIC_ANALYSIS" == true ]]; then
    CMAKE_OPTIONS+=(-DENABLE_STATIC_ANALYSIS=ON)
fi

echo -e "${GREEN}>>> Building project with Clang in ${BUILD_TYPE} mode...${NC}"
echo -e "${GREEN}>>> Build directory: ${BUILD_DIR}${NC}"

# Clean previous build if exists
if [ -d "${BUILD_DIR}" ]; then
    echo -e "${YELLOW}>>> Cleaning previous build...${NC}"
    rm -rf "${BUILD_DIR}"
fi

# Run Conan install
CONAN_DIR="${BUILD_DIR}/conan"
echo -e "${GREEN}>>> Running Conan install...${NC}"

# Create a temporary profile for Clang
CLANG_PROFILE="${CONAN_DIR}/clang-profile"
mkdir -p "${CONAN_DIR}"
cat > "${CLANG_PROFILE}" << EOF
[settings]
arch=x86_64
build_type=${BUILD_TYPE}
compiler=clang
compiler.version=18
compiler.libcxx=libstdc++11
compiler.cppstd=20
os=Linux
EOF

# Use the profile for both host and build
echo -e "${GREEN}>>> Running Conan install with Clang profile...${NC}"

# Ensure environment variables are set for Conan
export CC=clang
export CXX=clang++

# Try to install with Clang profile, if it fails, try with simpler approach
if ! conan install config/dependencies/ --output-folder="${CONAN_DIR}" --build=missing -pr:h "${CLANG_PROFILE}" -pr:b "${CLANG_PROFILE}" 2>&1 | tee "${CONAN_DIR}/conan-install.log"; then
    echo -e "${YELLOW}>>> Clang profile failed, trying with environment variables...${NC}"
    
    # Clean up and try with direct environment variables
    rm -rf "${CONAN_DIR}"
    mkdir -p "${CONAN_DIR}"
    
    # Create a simpler profile
    cat > "${CLANG_PROFILE}" << EOF
[settings]
arch=x86_64
build_type=${BUILD_TYPE}
compiler=clang
compiler.version=18
compiler.libcxx=libstdc++11
os=Linux
EOF
    
    # Try again with simpler configuration
    if ! conan install config/dependencies/ --output-folder="${CONAN_DIR}" --build=missing -pr:h "${CLANG_PROFILE}" -pr:b "${CLANG_PROFILE}" 2>&1 | tee "${CONAN_DIR}/conan-install.log"; then
        echo -e "${RED}Error: Conan install failed even with simplified configuration${NC}"
        echo -e "${YELLOW}>>> Check Conan install log: ${CONAN_DIR}/conan-install.log${NC}"
        exit 1
    fi
fi

# Filter out benign hwloc warnings
echo -e "${GREEN}>>> Processing Conan installation logs...${NC}"
if grep -q "libtool: warning.*hwloc.*has not been installed" "${CONAN_DIR}/conan-install.log"; then
    echo -e "${YELLOW}>>> Note: hwloc libtool warnings detected (these are benign and can be ignored)${NC}"
fi

# Check for critical errors in Conan log
if grep -q "ERROR:" "${CONAN_DIR}/conan-install.log" | grep -v "hwloc.*has not been installed"; then
    echo -e "${RED}Error: Critical errors found in Conan installation${NC}"
    echo -e "${YELLOW}>>> Check Conan install log: ${CONAN_DIR}/conan-install.log${NC}"
    exit 1
fi

# Run CMake configuration
echo -e "${GREEN}>>> Running CMake configuration...${NC}"
if [[ "$VERBOSE" == true ]]; then
    echo "CMake Options: ${CMAKE_OPTIONS[*]}"
fi

if ! cmake -S . -B "${BUILD_DIR}" \
    -DCMAKE_TOOLCHAIN_FILE="${CONAN_DIR}/conan_toolchain.cmake" \
    "${CMAKE_OPTIONS[@]}"; then
    echo -e "${RED}Error: CMake configuration failed${NC}"
    exit 1
fi

# Run build
echo -e "${GREEN}>>> Building project...${NC}"
if ! cmake --build "${BUILD_DIR}"; then
    echo -e "${RED}Error: Build failed${NC}"
    exit 1
fi

# Run static analysis if enabled
if [[ "$STATIC_ANALYSIS" == true ]]; then
    echo -e "${GREEN}>>> Running static analysis...${NC}"
    if command -v clang-tidy &> /dev/null; then
        echo "Running clang-tidy..."
        if ! ./scripts/lint.sh -b "${BUILD_DIR}"; then
            echo -e "${YELLOW}Warning: Static analysis found issues${NC}"
        fi
    else
        echo -e "${YELLOW}Warning: clang-tidy not found, skipping static analysis${NC}"
    fi
fi

# Verify build output
if [ ! -f "${BUILD_DIR}/FastQTools" ]; then
    echo -e "${RED}Error: Executable not found at ${BUILD_DIR}/FastQTools${NC}"
    exit 1
fi

echo -e "${GREEN}>>> Build complete! Executable is at: ${BUILD_DIR}/FastQTools${NC}"

# Print build summary
echo ""
echo -e "${BLUE}Clang Build Summary:${NC}"
echo "  Build Type: ${BUILD_TYPE}"
echo "  Sanitizers: ASAN=$ASAN, USAN=$USAN, TSAN=$TSAN"
echo "  Coverage: $COVERAGE"
echo "  Static Analysis: $STATIC_ANALYSIS"
echo "  LTO: $LTO"
echo "  Clang Version: $(clang++ --version | head -n1)"

# Test the executable if built successfully
echo ""
echo -e "${GREEN}>>> Testing built executable...${NC}"
if "${BUILD_DIR}/FastQTools" --help > /dev/null 2>&1; then
    echo -e "${GREEN}>>> Executable test passed!${NC}"
else
    echo -e "${YELLOW}Warning: Executable test failed, but build completed${NC}"
fi