FROM ubuntu:22.04

# Basic Setup
RUN apt-get update && apt-get install -y \
    build-essential \
    python3 \
    python3-pip \
    time \
    cmake \
    git \
    util-linux \
    && rm -rf /var/lib/apt/lists/*

RUN apt update && apt install openjdk-21-jdk -y

# INSTALL YOUR SOLVER AND DEPENDENCIES HERE
RUN apt-get install -y \
	libglpk-dev \
    libltdl-dev \
    zlib1g-dev

COPY pace25_heuristic.cpp /solver/pace25_heuristic.cpp
RUN g++ -O2 -std=c++17 /solver/pace25_heuristic.cpp -static -lglpk -lltdl -lz -o /solver/a.out
# END OF YOUR CODE HERE.

# working directory
WORKDIR /pace
COPY ../src/eval.py eval.py
COPY ../src/ds_verifier.py verifier.py

RUN mkdir -p /output && chmod 777 /output
CMD ["python3", "eval.py"]
