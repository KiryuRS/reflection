# Dockerfile for Compile Time Reflection Library
# Built with gcc 14, conan 2.x, cmake 4.1.2 on Ubuntu 24.04 LTS

FROM ubuntu:24.04

# Install system dependencies
RUN apt-get update && apt-get install -y \
    # Basic tools
    curl \
    wget \
    git \
    build-essential \
    software-properties-common \
    ca-certificates \
    gnupg \
    lsb-release \
    # Python 3 and pip for Conan
    python3 \
    python3-pip \
    python3-venv \
    # Additional build tools
    ninja-build \
    && rm -rf /var/lib/apt/lists/*

# Install GCC 14 from Ubuntu toolchain PPA
RUN add-apt-repository ppa:ubuntu-toolchain-r/test && \
    apt-get update && \
    apt-get install -y gcc-14 g++-14 && \
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-14 100 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-14 100 && \
    rm -rf /var/lib/apt/lists/*

# Install CMake 4.1.2
RUN wget https://github.com/Kitware/CMake/releases/download/v4.1.2/cmake-4.1.2-linux-x86_64.tar.gz && \
    tar -xzf cmake-4.1.2-linux-x86_64.tar.gz && \
    mv cmake-4.1.2-linux-x86_64 /opt/cmake && \
    ln -sf /opt/cmake/bin/cmake /usr/local/bin/cmake && \
    ln -sf /opt/cmake/bin/ctest /usr/local/bin/ctest && \
    ln -sf /opt/cmake/bin/cpack /usr/local/bin/cpack && \
    rm cmake-4.1.2-linux-x86_64.tar.gz

# Install Conan 2.x
RUN pip3 install --break-system-packages conan==2.*

# Set working directory
WORKDIR /app

# Copy source code
COPY . /app

# Make conan_build.sh executable
RUN chmod +x /app/conan_build.sh

# Build and run tests
CMD ["/app/conan_build.sh"]
