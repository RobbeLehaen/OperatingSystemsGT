#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h> // Include string.h for strcmp
#include "dplist.h"

typedef struct {
    int id;
    char* name;
} my_element_t;

void* element_copy(void * element);
void element_free(void ** element);
int element_compare(void * x, void * y);

void *element_copy(void *element)
{
    my_element_t *copy = malloc(sizeof(my_element_t));
    char *new_name;
    asprintf(&new_name, "%s", ((my_element_t *)element)->name);
    assert(copy != NULL);
    copy->id = ((my_element_t *)element)->id;
    copy->name = new_name;
    return (void *)copy;
}

void element_free(void **element) {
    if (element != NULL && *element != NULL) {
        free((((my_element_t *)*element))->name);
        free(*element);
        *element = NULL;
    }
}

int element_compare(void * x, void * y) {
    return ((((my_element_t*)x)->id < ((my_element_t*)y)->id) ? -1 :
            (((my_element_t*)x)->id == ((my_element_t*)y)->id) ? 0 : 1);
}

void ck_assert_msg(bool result, char * msg) {
    if(!result) {
        printf("%s\n", msg);
        exit(1); // Exit on failure to highlight the issue
    }
}

void yourtest1() {
    // Test 1: Create and free a list
    dplist_t *list = dpl_create(element_copy, element_free, element_compare);
    ck_assert_msg(list != NULL, "expected list to be created successfully, Test 1");

    // Test 2: Check size of an empty list
    ck_assert_msg(dpl_size(list) == 0, "expected list size to be 0, Test 2");

    // Test 3: Insert elements
    my_element_t *content1 = malloc(sizeof(my_element_t));
    content1->id = 1;
    content1->name = "Element 1";

    my_element_t *content2 = malloc(sizeof(my_element_t));
    content2->id = 2;
    content2->name = "Element 2";

    my_element_t *content3 = malloc(sizeof(my_element_t));
    content3->id = 3;
    content3->name = "Element 3";

    // Insert at valid indices
    list = dpl_insert_at_index(list, content1, 0, true);
    ck_assert_msg(dpl_size(list) == 1, "expected list size to be 1 after inserting first element, Test 3a");

    list = dpl_insert_at_index(list, content2, 1, true); // Insert at end
    ck_assert_msg(dpl_size(list) == 2, "expected list size to be 2 after inserting second element, Test 3b");

    list = dpl_insert_at_index(list, content3, 0, true); // Insert at beginning
    ck_assert_msg(dpl_size(list) == 3, "expected list size to be 3 after inserting third element, Test 3c");

    // Test 4: Insert at invalid indices
    list = dpl_insert_at_index(list, content1, -1, true); // Insert at start
    ck_assert_msg(dpl_size(list) == 4, "expected list size to be 4 after inserting at invalid index, Test 4a");

    list = dpl_insert_at_index(list, content1, 10, true); // Insert beyond the end
    ck_assert_msg(dpl_size(list) == 5, "expected list size to be 5 after inserting at invalid index, Test 4b");

    // Test 5: Retrieve elements by index
    void *element_at_index_0 = dpl_get_element_at_index(list, 0);
    ck_assert_msg(element_at_index_0 != NULL, "expected element at index 0 to not be NULL, Test 5");

    // Test 6: Retrieve elements using references
    dplist_node_t *node = dpl_get_reference_at_index(list, 2);
    void *element_at_reference = dpl_get_element_at_reference(list, node);
    ck_assert_msg(element_at_reference != NULL, "expected element at reference to not be NULL, Test 6");

    // Test 7: Remove elements from list
    list = dpl_remove_at_index(list, 1, true); // Remove second element
    ck_assert_msg(dpl_size(list) == 4, "expected list size to be 4 after removing one element, Test 7a");

    list = dpl_remove_at_index(list, 0, true); // Remove first element
    ck_assert_msg(dpl_size(list) == 3, "expected list size to be 3 after removing another element, Test 7b");

    // Test 8: Find index of element
    int index_of_element1 = dpl_get_index_of_element(list, content1);
    ck_assert_msg(index_of_element1 >= 0, "expected to find element1 in the list, Test 8");

    // Test 9: Free the list and check for memory leaks
    dpl_free(&list, true);
    ck_assert_msg(list == NULL, "expected list to be NULL after freeing, Test 9");

    // Test 10: Freeing a NULL list
    dplist_t *null_list = NULL;
    dpl_free(&null_list, true);

    // Test 11: Remove from an empty list
    dplist_t *empty_list = dpl_create(element_copy, element_free, element_compare);
    empty_list = dpl_remove_at_index(empty_list, 0, true);
    ck_assert_msg(dpl_size(empty_list) == 0, "expected empty list size to be 0 after remove attempt, Test 11");

    // Test 12: Retrieval from an empty list
    void *elem = dpl_get_element_at_index(empty_list, 0);
    ck_assert_msg(elem == NULL, "expected retrieval from empty list to return NULL, Test 12");

    // Freeing the empty list
    dpl_free(&empty_list, true);

    // Clean up remaining allocated content
    free(content1);
    free(content2);
    free(content3);

}


int main(void) {
    yourtest1();
    printf("All tests passed!\n");
    return 0;
}
