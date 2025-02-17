#include <stdio.h>
#include <stdlib.h>
#include "../allocator/allocator.h"
// Структура узла двусвязного списка
typedef struct Node {
    int data;
    struct Node* prev;
    struct Node* next;
} Node;

Node* create_node(int data) {
    Node* new_node = (Node*)allocate_new_object(sizeof(Node));
    new_node->data = data;
    new_node->prev = NULL;
    new_node->next = NULL;
    return new_node;
}

// Функция для добавления узла в конец списка
void append(Node** head, int data) {
    Node* new_node = create_node(data);
    if (*head == NULL) {
        *head = new_node;
        return;
    }
    Node* temp = *head;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = new_node;
    new_node->prev = temp;
}

// Функция для вывода списка
void print_list(Node* head) {
    Node* temp = head;
    printf("List: ");
    while (temp != NULL) {
        printf("%d ", temp->data);
        temp = temp->next;
    }
    printf("\n");
}

int main() {
    Node* head = NULL;
    int size = (int)1e7;
    printf("Usage: %d\n", 10 * size * sizeof(Node)/1024/1024);
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 10; j++) {
            append(&head, 1);
        }
        head = NULL;
    }

    return 0;
}
