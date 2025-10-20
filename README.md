# heap-dijkstra-benchmark

## Algorithms

- `dijkstra_fib.c`: Dijkstra's algorithm using a Fibonacci Heap.
- `dijkstra_heap.c`: Dijkstra's algorithm using a Binary Heap (Min-Heap).

## How to Use

### Prepare the Data

Unzip the USA road network file (`USA-road-t.USA.gr`) in the `data` directory.

```bash
cd data
cat USA-road-t.USA.gr.gz.part_* | gunzip -c > USA-road-t.USA.gr
```

### Compile the Algorithms

The benchmark script will automatically compile both C codes and place the executables in `code/bin/`.

Alternatively, you can compile manually:

```bash
gcc code/bin/dijkstra_fib -o code/src/dijkstra_fib.c -O2
gcc code/bin/dijkstra_heap -o code/src/dijkstra_heap.c -O2
```

### Run the Benchmark

```bash
cd benchmark
python benchmark.py
```

This will:

- Compile both algorithms.
- Generate 1000 random queries.
- Run both algorithms for each query.
- Record the results and runtime in a log file under `benchmark/log/`.

### Log File

The log file is in JSON format and contains:

- The list of queries.
- The output and runtime for each algorithm and query.
- Total runtime for both algorithms.
