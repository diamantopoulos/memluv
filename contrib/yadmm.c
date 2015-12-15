#include <stdio.h>
#include <string.h>

#include "memluv.c"


typedef
struct
{
    int is_available;
    int size;
} MCB, *MCB_P;


char *mem_start_p=NULL;
int max_mem;
int allocated_mem; /* this is the memory in use. */
int mcb_count;

const char *heap_end;

//MCB_P memallocate(MCB_P ,int );

enum {NEW_MCB=0,NO_MCB,REUSE_MCB};
enum {FREE,IN_USE};
void
InitMem(char *ptr, int size_in_bytes)
{
    /* store the ptr and size_in_bytes in global variable */
    int i;
    max_mem = size_in_bytes;
    mem_start_p = ptr;
    mcb_count = 0;
    allocated_mem = 0;
    heap_end = mem_start_p + size_in_bytes;
    //memset(mem_start_p,0x00,max_mem);
    for (i=0;i<1024;i++)
	    mem_start_p[i]=0x00;    
    
    
    /* This function is complete :-) */

}

void *
myalloc(int elem_size)
{
    /* check whether any chunk (allocated before) is free first */
    MCB_P p_mcb;
    int flag = NO_MCB;

    p_mcb = (MCB_P)mem_start_p;

    int sz;

    sz = sizeof(MCB);

    if( (elem_size + sz)  > (max_mem - (allocated_mem + mcb_count * sz ) ) )
    {
        printf("Max size Excedded!!!!!");
        return NULL;
    }
    while( heap_end > ( (char *)p_mcb + elem_size + sz)   )
    {

        if ( p_mcb->is_available == 0)
        {

            if( p_mcb->size == 0)
            {
                flag = NEW_MCB;
                break;
            }
            if( p_mcb->size > (elem_size + sz) )
            {
                flag = REUSE_MCB;
                break;
            }
        }
        p_mcb = (MCB_P) ( (char *)p_mcb + p_mcb->size);


    }

    if( flag != NO_MCB)
    {
        p_mcb->is_available = 1;

        if( flag == NEW_MCB)
        {
            p_mcb->size = elem_size + sizeof(MCB);
        }
        else if( flag == REUSE_MCB) {
	    elem_size = p_mcb->size - sizeof(MCB);
	}
        mcb_count++;
        allocated_mem += elem_size;
        return ( (char *) p_mcb + sz);
    }

    printf(" Returning as we could not allocate any MCB \n");
    return NULL;


    /* if size of the available chunk is equal to greater than required size, use that chunk */


}

int
MemEfficiency()
{
    /* keep track of number of MCBs in a global variable */
    return mcb_count;
    /* This function is complete as well. :-) */

}

void
myfree(void *p)
{
    /* Mark in MCB that this chunk is free */
    MCB_P ptr = (MCB_P)p;
    ptr--;

    printf("\nAllocated mem: %d ",ptr->size);
    if (ptr->is_available != FREE) {
	mcb_count--;
	ptr->is_available = FREE;
	allocated_mem -= (ptr->size - sizeof(MCB));
	printf("\nAllocated mem: %d ",allocated_mem);
	printf("\nMemory Freed...");
    }
}


int main()
{
    char buf[1024];
    memset(buf,0,1024);

    InitMem(buf,1024);

    char *str,*str1;

    str=myalloc(100);
    printf("\nMemory address: %p",str);
    printf("\nMCB count: %-3d \tAllocated Memory: %-10d",mcb_count,allocated_mem);
    myfree(str);
    str1=myalloc(200);
    printf("\n\nMemory address: %p",str1);
    printf("\nMCB count: %-3d \tAllocated Memory: %-10d\n",mcb_count,allocated_mem);
}