FROM ubuntu:22.04

# Stop Ubuntu from pausing for interactive timezone prompts
ENV DEBIAN_FRONTEND=noninteractive

# 1. Disable HTTP pipelining (prevents WSL/Docker packet stalls on large downloads)
# 2. Route all package downloads to Microsoft Azure's South Africa mirror
RUN echo 'Acquire::http::Pipeline-Depth "0";' > /etc/apt/apt.conf.d/99custom && \
    echo 'Acquire::http::No-Cache "true";' >> /etc/apt/apt.conf.d/99custom && \
    echo 'Acquire::BrokenProxy "true";' >> /etc/apt/apt.conf.d/99custom && \
    echo 'Acquire::Retries "3";' >> /etc/apt/apt.conf.d/99custom && \
    sed -i -e 's|http://archive.ubuntu.com/ubuntu/|http://azure.archive.ubuntu.com/ubuntu/|g' \
    -e 's|http://security.ubuntu.com/ubuntu/|http://azure.archive.ubuntu.com/ubuntu/|g' \
    /etc/apt/sources.list && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
    cmake \
    build-essential \
    valgrind \
    pkg-config \
    libpq-dev \
    libpqxx-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Move into /app folder in container
WORKDIR /app

# Copy project files into the container
COPY . .

# Configure and compile the project using CMake 
RUN cmake -B build -DCMAKE_BUILD_TYPE=Debug && \
    cmake --build build -- -j$(nproc)

# Default command when container starts: run the main app
CMD ["./build/bin/tividy_app"]