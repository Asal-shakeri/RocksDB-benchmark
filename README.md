# RocksDB Hash Benchmark

This project implements and documents a benchmarking challenge designed to test how **RocksDB** handles large-scale hash workloads.  
It’s based on the **“Problem of Many Hashes”**, where millions of hash records must be stored, looked up, and updated efficiently.

---

## 🧩 Problem Overview

Imagine having:
- 100 million unique hashes stored in a RocksDB database.
- Each day, millions of new hashes arrive — some are duplicates, some are new.
- We must check for duplicates (point lookups) and then bulk insert only the unique ones.

This setup creates alternating **read** and **write** phases:
- **Lookup phase** → Random access lookups using `MultiGet`.
- **Ingest phase** → Bulk inserts via SST file ingestion.

Our goal is to evaluate RocksDB’s performance under this workload.

---

## ⚙️ Features

- C++ benchmark 
- Bulk load via **SST file ingestion**
- Point lookups via **MultiGet**
- Benchmark configuration options:
  - Database sizes: 1M, 10M, 100M
  - Duplicate ratios: 0%, 25%, 50%, 75%, 100%
  - SST table formats: BlockBased, Plain, Cuckoo
  - Compaction styles: Leveled, Universal
  - Bloom filters: on/off
- Automatically records:
  - Throughput (ops/sec)
  - Average latency (s/op)
  - CPU, memory, and disk usage
  - Final database size

---

## 🧱 Project Structure

```
.
├── CMakeLists.txt
├── LICENSE
├── README.md
├── build
│   ├── CMakeCache.txt
│   ├── CMakeFiles
│   ├── Makefile
│   ├── cmake_install.cmake
│   ├── plots
│   ├── run_matrix_full.sh
│   ├── analyze_data.py
│   └── rocksdb_bench
└── src
    └── main.cpp
```

---

## 🚀 Setup and Execution

### 1. Prerequisites (macOS)
```bash
brew install rocksdb cmake gcc python3
pip3 install matplotlib pandas psutil
```

### Prerequisites (Ubuntu/Debian-based)
```bash
sudo apt update
sudo apt install -y build-essential cmake gcc g++ python3 python3-pip librocksdb-dev
pip3 install matplotlib pandas psutil
```

### Note:
On some distributions, librocksdb-dev might not include the latest version.
You can build RocksDB manually if needed:
```bash
git clone https://github.com/facebook/rocksdb.git
cd rocksdb
make static_lib
sudo make install
```

### 2. Build and Run
```bash
mkdir build && mv ../run_matrix_full.sh ../analyze_data.py build && cd build
cmake ..
make
```

### 3. Run Automated Benchmarks
```bash
./run_matrix_full.sh  
python3 analyze_results.py 
```

---

## 📊 Output

- Raw data saved in: `results/results.csv`
- Plots generated in: `results/plots/`
- Analysis documented in: [`docs/report.md`](docs/report.md)

---

