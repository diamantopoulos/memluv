//----------------------------------------------------------------
// Statically-allocated memory manager
//
// by Eli Bendersky (eliben@gmail.com)
// Adapted by Fabrizio Ferrandi to the PandA Framework (ferrandi.fabrizio@polimi.it)
//  
// This code is in the public domain.
//----------------------------------------------------------------
#include <stdio.h>
//
// Memory manager: dynamically allocates memory from 
// a fixed pool that is allocated statically at link-time.
// the builtin free and malloc have been replaced with functions
// defined in this file. No, include or initialization is needed.
// 
// Preprocessor flags you can define to customize the 
// memory manager:
//
// DEBUG_SAMM_FATAL
//    Allow printing out a message when allocations fail
//
// DEBUG_SAMM_SUPPORT_STATS
//    Allow printing out of stats in function 
//    SAMM_print_stats When this is disabled, 
//    SAMM_print_stats does not exist.
//
// Note that in production code on an embedded system 
// you'll probably want to keep those undefined, because
// they cause printf to be called.
//
// POOL_SIZE
//    Size of the pool for new allocations. This is 
//    effectively the heap size of the application, and can 
//    be changed in accordance with the available memory 
//    resources.
//
// MIN_POOL_ALLOC_QUANTAS
//    Internally, the memory manager allocates memory in
//    quantas roughly the size of two size_t objects. To
//    minimize pool fragmentation in case of multiple allocations
//    and deallocations, it is advisable to not allocate
//    blocks that are too small.
//    This flag sets the minimal ammount of quantas for 
//    an allocation. If the size of a size_t is 4 and you
//    set this flag to 16, the minimal size of an allocation
//    will be 4 * 2 * 16 = 128 bytes
//    If you have a lot of small allocations, keep this value
//    low to conserve memory. If you have mostly large 
//    allocations, it is best to make it higher, to avoid 
//    fragmentation.
//
// Notes:
// 1. This memory manager is *not thread safe*. Use it only
//    for single thread/task applications.
// 

#define POOL_SIZE 8 * 1024
#define MIN_POOL_ALLOC_QUANTAS 16
#define DEBUG_SAMM_SUPPORT_STATS

void* __builtin_malloc(size_t nbytes);
void __builtin_free(void* ap);


//union SAMM_header_union
//{
    struct SAMM_header_struct
    {
        // Pointer to the next block in the free list
        struct SAMM_header_struct* next;
        // Size of the block (in quantas of sizeof(SAMM_header_t))
        size_t size; 
    };

    // Used to align headers in memory to a boundary
    //size_t align_dummy;
//};

//typedef union SAMM_header_union SAMM_header_t;
typedef struct SAMM_header_struct SAMM_header_s;
    
// Initial empty list
//static SAMM_header_t base = {{0,0}};
static SAMM_header_s base = {0,0};

// Start of free list
//static SAMM_header_t* freep = 0;
static SAMM_header_s* freep = 0;

// Static pool for new allocations
static unsigned char SAMM_pool[POOL_SIZE] = {0};
static size_t SAMM_pool_free_pos = 0;


#ifdef DEBUG_SAMM_SUPPORT_STATS
void SAMM_print_stats()
{
    SAMM_header_s* p;

    printf("------ Memory manager stats ------\n\n");
    printf(    "Pool: free_pos = %lu (%lu bytes left)\n\n", 
            SAMM_pool_free_pos, POOL_SIZE - SAMM_pool_free_pos);

    p = (SAMM_header_s*) SAMM_pool;

    while (p < (SAMM_header_s*) (SAMM_pool + SAMM_pool_free_pos))
    {
        printf(    "  * Addr: %8p; Size: %8lu\n",
                p, p->size);

        p += p->size;
    }

    printf("\nFree list:\n\n");

    if (freep)
    {
        p = freep;

        while (1)
        {
            printf(    "  * Addr: %8p; Size: %8lu; Next: %8p\n", 
                    p, p->size, p->next);

            p = p->next;

            if (p == freep)
                break;
        }
    }
    else
    {
        printf("Empty\n");
    }
    
    printf("\n");
}
#endif // DEBUG_SAMM_SUPPORT_STATS


static SAMM_header_s* __hide_get_mem_from_pool(size_t nquantas)
{
    size_t total_req_size;

    SAMM_header_s* h;

    if (nquantas < MIN_POOL_ALLOC_QUANTAS)
        nquantas = MIN_POOL_ALLOC_QUANTAS;

    total_req_size = nquantas * sizeof(SAMM_header_s);

    if (SAMM_pool_free_pos + total_req_size <= POOL_SIZE)
    {
        h = (SAMM_header_s*) (SAMM_pool + SAMM_pool_free_pos);
        h->size = nquantas;
        __builtin_free((void*) (h + 1));
        SAMM_pool_free_pos += total_req_size;
    }
    else
    {
        return 0;
    }

    return freep;
}


// Allocations are done in 'quantas' of header size.
// The search for a free block of adequate size begins at the point 'freep' 
// where the last block was found.
// If a too-big block is found, it is split and the tail is returned (this 
// way the header of the original needs only to have its size adjusted).
// The pointer returned to the user points to the free space within the block,
// which begins one quanta after the header.
//
void* __builtin_malloc(size_t nbytes)
{
    SAMM_header_s* p;
    SAMM_header_s* prevp;
    unsigned int * temp;
    // Calculate how many quantas are required: we need enough to house all
    // the requested bytes, plus the header. The -1 and +1 are there to make sure
    // that if nbytes is a multiple of nquantas, we don't allocate too much
    //
    size_t nquantas = (nbytes + sizeof(SAMM_header_s) - 1) / sizeof(SAMM_header_s) + 1;

    // First alloc call, and no free list yet ? Use 'base' for an initial
    // denegerate block of size 0, which points to itself
    // 
    temp = freep;
    printf ("DEBUG: freep=%8p\n", temp);
    if ((prevp = temp) == 0)
    {
        base.next = freep = prevp = &base;
        base.size = 0;
    }

    for (p = prevp->next; ; prevp = p, p = p->next)
    {
        // big enough ?
        if (p->size >= nquantas) 
        {
            // exactly ?
            if (p->size == nquantas)
            {
                // just eliminate this block from the free list by pointing
                // its prev's next to its next
                //
                prevp->next = p->next;
            }
            else // too big
            {
                p->size -= nquantas;
                p += p->size;
                p->size = nquantas;
            }

            freep = prevp;
            return (void*) (p + 1);
        }
        // Reached end of free list ?
        // Try to allocate the block from the pool. If that succeeds,
        // __hide_get_mem_from_pool adds the new block to the free list and
        // it will be found in the following iterations. If the call
        // to __hide_get_mem_from_pool doesn't succeed, we've run out of
        // memory
        //
        else if (p == freep)
        {
            if ((p = __hide_get_mem_from_pool(nquantas)) == 0)
            {
#ifdef DEBUG_SAMM_FATAL
                printf("!! Memory allocation failed !!\n");
#endif
                return 0;
            }
        }
    }
}


// Scans the free list, starting at freep, looking the the place to insert the 
// free block. This is either between two existing blocks or at the end of the
// list. In any case, if the block being freed is adjacent to either neighbor,
// the adjacent blocks are combined.
//
void __builtin_free(void* ap)
{
    SAMM_header_s* block;
    SAMM_header_s* p;
    
    if(ap==NULL) return;

    // acquire pointer to block header
    block = ((SAMM_header_s*) ap) - 1;

    // Find the correct place to place the block in (the free list is sorted by
    // address, increasing order)
    //
    for (p = freep; !(block > p && block < p->next); p = p->next)
    {
        // Since the free list is circular, there is one link where a 
        // higher-addressed block points to a lower-addressed block. 
        // This condition checks if the block should be actually 
        // inserted between them
        //
        if (p >= p->next && (block > p || block < p->next))
            break;
    }

    // Try to combine with the higher neighbor
    //
    if (block + block->size == p->next)
    {
        block->size += p->next->size;
        block->next = p->next->next;
    }
    else
    {
        block->next = p->next;
    }

    // Try to combine with the lower neighbor
    //
    if (p + p->size == block)
    {
        p->size += block->size;
        p->next = block->next;
    }
    else
    {
        p->next = block;
    }

    freep = p;
}





char *malloc_ptr=NULL;
void *moufa_malloc(int size)
{
  void *ret;
  ret = (void*)malloc_ptr;
  malloc_ptr += size;
  return ret;
}
