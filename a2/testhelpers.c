#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "smalloc.h"

/* Functions to print the datastructures used by smalloc */
extern void *mem;
extern struct block *allocated_list;
extern struct block *freelist;

/* Prints each element of the list using the format string given below:*/
void print_list(struct block *list) {

        //printf("    [addr: %p, size: %d]\n", , );

    printf("\n");
}

void print_allocated() {
    print_list(allocated_list);
}

void print_free() {
    print_list(freelist);
}

/* write value size times to memory starting at ptr */
void write_to_mem(int size, char *ptr, char value) {
    int i = 0;
    for(i = 0; i < size; i++) {
        ptr[i] = value;
    }
}

/* Prints the contents of allocated memory. Each byte is printed as two
 * hexadecimal digits. */
void print_mem() {
    struct block *cur = allocated_list;
    
    while(cur != NULL) {
        printf("%p: size = %d\n", cur->addr, cur->size);
        
        /* print 16 bytes per line */
        int i, j;
        for(i = 0; i < cur->size / 8; i++){
            if((i)%2 == 0){
                printf("%5d:  ", i * 8);
            } else {
                printf("  ");
            }
            for(j = 0; j < 8; j++) {
                printf("%02x ", *((char *)cur->addr + ((i*8) + j)));
            }
            if((i+1) % 2 == 0 && ((i+1) * 8) != cur->size){
                printf("\n");
            }
        }
        printf("\n");
        cur =cur->next;
    }
    printf("\n");
}
