#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

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

// Binary Heap
typedef struct {
    int *heap;
    int *pos;
    int *dist;
    int size;
    int capacity;
} MinHeap;

// Function prototypes
Graph* read_graph(const char *filename);
void free_graph(Graph *g);
MinHeap* create_min_heap(int capacity);
void min_heap_push(MinHeap *h, int v, int d);
int min_heap_pop(MinHeap *h);
void min_heap_decrease_key(MinHeap *h, int v, int d);
void free_min_heap(MinHeap *h);
int dijkstra(Graph *g, int src, int tgt);

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

// MinHeap functions
MinHeap* create_min_heap(int capacity) {
    MinHeap *h = (MinHeap*)malloc(sizeof(MinHeap));
    h->heap = (int*)malloc(capacity * sizeof(int));
    h->pos = (int*)malloc(capacity * sizeof(int));
    h->dist = (int*)malloc(capacity * sizeof(int));
    h->size = 0;
    h->capacity = capacity;
    for (int i = 0; i < capacity; ++i) {
        h->pos[i] = -1;
        h->dist[i] = INT_MAX;
    }
    return h;
}

void swap(MinHeap *h, int i, int j) {
    int t = h->heap[i];
    h->heap[i] = h->heap[j];
    h->heap[j] = t;
    h->pos[h->heap[i]] = i;
    h->pos[h->heap[j]] = j;
}

void min_heapify_up(MinHeap *h, int idx) {
    while (idx > 0) {
        int p = (idx - 1) / 2;
        if (h->dist[h->heap[p]] > h->dist[h->heap[idx]]) {
            swap(h, p, idx);
            idx = p;
        } else break;
    }
}

void min_heapify_down(MinHeap *h, int idx) {
    int l, r, min_idx;
    while (1) {
        l = 2 * idx + 1;
        r = 2 * idx + 2;
        min_idx = idx;
        if (l < h->size && h->dist[h->heap[l]] < h->dist[h->heap[min_idx]])
            min_idx = l;
        if (r < h->size && h->dist[h->heap[r]] < h->dist[h->heap[min_idx]])
            min_idx = r;
        if (min_idx != idx) {
            swap(h, idx, min_idx);
            idx = min_idx;
        } else break;
    }
}

void min_heap_push(MinHeap *h, int v, int d) {
    h->heap[h->size] = v;
    h->dist[v] = d;
    h->pos[v] = h->size;
    min_heapify_up(h, h->size);
    h->size++;
}

int min_heap_pop(MinHeap *h) {
    if (h->size == 0) return -1;
    int v = h->heap[0];
    h->heap[0] = h->heap[h->size - 1];
    h->pos[h->heap[0]] = 0;
    h->size--;
    min_heapify_down(h, 0);
    h->pos[v] = -1;
    return v;
}

void min_heap_decrease_key(MinHeap *h, int v, int d) {
    h->dist[v] = d;
    min_heapify_up(h, h->pos[v]);
}

void free_min_heap(MinHeap *h) {
    free(h->heap);
    free(h->pos);
    free(h->dist);
    free(h);
}

// Dijkstra with Binary Heap
int dijkstra(Graph *g, int src, int tgt) {
    int n = g->n;
    int *dist = (int*)malloc(n * sizeof(int));
    int *visited = (int*)calloc(n, sizeof(int));
    for (int i = 0; i < n; ++i) dist[i] = INT_MAX;
    MinHeap *h = create_min_heap(n);
    dist[src] = 0;
    min_heap_push(h, src, 0);
    while (h->size > 0) {
        int u = min_heap_pop(h);
        if (visited[u]) continue;
        visited[u] = 1;
        if (u == tgt) {
            int ans = dist[u];
            free(dist);
            free(visited);
            free_min_heap(h);
            return ans;
        }
        for (Edge *e = g->adj[u]; e; e = e->next) {
            int v = e->to;
            if (!visited[v] && dist[u] + e->weight < dist[v]) {
                dist[v] = dist[u] + e->weight;
                if (h->pos[v] == -1) {
                    min_heap_push(h, v, dist[v]);
                } else {
                    min_heap_decrease_key(h, v, dist[v]);
                }
            }
        }
    }
    free(dist);
    free(visited);
    free_min_heap(h);
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
