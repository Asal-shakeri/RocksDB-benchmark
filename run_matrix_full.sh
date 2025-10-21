#!/bin/bash
# run_matrix_full.sh
cd "$(dirname "$0")"

export OUT=results_ssd.csv
rm -f "$OUT"
echo "size,duplicates,bloom,compaction,format,read_batch,write_time,read_time,write_throughput,read_throughput,write_latency,read_latency,found" > "$OUT"

export SSD_DB=/tmp/rocksdb_ssd
mkdir -p "$SSD_DB"

for size in 100000 1000000; do
  for dup in 0.0 0.5; do
    for bloom in on off; do
      for comp in leveled universal; do
        for format in BlockBasedTable PlainTable; do
          for batch in 1000 10000; do
            echo "▶️ Running size=$size dup=$dup bloom=$bloom comp=$comp format=$format batch=$batch"
            ./rocksdb_bench \
              --size $size \
              --duplicates $dup \
              --bloom $bloom \
              --compaction $comp \
              --format $format \
              --read_batch $batch \
              --path "$SSD_DB"
          done
        done
      done
    done
  done
done

cat results.csv >> "$OUT"
echo "✅ All done! Results saved to $OUT"
