# FastQTools

A modern C++ toolkit for processing FASTQ files used in bioinformatics.

## 🧬 Overview

FastQTools is a high-performance toolkit designed for processing FASTQ files, which are commonly used in bioinformatics to store nucleotide sequences and their quality scores. The tool provides various functionalities for analyzing, filtering, and transforming FASTQ data efficiently.

## 📁 Directory Structure

```
fastqtools/
├── app/                    # Main application and command implementations
├── cmake/                  # CMake modules and configuration
├── config/                 # Project configuration files
│   ├── cmake/             # CMake configurations
│   ├── dependencies/      # Dependency management files
│   └── deployment/        # Deployment configurations
├── docs/                   # Documentation
│   ├── design/            # Design documents
│   ├── dev/               # Developer documentation
│   ├── user/              # User documentation
│   └── references/        # External references
├── examples/               # Usage examples
├── scripts/                # Development and build scripts
├── src/                    # All source code
│   ├── CMakeLists.txt
│   ├── core/              # Core functionality
│   │   ├── common/        # Common utilities and base classes
│   │   └── fastq/         # FastQ file handling
│   ├── analysis/          # Analysis modules
│   │   ├── stats/         # Statistical analysis
│   │   └── processing/    # Data processing pipelines
│   ├── compression/       # Compression and encoding
│   │   └── encoder/       # Encoding functionality
│   └── cli/               # Command-line interface
│       └── commands/      # Command implementations
├── tests/                  # Unit and integration tests
└── tools/                  # Development tools
    ├── benchmark/         # Performance benchmarking tools
    ├── build/              # Build-related tools
    ├── development/       # Development tools
    │   ├── generators/    # Code generation tools
    │   ├── validators/    # Code quality tools
    │   └── profiling/     # Performance profiling tools
    ├── ci/                 # Continuous integration tools
    └── deploy/             # Deployment tools
```

## 🚀 Getting Started

### Prerequisites

- CMake 3.20 or higher
- A C++20 compatible compiler (GCC 10+, Clang 10+, or MSVC 2019+)
- Ninja build system
- Optional: Conan or vcpkg for dependency management

### Building

```bash
# Using the build script (recommended)
./scripts/build.sh -p release

# Or manually with CMake
mkdir build && cd build
cmake .. --preset release --config-dir ../config/build
cmake --build . --preset release
```

### Running

```bash
./build/release/app/FastQTools --help
```

## 📖 Documentation

- [User Guide](docs/user/overview.md) - Getting started and usage instructions
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