
/**
 * @file   src/types/arraylist.c
 * @author Dionysios Diamantopoulos(diamantd@microlab.ntua.gr)
 * @date   October 2014
 *
 * @brief  Memluv support for list data type: Array of lists
 */

#include "../memluv.h"
#include "../global.h"
#include "arraylist.h"

/*
 *   Basic strategy to convert a list data type to an hls synthesizable object (array of ptr structs)
 *
 *   A) Lists are structs which contain a member that is a pointer to a struct, i.e. next node.
 *      This way every new element may be dynamically allocated at run time with no static space 
 *      overhead, in a way that chains of nodes are formulated.
 * 
 *   ------------- *next -------------
 *   |   Node A  |------>|  Node B   |
 *   -------------       -------------
 * 
 *   B) We instantiate an array of struct pointers, so that every element of the array holds the 
 *      address (pointer) of a node. 
 *      Each node has two extra fields : i)  An index indicating the index in array
 *                                       ii) A next variable indicating the index of next node
 *      It is not necessery that (next=index-1). We allow every node ptr to be anywhere in the array,
 *      so that not restricting lists operations.
 * 
 *      -------------------------------------      
 *  i=0 |<Indicating end of list, like NULL>|      ------------
 *  i=1 |       ptr* to Node A              | ---> |  Node A  |  [index=1, next=4] (Header of list)
 *  i=2 |       ptr* to other Node          |      ------------
 *  i=3 |       ptr* to other Node          |      ------------
 *  i=4 |       ptr* to Node B              | ---> |  Node B  |  [index=4, next=0] (Tail of list)
 *      .       un    un    un    un        .      ------------
 *      .         us    us    us    us      .      
 *  i=N |           ed    ed    ed    ed    |
 *      -------------------------------------
 *      
 *      Aforementioned implementation statically allocates an array of N-pointers to struct,
 *      thus in a 32-bit x86 machine it translates to N*4 bytes or N*8 on 64-bit.
 */




// static node *array_list;
        
uint_t head = 0;
static uint_t curr = 0;

node * MemluvCreateList(int val) {
#ifdef __SYNTHESIS__                                        
#pragma AP inline                                               
#endif
node *array_list = (node *)MemluvAlloc(10*sizeof(node), 0);

    
    /* First creating NULL-emulation node */
//     node *ptr = (node *)MemluvAlloc(sizeof(node), 0);
    array_list[curr].index = curr;
    array_list[curr].val = 0;
//     array_list[ptr->index] = ptr;
    array_list[curr].next = 0;
    head = array_list[curr].index;
    curr++;
    //MemluvFree((uint_t*)array_list, 1*sizeof(node), 0);
    //array_list = (node *)MemluvAlloc(2*sizeof(node), 0);
    
    /* Then creating the first node of the list */
//     ptr = (node *)MemluvAlloc(sizeof(node), 0);
    array_list[curr].index = curr;
    array_list[curr].val = val;
//     array_list[ptr->index] = ptr;
    array_list[curr].next = 0;
    head = curr = array_list[curr].index;
    printf("TB-DEBUG-IN: New list headnode on addr %p, indexed on position %u\n", (void*)(array_list+curr), array_list[curr].index);
    return array_list;
}

void MemluvPrintList(node* nd) {
#ifndef __SYNTHESIS__
    node *ptr = &nd[head];

    printf("\n -------Printing list Start------- \n");
    while(ptr->index != 0)
    {
        printf("\nNode id %u: val = [%d], ptr->next=%u\n", ptr->index, ptr->val, ptr->next);
        ptr = &nd[ptr->next];
    }
    printf("\n -------Printing list End------- \n");

    return;
#endif
}


node* MemluvAddToList(node* testnode, int val, bool add_to_end) {
#ifdef __SYNTHESIS__                                        
#pragma AP inline                                               
#endif
//     if(0 == head)
//     {
//         return (MemluvCreateList(val));
//     }

    curr++;
    testnode[curr].val = val;
    
     if(add_to_end)
     {    
        printf("\n Adding node to end of list with value [%d]\n",val);
        testnode[curr-1].next=curr;
        testnode[curr].index=curr;
        testnode[curr].next = 0;
     }
     testnode=MemluvMemcpyBytes(testnode+5, testnode, sizeof(node)*3);
     //testnode=MemluvMemcpyWords(testnode+5, testnode, 3);

//     else
//         printf("\n Adding node to beginning of list with value [%d]\n",val);
//     {
//         ptr->next = head;
//         head = ptr;
//     }
    return testnode;
}


void MemluvFreeList(node* testnode) {
#ifdef __SYNTHESIS__                                        
#pragma AP inline                                               
#endif  
    MemluvFree((uint_t*)testnode, 10*sizeof(node), 0);
}

        
        