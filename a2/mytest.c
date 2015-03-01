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
    
    /**If there's no block in freelist has enough size to allocate memory.*/
    //Allocate all the bytes in freelist.
    ptrs[0] = smalloc(SIZE);
    write_to_mem(SIZE, ptrs[0], 0);
    printf("List of free blocks:\n");
    print_free();
    printf("List of allocated blocks:\n");
    print_allocated();
    
    //Attempt to allocate, but will fail.
    ptrs[1] = smalloc(10);
    if (ptrs[1] == NULL){
      printf("No memory left in freelist, smalloc fail.\n");
    }
    printf("List of free blocks:\n");
    print_free();
    printf("List of allocated blocks:\n");
    print_allocated();
    //Free the allocated memory back to freelist.
    //Resume to normal.
    printf("freeing %p result = %d\n", ptrs[0], sfree(ptrs[0]));  
    printf("List of allocated blocks:\n");
    print_allocated();
    printf("List of free blocks:\n");
    print_free();
    
    mem_clean();
    printf("\n\n---------------------------\n\n");

    
    
    /**In simpletest, we've tested the case where memory is allocated*/
    /**from a block that has a bigger size.*/
    /**So now, I will test if we allocate space from a block that has a equal size with nbytes.*/
    //Allocate 20 and 30 bytes first.
    mem_init(SIZE);
    ptrs[0] = smalloc(20);
    write_to_mem(20, ptrs[0], 0);
    ptrs[1] = smalloc(30);
    write_to_mem(30, ptrs[1], 1);
    printf("List of free blocks:\n");
    print_free();
    printf("List of allocated blocks:\n");
    print_allocated();
    print_mem();
    
    //Free the 30 bytes block.
    //(Here, we also tested the case where we're freeing the header block of allocated_list)
    printf("freeing %p result = %d\n", ptrs[1], sfree(ptrs[1]));  
    printf("List of allocated blocks:\n");
    print_allocated();
    printf("List of free blocks:\n");
    print_free();
    print_mem();
    
    //Allocate 30 bytes again, from the block we just freed.
    //(Case where not first time allocate memory from a block that has the same size.) 
    ptrs[1] = smalloc(30);
    write_to_mem(30, ptrs[1], 1);
    printf("List of allocated blocks:\n");
    print_allocated();
    printf("List of free blocks:\n");
    print_free();
    print_mem();
    
    //Free the whole allocated_list; 
    //(Casewhere we first allocate memory from a block that has the same size.)
    sfree(ptrs[0]);//Also testing we sfree the children block first;
    sfree(ptrs[1]);
    printf("List of allocated blocks:\n");
    print_allocated();
    printf("List of free blocks:\n");
    print_free();
    print_mem();
    
    ptrs[0] = smalloc(30);
    write_to_mem(30, ptrs[0], 0);
    printf("List of allocated blocks:\n");
    print_allocated();
    printf("List of free blocks:\n");
    print_free();
    print_mem();
    
    //Test if we can allocate from the last block at freelist.
    ptrs[1] = smalloc(40);
    write_to_mem(40, ptrs[1], 1);
    printf("List of allocated blocks:\n");
    print_allocated();
    printf("List of free blocks:\n");
    print_free();
    print_mem();
    
    mem_clean();
    printf("\n\n---------------------------\n\n");
    
   
    
    /**Interesting case: Allocate 10 blocks to ptrs, 
    *free all ptrs[i] with i even, then free the rest which is ptrs[odd]*/
    /**This will test if the freelist will still be in increasing order after calling
      *multiple smalloc and sfree.
    */
    //Allocate 10 blocks.
    mem_init(SIZE);
    for(i = 0; i < 10; i++) {
        int num_bytes = (i+2) * 10;
    
        ptrs[i] = smalloc(num_bytes);
        write_to_mem(num_bytes, ptrs[i], i);
    }
    printf("List of free blocks:\n");
    print_free();
    printf("List of allocated blocks:\n");
    print_allocated();

    printf("Contents of allocated memory:\n");
    print_mem();
    
    //Free even blocks.
    printf("freeing %p result = %d\n", ptrs[0], sfree(ptrs[0]));
    printf("freeing %p result = %d\n", ptrs[2], sfree(ptrs[2]));
    printf("freeing %p result = %d\n", ptrs[4], sfree(ptrs[4]));
    printf("freeing %p result = %d\n", ptrs[6], sfree(ptrs[6]));
    printf("freeing %p result = %d\n", ptrs[8], sfree(ptrs[8]));
    
    printf("List of allocated blocks:\n");
    print_allocated();
    printf("List of free blocks:\n");
    print_free();
    printf("Contents of allocated memory:\n");
    print_mem();
    
    //Free odd blocks.
    printf("freeing %p result = %d\n", ptrs[1], sfree(ptrs[1]));
    printf("freeing %p result = %d\n", ptrs[3], sfree(ptrs[3]));
    printf("freeing %p result = %d\n", ptrs[5], sfree(ptrs[5]));
    printf("freeing %p result = %d\n", ptrs[7], sfree(ptrs[7]));
    printf("freeing %p result = %d\n", ptrs[9], sfree(ptrs[9]));
    
    printf("List of allocated blocks:\n");
    print_allocated();
    printf("List of free blocks:\n");
    print_free();
    printf("Contents of allocated memory:\n");
    print_mem();

    mem_clean(); 

    return 0;
}
