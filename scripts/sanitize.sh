#!/bin/bash
# scripts/sanitize.sh
#
# A comprehensive sanitizer testing script for FastQTools.
#
# Usage:
# ./scripts/sanitize.sh [COMPILER] [SANITIZER]
#
# Parameters:
#   COMPILER: 'gcc' or 'clang'. Defaults to 'clang'.
#   SANITIZER: 'asan', 'usan', 'tsan', or 'all'. Defaults to 'all'.
#
# This script will:
# 1. Build the project with the specified sanitizer(s)
# 2. Run all tests to catch sanitizer issues
# 3. Run basic functionality tests

set -e

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default values
COMPILER=${1:-clang}
SANITIZER=${2:-all}

echo -e "${BLUE}FastQTools Sanitizer Testing${NC}"
echo -e "${BLUE}=========================${NC}"

# Validate compiler
if [[ "$COMPILER" != "gcc" && "$COMPILER" != "clang" ]]; then
    echo -e "${RED}Error: Invalid compiler specified. Choose 'gcc' or 'clang'.${NC}"
    exit 1
fi

# Function to run sanitizer tests
run_sanitizer_test() {
    local sanitizer=$1
    local build_dir="build-${COMPILER}-${sanitizer}"
    
    echo -e "${GREEN}>>> Testing with ${sanitizer}...${NC}"
    
    # Build with sanitizer
    case $sanitizer in
        "asan")
            if ! ./scripts/build.sh "${COMPILER}" Debug --asan; then
                echo -e "${RED}Error: ASAN build failed${NC}"
                return 1
            fi
            ;;
        "usan")
            if ! ./scripts/build.sh "${COMPILER}" Debug --usan; then
                echo -e "${RED}Error: USAN build failed${NC}"
                return 1
            fi
            ;;
        "tsan")
            if ! ./scripts/build.sh "${COMPILER}" Debug --tsan; then
                echo -e "${RED}Error: TSAN build failed${NC}"
                return 1
            fi
            ;;
    esac
    
    # Run tests
    echo "Running tests with ${sanitizer}..."
    cd "${build_dir}"
    
    # Set sanitizer environment variables
    case $sanitizer in
        "asan")
            export ASAN_OPTIONS="detect_leaks=1:check_initialization_order=1:strict_init_order=1:detect_stack_use_after_return=1"
            ;;
        "usan")
            export UBSAN_OPTIONS="print_stacktrace=1:print_summary=1:halt_on_error=1"
            ;;
        "tsan")
            export TSAN_OPTIONS="halt_on_error=1:report_signal_unsafe=1"
            ;;
    esac
    
    if ! ctest --preset debug --output-on-failure; then
        echo -e "${RED}Error: Tests failed with ${sanitizer}${NC}"
        cd - > /dev/null
        return 1
    fi
    
    cd - > /dev/null
    
    echo -e "${GREEN}>>> ${sanitizer} tests passed!${NC}"
    return 0
}

# Function to test basic functionality
test_basic_functionality() {
    local build_dir=$1
    local executable="${build_dir}/FastQTools"
    
    echo "Testing basic functionality..."
    
    # Test help command
    if ! "${executable}" --help > /dev/null 2>&1; then
        echo -e "${RED}Error: Help command failed${NC}"
        return 1
    fi
    
    # Test version command (if available)
    if "${executable}" --version > /dev/null 2>&1; then
        echo "Version command works"
    fi
    
    # Test stat command with invalid input (should fail gracefully)
    if "${executable}" stat /nonexistent/file 2>&1 | grep -q "Error\|Failed"; then
        echo "Error handling works correctly"
    else
        echo -e "${YELLOW}Warning: Error handling might not be working correctly${NC}"
    fi
    
    return 0
}

# Run the requested sanitizer tests
case $SANITIZER in
    "asan")
        run_sanitizer_test "asan"
        test_basic_functionality "build-${COMPILER}-asan"
        ;;
    "usan")
        run_sanitizer_test "usan"
        test_basic_functionality "build-${COMPILER}-usan"
        ;;
    "tsan")
        run_sanitizer_test "tsan"
        test_basic_functionality "build-${COMPILER}-tsan"
        ;;
    "all")
        echo -e "${YELLOW}Running all sanitizer tests...${NC}"
        
        # Test AddressSanitizer
        if ! run_sanitizer_test "asan"; then
            echo -e "${RED}ASAN tests failed${NC}"
            exit 1
        fi
        test_basic_functionality "build-${COMPILER}-asan"
        
        # Test UndefinedBehaviorSanitizer
        if ! run_sanitizer_test "usan"; then
            echo -e "${RED}USAN tests failed${NC}"
            exit 1
        fi
        test_basic_functionality "build-${COMPILER}-usan"
        
        # Test ThreadSanitizer (skip for GCC as it has limited support)
        if [[ "$COMPILER" == "clang" ]]; then
            if ! run_sanitizer_test "tsan"; then
                echo -e "${RED}TSAN tests failed${NC}"
                exit 1
            fi
            test_basic_functionality "build-${COMPILER}-tsan"
        else
            echo -e "${YELLOW}Skipping TSAN tests for GCC (limited support)${NC}"
        fi
        ;;
    *)
        echo -e "${RED}Error: Invalid sanitizer specified. Choose 'asan', 'usan', 'tsan', or 'all'.${NC}"
        exit 1
        ;;
esac

echo -e "${GREEN}>>> All sanitizer tests completed successfully!${NC}"
echo ""
echo -e "${BLUE}Sanitizer Testing Summary:${NC}"
echo "  Compiler: ${COMPILER}"
echo "  Sanitizer: ${SANITIZER}"
echo ""
echo -e "${YELLOW}Note: Sanitizers may have some performance overhead.${NC}"
echo -e "${YELLOW}      Use release builds for performance-critical testing.${NC}"