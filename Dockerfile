# Game Boy Development Environment with GBDK-2020, Node.js, and development tools
FROM ubuntu:22.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    wget \
    curl \
    unzip \
    git \
    make \
    cmake \
    pkg-config \
    libsdl2-dev \
    libsdl2-image-dev \
    python3 \
    python3-pip \
    sudo \
    bc \
    && rm -rf /var/lib/apt/lists/*

# Install Node.js (latest LTS) and npm for Claude Code
RUN curl -fsSL https://deb.nodesource.com/setup_lts.x | bash - \
    && apt-get install -y nodejs \
    && npm --version \
    && node --version

# Create development user to avoid running as root
RUN useradd -m -s /bin/bash gbdev \
    && echo "gbdev ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# Switch to development user
USER gbdev
WORKDIR /home/gbdev

# Install GBDK-2020 (Game Boy Development Kit)
RUN wget https://github.com/gbdk-2020/gbdk-2020/releases/download/4.2.0/gbdk-linux64.tar.gz \
    && tar -xzf gbdk-linux64.tar.gz \
    && rm gbdk-linux64.tar.gz

# Set up environment variables for GBDK
ENV GBDK_HOME=/home/gbdev/gbdk
ENV PATH="${GBDK_HOME}/bin:${PATH}"

# Note: SameBoy emulator installation skipped in container
# SameBoy requires GUI/X11 which is complex in containers
# Install via: apt install sameboy (if available) or use BGB/other emulators
# For now, ROMs can be tested by copying to host machine

# Install Claude Code globally as gbdev user
RUN curl -fsSL https://claude.ai/install.sh | bash

USER gbdev
# Create project structure
RUN mkdir -p /home/gbdev/gameboy-project/{src,include,build,assets,tools}

# Set working directory for the project
WORKDIR /home/gbdev/gameboy-project

# Copy development files when container starts
VOLUME ["/home/gbdev/gameboy-project"]

# Default command
CMD ["/bin/bash"]