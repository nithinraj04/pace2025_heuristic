FROM --platform=linux/amd64 ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    g++ \
    libglpk-dev \
    libltdl-dev \
    zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY fk2d_v4.cpp .

RUN g++ -O2 -std=c++17 fk2d_v4.cpp -static -lglpk -lltdl -lz -o a.out

CMD ["file", "./a.out"]
