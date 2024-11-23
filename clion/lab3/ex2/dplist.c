#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "dplist.h"

char *strduplicate(const char *str) {
    if (str == NULL) return NULL;
    size_t len = strlen(str) + 1;
    char *copy = malloc(len);
    if (copy != NULL) {
        memcpy(copy, str, len);
    }
    return copy;
}

element_t element_copy(element_t element) {
    return strduplicate(element);
}

void element_free(element_t element) {
    free(element);
}

struct dplist_node {
    struct dplist_node *prev, *next;
    element_t element;
};

struct dplist {
    struct dplist_node *head;
};

dplist_t *dpl_create() {
    dplist_t *list = malloc(sizeof(struct dplist));
    list->head = NULL;
    return list;
}

void dpl_free(dplist_t **list) {
    if (list == NULL || *list == NULL) return;

    dplist_node_t *current = (*list)->head;
    while (current != NULL) {
        dplist_node_t *next = current->next;
        element_free(current->element);
        free(current);
        current = next;
    }

    free(*list);
    *list = NULL;
}

dplist_t *dpl_insert_at_index(dplist_t *list, element_t element, int index) {
    if (list == NULL) {
        return NULL; 
    }

    dplist_node_t *list_node = malloc(sizeof(dplist_node_t));
    if (list_node == NULL) {
        return NULL; 
    }

    list_node->element = element_copy(element);
    if (list_node->element == NULL) {
        free(list_node);
        return NULL;
    }

    if (list->head == NULL) {
        if (index != 0) {
            free(list_node->element);
            free(list_node);
            return NULL; 
        }
        list_node->prev = NULL;
        list_node->next = NULL;
        list->head = list_node;
    } 
    else if (index <= 0) {
        list_node->prev = NULL;
        list_node->next = list->head;
        list->head->prev = list_node;
        list->head = list_node;
    } 
    else {
        dplist_node_t *ref_at_index = dpl_get_reference_at_index(list, index);
        
        if (ref_at_index == NULL) {
            ref_at_index = dpl_get_reference_at_index(list, dpl_size(list) - 1);
            list_node->prev = ref_at_index;
            list_node->next = NULL;
            ref_at_index->next = list_node;
        } else {

            list_node->prev = ref_at_index->prev;
            list_node->next = ref_at_index;
            if (ref_at_index->prev != NULL) {
                ref_at_index->prev->next = list_node;
            } else {
                list->head = list_node; 
            }
            ref_at_index->prev = list_node;
        }
    }

    return list;
}


dplist_t *dpl_remove_at_index(dplist_t *list, int index) {
    if (list == NULL || list->head == NULL) {
        return list; 
    }

    dplist_node_t *to_remove = dpl_get_reference_at_index(list, index);
    if (to_remove == NULL) {
        int size = dpl_size(list);
        if (size > 0) {
            to_remove = dpl_get_reference_at_index(list, size - 1);
        } else {
            return list;
        }
    }

    if (to_remove->prev != NULL) {
        to_remove->prev->next = to_remove->next;
    } else {
        list->head = to_remove->next;
    }

    if (to_remove->next != NULL) {
        to_remove->next->prev = to_remove->prev;
    }

    element_free(to_remove->element);
    free(to_remove);
    return list;
}

int dpl_size(dplist_t *list) {
    if (list == NULL) return -1;

    int count = 0;
    dplist_node_t *current = list->head;
    while (current != NULL) {
        count++;
        current = current->next;
    }

    return count;
}

dplist_node_t *dpl_get_reference_at_index(dplist_t *list, int index) {
    if (list == NULL || list->head == NULL) return NULL;

    dplist_node_t *current = list->head;
    int count = 0;

    while (current != NULL && count < index) {
        current = current->next;
        count++;
    }

    return current;
}

element_t dpl_get_element_at_index(dplist_t *list, int index) {
    if (list == NULL || index < 0 || index >= dpl_size(list)) {
        return NULL;
    }

    dplist_node_t *node = dpl_get_reference_at_index(list, index);
    return (node != NULL) ? node->element : NULL;
}

int dpl_get_index_of_element(dplist_t *list, const element_t element) {
    if (list == NULL || list->head == NULL) return -1;

    int index = 0;
    dplist_node_t *current = list->head;

    while (current != NULL) {
        if (strcmp(current->element, element) == 0) return index;
        current = current->next;
        index++;
    }

    return -1; 
}

