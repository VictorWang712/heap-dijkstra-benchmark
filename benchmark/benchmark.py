import os
import random
import subprocess
import time
import json
import argparse
from datetime import datetime
from tqdm import tqdm

BIN_DIR = "../code/bin" # Directory for compiled binaries 
SRC_DIR = "../code/src" # Directory for source code files 
LOG_DIR = "./log/main" # Directory for log files 
N_QUERIES = 2000 # Default number of queries 

def get_graph_info(graph_file): # Extract the number of nodes and edges from the graph file 
    n = m = None # Initialize node and edge counts 
    with open(graph_file, 'r') as f: # Open the graph file for reading 
        for line in f: # Iterate through each line in the file 
            if line.startswith('p'): # Look for the line starting with 'p' 
                parts = line.strip().split() # Split the line into parts 
                if len(parts) >= 4: # Ensure there are enough parts 
                    n = int(parts[2]) # Parse the number of nodes 
                    m = int(parts[3]) # Parse the number of edges 
                break # Stop after finding the relevant line 
    if n is None or m is None: # Check if node or edge count was not found 
        raise Exception("Graph size not found in file: {}".format(graph_file)) # Raise an error if not found 
    return n, m # Return the node and edge counts 

def generate_queries(n, num_queries): # Generate a set of unique source-target queries 
    queries = set() # Initialize an empty set for queries 
    while len(queries) < num_queries: # Continue until enough queries are generated 
        src = random.randint(1, n) # Randomly select a source node 
        tgt = random.randint(1, n) # Randomly select a target node 
        if src != tgt: # Ensure source and target are different 
            queries.add((src, tgt)) # Add the query to the set 
    return list(queries) # Return the queries as a list 

def compile_code(): # Compile the Dijkstra algorithm implementations 
    os.makedirs(BIN_DIR, exist_ok=True) # Ensure the binary directory exists 
    fib_src = os.path.join(SRC_DIR, "dijkstra_fib.c") # Path to Fibonacci heap source 
    fib_bin = os.path.join(BIN_DIR, "dijkstra_fib") # Path to Fibonacci heap binary 
    ret = os.system(f"gcc -O2 -o {fib_bin} {fib_src}") # Compile the Fibonacci heap version 
    if ret != 0: # Check if compilation failed 
        raise Exception("Failed to compile dijkstra_fib.c") # Raise an error if failed 
    heap_src = os.path.join(SRC_DIR, "dijkstra_heap.c") # Path to binary heap source 
    heap_bin = os.path.join(BIN_DIR, "dijkstra_heap") # Path to binary heap binary 
    ret = os.system(f"gcc -O2 -o {heap_bin} {heap_src}") # Compile the binary heap version 
    if ret != 0: # Check if compilation failed 
        raise Exception("Failed to compile dijkstra_heap.c") # Raise an error if failed 
    return fib_bin, heap_bin # Return paths to the compiled binaries 

def run_algorithm(bin_path, graph_file, src, tgt): # Run the algorithm binary on a single query 
    try: 
        start = time.time() # Record the start time 
        proc = subprocess.run([bin_path, graph_file, str(src), str(tgt)], capture_output=True, timeout=300) # Run the binary 
        elapsed = time.time() - start # Calculate elapsed time 
        output = proc.stdout.decode().strip() # Decode the output 
        result = int(output) if output and output != "-1" else -1 # Parse the result 
        return result, elapsed # Return the result and elapsed time 
    except Exception as e: # Handle any exceptions 
        return -1, -1 # Return -1 for both result and time if error occurs 

def main():
    parser = argparse.ArgumentParser(description="Benchmark Dijkstra algorithms on a given graph.") # Argument parser for command-line options 
    parser.add_argument('--graph', '-g', type=str, required=True, help="Path to the input graph file") # Graph file path argument 
    parser.add_argument('--queries', '-q', type=int, default=N_QUERIES, help="Number of queries to run (default: 2000)") # Number of queries argument 
    args = parser.parse_args() # Parse the command-line arguments 

    graph_file = args.graph # Get the graph file path 
    num_queries = args.queries # Get the number of queries 

    if not os.path.isfile(graph_file): # Check if the graph file exists 
        print(f"Error: Graph file '{graph_file}' does not exist.") # Print error if not found 
        return 

    os.makedirs(LOG_DIR, exist_ok=True) # Ensure the log directory exists 
    print(f"Reading graph info from: {graph_file}") # Inform about reading graph info 
    n, m = get_graph_info(graph_file) # Get the number of nodes and edges 
    print(f"Graph nodes: {n}, edges: {m}") # Print graph statistics 
    print(f"Generating {num_queries} queries...") # Inform about query generation 
    queries = generate_queries(n, num_queries) # Generate the queries 
    print("Compiling code...") # Inform about compilation 
    fib_bin, heap_bin = compile_code() # Compile the code and get binary paths 
    print("Running benchmarks...") # Inform about starting benchmarks 

    fib_results = [] # Store results for Fibonacci heap 
    heap_results = [] # Store results for binary heap 
    fib_total_time = 0.0 # Total time for Fibonacci heap 
    heap_total_time = 0.0 # Total time for binary heap 

    for idx, (src, tgt) in enumerate(tqdm(queries, desc="Benchmark Progress", unit="query")): # Iterate through queries with progress bar 
        fib_res, fib_time = run_algorithm(fib_bin, graph_file, src, tgt) # Run Fibonacci heap algorithm 
        heap_res, heap_time = run_algorithm(heap_bin, graph_file, src, tgt) # Run binary heap algorithm 
        fib_results.append({"src": src, "tgt": tgt, "result": fib_res, "time": fib_time}) # Store Fibonacci heap result 
        heap_results.append({"src": src, "tgt": tgt, "result": heap_res, "time": heap_time}) # Store binary heap result 
        fib_total_time += fib_time # Accumulate Fibonacci heap time 
        heap_total_time += heap_time # Accumulate binary heap time 

    now = datetime.now().strftime("%Y%m%d_%H%M%S") # Get current timestamp for log file 
    log_file = os.path.join(LOG_DIR, f"benchmark_{now}.json") # Path for the log file 

    log_data = { # Prepare the log data dictionary 
        "graph_file": graph_file, # Store the graph file path 
        "graph_info": { # Store graph statistics 
            "nodes": n, # Number of nodes 
            "edges": m # Number of edges 
        },
        "fib_total_time": fib_total_time, # Total time for Fibonacci heap 
        "heap_total_time": heap_total_time, # Total time for binary heap 
        "queries": [] # List to store query results 
    }

    for i in range(num_queries): # Iterate through all queries 
        log_data["queries"].append({ # Append query results to log 
            "src": fib_results[i]["src"], # Source node 
            "tgt": fib_results[i]["tgt"], # Target node 
            "fib_result": fib_results[i]["result"], # Result from Fibonacci heap 
            "fib_time": fib_results[i]["time"], # Time for Fibonacci heap 
            "heap_result": heap_results[i]["result"], # Result from binary heap 
            "heap_time": heap_results[i]["time"] # Time for binary heap 
        })

    with open(log_file, "w") as f: # Open the log file for writing 
        json.dump(log_data, f, indent=2) # Write the log data as JSON 

    print(f"Benchmark finished. Log saved to {log_file}") # Inform about completion and log location 

if __name__ == "__main__":
    main()
