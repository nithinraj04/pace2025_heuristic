FROM --platform=linux/amd64 ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    g++ \
    libglpk-dev \
    libltdl-dev \
    zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY pace25_heuristic.cpp .

RUN g++ -O2 -std=c++17 pace25_heuristic.cpp -static -lglpk -lltdl -lz -o a.out

CMD ["file", "./a.out"]
