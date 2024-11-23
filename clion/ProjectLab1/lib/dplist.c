#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "dplist.h"

struct dplist_node {
    dplist_node_t *prev, *next;
    void *element;
};

typedef struct {
    int id;
    char *name;
} my_element_t;

struct dplist {
    dplist_node_t *head;
    void *(*element_copy)(void *src_element);
    void (*element_free)(void **element);
    int (*element_compare)(void *x, void *y);
};

dplist_t *dpl_create(
        void *(*element_copy)(void *src_element),
        void (*element_free)(void **element),
        int (*element_compare)(void *x, void *y)
) {
    dplist_t *list = malloc(sizeof(struct dplist));
    if (list != NULL) {
        list->head = NULL;
        list->element_copy = element_copy;
        list->element_free = element_free;
        list->element_compare = element_compare;
    }
    return list;
}

void dpl_free(dplist_t **list, bool free_element) {
    if (list == NULL || *list == NULL) return;

    dplist_node_t *current = (*list)->head;
    while (current != NULL) {
        dplist_node_t *next = current->next;

        if (free_element && current->element != NULL) {
            (*list)->element_free(&(current->element));
            current->element = NULL;
        }

        free(current);
        current = next;
    }
    free(*list);
    *list = NULL;
}

dplist_t *dpl_insert_at_index(dplist_t *list, void *element, int index, bool insert_copy) {
    if (list == NULL) return NULL;

    printf("dpl_insert_at_index called with index %d\n", index); // Debug print
    dplist_node_t *new_node = malloc(sizeof(dplist_node_t));
    if (new_node == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for new node\n");
        return NULL;
    }

    new_node->element = insert_copy ? list->element_copy(element) : element;
    new_node->next = NULL;
    new_node->prev = NULL;

    // Debug: Confirm new node is created
    fprintf(stderr, "Inserting element at index %d: %p\n", index, new_node);
    fflush(stderr); // Flush stderr to ensure it prints

    if (index <= 0 || list->head == NULL) {
        // Insert at the head
        new_node->next = list->head;
        if (list->head != NULL) {
            list->head->prev = new_node;
        }
        list->head = new_node;
        fprintf(stderr, "Inserted at head, new head: %p\n", list->head);
    } else {
        // Insert somewhere after the head
        dplist_node_t *current = list->head;
        int i = 0;

        // Traverse to the index
        while (current != NULL && i < index) {
            current = current->next;
            i++;
        }

        if (current == NULL) {
            // If we reach the end of the list, insert at the tail
            current = list->head;
            while (current->next != NULL) {
                current = current->next;
            }
            current->next = new_node;
            new_node->prev = current;
            fprintf(stderr, "Inserted at tail\n");
        } else {
            // Insert in the middle
            new_node->next = current;
            new_node->prev = current->prev;

            if (current->prev != NULL) {
                current->prev->next = new_node;
            }
            current->prev = new_node;
            fprintf(stderr, "Inserted in the middle\n");
        }
    }

    // Debugging: Print list size after insertion
    fprintf(stderr, "List size after insertion: %d\n", dpl_size(list));
    fflush(stderr); // Flush stderr to ensure it prints
    return list;
}


dplist_t *dpl_remove_at_index(dplist_t *list, int index, bool free_element) {
    if (list == NULL || list->head == NULL) return list;

    dplist_node_t *current = list->head;

    if (index <= 0) {
        index = 0;
    }

    int i = 0;
    while (current->next != NULL && i < index) {
        current = current->next;
        i++;
    }

    if (free_element && current->element != NULL) {
        list->element_free(&(current->element));
        current->element = NULL;
    }

    if (current->prev != NULL) {
        current->prev->next = current->next;
    } else {
        list->head = current->next;
    }
    if (current->next != NULL) {
        current->next->prev = current->prev;
    }

    free(current);
    return list;
}


int dpl_size(dplist_t *list) {
    if (list == NULL) return 0;

    int size = 0;
    dplist_node_t *current = list->head;
    while (current != NULL) {
        size++;
        current = current->next;
    }

    // Debug: Print the size as we count the nodes
    fprintf(stderr, "dpl_size: %d\n", size);
    return size;
}


void *dpl_get_element_at_index(dplist_t *list, int index) {
    if (list == NULL || list->head == NULL) return NULL;

    dplist_node_t *current = list->head;

    if (index <= 0) {
        return current->element;
    }

    for (int i = 0; current->next != NULL && i < index; i++) {
        current = current->next;
    }

    return current->element;
}

int dpl_get_index_of_element(dplist_t *list, void *element) {
    if (list == NULL || list->head == NULL) return -1;

    dplist_node_t *current = list->head;
    int index = 0;
    while (current != NULL) {
        if (list->element_compare(current->element, element) == 0) {
            return index;
        }
        index++;
        current = current->next;
    }
    return -1;
}

dplist_node_t *dpl_get_reference_at_index(dplist_t *list, int index) {
    if (list == NULL || list->head == NULL) {
        return NULL;
    }

    dplist_node_t *current = list->head;

    if (index <= 0) {
        return current;
    }

    for (int i = 0; i < index && current->next != NULL; i++) {
        current = current->next;
    }

    return current;
}

void *dpl_get_element_at_reference(dplist_t *list, dplist_node_t *reference) {
    if (list == NULL || list->head == NULL || reference == NULL) {
        return NULL;
    }

    dplist_node_t *current = list->head;

    while (current != NULL) {
        if (current == reference) {
            return reference->element;
        }
        current = current->next;
    }

    return NULL;
}
