# 构建阶段
FROM gcc:14.2-bookworm AS build

# 安装构建依赖
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    pkg-config \
    git \
    python3 \
    python3-pip \
    clang-17 \
    clang++-17 \
    ccache \
    && rm -rf /var/lib/apt/lists/*

# 安装Conan
RUN pip3 install --no-cache-dir conan==2.0.16

# 设置工作目录
WORKDIR /workspace

# 复制源代码
COPY . .

# 配置构建环境
ENV CC=clang-17
ENV CXX=clang++-17
ENV CONAN_HOME=/root/.conan2
ENV CCACHE_DIR=/root/.ccache
ENV PATH=/usr/lib/ccache:$PATH

# 安装依赖
RUN conan profile detect --force
RUN conan install config/dependencies/ --build=missing -s build_type=Release

# 构建项目
RUN cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DENABLE_COVERAGE=OFF \
    -DBUILD_TESTING=OFF

RUN ninja -C build

# 生产阶段
FROM debian:12-slim AS production

# 安装运行时依赖
RUN apt-get update && apt-get install -y \
    ca-certificates \
    libtbb12 \
    libmimalloc2.0 \
    zlib1g \
    libbz2-1.0 \
    liblzma5 \
    && rm -rf /var/lib/apt/lists/*

# 创建非root用户
ARG USERNAME=fastqtools
ARG USER_UID=1000
ARG USER_GID=1000

RUN groupadd -g $USER_GID $USERNAME \
    && useradd -m -s /bin/bash -u $USER_UID -g $USER_GID $USERNAME

# 设置工作目录
WORKDIR /app

# 从构建阶段复制二进制文件
COPY --from=build /workspace/build/FastQTools /usr/local/bin/
COPY --from=build /workspace/build/lib* /usr/local/lib/

# 创建符号链接
RUN ln -sf /usr/local/bin/FastQTools /usr/local/bin/fastqtools

# 切换到非root用户
USER $USERNAME

# 设置环境变量
ENV PATH=/usr/local/bin:$PATH
ENV LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

# 暴露端口（如果需要网络服务）
EXPOSE 8080

# 健康检查
HEALTHCHECK --interval=30s --timeout=5s --start-period=5s --retries=3 \
    CMD fastqtools --help || exit 1

CMD ["fastqtools", "--help"]