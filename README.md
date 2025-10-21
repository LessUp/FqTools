# FastQTools

A modern C++ toolkit for processing FASTQ files used in bioinformatics.

## 🧬 Overview

FastQTools is a high-performance toolkit designed for processing FASTQ files, which are commonly used in bioinformatics to store nucleotide sequences and their quality scores. The tool provides various functionalities for analyzing, filtering, and transforming FASTQ data efficiently.

## 📁 Directory Structure

```
fastqtools/
├── cmake/                       # CMake modules and *.in templates
├── config/
│   ├── dependencies/            # Dependency managers (Conan/vcpkg) metadata
│   └── deployment/              # Docker, packaging configs
├── docs/
│   ├── user/                    # User docs
│   ├── dev/                     # Developer docs
│   └── internal/                # Internal notes/reports (not user-facing)
├── examples/                    # Usage examples
├── scripts/                     # Build, test, lint, coverage, generators/validators
├── src/
│   ├── CMakeLists.txt
│   ├── cli/                     # CLI entry and commands
│   ├── modules/                 # C++20 modules (common/error/config/core/io/fastq/...)
│   ├── core_legacy/             # Transitional legacy code
│   ├── interfaces/              # Interfaces
│   ├── processing/              # Pipelines & operators
│   ├── statistics/              # Stats components
│   └── memory/                  # Memory helpers
├── tests/                       # Unit tests
├── third_party/                 # Vendored third-party headers (with license notes)
│   └── gzstream/include/gzstream.h
├── tools/
│   └── benchmark/               # Performance benchmarks
├── dist/                        # Packaging recipes (conda, homebrew)
├── docker/                      # Dockerfiles
└── .github/workflows/ci.yml     # CI pipeline (build/lint/test/coverage)
```

## 🚀 Getting Started

### Prerequisites

- CMake 3.20 or higher
- A C++20 compatible compiler (GCC 11+, Clang 12+, or MSVC 2019+)
- Ninja build system
- Optional: Conan or vcpkg for dependency management

### Building

```bash
# Using the unified build script (recommended)
#   Usage: ./scripts/build.sh [COMPILER] [BUILD_TYPE] [OPTIONS]
#   Examples:
#     ./scripts/build.sh clang Release
#     ./scripts/build.sh gcc Debug --asan
./scripts/build.sh clang Release

# Or manually with CMake + Conan toolchain
conan install config/dependencies/ --output-folder=build/conan-release --build=missing -s build_type=Release
cmake -S . -B build-clang-release \
  -G "Ninja" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=build/conan-release/conan_toolchain.cmake
cmake --build build-clang-release
```

### Running

```bash
./build-clang-release/FastQTools --help
```

## 📖 Documentation

- [User Guide](docs/user/usage.md) - Getting started and usage instructions
- [Developer Guide](docs/dev/architecture.md) - Architecture and development information
- [API Documentation](docs/api/) - Detailed API reference

## 🧪 Testing

Run the test suite:

```bash
./scripts/test.sh
# Or directly with ctest
ctest --preset release
```

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.