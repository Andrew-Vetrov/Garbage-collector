#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "../allocator/allocator.h"

#define BIG_GRAPH_SIZE 256 * 1024 * 1024

typedef struct Node {
    int id;
    struct Node** neighbors;
    int neighbor_count;
} Node;

Node* bigGraph = NULL;

Node* createNode(int id) {
    Node* node = (Node*)gc_malloc(sizeof(Node));
    if (node == NULL) {
        fprintf(stderr, "Failed to allocate memory for node\n");
        exit(1);
    }
    node->id = id;
    node->neighbors = NULL;
    node->neighbor_count = 0;
    return node;
}

void addNeighbor(Node* node, Node* neighbor) {
    node->neighbor_count++;
    node->neighbors = (Node**)gc_malloc(node->neighbor_count * sizeof(Node*));
    if (node->neighbors == NULL) {
        fprintf(stderr, "Failed to allocate memory for neighbors\n");
        exit(1);
    }
    node->neighbors[node->neighbor_count - 1] = neighbor;
}

void createBigGraph() {
    printf("Creating big graph (~1 GB)...\n");
    bigGraph = createNode(0);

    int nodeCount = BIG_GRAPH_SIZE / sizeof(Node);
    Node* prevNode = bigGraph;
    for (int i = 1; i < nodeCount; i++) {
        Node* newNode = createNode(i);
        addNeighbor(prevNode, newNode);
        prevNode = newNode;
    }
    printf("Big graph created.\n");
}

Node* createSmallGraph(int size) {
    Node* root = createNode(0);
    Node* prevNode = root;
    for (int i = 1; i < size; i++) {
        Node* newNode = createNode(i);
        addNeighbor(prevNode, newNode);
        prevNode = newNode;
    }
    return root;
}

void simulateProgram() {
    createBigGraph();

    printf("Creating small graphs...\n");
    Node* smallGraphs[100];
    for (int i = 0; i < 1000; i++) {
        for (int j = 0; j < 100; j++) {
            smallGraphs[j] = createSmallGraph(1000);
        }
    }

    printf("Simulation completed.\n");
}

int main() {
    simulateProgram();
    return 0;
}