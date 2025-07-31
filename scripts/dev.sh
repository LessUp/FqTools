#!/bin/bash

# FastQTools Development Helper Script
# Quick commands for common development tasks

set -e

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_usage() {
    echo "FastQTools Development Helper"
    echo ""
    echo "Usage: $0 COMMAND [OPTIONS]"
    echo ""
    echo "Commands:"
    echo "  quick-test         Quick debug build and test"
    echo "  quick-run          Quick debug build and run with sample args"
    echo "  format             Format code using clang-format"
    echo "  clean              Clean all build directories"
    echo "  coverage           Generate coverage report"
    echo "  memcheck           Run tests with valgrind"
    echo "  benchmark          Build and run performance tests"
    echo "  package            Create release package"
    echo "  list-presets       List available CMake presets"
    echo "  deps               Check and install dependencies"
    echo ""
    echo "Examples:"
    echo "  $0 quick-test      # Fast build and test cycle"
    echo "  $0 coverage        # Generate test coverage report"
    echo "  $0 format          # Format all source code"
}

case "${1:-help}" in
    quick-test)
        echo -e "${BLUE}Quick test cycle...${NC}"
        ./build.sh -p debug -c -t
        ;;
        
    quick-run)
        echo -e "${BLUE}Quick build and run...${NC}"
        ./build.sh -p debug
        echo -e "${GREEN}Running FastQTools with --help:${NC}"
        ./build/debug/app/FastQTools --help
        ;;
        
    format)
        echo -e "${BLUE}Formatting code...${NC}"
        if command -v clang-format &> /dev/null; then
            find src app tests -name "*.cpp" -o -name "*.h" -o -name "*.hpp" | \
                xargs clang-format -i --style=file
            echo -e "${GREEN}Code formatting completed${NC}"
        else
            echo -e "${RED}clang-format not found${NC}"
            exit 1
        fi
        ;;
        
    clean)
        echo -e "${BLUE}Cleaning all build directories...${NC}"
        rm -rf build/
        echo -e "${GREEN}Clean completed${NC}"
        ;;
        
    coverage)
        echo -e "${BLUE}Generating coverage report...${NC}"
        ./build.sh --coverage -t
        ;;
        
    memcheck)
        echo -e "${BLUE}Running memory check...${NC}"
        ./build.sh -p debug -t
        if command -v valgrind &> /dev/null; then
            cd build/debug
            make test_memcheck 2>/dev/null || cmake --build . --target test_memcheck
            cd ../..
        else
            echo -e "${YELLOW}valgrind not found, skipping memory check${NC}"
        fi
        ;;
        
    benchmark)
        echo -e "${BLUE}Building for benchmarking...${NC}"
        ./build.sh -p release
        echo -e "${GREEN}Release build ready for benchmarking${NC}"
        ;;
        
    package)
        echo -e "${BLUE}Creating release package...${NC}"
        ./build.sh -p release -i
        echo -e "${GREEN}Package created${NC}"
        ;;
        
    list-presets)
        echo -e "${BLUE}Available CMake presets:${NC}"
        cmake --list-presets=all 2>/dev/null || echo "CMake 3.20+ required for preset listing"
        ;;
        
    deps)
        echo -e "${BLUE}Checking dependencies...${NC}"
        echo "Required tools:"
        echo -n "  cmake: "
        if command -v cmake &> /dev/null; then
            echo -e "${GREEN}$(cmake --version | head -n1)${NC}"
        else
            echo -e "${RED}NOT FOUND${NC}"
        fi
        
        echo -n "  ninja: "
        if command -v ninja &> /dev/null; then
            echo -e "${GREEN}$(ninja --version)${NC}"
        else
            echo -e "${RED}NOT FOUND${NC}"
        fi
        
        echo -n "  vcpkg: "
        if [[ -n "$VCPKG_ROOT" ]] && [[ -f "$VCPKG_ROOT/vcpkg" ]]; then
            echo -e "${GREEN}found at $VCPKG_ROOT${NC}"
        else
            echo -e "${YELLOW}VCPKG_ROOT not set or vcpkg not found${NC}"
        fi
        
        echo -n "  clang-format: "
        if command -v clang-format &> /dev/null; then
            echo -e "${GREEN}$(clang-format --version | head -n1)${NC}"
        else
            echo -e "${YELLOW}not found (optional)${NC}"
        fi
        
        echo -n "  valgrind: "
        if command -v valgrind &> /dev/null; then
            echo -e "${GREEN}found${NC}"
        else
            echo -e "${YELLOW}not found (optional)${NC}"
        fi
        ;;
        
    help|--help|-h)
        print_usage
        ;;
        
    *)
        echo -e "${RED}Unknown command: $1${NC}"
        print_usage
        exit 1
        ;;
esac