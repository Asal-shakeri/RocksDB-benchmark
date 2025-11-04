#!/usr/bin/env python3
"""
Visualize RocksDB benchmark results
Generates bar charts for Bloom filters, compaction styles, formats, batch sizes, etc.
"""

import pandas as pd
import matplotlib.pyplot as plt
import os

# --- Load data ---
cols = ["size","duplicates","bloom","compaction","format","batch",
        "write_time","read_time","write_throughput","read_throughput",
        "write_latency","read_latency","mem_kb","db_size"]
df = pd.read_csv("results.csv", names=cols, header=None)


# --- Prepare output folder ---
os.makedirs("plots", exist_ok=True)

def save_plot(df_grouped, x_label, y_label, title, filename):
    plt.figure(figsize=(7,5))
    df_grouped.plot(kind="bar", legend=False)
    plt.title(title)
    plt.xlabel(x_label)
    plt.ylabel(y_label)
    plt.tight_layout()
    plt.grid(axis='y', linestyle='--', alpha=0.6)
    plt.savefig(f"plots/{filename}.png")
    plt.close()

# --- 1. Bloom filter effect on read throughput ---
save_plot(
    df.groupby("bloom")["read_throughput"].mean(),
    "Bloom filter",
    "Ops/sec",
    "Read Throughput vs Bloom Filter",
    "read_throughput_vs_bloom"
)

# --- 2. Compaction style vs write time ---
save_plot(
    df.groupby("compaction")["write_time"].mean(),
    "Compaction style",
    "Seconds",
    "Write Time vs Compaction Style",
    "write_time_vs_compaction"
)

# --- 3. Batch size vs read latency ---
save_plot(
    df.groupby("batch")["read_latency"].mean(),
    "Batch size",
    "Seconds per op",
    "Read Latency vs Batch Size",
    "read_latency_vs_batch"
)

# --- 4. Table format vs final DB size ---
save_plot(
    df.groupby("format")["db_size"].mean() / (1024**2),
    "SST Format",
    "DB Size (MB)",
    "Final DB Size vs Table Format",
    "db_size_vs_format"
)

# --- 5. Duplicate ratio vs write throughput ---
save_plot(
    df.groupby("duplicates")["write_throughput"].mean(),
    "Duplicate ratio",
    "Ops/sec",
    "Write Throughput vs Duplicates",
    "write_throughput_vs_duplicates"
)

# --- 6. Database size vs memory usage ---
save_plot(
    df.groupby("size")["mem_kb"].mean() / 1024,
    "Database size (items)",
    "Memory (MB)",
    "Memory Usage vs Database Size",
    "memory_vs_size"
)

print("✅ All plots saved to ./plots/")
