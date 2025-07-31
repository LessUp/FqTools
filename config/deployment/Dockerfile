# Dockerfile for FastQTools
# This Dockerfile creates a consistent build and runtime environment.

# ---- Builder Stage ----
# Uses Ubuntu 22.04 as the base for the build environment.
FROM ubuntu:22.04 AS builder

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# 1. Install base dependencies and GCC
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    wget \
    gnupg \
    software-properties-common \
    python3-pip \
    cmake \
    ninja-build

# 2. Install specific Clang version (19)
RUN wget https://apt.llvm.org/llvm.sh && \
    chmod +x llvm.sh && \
    ./llvm.sh 19 && \
    apt-get install -y clang-19 clang-tidy-19 lld-19

# 3. Install specific Conan version
RUN pip3 install conan==2.19.0

# 4. Copy project source code
WORKDIR /app
COPY . .

# 5. Run Conan to install project dependencies
# We use the system's default GCC (which should be >= 15.1 on modern systems)
# and the installed Clang 19. Here we configure Conan to use GCC.
RUN conan profile detect --force && \
    conan install config/dependencies/ --output-folder=build/conan-release --build=missing -s build_type=Release

# 6. Configure and build the project with CMake
RUN cmake -B build -S . \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=build/conan-release/conan_toolchain.cmake \
    -G "Ninja"
RUN cmake --build build

# ---- Final Stage ----
# Uses a slim base image for a smaller footprint.
FROM ubuntu:22.04

# Copy the compiled executable from the builder stage
COPY --from=builder /app/build/FastQTools /usr/local/bin/FastQTools

# Set the entrypoint for the container
ENTRYPOINT ["/usr/local/bin/FastQTools"]
CMD ["--help"]