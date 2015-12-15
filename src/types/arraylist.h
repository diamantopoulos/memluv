
/**
 * @file   src/global.h
 * @author Dionysios Diamantopoulos(diamantd@microlab.ntua.gr)
 * @date   February 2014
 *
 * @brief  Memluv header file for global variables, i.e. core, freelist etc.
 */

#ifndef _ARRAYLIST_H_
#define _ARRAYLIST_H_

#include <stdbool.h>


struct template_hls_struct
{
    /* -- Extra variables for holding list behaviour -- */
    uint_t index;
    uint_t next;
    uint_t xaxa;
    /* -- Actual list's variables -- */
    int val;
};

typedef struct template_hls_struct node;

node* MemluvCreateList(int val);
void MemluvPrintList(node* nd);
node* MemluvAddToList(node* nd, int val, bool add_to_end);
void MemluvFreeList(node* testnode);

#endif