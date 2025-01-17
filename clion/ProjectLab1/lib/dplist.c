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

    dplist_node_t *new_node = malloc(sizeof(dplist_node_t));
    if (new_node == NULL) return NULL;

    new_node->element = insert_copy ? list->element_copy(element) : element;
    new_node->next = NULL;
    new_node->prev = NULL;

    if (index <= 0 || list->head == NULL) {
        new_node->next = list->head;
        if (list->head != NULL) {
            list->head->prev = new_node;
        }
        list->head = new_node;
    } else {
        dplist_node_t *current = list->head;
        int i = 0;

        while (current != NULL && i < index) {
            current = current->next;
            i++;
        }

        if (current == NULL) {
            current = list->head;
            while (current->next != NULL) {
                current = current->next;
            }
            current->next = new_node;
            new_node->prev = current;
        } else {
            new_node->next = current;
            new_node->prev = current->prev;

            if (current->prev != NULL) {
                current->prev->next = new_node;
            }
            current->prev = new_node;
        }
    }
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