#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "smalloc.h"



void *mem;
struct block *freelist;
struct block *allocated_list;

void *smalloc(unsigned int nbytes) {
    //Initialize the helper pointers.
    struct block *target;
    struct block *target_parent;
    struct block *temporary;
    
    //Use target to loop over freelist. 
    target = freelist;
    target_parent = NULL;
    
    //Use while loop to find the block that has at least nbytes size.
    while (target != NULL){
        if (target->size >= nbytes){//Condition.
            break;//Break the loop, because we find our target block.
        }else{
            if (target->next != NULL){//If there's children for this block.
                target_parent = target;//Store the parent.
                target = target->next;
            }else{
                break;//If no children for this block, break the loop.
            }	
        }
    }
    
    //target has pointed the block that might work;
    if (target != NULL && target->size > nbytes){//If target has size greater than nbytes.  	
        if (allocated_list == NULL){//First time allocate memory.
            allocated_list = malloc(sizeof(struct block));//Allocate memory for allocated_list.
            allocated_list->addr = target->addr;//Block's address is the same as target.
            allocated_list->size = nbytes;
            allocated_list->next = NULL;
	    //Adjust the block in freelist.
	    target->size = target->size - nbytes;
	    target->addr += nbytes;
	    return allocated_list->addr;
        }else{//It's not the first time allocate.
            //Create a new block of nbytes size.
	    temporary =  malloc(sizeof(struct block));
	    temporary->addr = target->addr;
	    temporary-> size = nbytes;
	    temporary->next = allocated_list;
	    allocated_list = temporary;
	    //Adjust the target block in freelist.
	    target->size = target->size - nbytes;
	    target->addr += nbytes;            
	    return allocated_list->addr;
      	}
    
    //The target block has size equals to nbytes.       
    }else if(target != NULL && target->size == nbytes){
        //Remove the target block in freelist.
        if (target_parent){//If target is not the header block.
    	    target_parent->next = target->next;
        }else{//It's the header block.
    	    freelist = freelist->next;
    	}
    	//Append target to header of allocated?list.
        if (allocated_list == NULL){//First time allocate memory.  
	    allocated_list = target;
	    allocated_list->next = NULL;	    
	    return allocated_list->addr;
	}else{//It's not the first time allocate.
	    temporary = allocated_list;//Store the allocated_list.
	    allocated_list = target;//Make allocated_list points to the target block.
	    allocated_list->next = temporary;	    
	    return allocated_list->addr;
        }     
    }else{//No available block in freelist.
        return NULL;
    }	
}


int sfree(void *addr) {
    //Initialize temporary ptrs.
    struct block *destination;
    struct block *parent;
    struct block *gap_parent;
    struct block *gap;

    //Use destinatopn to loop over allocated_list
    //and check which block the addr belong to.
    destination = allocated_list;
    parent = NULL;
    while(destination != NULL){
	if (destination->addr == addr){
	    break;
	}else{
	    //Store the parent block for current block.
	    parent = destination;
	    destination = destination->next;
	}
    }
	
    //Re-Check if the address identical.
    if (destination->addr == addr){		
	//Use helper pointer gap to loop over freelist to locate
	//the  postion of appending the destination block(Since increasing order).	
	gap = freelist;
	gap_parent = NULL;//No parent at the beginning.
	
	//Find the gap block where we insert back the block we want to free.
	while(gap != NULL){
	    //Address of the block is smaller than the block that we want to free.
	    //Move to next block since it's not the right position.
	    if(gap->addr < addr){
		gap_parent = gap;
	    	gap = gap->next;//Move to next round.
	    }else{
	    	break;//This gap is good.
	    }
	}
       
       //Fllowing code: Start inserting the block in freelist.
       
       //Not the first block to free in allocated_list.
	if(parent){
	    parent->next = destination->next; //Remove the block we are freeing.
            //Append the destination block to the free_list.
            if (gap_parent){//The postion we insert in freelist has a parent block.
	        gap_parent->next = destination;
		destination->next = gap;
	    }else{
		freelist = destination;
	        freelist->next = gap;
            }
	
	//Free the header block of allocated_list.	  
	}else{
	    allocated_list = allocated_list->next;//Remove the header.
	    //Insert the block to the free_list.
	    if (gap_parent){
		gap_parent->next = destination;
		destination->next = gap;
	    }else{
		freelist = destination;
		freelist->next = gap;
	    }
        }      
        return 0;
    
    //No block of this addr found.
    }else{
        return -1;
    }
	
}


/* Initialize the memory space used by smalloc,
 * freelist, and allocated_list
 * Note:  mmap is a system call that has a wide variety of uses.  In our
 * case we are using it to allocate a large region of memory. 
 * - mmap returns a pointer to the allocated memory
 * Arguments:
 * - NULL: a suggestion for where to place the memory. We will let the 
 *         system decide where to place the memory.
 * - PROT_READ | PROT_WRITE: we will use the memory for both reading
 *         and writing.
 * - MAP_PRIVATE | MAP_ANON: the memory is just for this process, and 
 *         is not associated with a file.
 * - -1: because this memory is not associated with a file, the file 
 *         descriptor argument is set to -1
 * - 0: only used if the address space is associated with a file.
 */
void mem_init(int size) {
    mem = mmap(NULL, size,  PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if(mem == MAP_FAILED) {
         perror("mmap");
         exit(1);
    }
    //Initialize freelist.
    freelist = malloc(sizeof(struct block));
    freelist->addr = mem;
    freelist->size = size;
    freelist->next = NULL;
    
    //Initialize allocated_list.
    allocated_list = NULL;

}

void mem_clean(){
    //Some healper ptrs for storing and looping.
    struct block *temporary1;
    struct block *free_alloc;
    struct block *free_free;
    
    //Use free_alloc points to alloccated_list in order to free it.
    //Use free_free points to freelist in order to free it.
    free_alloc = allocated_list;
    free_free = freelist;
    
    while(free_alloc != NULL){
        temporary1 = free_alloc;
        free_alloc = free_alloc->next;
        free(temporary1);
    }
    while(free_free != NULL){
        temporary1 = free_free;
        free_free = free_free->next;
        free(temporary1);
    }
}

