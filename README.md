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
python benchmark.py --graph <graph_path> --queries <number_of_queries>
```

Parameters:

- `--graph_path`: Path to the graph to be used
- `--queries`: The number of times random queries are executed (default: 2000)

This will:

- Compile both algorithms.
- Generate 1000 random queries.
- Run both algorithms for each query.
- Record the results and runtime in a log file under `benchmark/log/main`.

Example:

```bash
cd benchmark
python benchmark.py --graph ../data/USA-road-t.USA.gr
```

The log file is in JSON format and contains:

- The list of queries.
- The output and runtime for each algorithm and query.
- Total runtime for both algorithms.

### Correctness Verification

To ensure the correctness of the shortest path results, a dedicated script is provided to perform cross-validation between the two Dijkstra implementations and the Floyd-Warshall algorithm on a small subgraph.

```bash
cd benchmark
python correctness_check.py --graph <graph_path> --nodes <subgraph_node_count> --queries <number_of_queries>
```

- `--graph`: Path to the original (large) graph file.
- `--nodes`: Number of nodes to extract for the subgraph (default: 500).
- `--queries`: Number of random queries to validate (default: 100).

The script will:

- Automatically compile `dijkstra_fib.c`, `dijkstra_heap.c`, and `floyd.c` from `code/src/`.
- Extract a small subgraph from the original graph (default 500 nodes).
- Run all three algorithms on the same set of randomly selected queries.
- Compare the results for each query and report any discrepancies.

A JSON log file will be saved under `benchmark/log/correctness/`, containing:

- The subgraph information.
- The list of queries and results from all three algorithms.
- Cross-check results for each query.
- This allows you to quickly verify the correctness of your implementations on manageable subgraphs.
