#include <stdio.h>

void swap_pointers(void **ptr1, void **ptr2) {
    void *temp = *ptr1; // Store the address pointed to by ptr1 in temp
    *ptr1 = *ptr2;      // Assign the address pointed to by ptr2 to ptr1
    *ptr2 = temp;      // Assign the address stored in temp to ptr2
}

int main() {
    int a = 1;
    int b = 2;

    int *p = &a;
    int *q = &b;

    printf("Before swap:\n");
    printf("address of p = %p and q = %p\n", (void *)p, (void *)q);

    swap_pointers((void **)&p, (void **)&q); // Cast p and q to void**

    printf("After swap:\n");
    printf("address of p = %p and q = %p\n", (void *)p, (void *)q);

    return 0;
}

