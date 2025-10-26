#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <time.h>

typedef struct FibNode { // Structure for a node in Fibonacci Heap 
    int vertex; // Index of the vertex this node represents 
    int degree; // Number of children 
    int mark; // Mark for cascading cut 
    int dist; // Distance value for Dijkstra 
    struct FibNode *parent, *child, *left, *right; // Pointers for heap structure 
} FibNode;

typedef struct { // Structure for the Fibonacci Heap itself 
    FibNode *min; // Pointer to the minimum node in the heap 
    int n; // Number of nodes in the heap 
} FibHeap;

typedef struct Edge { // Structure for an edge in the graph 
    int to; // Target vertex of the edge 
    int weight; // Weight of the edge 
    struct Edge *next; // Pointer to the next edge in adjacency list 
} Edge;

typedef struct { // Structure for the graph 
    int n; // Number of vertices (1-based indexing) 
    Edge **adj; // Array of adjacency lists 
} Graph;

FibHeap* create_fib_heap(); // Create a new Fibonacci Heap 
FibNode* fib_insert(FibHeap *H, int vertex, int dist); // Insert a node into the heap 
FibNode* fib_extract_min(FibHeap *H); // Extract the node with minimum distance 
void fib_decrease_key(FibHeap *H, FibNode *x, int k); // Decrease the key of a node 
void fib_consolidate(FibHeap *H); // Consolidate the heap after extraction 
void fib_link(FibHeap *H, FibNode *y, FibNode *x); // Link two trees in the heap 
void cut(FibHeap *H, FibNode *x, FibNode *y); // Cut a node from its parent 
void cascading_cut(FibHeap *H, FibNode *y); // Perform cascading cut operation 
void free_fib_heap(FibHeap *H); // Free the heap structure 
Graph* read_graph(const char *filename); // Read a graph from file 
void free_graph(Graph *g); // Free the graph structure 
int dijkstra(Graph *g, int src, int tgt); // Dijkstra's algorithm using Fibonacci Heap 

#define MAX_NODES 3000000 // Maximum number of nodes supported 
FibNode *node_ptrs[MAX_NODES]; // Array to store pointers for decrease_key operation 

FibHeap* create_fib_heap() {
    FibHeap *H = (FibHeap*)malloc(sizeof(FibHeap)); // Allocate memory for heap 
    H->min = NULL; // Initialize min pointer 
    H->n = 0; // Initialize node count 
    return H; // Return the created heap 
}

FibNode* fib_insert(FibHeap *H, int vertex, int dist) {
    FibNode *x = (FibNode*)malloc(sizeof(FibNode)); // Allocate memory for new node 
    x->vertex = vertex; // Set vertex index 
    x->dist = dist; // Set distance value 
    x->degree = 0; // No children initially 
    x->mark = 0; // Not marked 
    x->parent = x->child = NULL; // No parent or child 
    x->left = x->right = x; // Circular doubly linked list 
    if (H->min == NULL) { // If heap is empty 
        H->min = x; // Set as min node 
    } else {
        x->right = H->min->right; // Insert x into root list 
        x->left = H->min; // Set left pointer 
        H->min->right->left = x; // Update neighbor's left pointer 
        H->min->right = x; // Update min's right pointer 
        if (x->dist < H->min->dist) // If new node has smaller key 
            H->min = x; // Update min pointer 
    }
    H->n++; // Increment node count 
    node_ptrs[vertex] = x; // Store pointer for decrease_key 
    return x; // Return inserted node 
}

void fib_link(FibHeap *H, FibNode *y, FibNode *x) {
    y->left->right = y->right; // Remove y from root list 
    y->right->left = y->left; // Update pointers 
    y->parent = x; // Set parent pointer 
    if (x->child == NULL) { // If x has no child 
        x->child = y; // Set y as child 
        y->left = y->right = y; // Single child circular list 
    } else {
        y->right = x->child->right; // Insert y into child list 
        y->left = x->child; // Set left pointer 
        x->child->right->left = y; // Update neighbor's left pointer 
        x->child->right = y; // Update child's right pointer 
    }
    x->degree++; // Increment degree 
    y->mark = 0; // Reset mark 
}

void fib_consolidate(FibHeap *H) {
    int D = 32; // Maximum degree (log2(MAX_NODES)) 
    FibNode *A[32] = {NULL}; // Array for degree roots 
    FibNode *w = H->min; // Start from min node 
    if (!w) return; // If heap is empty, return 
    int cnt = 0; // Counter for root list nodes 
    FibNode *x = w; // Iterator for root list 
    do {
        cnt++; // Count nodes in root list 
        x = x->right; // Move to next node 
    } while (x != w); // Loop until back to start 

    FibNode *nodes[cnt]; // Temporary array to store root nodes 
    x = w; // Reset iterator 
    for (int i = 0; i < cnt; ++i) {
        nodes[i] = x; // Store node pointer 
        x = x->right; // Move to next node 
    }
    for (int i = 0; i < cnt; ++i) {
        x = nodes[i]; // Get node 
        int d = x->degree; // Get degree 
        while (A[d]) { // If another tree with same degree exists 
            FibNode *y = A[d]; // Get the other tree 
            if (x->dist > y->dist) { // Compare keys 
                FibNode *tmp = x; x = y; y = tmp; // Swap if needed 
            }
            fib_link(H, y, x); // Link y under x 
            A[d] = NULL; // Clear the slot 
            d++; // Increase degree 
        }
        A[d] = x; // Store tree in array 
    }
    H->min = NULL; // Reset min pointer 
    for (int i = 0; i < D; ++i) {
        if (A[i]) { // For each non-empty slot 
            if (!H->min) { // If min is not set 
                H->min = A[i]; // Set min 
                H->min->left = H->min->right = H->min; // Single node list 
            } else {
                A[i]->right = H->min->right; // Insert into root list 
                A[i]->left = H->min; // Set left pointer 
                H->min->right->left = A[i]; // Update neighbor's left pointer 
                H->min->right = A[i]; // Update min's right pointer 
                if (A[i]->dist < H->min->dist) // If smaller key 
                    H->min = A[i]; // Update min 
            }
        }
    }
}

FibNode* fib_extract_min(FibHeap *H) {
    FibNode *z = H->min; // Get min node 
    if (z) {
        if (z->child) { // If min has children 
            FibNode *x = z->child; // Start from child 
            do {
                FibNode *next = x->right; // Store next child 
                x->left = H->min; // Add x to root list 
                x->right = H->min->right; // Set right pointer 
                H->min->right->left = x; // Update neighbor's left pointer 
                H->min->right = x; // Update min's right pointer 
                x->parent = NULL; // Remove parent pointer 
                x = next; // Move to next child 
            } while (x != z->child); // Loop through all children 
        }
        z->left->right = z->right; // Remove z from root list 
        z->right->left = z->left; // Update pointers 
        if (z == z->right) { // If z was the only node 
            H->min = NULL; // Heap is now empty 
        } else {
            H->min = z->right; // Set new min 
            fib_consolidate(H); // Consolidate heap 
        }
        H->n--; // Decrement node count 
    }
    return z; // Return extracted node 
}

void cut(FibHeap *H, FibNode *x, FibNode *y) {
    if (y->child == x) { // If x is y's child pointer 
        if (x->right != x) // If more than one child 
            y->child = x->right; // Update child pointer 
        else
            y->child = NULL; // No more children 
    }
    x->left->right = x->right; // Remove x from child list 
    x->right->left = x->left; // Update pointers 
    y->degree--; // Decrement degree 
    x->right = H->min->right; // Add x to root list 
    x->left = H->min; // Set left pointer 
    H->min->right->left = x; // Update neighbor's left pointer 
    H->min->right = x; // Update min's right pointer 
    x->parent = NULL; // Remove parent pointer 
    x->mark = 0; // Reset mark 
}

void cascading_cut(FibHeap *H, FibNode *y) {
    FibNode *z = y->parent; // Get parent 
    if (z) {
        if (!y->mark) // If y is not marked 
            y->mark = 1; // Mark it 
        else {
            cut(H, y, z); // Cut y from parent 
            cascading_cut(H, z); // Recursively cut parent 
        }
    }
}

void fib_decrease_key(FibHeap *H, FibNode *x, int k) {
    if (k > x->dist) return; // New key must be smaller 
    x->dist = k; // Update key 
    FibNode *y = x->parent; // Get parent 
    if (y && x->dist < y->dist) { // If heap property violated 
        cut(H, x, y); // Cut x from parent 
        cascading_cut(H, y); // Perform cascading cut 
    }
    if (x->dist < H->min->dist) // If new key is new min 
        H->min = x; // Update min pointer 
}

void free_fib_heap(FibHeap *H) {
    free(H); // Only free heap structure (nodes not freed) 
}

Graph* read_graph(const char *filename) {
    FILE *fp = fopen(filename, "r"); // Open file for reading 
    if (!fp) {
        fprintf(stderr, "Cannot open file %s\n", filename); // Error if file not found 
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

int dijkstra(Graph *g, int src, int tgt) {
    int n = g->n; // Number of vertices 
    int *dist = (int*)malloc(n * sizeof(int)); // Distance array 
    int *visited = (int*)calloc(n, sizeof(int)); // Visited array 
    for (int i = 0; i < n; ++i) dist[i] = INT_MAX; // Initialize distances 
    FibHeap *H = create_fib_heap(); // Create Fibonacci Heap 
    dist[src] = 0; // Distance to source is 0 
    fib_insert(H, src, 0); // Insert source into heap 
    while (H->n > 0) { // While heap is not empty 
        FibNode *min_node = fib_extract_min(H); // Extract node with smallest distance 
        int u = min_node->vertex; // Get vertex index 
        if (visited[u]) { // If already visited 
            free(min_node); // Free node 
            continue; // Skip to next iteration 
        }
        visited[u] = 1; // Mark as visited 
        if (u == tgt) { // If target reached 
            int ans = dist[u]; // Store answer 
            free(min_node); // Free node 
            free(dist); // Free distance array 
            free(visited); // Free visited array 
            free_fib_heap(H); // Free heap 
            return ans; // Return shortest distance 
        }
        for (Edge *e = g->adj[u]; e; e = e->next) { // For each neighbor 
            int v = e->to; // Get neighbor vertex 
            if (!visited[v] && dist[u] + e->weight < dist[v]) { // If shorter path found 
                dist[v] = dist[u] + e->weight; // Update distance 
                if (node_ptrs[v]) { // If node already in heap 
                    fib_decrease_key(H, node_ptrs[v], dist[v]); // Decrease key 
                } else {
                    fib_insert(H, v, dist[v]); // Insert new node 
                }
            }
        }
        free(min_node); // Free extracted node 
    }
    free(dist); // Free distance array 
    free(visited); // Free visited array 
    free_fib_heap(H); // Free heap 
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
