#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// Graph structure
typedef struct {
    int n;
    int **adj;
} Graph;

// Read graph in DIMACS format
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
    g->adj = (int**)malloc(g->n * sizeof(int*));
    for (int i = 0; i < g->n; ++i) {
        g->adj[i] = (int*)malloc(g->n * sizeof(int));
        for (int j = 0; j < g->n; ++j)
            g->adj[i][j] = (i == j ? 0 : INT_MAX / 2); // INT_MAX/2 防止溢出
    }
    rewind(fp);
    while (fgets(buf, sizeof(buf), fp)) {
        if (buf[0] == 'a') {
            int u, v, w;
            sscanf(buf, "a %d %d %d", &u, &v, &w);
            if (g->adj[u][v] > w)
                g->adj[u][v] = w;
        }
    }
    fclose(fp);
    return g;
}

void free_graph(Graph *g) {
    for (int i = 0; i < g->n; ++i)
        free(g->adj[i]);
    free(g->adj);
    free(g);
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

    int n = g->n;
    // Floyd-Warshall
    for (int k = 1; k < n; ++k)
        for (int i = 1; i < n; ++i)
            for (int j = 1; j < n; ++j)
                if (g->adj[i][j] > g->adj[i][k] + g->adj[k][j])
                    g->adj[i][j] = g->adj[i][k] + g->adj[k][j];

    int result = g->adj[src][tgt];
    if (result >= INT_MAX / 4)
        printf("-1\n");
    else
        printf("%d\n", result);

    free_graph(g);
    return 0;
}
