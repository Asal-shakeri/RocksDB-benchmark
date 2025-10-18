#include <rocksdb/db.h>
#include <rocksdb/options.h>
#include <rocksdb/filter_policy.h>
#include <rocksdb/table.h>
#include <rocksdb/write_batch.h>
#include <chrono>
#include <random>
#include <fstream>
#include <iostream>
#include <vector>
#include <iomanip>
#include <cstring>

using namespace rocksdb;

// -------- Utility: Generate random 128-bit hash --------
std::string randomHash() {
    std::string hash(16, '\0');
    static std::mt19937_64 rng(std::random_device{}());
    static std::uniform_int_distribution<unsigned long long> dist;
    for (int i = 0; i < 2; ++i) {
        unsigned long long val = dist(rng);
        memcpy(&hash[i * 8], &val, 8);
    }
    return hash;
}

// -------- Benchmark function --------
struct Result {
    size_t size;
    double duplicates;
    std::string bloom;
    std::string compaction;
    double write_time;
    double read_time;
    size_t found;
};

Result run_benchmark(size_t size, double dup_ratio, const std::string& bloom,
                     const std::string& compaction, const std::string& path) {
    Options options;
    options.create_if_missing = true;
    options.compression = kNoCompression;
    options.IncreaseParallelism();
    if (compaction == "universal")
        options.compaction_style = kCompactionStyleUniversal;
    else
        options.compaction_style = kCompactionStyleLevel;

    BlockBasedTableOptions table_options;
    if (bloom == "on")
        table_options.filter_policy.reset(NewBloomFilterPolicy(10));
    options.table_factory.reset(NewBlockBasedTableFactory(table_options));

    DB* db;
    DestroyDB(path, options); // clear previous
    Status s = DB::Open(options, path, &db);
    if (!s.ok()) throw std::runtime_error("Open DB failed: " + s.ToString());

    std::vector<std::string> hashes;
    hashes.reserve(size);
    for (size_t i = 0; i < size; ++i) hashes.push_back(randomHash());
    size_t dup_count = static_cast<size_t>(size * dup_ratio);
    for (size_t i = 0; i < dup_count; ++i)
        hashes[i] = hashes[i + 1]; // create duplicates

    // ---- Write phase ----
    auto t1 = std::chrono::high_resolution_clock::now();
    WriteBatch batch;
    for (auto& h : hashes) batch.Put(h, "1");
    s = db->Write(WriteOptions(), &batch);
    auto t2 = std::chrono::high_resolution_clock::now();
    double write_time = std::chrono::duration<double>(t2 - t1).count();

    // ---- Read phase ----
    size_t found = 0;
    auto t3 = std::chrono::high_resolution_clock::now();
    for (auto& h : hashes) {
        std::string value;
        if (db->Get(ReadOptions(), h, &value).ok()) found++;
    }
    auto t4 = std::chrono::high_resolution_clock::now();
    double read_time = std::chrono::duration<double>(t4 - t3).count();

    delete db;
    return {size, dup_ratio, bloom, compaction, write_time, read_time, found};
}

int main(int argc, char** argv) {
    // -------- Defaults --------
    std::vector<size_t> sizes = {100000};
    std::vector<double> dups = {0.0};
    std::vector<std::string> blooms = {"off"};
    std::vector<std::string> compactions = {"leveled"};
    std::string db_path = "testdb";

    // -------- Parse arguments --------
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--size") sizes = {std::stoull(argv[++i])};
        else if (arg == "--duplicates") dups = {std::stod(argv[++i])};
        else if (arg == "--bloom") blooms = {argv[++i]};
        else if (arg == "--compaction") compactions = {argv[++i]};
        else if (arg == "--path") db_path = argv[++i];
    }

    // -------- Create CSV --------
    std::ofstream csv("results.csv", std::ios::app);
    if (csv.tellp() == 0)
        csv << "size,duplicates,bloom,compaction,write_time,read_time,found\n";

    std::vector<Result> results;

    // -------- Run combinations --------
    for (auto s : sizes)
        for (auto d : dups)
            for (auto& b : blooms)
                for (auto& c : compactions) {
                    std::cout << "\nRunning: size=" << s
                              << " dup=" << d
                              << " bloom=" << b
                              << " compaction=" << c
                              << " ...\n";
                    try {
                        Result r = run_benchmark(s, d, b, c, db_path);
                        results.push_back(r);
                        csv << r.size << "," << r.duplicates << "," << r.bloom << ","
                            << r.compaction << "," << r.write_time << ","
                            << r.read_time << "," << r.found << "\n";
                    } catch (const std::exception& e) {
                        std::cerr << "Error: " << e.what() << "\n";
                    }
                }

    // -------- Print summary --------
    std::cout << "\n-----------------------------------------------\n";
    std::cout << "Size | Dup% | Bloom | Compaction | W(s) | R(s)\n";
    std::cout << "-----------------------------------------------\n";
    for (auto& r : results)
        std::cout << std::setw(7) << r.size << " | "
                  << std::setw(4) << r.duplicates << " | "
                  << std::setw(5) << r.bloom << " | "
                  << std::setw(10) << r.compaction << " | "
                  << std::setw(5) << std::fixed << std::setprecision(2) << r.write_time << " | "
                  << std::setw(5) << r.read_time << "\n";
    std::cout << "-----------------------------------------------\n";
    std::cout << "Saved results to results.csv\n";
}
