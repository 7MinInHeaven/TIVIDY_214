FROM ubuntu:22.04

#stops Ubuntu from pausing to ask timezone questions during installation - annoying
ENV DEBIAN_FRONTEND=noninteractive

# install C++ compiler, CMake, Valgrind, and the PostgreSQL C++ library (libpqxx)
RUN apt-get update && apt-get install -y \
    cmake \
    build-essential \
    valgrind \
    pkg-config \
    libpq-dev \
    libpqxx-dev \
    git \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# move into /app folder in container
WORKDIR /app

# copy project files into the container
COPY . .

# configure and compile the project using CMake 
RUN cmake -B build -DCMAKE_BUILD_TYPE=Debug && \
    cmake --build build -- -j$(nproc)

# default command when the container starts: run the main app
CMD ["./build/bin/tividy_app"]