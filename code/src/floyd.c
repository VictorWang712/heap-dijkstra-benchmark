#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

typedef struct { // Define a structure for the graph 
    int n; // Number of nodes in the graph 
    int **adj; // Adjacency matrix for edge weights 
} Graph;

Graph* read_graph(const char *filename) { // Function to read a graph from file 
    FILE *fp = fopen(filename, "r"); // Open the file for reading 
    if (!fp) { // If file cannot be opened 
        fprintf(stderr, "Cannot open file %s\n", filename); // Print error message 
        exit(1); // Exit the program 
    }
    int n = 0, m = 0; // Initialize number of nodes and edges 
    char buf[256]; // Buffer for reading lines 
    while (fgets(buf, sizeof(buf), fp)) { // Read each line from file 
        if (buf[0] == 'p') { // If line starts with 'p', it's the problem line 
            sscanf(buf, "p sp %d %d", &n, &m); // Parse number of nodes and edges 
            break; // Exit loop after reading problem line 
        }
    }
    Graph *g = (Graph*)malloc(sizeof(Graph)); // Allocate memory for graph 
    g->n = n + 1; // Set number of nodes (+1 for 1-based indexing) 
    g->adj = (int**)malloc(g->n * sizeof(int*)); // Allocate adjacency matrix rows 
    for (int i = 0; i < g->n; ++i) { // For each row in adjacency matrix 
        g->adj[i] = (int*)malloc(g->n * sizeof(int)); // Allocate columns 
        for (int j = 0; j < g->n; ++j) // For each cell in row 
            g->adj[i][j] = (i == j ? 0 : INT_MAX / 2); // Set 0 for self-loop, large value for others 
    }
    rewind(fp); // Reset file pointer to beginning 
    while (fgets(buf, sizeof(buf), fp)) { // Read each line again 
        if (buf[0] == 'a') { // If line starts with 'a', it's an arc (edge) 
            int u, v, w; // Variables for edge endpoints and weight 
            sscanf(buf, "a %d %d %d", &u, &v, &w); // Parse edge data 
            if (g->adj[u][v] > w) // If current edge is heavier than new one 
                g->adj[u][v] = w; // Update with smaller weight 
        }
    }
    fclose(fp); // Close the file 
    return g; // Return pointer to the graph 
}

void free_graph(Graph *g) { // Function to free memory used by graph 
    for (int i = 0; i < g->n; ++i) // For each row in adjacency matrix 
        free(g->adj[i]); // Free the row 
    free(g->adj); // Free the row pointers 
    free(g); // Free the graph structure 
}

int main(int argc, char *argv[]) {
    if (argc != 4) { // Check for correct number of arguments 
        printf("Usage: %s <graph_file> <src> <tgt>\n", argv[0]); // Print usage 
        return 1;
    }
    const char *graph_file = argv[1]; // Get graph file name from arguments 
    int src = atoi(argv[2]); // Parse source node 
    int tgt = atoi(argv[3]); // Parse target node 
    Graph *g = read_graph(graph_file); // Read the graph from file 

    int n = g->n; // Number of nodes in the graph 
    for (int k = 1; k < n; ++k) // Iterate over all intermediate nodes 
        for (int i = 1; i < n; ++i) // Iterate over all source nodes 
            for (int j = 1; j < n; ++j) // Iterate over all destination nodes 
                if (g->adj[i][j] > g->adj[i][k] + g->adj[k][j]) // If a shorter path is found 
                    g->adj[i][j] = g->adj[i][k] + g->adj[k][j]; // Update shortest path 

    int result = g->adj[src][tgt]; // Get the shortest path from src to tgt 
    if (result >= INT_MAX / 4) // If the path is unreachable 
        printf("-1\n"); // Output -1 
    else
        printf("%d\n", result); // Output the shortest path length 

    free_graph(g); // Free memory used by the graph 
    return 0;
}
