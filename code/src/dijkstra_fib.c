#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <time.h>

// Fibonacci Heap Node
typedef struct FibNode {
    int vertex;
    int degree;
    int mark;
    int dist;
    struct FibNode *parent, *child, *left, *right;
} FibNode;

// Fibonacci Heap
typedef struct {
    FibNode *min;
    int n;
} FibHeap;

// Edge structure
typedef struct Edge {
    int to;
    int weight;
    struct Edge *next;
} Edge;

// Graph structure
typedef struct {
    int n;
    Edge **adj;
} Graph;

// Function prototypes
FibHeap* create_fib_heap();
FibNode* fib_insert(FibHeap *H, int vertex, int dist);
FibNode* fib_extract_min(FibHeap *H);
void fib_decrease_key(FibHeap *H, FibNode *x, int k);
void fib_consolidate(FibHeap *H);
void fib_link(FibHeap *H, FibNode *y, FibNode *x);
void cut(FibHeap *H, FibNode *x, FibNode *y);
void cascading_cut(FibHeap *H, FibNode *y);
void free_fib_heap(FibHeap *H);
Graph* read_graph(const char *filename);
void free_graph(Graph *g);
int dijkstra(Graph *g, int src, int tgt);

// Helper functions for Fibonacci Heap
#define MAX_NODES 3000000
FibNode *node_ptrs[MAX_NODES]; // For decrease_key

FibHeap* create_fib_heap() {
    FibHeap *H = (FibHeap*)malloc(sizeof(FibHeap));
    H->min = NULL;
    H->n = 0;
    return H;
}

FibNode* fib_insert(FibHeap *H, int vertex, int dist) {
    FibNode *x = (FibNode*)malloc(sizeof(FibNode));
    x->vertex = vertex;
    x->dist = dist;
    x->degree = 0;
    x->mark = 0;
    x->parent = x->child = NULL;
    x->left = x->right = x;
    if (H->min == NULL) {
        H->min = x;
    } else {
        // Insert x into root list
        x->right = H->min->right;
        x->left = H->min;
        H->min->right->left = x;
        H->min->right = x;
        if (x->dist < H->min->dist)
            H->min = x;
    }
    H->n++;
    node_ptrs[vertex] = x;
    return x;
}

void fib_link(FibHeap *H, FibNode *y, FibNode *x) {
    // Remove y from root list
    y->left->right = y->right;
    y->right->left = y->left;
    // Make y a child of x
    y->parent = x;
    if (x->child == NULL) {
        x->child = y;
        y->left = y->right = y;
    } else {
        y->right = x->child->right;
        y->left = x->child;
        x->child->right->left = y;
        x->child->right = y;
    }
    x->degree++;
    y->mark = 0;
}

void fib_consolidate(FibHeap *H) {
    int D = 32; // log2(MAX_NODES)
    FibNode *A[32] = {NULL};
    FibNode *w = H->min;
    if (!w) return;
    int cnt = 0;
    FibNode *x = w;
    do {
        cnt++;
        x = x->right;
    } while (x != w);

    FibNode *nodes[cnt];
    x = w;
    for (int i = 0; i < cnt; ++i) {
        nodes[i] = x;
        x = x->right;
    }
    for (int i = 0; i < cnt; ++i) {
        x = nodes[i];
        int d = x->degree;
        while (A[d]) {
            FibNode *y = A[d];
            if (x->dist > y->dist) {
                FibNode *tmp = x; x = y; y = tmp;
            }
            fib_link(H, y, x);
            A[d] = NULL;
            d++;
        }
        A[d] = x;
    }
    H->min = NULL;
    for (int i = 0; i < D; ++i) {
        if (A[i]) {
            if (!H->min) {
                H->min = A[i];
                H->min->left = H->min->right = H->min;
            } else {
                // Insert into root list
                A[i]->right = H->min->right;
                A[i]->left = H->min;
                H->min->right->left = A[i];
                H->min->right = A[i];
                if (A[i]->dist < H->min->dist)
                    H->min = A[i];
            }
        }
    }
}

FibNode* fib_extract_min(FibHeap *H) {
    FibNode *z = H->min;
    if (z) {
        if (z->child) {
            FibNode *x = z->child;
            do {
                FibNode *next = x->right;
                // Add x to root list
                x->left = H->min;
                x->right = H->min->right;
                H->min->right->left = x;
                H->min->right = x;
                x->parent = NULL;
                x = next;
            } while (x != z->child);
        }
        // Remove z from root list
        z->left->right = z->right;
        z->right->left = z->left;
        if (z == z->right) {
            H->min = NULL;
        } else {
            H->min = z->right;
            fib_consolidate(H);
        }
        H->n--;
    }
    return z;
}

void cut(FibHeap *H, FibNode *x, FibNode *y) {
    // Remove x from child list of y
    if (y->child == x) {
        if (x->right != x)
            y->child = x->right;
        else
            y->child = NULL;
    }
    x->left->right = x->right;
    x->right->left = x->left;
    y->degree--;
    // Add x to root list
    x->right = H->min->right;
    x->left = H->min;
    H->min->right->left = x;
    H->min->right = x;
    x->parent = NULL;
    x->mark = 0;
}

void cascading_cut(FibHeap *H, FibNode *y) {
    FibNode *z = y->parent;
    if (z) {
        if (!y->mark)
            y->mark = 1;
        else {
            cut(H, y, z);
            cascading_cut(H, z);
        }
    }
}

void fib_decrease_key(FibHeap *H, FibNode *x, int k) {
    if (k > x->dist) return;
    x->dist = k;
    FibNode *y = x->parent;
    if (y && x->dist < y->dist) {
        cut(H, x, y);
        cascading_cut(H, y);
    }
    if (x->dist < H->min->dist)
        H->min = x;
}

void free_fib_heap(FibHeap *H) {
    // Not implemented for brevity
    free(H);
}

// Graph reading
Graph* read_graph(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Cannot open file %s\n", filename);
        exit(1);
    }
    int n = 0, m = 0;
    char buf[256];
    while (fgets(buf, sizeof(buf), fp)) {
        if (buf[0] == 'p') {
            sscanf(buf, "p sp %d %d", &n, &m);
            break;
        }
    }
    Graph *g = (Graph*)malloc(sizeof(Graph));
    g->n = n + 1;
    g->adj = (Edge**)calloc(g->n, sizeof(Edge*));
    rewind(fp);
    while (fgets(buf, sizeof(buf), fp)) {
        if (buf[0] == 'a') {
            int u, v, w;
            sscanf(buf, "a %d %d %d", &u, &v, &w);
            Edge *e = (Edge*)malloc(sizeof(Edge));
            e->to = v;
            e->weight = w;
            e->next = g->adj[u];
            g->adj[u] = e;
        }
    }
    fclose(fp);
    return g;
}

void free_graph(Graph *g) {
    for (int i = 0; i < g->n; ++i) {
        Edge *e = g->adj[i];
        while (e) {
            Edge *tmp = e;
            e = e->next;
            free(tmp);
        }
    }
    free(g->adj);
    free(g);
}

// Dijkstra with Fibonacci Heap
int dijkstra(Graph *g, int src, int tgt) {
    int n = g->n;
    int *dist = (int*)malloc(n * sizeof(int));
    int *visited = (int*)calloc(n, sizeof(int));
    for (int i = 0; i < n; ++i) dist[i] = INT_MAX;
    FibHeap *H = create_fib_heap();
    dist[src] = 0;
    fib_insert(H, src, 0);
    while (H->n > 0) {
        FibNode *min_node = fib_extract_min(H);
        int u = min_node->vertex;
        if (visited[u]) {
            free(min_node);
            continue;
        }
        visited[u] = 1;
        if (u == tgt) {
            int ans = dist[u];
            free(min_node);
            free(dist);
            free(visited);
            free_fib_heap(H);
            return ans;
        }
        for (Edge *e = g->adj[u]; e; e = e->next) {
            int v = e->to;
            if (!visited[v] && dist[u] + e->weight < dist[v]) {
                dist[v] = dist[u] + e->weight;
                if (node_ptrs[v]) {
                    fib_decrease_key(H, node_ptrs[v], dist[v]);
                } else {
                    fib_insert(H, v, dist[v]);
                }
            }
        }
        free(min_node);
    }
    free(dist);
    free(visited);
    free_fib_heap(H);
    return -1;
}

// Main
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
