#include <rocksdb/db.h>
#include <rocksdb/options.h>
#include <rocksdb/table.h>
#include <rocksdb/write_batch.h>
#include <rocksdb/filter_policy.h> 
#include <chrono>
#include <random>
#include <fstream>
#include <iostream>
#include <vector>
#include <filesystem>

using namespace rocksdb;
namespace fs = std::filesystem;

// ------------------- Generate random 128-bit hashes -------------------
std::string randomHash() {
    static std::mt19937_64 rng(std::random_device{}());
    static std::uniform_int_distribution<unsigned long long> dist;
    std::string hash(16, '\0');
    for (int i = 0; i < 2; ++i) {
        unsigned long long val = dist(rng);
        std::memcpy(&hash[i * 8], &val, 8);
    }
    return hash;
}

// ------------------- Measure elapsed time helper -------------------
double seconds_since(const std::chrono::high_resolution_clock::time_point &start) {
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(end - start).count();
}

// ------------------- Main benchmark -------------------
int main(int argc, char **argv) {
    // Defaults
    size_t size = 100000;
    double duplicates = 0.0;
    std::string bloom = "off";
    std::string compaction = "leveled";
    std::string format = "BlockBasedTable";
    int read_batch = 1000;
    std::string db_path = "/tmp/rocksdb_bench";

    // Parse args
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--size" && i + 1 < argc) size = std::stoll(argv[++i]);
        else if (arg == "--duplicates" && i + 1 < argc) duplicates = std::stod(argv[++i]);
        else if (arg == "--bloom" && i + 1 < argc) bloom = argv[++i];
        else if (arg == "--compaction" && i + 1 < argc) compaction = argv[++i];
        else if (arg == "--format" && i + 1 < argc) format = argv[++i];
        else if (arg == "--read_batch" && i + 1 < argc) read_batch = std::stoi(argv[++i]);
        else if (arg == "--path" && i + 1 < argc) db_path = argv[++i];
    }

    // Clean old DB
    fs::remove_all(db_path);

    Options options;
    options.create_if_missing = true;

    // Compaction style
    if (compaction == "universal")
        options.compaction_style = kCompactionStyleUniversal;
    else
        options.compaction_style = kCompactionStyleLevel;

    // Table format
    if (format == "PlainTable") {
        PlainTableOptions plain_opts;
        options.table_factory.reset(NewPlainTableFactory(plain_opts));
    } else if (format == "CuckooTable") {
        options.table_factory.reset(NewCuckooTableFactory());
    } else {
        BlockBasedTableOptions table_options;
        if (bloom == "on")
            table_options.filter_policy.reset(NewBloomFilterPolicy(10));
        options.table_factory.reset(NewBlockBasedTableFactory(table_options));
    }

    DB* db;
    Status s = DB::Open(options, db_path, &db);
    if (!s.ok()) {
        std::cerr << "Failed to open DB: " << s.ToString() << std::endl;
        return 1;
    }

    // ------------------- WRITE PHASE -------------------
    size_t num_duplicates = static_cast<size_t>(size * duplicates);
    size_t unique_count = size - num_duplicates;

    std::vector<std::string> keys;
    keys.reserve(size);

    for (size_t i = 0; i < unique_count; ++i)
        keys.push_back(randomHash());
    for (size_t i = 0; i < num_duplicates; ++i)
        keys.push_back(keys[i % unique_count]);

    auto start_write = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < size; ++i) {
        s = db->Put(WriteOptions(), keys[i], "val");
        if (!s.ok()) std::cerr << "Write error: " << s.ToString() << std::endl;
    }
    double write_time = seconds_since(start_write);
    double write_throughput = size / write_time;
    double write_latency = write_time / size;

    // ------------------- READ PHASE -------------------
    size_t found = 0;
    auto start_read = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < size; i += read_batch) {
        size_t end = std::min(i + (size_t)read_batch, size);
        std::vector<Slice> slices;
        std::vector<std::string> values(end - i);
        slices.reserve(end - i);
        for (size_t j = i; j < end; ++j)
            slices.push_back(Slice(keys[j]));

        std::vector<Status> statuses = db->MultiGet(ReadOptions(), slices, &values);
        for (const auto& st : statuses)
            if (st.ok()) found++;
    }

    double read_time = seconds_since(start_read);
    double read_throughput = size / read_time;
    double read_latency = read_time / size;

    delete db;

    // ------------------- LOG RESULTS -------------------
    std::ofstream outfile("results.csv", std::ios::app);
    outfile << size << ","
            << duplicates << ","
            << bloom << ","
            << compaction << ","
            << format << ","
            << read_batch << ","
            << write_time << ","
            << read_time << ","
            << write_throughput << ","
            << read_throughput << ","
            << write_latency << ","
            << read_latency << ","
            << found
            << std::endl;
    outfile.close();

    std::cout << "✅ Done: size=" << size
              << " dup=" << duplicates
              << " bloom=" << bloom
              << " comp=" << compaction
              << " format=" << format
              << " read_batch=" << read_batch
              << " → written " << found << " keys.\n";

    return 0;
}
