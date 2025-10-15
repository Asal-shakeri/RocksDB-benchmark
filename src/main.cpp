#include <rocksdb/db.h>
#include <rocksdb/options.h>
#include <rocksdb/slice.h>
#include <rocksdb/write_batch.h>
#include <iostream>
#include <chrono>
#include <random>

using namespace rocksdb;

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

int main(int argc, char** argv) {
    Options options;
    options.create_if_missing = true;
    options.compression = kNoCompression;
    options.IncreaseParallelism();
    options.OptimizeLevelStyleCompaction();

    DB* db;
    Status s = DB::Open(options, "testdb", &db);
    if (!s.ok()) { std::cerr << "Open DB failed: " << s.ToString() << "\n"; return 1; }

    const int N = 100000;  // adjust
    std::vector<std::string> hashes;
    hashes.reserve(N);
    for (int i = 0; i < N; ++i) hashes.push_back(randomHash());

    // --- Write phase ---
    auto t1 = std::chrono::high_resolution_clock::now();
    WriteBatch batch;
    for (auto &h : hashes) batch.Put(h, "1");
    s = db->Write(WriteOptions(), &batch);
    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> write_time = t2 - t1;
    std::cout << "Inserted " << N << " hashes in " << write_time.count() << "s\n";

    // --- Read phase ---
    int found = 0;
    auto t3 = std::chrono::high_resolution_clock::now();
    for (auto &h : hashes) {
        std::string value;
        if (db->Get(ReadOptions(), h, &value).ok()) found++;
    }
    auto t4 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> read_time = t4 - t3;
    std::cout << "Read " << found << " hashes in " << read_time.count() << "s\n";

    delete db;
    return 0;
}
