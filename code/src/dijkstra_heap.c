#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

typedef struct Edge { // Structure for an edge in the graph 
    int to; // Target vertex of the edge 
    int weight; // Weight of the edge 
    struct Edge *next; // Pointer to next edge in adjacency list 
} Edge;

typedef struct { // Structure for the graph 
    int n; // Number of vertices (1-based indexing) 
    Edge **adj; // Array of adjacency lists 
} Graph;

typedef struct { // Structure for the binary min-heap 
    int *heap; // Array to store heap elements (vertex indices) 
    int *pos; // Position of each vertex in the heap 
    int *dist; // Distance values for each vertex 
    int size; // Current size of the heap 
    int capacity; // Maximum capacity of the heap 
} MinHeap;

Graph* read_graph(const char *filename); // Read a graph from file 
void free_graph(Graph *g); // Free the graph structure 
MinHeap* create_min_heap(int capacity); // Create a new min-heap 
void min_heap_push(MinHeap *h, int v, int d); // Insert a vertex with distance into heap 
int min_heap_pop(MinHeap *h); // Remove and return the vertex with minimum distance 
void min_heap_decrease_key(MinHeap *h, int v, int d); // Decrease the key (distance) of a vertex 
void free_min_heap(MinHeap *h); // Free the min-heap structure 
int dijkstra(Graph *g, int src, int tgt); // Dijkstra's algorithm using binary heap 

Graph* read_graph(const char *filename) {
    FILE *fp = fopen(filename, "r"); // Open file for reading 
    if (!fp) {
        fprintf(stderr, "Cannot open file %s\n", filename); // Print error if file can't be opened 
        exit(1); // Exit program 
    }
    int n = 0, m = 0; // Number of vertices and edges 
    char buf[256]; // Buffer for reading lines 
    while (fgets(buf, sizeof(buf), fp)) { // Read file line by line 
        if (buf[0] == 'p') { // Look for problem line 
            sscanf(buf, "p sp %d %d", &n, &m); // Parse number of vertices and edges 
            break; // Stop after finding 
        }
    }
    Graph *g = (Graph*)malloc(sizeof(Graph)); // Allocate memory for graph 
    g->n = n + 1; // 1-based indexing 
    g->adj = (Edge**)calloc(g->n, sizeof(Edge*)); // Allocate adjacency lists 
    rewind(fp); // Reset file pointer to start 
    while (fgets(buf, sizeof(buf), fp)) { // Read file again 
        if (buf[0] == 'a') { // Look for arc lines 
            int u, v, w; // Edge data 
            sscanf(buf, "a %d %d %d", &u, &v, &w); // Parse edge 
            Edge *e = (Edge*)malloc(sizeof(Edge)); // Allocate memory for edge 
            e->to = v; // Set target vertex 
            e->weight = w; // Set edge weight 
            e->next = g->adj[u]; // Insert at head of adjacency list 
            g->adj[u] = e; // Update adjacency list 
        }
    }
    fclose(fp); // Close file 
    return g; // Return constructed graph 
}

void free_graph(Graph *g) {
    for (int i = 0; i < g->n; ++i) { // For each vertex 
        Edge *e = g->adj[i]; // Get adjacency list 
        while (e) { // Traverse list 
            Edge *tmp = e; // Store pointer to current edge 
            e = e->next; // Move to next edge 
            free(tmp); // Free current edge 
        }
    }
    free(g->adj); // Free adjacency list array 
    free(g); // Free graph structure 
}

MinHeap* create_min_heap(int capacity) {
    MinHeap *h = (MinHeap*)malloc(sizeof(MinHeap)); // Allocate memory for heap 
    h->heap = (int*)malloc(capacity * sizeof(int)); // Allocate array for heap elements 
    h->pos = (int*)malloc(capacity * sizeof(int)); // Allocate array for positions 
    h->dist = (int*)malloc(capacity * sizeof(int)); // Allocate array for distances 
    h->size = 0; // Initialize heap size 
    h->capacity = capacity; // Set heap capacity 
    for (int i = 0; i < capacity; ++i) { // Initialize arrays 
        h->pos[i] = -1; // -1 means not in heap 
        h->dist[i] = INT_MAX; // Set initial distances to infinity 
    }
    return h; // Return created heap 
}

void swap(MinHeap *h, int i, int j) {
    int t = h->heap[i]; // Store value at i 
    h->heap[i] = h->heap[j]; // Swap values 
    h->heap[j] = t; // Complete swap 
    h->pos[h->heap[i]] = i; // Update position for swapped element 
    h->pos[h->heap[j]] = j; // Update position for swapped element 
}

void min_heapify_up(MinHeap *h, int idx) {
    while (idx > 0) { // While not at root 
        int p = (idx - 1) / 2; // Parent index 
        if (h->dist[h->heap[p]] > h->dist[h->heap[idx]]) { // If parent is greater than child 
            swap(h, p, idx); // Swap parent and child 
            idx = p; // Move up the tree 
        } else break; // Heap property satisfied 
    }
}

void min_heapify_down(MinHeap *h, int idx) {
    int l, r, min_idx; // Left, right, and minimum index 
    while (1) {
        l = 2 * idx + 1; // Left child index 
        r = 2 * idx + 2; // Right child index 
        min_idx = idx; // Assume current is minimum 
        if (l < h->size && h->dist[h->heap[l]] < h->dist[h->heap[min_idx]]) // If left child is smaller 
            min_idx = l; // Update minimum index 
        if (r < h->size && h->dist[h->heap[r]] < h->dist[h->heap[min_idx]]) // If right child is smaller 
            min_idx = r; // Update minimum index 
        if (min_idx != idx) { // If minimum is not current 
            swap(h, idx, min_idx); // Swap with child 
            idx = min_idx; // Move down the tree 
        } else break; // Heap property satisfied 
    }
}

void min_heap_push(MinHeap *h, int v, int d) {
    h->heap[h->size] = v; // Insert vertex at end 
    h->dist[v] = d; // Set distance value 
    h->pos[v] = h->size; // Set position in heap 
    min_heapify_up(h, h->size); // Restore heap property 
    h->size++; // Increment heap size 
}

int min_heap_pop(MinHeap *h) {
    if (h->size == 0) return -1; // Return -1 if heap is empty 
    int v = h->heap[0]; // Get vertex with minimum distance 
    h->heap[0] = h->heap[h->size - 1]; // Move last element to root 
    h->pos[h->heap[0]] = 0; // Update position 
    h->size--; // Decrement heap size 
    min_heapify_down(h, 0); // Restore heap property 
    h->pos[v] = -1; // Mark as not in heap 
    return v; // Return vertex index 
}

void min_heap_decrease_key(MinHeap *h, int v, int d) {
    h->dist[v] = d; // Update distance value 
    min_heapify_up(h, h->pos[v]); // Restore heap property upwards 
}

void free_min_heap(MinHeap *h) {
    free(h->heap); // Free heap array 
    free(h->pos); // Free position array 
    free(h->dist); // Free distance array 
    free(h); // Free heap structure 
}

int dijkstra(Graph *g, int src, int tgt) {
    int n = g->n; // Number of vertices 
    int *dist = (int*)malloc(n * sizeof(int)); // Distance array 
    int *visited = (int*)calloc(n, sizeof(int)); // Visited array 
    for (int i = 0; i < n; ++i) dist[i] = INT_MAX; // Initialize distances 
    MinHeap *h = create_min_heap(n); // Create min-heap 
    dist[src] = 0; // Distance to source is 0 
    min_heap_push(h, src, 0); // Insert source into heap 
    while (h->size > 0) { // While heap is not empty 
        int u = min_heap_pop(h); // Extract vertex with minimum distance 
        if (visited[u]) continue; // Skip if already visited 
        visited[u] = 1; // Mark as visited 
        if (u == tgt) { // If target reached 
            int ans = dist[u]; // Store answer 
            free(dist); // Free distance array 
            free(visited); // Free visited array 
            free_min_heap(h); // Free heap 
            return ans; // Return shortest distance 
        }
        for (Edge *e = g->adj[u]; e; e = e->next) { // For each neighbor 
            int v = e->to; // Get neighbor vertex 
            if (!visited[v] && dist[u] + e->weight < dist[v]) { // If shorter path found 
                dist[v] = dist[u] + e->weight; // Update distance 
                if (h->pos[v] == -1) { // If not in heap 
                    min_heap_push(h, v, dist[v]); // Insert into heap 
                } else {
                    min_heap_decrease_key(h, v, dist[v]); // Decrease key 
                }
            }
        }
    }
    free(dist); // Free distance array 
    free(visited); // Free visited array 
    free_min_heap(h); // Free heap 
    return -1; // Return -1 if target not reachable 
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <graph_file> <src> <tgt>\n", argv[0]);
        return 1;
    }
    const char *graph_file = argv[1];
    int src = atoi(argv[2]);
    int tgt = atoi(argv[3]);
    Graph *g = read_graph(graph_file);
    int result = dijkstra(g, src, tgt);
    printf("%d\n", result);
    free_graph(g);
    return 0;
}
