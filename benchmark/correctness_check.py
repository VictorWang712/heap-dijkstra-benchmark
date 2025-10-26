import os
import random
import subprocess
import time
import json
import argparse
from datetime import datetime

BIN_DIR = "../code/bin"  # Directory for compiled binaries
SRC_DIR = "../code/src"  # Directory for source code files
LOG_DIR = "./log/correctness"  # Directory for correctness logs
TMP_SUBGRAPH = "./tmp_subgraph.gr"  # Temporary file for extracted subgraph
FLOYD_MAX_NODES = 500  # Maximum nodes for Floyd's algorithm
N_QUERIES = 100  # Number of random queries to generate

def get_graph_info(graph_file):  # Retrieve node and edge count from graph file
    n = m = None  # Initialize node and edge counts
    with open(graph_file, 'r') as f:  # Open the graph file for reading
        for line in f:  # Iterate through each line
            if line.startswith('p'):  # Look for the line starting with 'p'
                parts = line.strip().split()  # Split the line into parts
                if len(parts) >= 4:  # Ensure enough parts are present
                    n = int(parts[2])  # Extract number of nodes
                    m = int(parts[3])  # Extract number of edges
                break  # Stop after finding the graph info
    if n is None or m is None:  # If info not found, raise exception
        raise Exception("Graph size not found in file: {}".format(graph_file))
    return n, m  # Return node and edge counts

def extract_subgraph(graph_file, subgraph_file, max_nodes):  # Extract a subgraph with limited nodes
    n, m = get_graph_info(graph_file)  # Get original graph size
    selected_nodes = set(range(1, min(n, max_nodes) + 1))  # Select a subset of nodes
    edge_lines = []  # Store valid edge lines
    with open(graph_file, 'r') as f:  # Open the graph file
        for line in f:  # Iterate through lines
            if line.startswith('a'):  # Look for edge lines
                parts = line.strip().split()  # Split the line
                u, v = int(parts[1]), int(parts[2])  # Extract node indices
                if u in selected_nodes and v in selected_nodes:  # Check if both nodes are selected
                    edge_lines.append(line)  # Add edge to subgraph
    with open(subgraph_file, 'w') as f:  # Write the subgraph to file
        f.write(f"p sp {len(selected_nodes)} {len(edge_lines)}\n")  # Write header line
        for line in edge_lines:  # Write each edge line
            f.write(line)  # Write edge to file
    return len(selected_nodes), len(edge_lines)  # Return subgraph size

def generate_queries(n, num_queries):  # Generate random source-target pairs
    queries = set()  # Use a set to avoid duplicates
    while len(queries) < num_queries:  # Continue until enough queries
        src = random.randint(1, n)  # Randomly select source node
        tgt = random.randint(1, n)  # Randomly select target node
        if src != tgt:  # Ensure source and target are different
            queries.add((src, tgt))  # Add query pair
    return list(queries)  # Convert set to list

def compile_code():  # Compile all required C programs
    os.makedirs(BIN_DIR, exist_ok=True)  # Ensure binary directory exists
    fib_src = os.path.join(SRC_DIR, "dijkstra_fib.c")  # Path to dijkstra_fib source
    fib_bin = os.path.join(BIN_DIR, "dijkstra_fib")  # Output binary path
    ret = os.system(f"gcc -O2 -o {fib_bin} {fib_src}")  # Compile dijkstra_fib
    if ret != 0:  # Check for compilation failure
        raise Exception("Failed to compile dijkstra_fib.c")  # Raise error if failed
    heap_src = os.path.join(SRC_DIR, "dijkstra_heap.c")  # Path to dijkstra_heap source
    heap_bin = os.path.join(BIN_DIR, "dijkstra_heap")  # Output binary path
    ret = os.system(f"gcc -O2 -o {heap_bin} {heap_src}")  # Compile dijkstra_heap
    if ret != 0:  # Check for compilation failure
        raise Exception("Failed to compile dijkstra_heap.c")  # Raise error if failed
    floyd_src = os.path.join(SRC_DIR, "floyd.c")  # Path to floyd source
    floyd_bin = os.path.join(BIN_DIR, "floyd")  # Output binary path
    if os.path.exists(floyd_src):  # Check if floyd source exists
        ret = os.system(f"gcc -O2 -o {floyd_bin} {floyd_src}")  # Compile floyd
        if ret != 0:  # Check for compilation failure
            raise Exception("Failed to compile floyd.c")  # Raise error if failed
    else:
        raise Exception("floyd.c not found in src directory!")  # Raise error if not found
    return fib_bin, heap_bin, floyd_bin  # Return paths to binaries

def run_algorithm(bin_path, graph_file, src, tgt):  # Run a binary on a query
    try:
        start = time.time()  # Record start time
        proc = subprocess.run([bin_path, graph_file, str(src), str(tgt)], capture_output=True, timeout=300)  # Run process
        elapsed = time.time() - start  # Calculate elapsed time
        output = proc.stdout.decode().strip()  # Decode output
        result = int(output) if output and output != "-1" else -1  # Parse result
        return result, elapsed  # Return result and time
    except Exception as e:  # Handle exceptions
        return -1, -1  # Return error values

def main():
    parser = argparse.ArgumentParser(description="Correctness check for Dijkstra and Floyd algorithms on subgraph.")  # Set up argument parser
    parser.add_argument('--graph', '-g', type=str, required=True, help="Path to the input graph file")  # Add graph argument
    parser.add_argument('--nodes', '-n', type=int, default=FLOYD_MAX_NODES, help=f"Subgraph node count (default: {FLOYD_MAX_NODES})")  # Add nodes argument
    parser.add_argument('--queries', '-q', type=int, default=N_QUERIES, help=f"Number of queries (default: {N_QUERIES})")  # Add queries argument
    args = parser.parse_args()  # Parse command-line arguments

    graph_file = args.graph  # Get graph file path
    subgraph_nodes = args.nodes  # Get number of nodes for subgraph
    num_queries = args.queries  # Get number of queries

    if not os.path.isfile(graph_file):  # Check if graph file exists
        print(f"Error: Graph file '{graph_file}' does not exist.")  # Print error if not found
        return  # Exit if file missing

    os.makedirs(LOG_DIR, exist_ok=True)  # Ensure log directory exists

    print("Compiling C codes ...")  # Notify user of compilation
    fib_bin, heap_bin, floyd_bin = compile_code()  # Compile codes

    print(f"Extracting subgraph with up to {subgraph_nodes} nodes from {graph_file} ...")  # Notify user of subgraph extraction
    n, m = extract_subgraph(graph_file, TMP_SUBGRAPH, subgraph_nodes)  # Extract subgraph
    print(f"Subgraph nodes: {n}, edges: {m}")  # Print subgraph info

    print(f"Generating {num_queries} random queries on subgraph...")  # Notify user of query generation
    queries = generate_queries(n, num_queries)  # Generate queries

    results = []  # Store results for each query
    for idx, (src, tgt) in enumerate(queries):  # Iterate through queries
        floyd_res, floyd_time = run_algorithm(floyd_bin, TMP_SUBGRAPH, src, tgt)  # Run Floyd's algorithm
        fib_res, fib_time = run_algorithm(fib_bin, TMP_SUBGRAPH, src, tgt)  # Run Dijkstra with Fibonacci heap
        heap_res, heap_time = run_algorithm(heap_bin, TMP_SUBGRAPH, src, tgt)  # Run Dijkstra with binary heap
        results.append({  # Store results and correctness checks
            "src": src,  # Source node
            "tgt": tgt,  # Target node
            "floyd_result": floyd_res,  # Floyd's result
            "floyd_time": floyd_time,  # Floyd's runtime
            "fib_result": fib_res,  # Dijkstra (fib) result
            "fib_time": fib_time,  # Dijkstra (fib) runtime
            "fib_correctness": (fib_res == floyd_res),  # Correctness check
            "heap_result": heap_res,  # Dijkstra (heap) result
            "heap_time": heap_time,  # Dijkstra (heap) runtime
            "heap_correctness": (heap_res == floyd_res)  # Correctness check
        })

    now = datetime.now().strftime("%Y%m%d_%H%M%S")  # Get current timestamp
    log_file = os.path.join(LOG_DIR, f"correctness_{now}.json")  # Construct log file path
    log_data = {  # Prepare log data
        "original_graph_file": graph_file,  # Original graph file path
        "subgraph_file": TMP_SUBGRAPH,  # Subgraph file path
        "subgraph_info": {  # Subgraph statistics
            "nodes": n,  # Number of nodes
            "edges": m  # Number of edges
        },
        "queries": results  # Query results
    }

    with open(log_file, "w") as f:  # Open log file for writing
        json.dump(log_data, f, indent=2)  # Write log data as JSON
    print(f"Correctness check finished. Log saved to {log_file}")  # Notify user of completion

    if os.path.exists(TMP_SUBGRAPH):  # Check if temporary subgraph exists
        os.remove(TMP_SUBGRAPH)  # Remove temporary subgraph file

if __name__ == "__main__":
    main()
