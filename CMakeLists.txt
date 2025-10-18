cmake_minimum_required(VERSION 3.10)
project(rocksdb_bench)
set(CMAKE_CXX_STANDARD 17)

include_directories(/opt/homebrew/opt/rocksdb/include)
link_directories(/opt/homebrew/opt/rocksdb/lib)

add_executable(rocksdb_bench src/main.cpp)
target_link_libraries(rocksdb_bench rocksdb pthread)

