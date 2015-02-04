#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "smalloc.h"


#define SIZE 4096 * 64


/* Simple test for smalloc and sfree. */

int main(void) {

    mem_init(SIZE);
    
    char *ptrs[10];
    int i;

    /* Call smalloc 4 times */
    
    for(i = 0; i < 4; i++) {
        int num_bytes = (i+1) * 10;
    
        ptrs[i] = smalloc(num_bytes);
        write_to_mem(num_bytes, ptrs[i], i);
    }
    printf("List of allocated blocks:\n");
    print_allocated();
    printf("List of free blocks:\n");
    print_free();
    printf("Contents of allocated memory:\n");
    print_mem();
    
    printf("freeing %p result = %d\n", ptrs[1], sfree(ptrs[1]));
    
    printf("List of allocated blocks:\n");
    print_allocated();
    printf("List of free blocks:\n");
    print_free();
    printf("Contents of allocated memory:\n");
    print_mem();

    mem_clean();
    return 0;
}
