#!/bin/bash
# scripts/coverage.sh
#
# A comprehensive coverage analysis script for FastQTools.
#
# Usage:
# ./scripts/coverage.sh [COMPILER]
#
# Parameters:
#   COMPILER: 'gcc' or 'clang'. Defaults to 'clang'.
#
# This script will:
# 1. Build the project with coverage instrumentation
# 2. Run all tests
# 3. Generate coverage reports
# 4. Generate HTML coverage report

set -e

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default values
COMPILER=${1:-clang}
BUILD_DIR="build-${COMPILER}-coverage"
COVERAGE_DIR="${BUILD_DIR}/coverage"
HTML_DIR="${COVERAGE_DIR}/html"

echo -e "${BLUE}FastQTools Coverage Analysis${NC}"
echo -e "${BLUE}===========================${NC}"

# Check if lcov is available
if ! command -v lcov &> /dev/null; then
    echo -e "${RED}Error: lcov is not installed${NC}"
    echo -e "${YELLOW}Please install lcov:${NC}"
    echo -e "${YELLOW}  Ubuntu/Debian: sudo apt-get install lcov${NC}"
    echo -e "${YELLOW}  CentOS/RHEL: sudo yum install lcov${NC}"
    echo -e "${YELLOW}  macOS: brew install lcov${NC}"
    exit 1
fi

# Check if genhtml is available
if ! command -v genhtml &> /dev/null; then
    echo -e "${RED}Error: genhtml is not installed${NC}"
    echo -e "${YELLOW}Please install genhtml:${NC}"
    echo -e "${YELLOW}  Ubuntu/Debian: sudo apt-get install lcov${NC}"
    echo -e "${YELLOW}  CentOS/RHEL: sudo yum install lcov${NC}"
    echo -e "${YELLOW}  macOS: brew install lcov${NC}"
    exit 1
fi

echo -e "${GREEN}>>> Building project with coverage instrumentation...${NC}"

# Build project with coverage (unified with build.sh directory naming)
if ! ./scripts/build.sh "${COMPILER}" Coverage --coverage; then
    echo -e "${RED}Error: Build failed${NC}"
    exit 1
fi

echo -e "${GREEN}>>> Running tests...${NC}"

# Run tests in the coverage build directory
if ! ctest --test-dir "${BUILD_DIR}" --output-on-failure; then
    echo -e "${RED}Error: Tests failed${NC}"
    exit 1
fi

echo -e "${GREEN}>>> Generating coverage data...${NC}"

# Create coverage directory
mkdir -p "${COVERAGE_DIR}"

# Select proper gcov tool (prefer wrapper to support llvm-cov gcov)
GCOV_TOOL=""
WRAPPER_TOOL="$(dirname "$0")/gcov-wrapper.sh"
if [ -x "$WRAPPER_TOOL" ]; then
    GCOV_TOOL="$WRAPPER_TOOL"
else
    if [[ "${COMPILER}" == "clang" ]]; then
        if command -v llvm-cov &> /dev/null; then
            # Create a temporary wrapper to pass subcommand 'gcov'
            GCOV_TOOL="$(mktemp)" && echo -e "#!/usr/bin/env bash\nexec llvm-cov gcov \"$@\"" > "$GCOV_TOOL" && chmod +x "$GCOV_TOOL"
        fi
    else
        if command -v gcov &> /dev/null; then
            GCOV_TOOL="$(command -v gcov)"
        fi
    fi
fi

# Capture coverage data
echo "Capturing coverage data..."
LCOV_CAPTURE_CMD=(lcov --capture \
    --directory "${BUILD_DIR}" \
    --output-file "${COVERAGE_DIR}/coverage.info" \
    --rc lcov_branch_coverage=1 \
    --rc geninfo_unexecuted_blocks=1 \
    --ignore-errors mismatch \
    --ignore-errors version \
    --ignore-errors inconsistent \
    --ignore-errors empty)

if [[ -n "${GCOV_TOOL}" ]]; then
    LCOV_CAPTURE_CMD+=(--gcov-tool "${GCOV_TOOL}")
fi

"${LCOV_CAPTURE_CMD[@]}"

# Remove system headers and test files from coverage
echo "Filtering coverage data..."
lcov --remove "${COVERAGE_DIR}/coverage.info" \
    '/usr/*' \
    '*/miniconda3/*' \
    '*/.conan2/*' \
    '*/tests/*' \
    '*/test_*' \
    '*/build-*/*' \
    --output-file "${COVERAGE_DIR}/coverage-filtered.info" \
    --rc lcov_branch_coverage=1 \
    --ignore-errors empty \
    --ignore-errors inconsistent \
    --ignore-errors version \
    --ignore-errors unused

echo -e "${GREEN}>>> Generating HTML coverage report...${NC}"

# Generate HTML report
genhtml "${COVERAGE_DIR}/coverage-filtered.info" \
    --output-directory "${HTML_DIR}" \
    --branch-coverage \
    --function-coverage \
    --title "FastQTools Coverage Report" \
    --ignore-errors empty \
    --ignore-errors inconsistent \
    --ignore-errors version

echo -e "${GREEN}>>> Coverage analysis complete!${NC}"
echo ""
echo -e "${BLUE}Coverage Report:${NC}"
echo "  HTML Report: ${HTML_DIR}/index.html"
echo "  Coverage Data: ${COVERAGE_DIR}/coverage-filtered.info"

# Show coverage summary
if command -v python3 &> /dev/null; then
    echo ""
    echo -e "${BLUE}Coverage Summary:${NC}"
    INFO_FILE="${COVERAGE_DIR}/coverage-filtered.info"
    python3 - "$INFO_FILE" << 'EOF'
import re
import sys
import os

def extract_coverage_info(info_file):
    lines_hit = 0
    lines_total = 0
    functions_hit = 0
    functions_total = 0
    branches_hit = 0
    branches_total = 0
    
    with open(info_file, 'r') as f:
        content = f.read()
        
    # Extract line coverage
    line_match = re.search(r'LH:(\d+)', content)
    if line_match:
        lines_hit = int(line_match.group(1))
    
    line_total_match = re.search(r'LF:(\d+)', content)
    if line_total_match:
        lines_total = int(line_total_match.group(1))
    
    # Extract function coverage
    fn_hit_match = re.search(r'FH:(\d+)', content)
    if fn_hit_match:
        functions_hit = int(fn_hit_match.group(1))
    
    fn_total_match = re.search(r'FNF:(\d+)', content)
    if fn_total_match:
        functions_total = int(fn_total_match.group(1))
    
    # Extract branch coverage
    br_hit_match = re.search(r'BRH:(\d+)', content)
    if br_hit_match:
        branches_hit = int(br_hit_match.group(1))
    
    br_total_match = re.search(r'BRF:(\d+)', content)
    if br_total_match:
        branches_total = int(br_total_match.group(1))
    
    return {
        'lines': (lines_hit, lines_total),
        'functions': (functions_hit, functions_total),
        'branches': (branches_hit, branches_total)
    }

def calculate_percentage(hit, total):
    if total == 0:
        return 0.0
    return (hit / total) * 100

if __name__ == "__main__":
    info_file = sys.argv[1] if len(sys.argv) > 1 else "coverage-filtered.info"
    
    try:
        coverage_data = extract_coverage_info(info_file)
        
        # Line coverage
        line_hit, line_total = coverage_data['lines']
        line_pct = calculate_percentage(line_hit, line_total)
        
        # Function coverage
        fn_hit, fn_total = coverage_data['functions']
        fn_pct = calculate_percentage(fn_hit, fn_total)
        
        # Branch coverage
        br_hit, br_total = coverage_data['branches']
        br_pct = calculate_percentage(br_hit, br_total)
        
        print(f"  Lines:      {line_hit:6d} / {line_total:6d} ({line_pct:5.1f}%)")
        print(f"  Functions:  {fn_hit:6d} / {fn_total:6d} ({fn_pct:5.1f}%)")
        print(f"  Branches:   {br_hit:6d} / {br_total:6d} ({br_pct:5.1f}%)")
        
    except Exception as e:
        print(f"  Error calculating coverage: {e}")
EOF
fi

echo ""
echo -e "${YELLOW}To view the detailed HTML report, open:${NC}"
echo -e "${YELLOW}  file://${HTML_DIR}/index.html${NC}"