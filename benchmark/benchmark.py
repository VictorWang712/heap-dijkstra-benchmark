import os
import random
import subprocess
import time
import json
from datetime import datetime
from tqdm import tqdm

GRAPH_FILE = "../data/USA-road-t.USA.gr"
BIN_DIR = "../code/bin"
SRC_DIR = "../code/src"
LOG_DIR = "./log"
N_QUERIES = 1000

def get_graph_size(graph_file):
    with open(graph_file, 'r') as f:
        for line in f:
            if line.startswith('p'):
                parts = line.strip().split()
                return int(parts[2])
    raise Exception("Graph size not found.")

def generate_queries(n, num_queries):
    queries = set()
    while len(queries) < num_queries:
        src = random.randint(1, n)
        tgt = random.randint(1, n)
        if src != tgt:
            queries.add((src, tgt))
    return list(queries)

def compile_code():
    os.makedirs(BIN_DIR, exist_ok=True)
    # Compile dijkstra_fib
    fib_src = os.path.join(SRC_DIR, "dijkstra_fib.c")
    fib_bin = os.path.join(BIN_DIR, "dijkstra_fib")
    ret = os.system(f"gcc -O2 -o {fib_bin} {fib_src}")
    if ret != 0:
        raise Exception("Failed to compile dijkstra_fib.c")
    # Compile dijkstra_heap
    heap_src = os.path.join(SRC_DIR, "dijkstra_heap.c")
    heap_bin = os.path.join(BIN_DIR, "dijkstra_heap")
    ret = os.system(f"gcc -O2 -o {heap_bin} {heap_src}")
    if ret != 0:
        raise Exception("Failed to compile dijkstra_heap.c")
    return fib_bin, heap_bin

def run_algorithm(bin_path, graph_file, src, tgt):
    try:
        start = time.time()
        proc = subprocess.run([bin_path, graph_file, str(src), str(tgt)], capture_output=True, timeout=300)
        elapsed = time.time() - start
        output = proc.stdout.decode().strip()
        result = int(output) if output and output != "-1" else -1
        return result, elapsed
    except Exception as e:
        return -1, -1

def main():
    os.makedirs(LOG_DIR, exist_ok=True)
    n = get_graph_size(GRAPH_FILE)
    print(f"Graph size: {n}")
    print("Generating queries...")
    queries = generate_queries(n, N_QUERIES)
    print("Compiling code...")
    fib_bin, heap_bin = compile_code()
    print("Running benchmarks...")
    fib_results = []
    heap_results = []
    fib_total_time = 0.0
    heap_total_time = 0.0
    for idx, (src, tgt) in enumerate(tqdm(queries, desc="Benchmark Progress", unit="query")):
        fib_res, fib_time = run_algorithm(fib_bin, GRAPH_FILE, src, tgt)
        heap_res, heap_time = run_algorithm(heap_bin, GRAPH_FILE, src, tgt)
        fib_results.append({"src": src, "tgt": tgt, "result": fib_res, "time": fib_time})
        heap_results.append({"src": src, "tgt": tgt, "result": heap_res, "time": heap_time})
        fib_total_time += fib_time
        heap_total_time += heap_time
    now = datetime.now().strftime("%Y%m%d_%H%M%S")
    log_file = os.path.join(LOG_DIR, f"benchmark_{now}.json")
    log_data = {
        "queries": queries,
        "fib_results": fib_results,
        "heap_results": heap_results,
        "fib_total_time": fib_total_time,
        "heap_total_time": heap_total_time
    }
    with open(log_file, "w") as f:
        json.dump(log_data, f, indent=2)
    print(f"Benchmark finished. Log saved to {log_file}")

if __name__ == "__main__":
    main()
