#!/bin/bash

# FastQTools 测试运行脚本
# 提供便捷的测试执行和管理功能

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 默认值
PRESET="debug"
VERBOSE=false
COVERAGE=false
FILTER=""
PARALLEL=true
REPEAT=1

print_usage() {
    echo "FastQTools Test Runner"
    echo ""
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -p, --preset PRESET    CMake preset to use (default: debug)"
    echo "  -v, --verbose          Enable verbose output"
    echo "  -c, --coverage         Generate coverage report"
    echo "  -f, --filter PATTERN   Run only tests matching pattern"
    echo "  -j, --parallel         Run tests in parallel (default: true)"
    echo "  -r, --repeat COUNT     Repeat tests COUNT times (default: 1)"
    echo "  -h, --help             Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                     # Run all tests with debug preset"
    echo "  $0 -p release          # Run tests with release preset"
    echo "  $0 -f \"*timer*\"        # Run only timer-related tests"
    echo "  $0 -c                  # Run tests and generate coverage"
    echo "  $0 -r 5                # Run tests 5 times"
}

# 解析命令行参数
while [[ $# -gt 0 ]]; do
    case $1 in
        -p|--preset)
            PRESET="$2"
            shift 2
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -c|--coverage)
            COVERAGE=true
            shift
            ;;
        -f|--filter)
            FILTER="$2"
            shift 2
            ;;
        -j|--parallel)
            PARALLEL=true
            shift
            ;;
        -r|--repeat)
            REPEAT="$2"
            shift 2
            ;;
        -h|--help)
            print_usage
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            print_usage
            exit 1
            ;;
    esac
done

echo -e "${BLUE}FastQTools Test Runner${NC}"
echo -e "${BLUE}=====================${NC}"
echo ""

# 检查构建目录是否存在
BUILD_DIR="build/$PRESET"
if [[ ! -d "$BUILD_DIR" ]]; then
    echo -e "${YELLOW}Build directory not found. Building project first...${NC}"
    ./scripts/build.sh -p $PRESET
fi

# 进入构建目录
cd $BUILD_DIR

# 构建测试参数
TEST_ARGS=""
if [[ "$VERBOSE" == true ]]; then
    TEST_ARGS="$TEST_ARGS --verbose"
fi

if [[ -n "$FILTER" ]]; then
    TEST_ARGS="$TEST_ARGS -R \"$FILTER\""
fi

if [[ "$PARALLEL" == true ]]; then
    TEST_ARGS="$TEST_ARGS --parallel"
fi

# 运行测试
echo -e "${BLUE}Running tests with preset: $PRESET${NC}"
echo -e "${BLUE}Test arguments: $TEST_ARGS${NC}"
echo ""

for ((i=1; i<=REPEAT; i++)); do
    if [[ $REPEAT -gt 1 ]]; then
        echo -e "${BLUE}Test run $i of $REPEAT${NC}"
    fi
    
    if eval "ctest --preset $PRESET $TEST_ARGS"; then
        echo -e "${GREEN}Tests passed!${NC}"
    else
        echo -e "${RED}Tests failed!${NC}"
        exit 1
    fi
    
    if [[ $i -lt $REPEAT ]]; then
        echo ""
    fi
done

# 生成覆盖率报告
if [[ "$COVERAGE" == true ]]; then
    echo ""
    echo -e "${BLUE}Generating coverage report...${NC}"
    
    if command -v lcov &> /dev/null; then
        # 使用lcov生成覆盖率报告
        lcov --capture --directory . --output-file coverage.info
        lcov --remove coverage.info '/usr/*' --output-file coverage.info
        lcov --list coverage.info
        
        if command -v genhtml &> /dev/null; then
            genhtml coverage.info --output-directory coverage_html
            echo -e "${GREEN}Coverage report generated in: $BUILD_DIR/coverage_html/index.html${NC}"
        fi
    else
        echo -e "${YELLOW}lcov not found. Coverage report not generated.${NC}"
    fi
fi

echo ""
echo -e "${GREEN}Test execution completed!${NC}"
