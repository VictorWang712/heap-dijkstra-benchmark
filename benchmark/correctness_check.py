import os
import random
import subprocess
import time
import json
import argparse
from datetime import datetime

BIN_DIR = "../code/bin"
SRC_DIR = "../code/src"
LOG_DIR = "./log/correctness"
TMP_SUBGRAPH = "./tmp_subgraph.gr"
FLOYD_MAX_NODES = 500  # 可根据内存调整
N_QUERIES = 100         # 子图上验证的点对数量

def get_graph_info(graph_file):
    n = m = None
    with open(graph_file, 'r') as f:
        for line in f:
            if line.startswith('p'):
                parts = line.strip().split()
                if len(parts) >= 4:
                    n = int(parts[2])
                    m = int(parts[3])
                break
    if n is None or m is None:
        raise Exception("Graph size not found in file: {}".format(graph_file))
    return n, m

def extract_subgraph(graph_file, subgraph_file, max_nodes):
    n, m = get_graph_info(graph_file)
    selected_nodes = set(range(1, min(n, max_nodes) + 1))
    edge_lines = []
    with open(graph_file, 'r') as f:
        for line in f:
            if line.startswith('a'):
                parts = line.strip().split()
                u, v = int(parts[1]), int(parts[2])
                if u in selected_nodes and v in selected_nodes:
                    edge_lines.append(line)
    with open(subgraph_file, 'w') as f:
        # 写头部
        f.write(f"p sp {len(selected_nodes)} {len(edge_lines)}\n")
        for line in edge_lines:
            f.write(line)
    return len(selected_nodes), len(edge_lines)

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
    # Compile floyd
    floyd_src = os.path.join(SRC_DIR, "floyd.c")
    floyd_bin = os.path.join(BIN_DIR, "floyd")
    if os.path.exists(floyd_src):
        ret = os.system(f"gcc -O2 -o {floyd_bin} {floyd_src}")
        if ret != 0:
            raise Exception("Failed to compile floyd.c")
    else:
        raise Exception("floyd.c not found in src directory!")
    return fib_bin, heap_bin, floyd_bin

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
    parser = argparse.ArgumentParser(description="Correctness check for Dijkstra and Floyd algorithms on subgraph.")
    parser.add_argument('--graph', '-g', type=str, required=True, help="Path to the input graph file")
    parser.add_argument('--nodes', '-n', type=int, default=FLOYD_MAX_NODES, help=f"Subgraph node count (default: {FLOYD_MAX_NODES})")
    parser.add_argument('--queries', '-q', type=int, default=N_QUERIES, help=f"Number of queries (default: {N_QUERIES})")
    args = parser.parse_args()

    graph_file = args.graph
    subgraph_nodes = args.nodes
    num_queries = args.queries

    if not os.path.isfile(graph_file):
        print(f"Error: Graph file '{graph_file}' does not exist.")
        return

    os.makedirs(LOG_DIR, exist_ok=True)

    print("Compiling C codes ...")
    fib_bin, heap_bin, floyd_bin = compile_code()

    print(f"Extracting subgraph with up to {subgraph_nodes} nodes from {graph_file} ...")
    n, m = extract_subgraph(graph_file, TMP_SUBGRAPH, subgraph_nodes)
    print(f"Subgraph nodes: {n}, edges: {m}")

    print(f"Generating {num_queries} random queries on subgraph...")
    queries = generate_queries(n, num_queries)

    results = []
    for idx, (src, tgt) in enumerate(queries):
        floyd_res, floyd_time = run_algorithm(floyd_bin, TMP_SUBGRAPH, src, tgt)
        fib_res, fib_time = run_algorithm(fib_bin, TMP_SUBGRAPH, src, tgt)
        heap_res, heap_time = run_algorithm(heap_bin, TMP_SUBGRAPH, src, tgt)
        results.append({
            "src": src,
            "tgt": tgt,
            "floyd_result": floyd_res,
            "floyd_time": floyd_time,
            "fib_result": fib_res,
            "fib_time": fib_time,
            "fib_correctness": (fib_res == floyd_res),
            "heap_result": heap_res,
            "heap_time": heap_time,
            "heap_correctness": (heap_res == floyd_res)
        })

    now = datetime.now().strftime("%Y%m%d_%H%M%S")
    log_file = os.path.join(LOG_DIR, f"correctness_{now}.json")
    log_data = {
        "original_graph_file": graph_file,
        "subgraph_file": TMP_SUBGRAPH,
        "subgraph_info": {
            "nodes": n,
            "edges": m
        },
        "queries": results
    }

    with open(log_file, "w") as f:
        json.dump(log_data, f, indent=2)
    print(f"Correctness check finished. Log saved to {log_file}")

    # 清理临时子图文件
    if os.path.exists(TMP_SUBGRAPH):
        os.remove(TMP_SUBGRAPH)

if __name__ == "__main__":
    main()
