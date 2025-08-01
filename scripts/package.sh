#!/bin/bash
# scripts/package.sh
#
# Creates a distributable package for FastQTools.
#
# Usage:
# ./scripts/package.sh [VERSION]
#
# Parameters:
#   VERSION: The version number for the package (e.g., 3.1.0). Defaults to 'dev'.

set -e

VERSION=${1:-dev}
PACKAGE_NAME="fastqtools-v${VERSION}-linux-x86_64"
PACKAGE_DIR="dist/${PACKAGE_NAME}"

echo ">>> Preparing package for version ${VERSION}..."

# 1. Build the project in Release mode using the build script
echo ">>> Building project in Release mode..."
./scripts/build.sh clang Release

# 2. Create the package directory structure
echo ">>> Creating package directory: ${PACKAGE_DIR}"
rm -rf "${PACKAGE_DIR}"
mkdir -p "${PACKAGE_DIR}/bin"
mkdir -p "${PACKAGE_DIR}/docs"

# 3. Copy artifacts into the package directory
echo ">>> Copying artifacts..."
cp "build-clang-Release/FastQTools" "${PACKAGE_DIR}/bin/"
cp "LICENSE" "${PACKAGE_DIR}/"
cp "README.md" "${PACKAGE_DIR}/"
cp "CHANGELOG.md" "${PACKAGE_DIR}/"
cp -r "docs/user" "${PACKAGE_DIR}/docs/"

# 4. Create the final compressed archive
echo ">>> Creating compressed archive..."
(cd dist && tar -czf "${PACKAGE_NAME}.tar.gz" "${PACKAGE_NAME}")

# 5. Clean up the temporary package directory
rm -rf "${PACKAGE_DIR}"

echo ">>> Package created successfully: dist/${PACKAGE_NAME}.tar.gz"
