FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y \
    python3 \
    python3-pip \
    python3-venv \
    python3-dev \
    git \
    curl \
    wget \
    vim \
    nano \
    sudo \
    build-essential \
    ca-certificates \
    cmake \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src

# Copy project sources into the image
COPY . /src

# Build and install FLDLib (defaults to enable tests)
RUN mkdir -p build && cd build && \
    cmake -DCMAKE_INSTALL_PREFIX=/usr/local -DFLDLIB_ENABLE_TESTS=ON .. && \
    make -j$(nproc) && \
    make install

ENV FLOATDIAGNOSISHOME=/usr/local
ENV PATH=$PATH:/usr/local/bin

CMD ["/bin/bash"]
