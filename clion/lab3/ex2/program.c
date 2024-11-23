#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "dplist.h"

// Function to print the entire list (for testing)
void print_list(dplist_t *list) {
    int size = dpl_size(list);
    printf("List (size = %d): ", size);
    for (int i = 0; i < size; i++) {
        printf("%s -> ", dpl_get_element_at_index(list, i));
    }
    printf("NULL\n");
}

int main(void) {
    // Create a new list
    dplist_t *list = dpl_create();
    assert(list != NULL);
    printf("Created a new list.\n");

    // Insert elements at various positions
    dpl_insert_at_index(list, "first", 0);  // Insert at the start
    dpl_insert_at_index(list, "second", 1); // Insert at index 1 (end)
    dpl_insert_at_index(list, "third", -1); // Insert at the start
    dpl_insert_at_index(list, "fourth", 100); // Insert at end
    print_list(list);

    // Test dpl_size function
    int size = dpl_size(list);
    printf("List size after inserts: %d\n", size); // Should print 4

    // Test dpl_get_element_at_index function
    char *element = dpl_get_element_at_index(list, 2);
    printf("Element at index 2: %s\n", element); // Should print "second"

    // Test dpl_get_index_of_element function
    int index = dpl_get_index_of_element(list, "third");
    printf("Index of 'third': %d\n", index); // Should print 0

    // Test removing an element
    printf("Removing element at index 1.\n");
    dpl_remove_at_index(list, 1);
    print_list(list); // Should print the list without "first"

    // Test removing the first element
    printf("Removing element at index 0.\n");
    dpl_remove_at_index(list, 0);
    print_list(list); // Should print the list without "third"

    // Test removing an element out of bounds (should remove last)
    printf("Removing element at index 100 (out of bounds, should remove last).\n");
    dpl_remove_at_index(list, 100);
    print_list(list); // Should print the list without "fourth"

    // Test the list size after removals
    size = dpl_size(list);
    printf("List size after removals: %d\n", size); // Should print 1

    // Free the list and check if it's null
    dpl_free(&list);
    assert(list == NULL);
    printf("List freed and set to NULL.\n");

    return 0;
}

