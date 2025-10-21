# heap-dijkstra-benchmark

A benchmarking suite for comparing different Dijkstra algorithm implementations on large-scale road network graphs.

## Algorithms

- `dijkstra_fib.c`: Dijkstra's algorithm using a Fibonacci Heap.
- `dijkstra_heap.c`: Dijkstra's algorithm using a Binary Heap (Min-Heap).

## How to Use

### Prepare the Data

Download the USA road network data from [9th DIMACS Implementation Challenge: Shortest Paths](http://www.diag.uniroma1.it/challenge9/download.shtml) in the `data` directory:

```bash
cd data
wget <graph_data_link>
gunzip <file_archive_path>
```

Example:

```bash
cd data
wget http://www.diag.uniroma1.it/challenge9/data/USA-road-t/USA-road-t.USA.gr.gz
gunzip USA-road-t.USA.gr.gz
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
python benchmark.py --graph <graph_path>
```

Parameters:

- `--graph_path`: Path to the graph to be used

This will:

- Compile both algorithms.
- Generate 1000 random queries.
- Run both algorithms for each query.
- Record the results and runtime in a log file under `benchmark/log/`.

Example:

```bash
cd benchmark
python benchmark.py --graph ../data/USA-road-t.USA.gr
```

### Log File

The log file is in JSON format and contains:

- The list of queries.
- The output and runtime for each algorithm and query.
- Total runtime for both algorithms.
