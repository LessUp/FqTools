#!/bin/bash
# scripts/install_dependencies.sh
#
# This script installs the precise toolchain required for building FastQTools.
# It is designed for Ubuntu-based systems.

set -e

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}>>> Starting FastQTools toolchain installation...${NC}"

# 1. Check if running on Ubuntu
if ! grep -q "Ubuntu" /etc/os-release 2>/dev/null; then
    echo -e "${YELLOW}Warning: This script is designed for Ubuntu systems.${NC}"
    echo -e "${YELLOW}Your system may not be fully compatible.${NC}"
    read -p "Continue anyway? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# 2. Update package lists
echo -e "${GREEN}>>> Updating package lists...${NC}"
if ! sudo apt-get update; then
    echo -e "${RED}Error: Failed to update package lists${NC}"
    exit 1
fi

# 3. Install essential build tools
echo -e "${GREEN}>>> Installing essential build tools (GCC, CMake, Python)...${NC}"
ESSENTIAL_PACKAGES="build-essential wget gnupg software-properties-common python3-pip cmake ninja-build"
if ! sudo apt-get install -y $ESSENTIAL_PACKAGES; then
    echo -e "${RED}Error: Failed to install essential packages${NC}"
    exit 1
fi

# 4. Install Clang/LLVM version 19
echo -e "${GREEN}>>> Installing Clang/LLVM version 19...${NC}"
if [ ! -f "llvm.sh" ]; then
    if ! wget https://apt.llvm.org/llvm.sh; then
        echo -e "${RED}Error: Failed to download LLVM installer${NC}"
        exit 1
    fi
fi

chmod +x llvm.sh
if ! sudo ./llvm.sh 19; then
    echo -e "${RED}Error: Failed to install LLVM 19${NC}"
    exit 1
fi

if ! sudo apt-get install -y clang-19 clang-tidy-19 clang-format-19 libclang-19-dev lld-19; then
    echo -e "${RED}Error: Failed to install LLVM tools${NC}"
    exit 1
fi

# 5. Set default clang to version 19
echo -e "${GREEN}>>> Setting default clang to version 19...${NC}"
sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-19 100
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-19 100
sudo update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-19 100
sudo update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-19 100

# 6. Install Conan
echo -e "${GREEN}>>> Installing Conan...${NC}"
if ! pip3 install conan==2.19.0; then
    echo -e "${RED}Error: Failed to install Conan${NC}"
    exit 1
fi

# 7. Verify installation
echo -e "${GREEN}>>> Verifying installation...${NC}"
for cmd in gcc g++ clang clang++ cmake ninja conan; do
    if ! command -v $cmd &> /dev/null; then
        echo -e "${RED}Error: $cmd is not available after installation${NC}"
        exit 1
    fi
done

# 8. Clean up
echo -e "${GREEN}>>> Cleaning up...${NC}"
rm -f llvm.sh

echo -e "${GREEN}>>> Toolchain installation complete!${NC}"
echo -e "${GREEN}    GCC:       $(gcc --version | head -n1)${NC}"
echo -e "${GREEN}    Clang:     $(clang --version | head -n1)${NC}"
echo -e "${GREEN}    CMake:     $(cmake --version | head -n1)${NC}"
echo -e "${GREEN}    Ninja:     $(ninja --version)${NC}"
echo -e "${GREEN}    Conan:     $(conan --version)${NC}"
