# DevOps Challenge: RocksDB Hash Benchmark

This project implements and documents a benchmarking challenge designed to test how **RocksDB** handles large-scale hash workloads.  
It’s based on the **“Problem of Many Hashes”** from the DevOps Challenge, where millions of hash records must be stored, looked up, and updated efficiently.

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

- C++ benchmark using the RocksDB native API  
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
devops-hash-benchmark/
├── README.md
├── src/
│   ├── benchmark.cpp
│   ├── utils.hpp
│   └── CMakeLists.txt
├── scripts/
│   ├── run_benchmarks.sh
│   └── plot_results.py
├── results/
│   ├── results.csv
│   └── plots/
│       ├── throughput.png
│       └── latency.png
├── docs/
│   ├── report.md
│   └── setup_guide.md
└── LICENSE
```

---

## 🚀 Setup and Execution

### 1. Prerequisites (macOS)
```bash
brew install rocksdb cmake gcc python3
pip3 install matplotlib pandas psutil
```

### 2. Build and Run
```bash
mkdir build && cd build
cmake ..
make
./benchmark
```

### 3. Run Automated Benchmarks
```bash
bash scripts/run_benchmarks.sh
python3 scripts/plot_results.py
```

---

## 📊 Output

- Raw data saved in: `results/results.csv`
- Plots generated in: `results/plots/`
- Analysis documented in: [`docs/report.md`](docs/report.md)

---

## 🧠 Acknowledgments

- **RocksDB Team** — for documentation and benchmarking tools  
- **Facebook / Meta Open Source** — for RocksDB  
- **OpenAI ChatGPT (GPT-5)** — assisted in documentation, design, and automation setup  

---

## 📚 References

- [RocksDB Wiki](https://github.com/facebook/rocksdb/wiki)
- [RocksDB Blog](https://rocksdb.org/blog/)
- [Designing Data-Intensive Applications](https://dataintensive.net/)
- [DevOps Challenge PDF](docs/report.md)
