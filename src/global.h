
/**
 * @file   src/global.h
 * @author Dionysios Diamantopoulos(diamantd@microlab.ntua.gr)
 * @date   February 2014
 *
 * @brief  Memluv header file for global variables, i.e. core, freelist etc.
 */

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "memluv.h"
#include "types/arraylist.h"

#ifndef __SYNTHESIS__
#include <sys/types.h>
#include <unistd.h>
#endif

struct MemLuvConf
{
uint8_t num_hw_heaps;
FILE *dbg_fd;
#ifndef __SYNTHESIS__
pid_t pid;
#endif
};

struct MemLuvStats
{
uint_t max_address_reached;
uint_t result;
uint_t used_percentage;
uint_t total_bytes_requested;
uint_t total_bytes_allocated;
uint_t total_addresses_allocated;
uint_t total_fragmented_bytes;
uint_t total_bytes_for_headers;
uint_t total_request_fragmented_bytes;
uint8_t lock;
};

struct MemLuvCore
{
// Static buffer for new allocations
CORE_UINT_T core[MEMLUV_DEPTH];
uint8_t id;
uint_t freelist_depth;
uint_t freelist_total_bits;
FREELIST_UINT_T freelist[MEMLUV_DEPTH*sizeof(CORE_UINT_T)/(sizeof(FREELIST_UINT_T)*8)];     // 1bit represents 1 distinct memory location

uint_t size;
uint_t depth;
var_type_t *base;
var_type_t *top; //= (void*)(&core+MEMLUV_DEPTH);

uint_t aligned, aligned_pad;
uint8_t log_align;
uint16_t alloc_rqst, free_rqst, tot_rqst;
uint_t ibitpos, ibitpos_pad;
memluv_action_status_t action_status;
#ifndef __SYNTHESIS__
FILE *fd;
char filename[64];
unsigned int file_init;
#endif
#if HW_DEBUG_MEMLUV==1
MemLuvStats stats;
#endif
#if MEMLUV_FIRST_FIT_CACHE_LAST_BIN
uint_t ff_cache_bin_in;
#endif
};

MemLuvStats ReturnMemLuvStats(void);
MemLuvCore *ReturnMemLuvCore(void);

#endif