
/**
 * @file   src/malloc.c
 * @author Dionysios Diamantopoulos(diamantd@microlab.ntua.gr)
 * @date   December 2013
 *
 * @brief   Memluv malloc-related functions.
 */

#include "malloc.h"
#include "memluv.h"
#include "util.h"
#include "freelist.h"

uint_t MemluvFirstFit(MemLuvCore *CurMemLuvCore, uint_t nbytes) {
#ifdef __SYNTHESIS__
#if MEMLUV_HLS_INLINE == 0
#pragma AP inline off
#else
#pragma AP inline
#endif
#endif
#if MEMLUV_FIRST_FIT_POLICY==0
    uint_t free_bins=0, cand_base=0;
    uint8_t first_visit=0;
#if MEMLUV_FIRST_FIT_CACHE_LAST_BIN==1
    CurMemLuvCore->ibitpos=CurMemLuvCore->ff_cache_bin_in+1;
    Dfprintf(3, CurMemLuvConf->dbg_fd, "DEBUG: In malloc start CurMemLuvCore->ff_cache_bin_in =%u\n",
             CurMemLuvCore->ff_cache_bin_in);
#else
    CurMemLuvCore->ibitpos=1;
#endif
    
    // 0x1: Go to a while loop to check every sinlge bit if it is candidate for allocation
    
    while (CurMemLuvCore->ibitpos<=CurMemLuvCore->freelist_total_bits) {
        Dfprintf(3, CurMemLuvConf->dbg_fd, "DEBUG: Checking bit #%d (free-pos %d)...",
                 CurMemLuvCore->ibitpos, CurMemLuvCore->ibitpos-1);
        
            // 0x2: Check this bit
        
            if (MemluvTestBitFreelist(CurMemLuvCore->freelist, CurMemLuvCore->ibitpos-1) == 0) {
                //bprintf(" FREE ");
                
                // 0x3: If this bit is the first free found on freelist, start count free positions
                
                if (first_visit == 0) {
                    cand_base = CurMemLuvCore->ibitpos-1;
                    first_visit = 1;
                    Dfprintf(3, CurMemLuvConf->dbg_fd, " FIRST ");
                }
                free_bins++;
            }
            
            // 0x4: If current bit is reserved, cancel the count of free positions up to now
            
            else {
                Dfprintf(3, CurMemLuvConf->dbg_fd, " RESERVED ");
                    free_bins = 0;
                    cand_base = 0;
                    first_visit = 0;
            }
            Dfprintf(3, CurMemLuvConf->dbg_fd, " cand_base=%u free_bins=%u\n", cand_base, free_bins);
            
            // 0x5: Check if free bins found up to now are equivalent to the bins requested
            
            if (free_bins == nbytes + MEMLUV_ALLOC_HEADER*sizeof(CORE_UINT_T)) {
                Dfprintf(2, CurMemLuvConf->dbg_fd, 
                         "DEBUG: Found %u free bins starting on position %u, ibitpos=%u" 
                         "ReqPadding(ibitpos)-1=%u\n", 
                         free_bins, cand_base, CurMemLuvCore->ibitpos, 
                         ReqPadding(CurMemLuvCore->ibitpos)-1);
                
                    // 0x6: If we found requested bins, make padding according to alingment
                
                    CurMemLuvCore->ibitpos_pad = ReqPadding(CurMemLuvCore->ibitpos)-1;
                    Dfprintf(3, CurMemLuvConf->dbg_fd, "DEBUG: Marking as RESERVED bins %u to %u ",
                             cand_base, CurMemLuvCore->ibitpos_pad);
                    
                    // 0x7: Mark requested free bins as reserved in freelist, malloc succceeded
                    
                    MemluvSetFreelist(CurMemLuvCore->freelist, cand_base, CurMemLuvCore->ibitpos_pad);
#if MEMLUV_FIRST_FIT_CACHE_LAST_BIN==1
                    CurMemLuvCore->ff_cache_bin_in = CurMemLuvCore->ibitpos;
                    Dfprintf(3, CurMemLuvConf->dbg_fd, 
                             "DEBUG: In malloc success CurMemLuvCore->ff_cache_bin_in =%u\n",
                             CurMemLuvCore->ff_cache_bin_in);
#endif
                    CurMemLuvCore->action_status = MALLOC_SUCCESS;
                    
                    // 0x8: Return the position of first free bin, to allocator
                    
                    return cand_base;
            }
            
            // 0x9: Check next bin
            
            CurMemLuvCore->ibitpos++;
    }
    
    /* 0xa: If never returned on step 0x8, and freelist was scanned entirely, there are no 
     *      continuous free bins for current request, thus allocation failed.
     */    
    
    Dfprintf(2, CurMemLuvConf->dbg_fd, 
             "DEBUG: Couldn't found %u+%u(for header) continuous free bins on freelist. "
             "Found only %u.\n", nbytes, MEMLUV_ALLOC_HEADER*sizeof(CORE_UINT_T), free_bins);
    CurMemLuvCore->action_status = MALLOC_FAIL;
    CurMemLuvCore->ibitpos_pad = 0; /* FIXME: THis means that the current allocated value will 
    be 0+1-0=1 byte, which is false, since no byte is reseved. Maybe use signed type? */
    return (0);

#elif MEMLUV_FIRST_FIT_POLICY==1
    var_type_t free_bins=0, cand_bin_start=0, cand_bin_end, i;
    var_type_t start_reg, end_reg;
    var_type_t start_pos_in_reg, end_pos_in_reg;
    FREELIST_UINT_T mask, zero=0; 
    FREELIST_UINT_TD flag=1; /* NOTE: Double the size of FREELIST_UINT_T, 
    since we need to calculate the number 2^FREELIST_WIDTH */
    uint8_t search_footer=0;
#if MEMLUV_FIRST_FIT_CACHE_LAST_BIN==1
    CurMemLuvCore->ibitpos=CurMemLuvCore->ff_cache_bin_in+1;
    Dfprintf(3, CurMemLuvConf->dbg_fd, "DEBUG: In malloc start CurMemLuvCore->ff_cache_bin_in =%u\n",
             CurMemLuvCore->ff_cache_bin_in);
#else
    CurMemLuvCore->ibitpos=1;
#endif

    // 0x1: Go to a while loop to check every sinlge bit if it is candidate for allocation

    while (CurMemLuvCore->ibitpos<=CurMemLuvCore->freelist_total_bits) {
        
        /* 0x2: Given this bit, check: i)   the last bit based on current request (bytes + header)
         *                             ii)  the registers that start and last bit are indexed
         *                             iii) the position in these registers
         */    
        
        cand_bin_start=CurMemLuvCore->ibitpos-1;
        cand_bin_end=(var_type_t)(cand_bin_start+nbytes+MEMLUV_ALLOC_HEADER*sizeof(CORE_UINT_T)-1);
        Dfprintf(3, CurMemLuvConf->dbg_fd, "DEBUG: Checking bit #%u (free-pos %u-%u)...\n", 
                 CurMemLuvCore->ibitpos, cand_bin_start, cand_bin_end);

        start_reg = (FREELIST_UINT_T)(cand_bin_start/(sizeof(FREELIST_UINT_T)*8));
        end_reg = (FREELIST_UINT_T)(cand_bin_end/(sizeof(FREELIST_UINT_T)*8));
        start_pos_in_reg = cand_bin_start%(sizeof(FREELIST_UINT_T)*8);
        end_pos_in_reg = cand_bin_end%(sizeof(FREELIST_UINT_T)*8);
        Dfprintf(3, CurMemLuvConf->dbg_fd, "DEBUG: Inside MemluvFirstFit: cand_bin_start=%u, "
        "cand_bin_end=%u, start_reg=%u, end_reg=%u, start_pos_in_reg=%u, end_pos_in_reg=%u\n",
        cand_bin_start, cand_bin_end, start_reg, end_reg, start_pos_in_reg, end_pos_in_reg);
        free_bins=0;
        
        // 0x3: Check if requested continuous bins from cand_bin_start to cand_bin_end are free

        switch ( end_reg - start_reg ) {
            
        /* 0x4: If the request affects bits of the same register, e.g. check bits 5-13
         * 
         *      Register 0: MSB=15 [xxxx xxxx xxxx xxxx] LSB=0  (check directly these bits)
         *                             ^         ^
         *      we create a mask with zeros on the space MSB[cand_bin_end,cand_bin_start]LSB and we 
         *      check for free positions by ANDIng this mask with the inverted freelist register 
         *      (since we check for zeros)
         */
        
        case 0:
            mask = (FREELIST_UINT_T)((((flag<<(end_pos_in_reg+1))-1) - ((flag<<start_pos_in_reg)-1)));
            Dfprintf(3, CurMemLuvConf->dbg_fd, "DEBUG: mask=%u, checking if equal to "
                     "(~CurMemLuvCore->freelist[start_reg] & mask)=%u \n", 
                     mask, ~CurMemLuvCore->freelist[start_reg] & mask);
            if ((~CurMemLuvCore->freelist[start_reg] & mask) == mask) {
                CurMemLuvCore->ibitpos=cand_bin_end+1;
                free_bins=(var_type_t)(nbytes + MEMLUV_ALLOC_HEADER*sizeof(CORE_UINT_T));
            }
            break;
            
        /* 0x5: If the request affects bits of different registers, e.g. check bits 5-42
         * 
         *      Register 0: MSB=15 [xxxx xxxx xxxx xxxx] LSB=0   (i: check bits 5-15)
         *                                       ^
         *      Register 1: MSB=31 [xxxx xxxx xxxx xxxx] LSB=16  (ii: check bits 16-31, entire Reg.1)
         * 
         *      Register 2: MSB=47 [xxxx xxxx xxxx xxxx] LSB=32  (iii: check bits 32-42)
         *                                 ^
         *      We do the job in three steps: see i,ii,iii below
         */
         
        default:
            search_footer=0;
            mask = (FREELIST_UINT_T)(((~zero)) - ((flag<<start_pos_in_reg)-1));
            Dfprintf(3, CurMemLuvConf->dbg_fd, "DEBUG: start mask=%u\n", mask);
            
            /* 0x5 i)  Create a mask with zeros on the space [MSB,cand_bin_start] and check for 
             *         free positions by ANDIng this mask with the inverted freelist register 
             *         (since we check for zeros)
             */
            if  ((~CurMemLuvCore->freelist[start_reg] & mask) == mask) {
                free_bins = FREELIST_WIDTH - start_pos_in_reg;
                search_footer=1; 
                /* continue to footer in case the request is not big enough to search for 
                 * intermediate registers 
                 */
                Dfprintf(3, CurMemLuvConf->dbg_fd, "DEBUG: Header is ok. free_bins=%u\n", free_bins);
                
                /* 0x5 ii) Check if intermediate registers (start_reg+1,end_reg) are zero (free). 
                 *         This step may be skipped if the request affects bits of continuous 
                 *         registers (i.e. 23,24)
                 */
                
                for (i=start_reg+1; i<end_reg; i++) {
                    if  (CurMemLuvCore->freelist[i] == zero) {
                        free_bins+=FREELIST_WIDTH;
                        Dfprintf(3, CurMemLuvConf->dbg_fd, "DEBUG: Inter reg %u is free. free_bins=%u\n",
                                 i, free_bins);
                        search_footer=1;
                    }
                    else {
                        /* TODO: If we need the exact value of free_bins, we have to decrease 
                         * free_bins according to what register-i contains from LSB to first 1
                         */
                        search_footer=0;
                        break; /*NOTE: It seems to be synthesizable with Vivado 2013.4 */
                        }
                }
                
                /* 0x5 iii) Create a mask with zeros on the space [cand_bin_end,LSB] and check for 
                 *          free positions by ANDIng this mask with the inverted freelist register 
                 */
                
                if (search_footer == 1) {
                    mask = (FREELIST_UINT_T)(((flag<<(end_pos_in_reg+1))-1));
                    Dfprintf(3, CurMemLuvConf->dbg_fd, "DEBUG: end mask=%u\n", mask);
                    if ((~CurMemLuvCore->freelist[end_reg] & mask) == mask) {
                        free_bins += end_pos_in_reg+1;
                        CurMemLuvCore->ibitpos=cand_bin_end+1;
                        Dfprintf(3, CurMemLuvConf->dbg_fd, "DEBUG: Footer is ok. free_bins=%u\n",
                                 free_bins);
                    }
                }
            }
            break;
        }
        Dfprintf(3, CurMemLuvConf->dbg_fd, "DEBUG: Finally for bin cand_bin_start=(ibitpos-1)=%u "
                 "free_bins=%u\n", cand_bin_start, free_bins);

        // 0x6: Check if free bins found up to now are equivalent to the bins requested

        if (free_bins == nbytes + MEMLUV_ALLOC_HEADER*sizeof(CORE_UINT_T)) {
            Dfprintf(2, CurMemLuvConf->dbg_fd, "DEBUG: Found %u free bins starting on position %u, "
                     "ibitpos=%u ReqPadding(ibitpos)-1=%u\n", free_bins, cand_bin_start, 
                     CurMemLuvCore->ibitpos, ReqPadding(CurMemLuvCore->ibitpos)-1);
            
            // 0x7: If we found requested bins, make padding according to alingment
            
            CurMemLuvCore->ibitpos_pad = ReqPadding(CurMemLuvCore->ibitpos)-1;
            Dfprintf(3, CurMemLuvConf->dbg_fd, "DEBUG: Marking as RESERVED bins %u to %u ", 
                     cand_bin_start, CurMemLuvCore->ibitpos_pad);
#if MEMLUV_FIRST_FIT_CACHE_LAST_BIN==1
            Dfprintf(3, CurMemLuvConf->dbg_fd, "DEBUG: In malloc CurMemLuvCore->ff_cache_bin_in =%u,"
                     " cand_bin_start=%u, CurMemLuvCore->ibitpos_pad=%u\n",
                     CurMemLuvCore->ff_cache_bin_in, cand_bin_start, CurMemLuvCore->ibitpos_pad);
            if (CurMemLuvCore->ibitpos_pad>CurMemLuvCore->ff_cache_bin_in)
                CurMemLuvCore->ff_cache_bin_in = CurMemLuvCore->ibitpos_pad;
                Dfprintf(3, CurMemLuvConf->dbg_fd, "DEBUG: In malloc CurMemLuvCore->ff_cache_bin_in=%u,"
                         " cand_bin_start=%u, CurMemLuvCore->ibitpos_pad=%u\n",
                         CurMemLuvCore->ff_cache_bin_in, cand_bin_start, CurMemLuvCore->ibitpos_pad);
#endif
                // 0x8: Mark requested free bins as reserved in freelist, malloc succceeded
                
                MemluvSetFreelist(CurMemLuvCore->freelist, cand_bin_start, CurMemLuvCore->ibitpos_pad);
                CurMemLuvCore->action_status = MALLOC_SUCCESS;
                
                // 0x9: Return the position of first free bin, to allocator
               
                return cand_bin_start;
        }
        
        // 0xa: Check next bin
        
        CurMemLuvCore->ibitpos++;
    }
    
    /* 0xb: If never returned on step 0x9, and freelist was scanned entirely, there are no 
     *      continuous free bins for current request, thus allocation failed.
     */    
        
    Dfprintf(2, CurMemLuvConf->dbg_fd, "DEBUG: Couldn't found %u+%u(for header) continuous free "
             "bins on freelist. Found only %u.\n", nbytes, MEMLUV_ALLOC_HEADER*sizeof(CORE_UINT_T),
             free_bins);
    CurMemLuvCore->action_status = MALLOC_FAIL;
    CurMemLuvCore->ibitpos_pad = 0; /* FIXME: THis means that the current allocated value will be 
                                     * 0+1-0=1 byte, which is false, since no byte is reseved. 
                                     * Maybe use signed type? 
                                     */
    return (0);
#else
    Dprintf(0, "ERROR: Unsupported MEMLUV_FIRST_FIT_POLICY=%u on src/memluv.h. Aborting...\n\n", 
            MEMLUV_FIRST_FIT_POLICY);
    exit(-1);
#endif
}


uint_t MemluvFit(MemLuvCore *CurMemLuvCore, uint_t nbytes) {
#ifdef __SYNTHESIS__
#if MEMLUV_HLS_INLINE == 0
#pragma AP inline off
#else
#pragma AP inline
#endif
#endif
#if MEMLUV_FIT_ALGORITHM==0
    return (MemluvFirstFit(CurMemLuvCore, nbytes));
#else
    Dprintf(0, "ERROR: Unsupported MEMLUV_FIT_ALGORITHM=%u on src/memluv.h. Aborting...\n\n", 
            MEMLUV_FIT_ALGORITHM);
    exit(-1);
#endif
}


/* MemluvAlloc wrapper is automated generated by gen_memluv_wrappers.c, included in global.c */
/* void* MemluvAlloc(uint_t nbytes, uint_t heap_id) */

void MemluvAllocUpdateStas      (
#if HW_DEBUG_MEMLUV==1
                                    MemLuvCore *CurMemLuvCore, uint_t nbytes, uint_t allocated
#endif
                                ) {
#ifdef __SYNTHESIS__
#if MEMLUV_HLS_INLINE == 0
#pragma AP inline off
#else
#pragma AP inline
#endif
#endif
#if HW_DEBUG_MEMLUV==1
    if (CurMemLuvCore->action_status == MALLOC_SUCCESS) {
        
        CurMemLuvCore->stats.total_bytes_requested += nbytes;
        
        CurMemLuvCore->stats.total_bytes_allocated += allocated; /* Since 1bit in freelist bitmap 
                                                                  * represents 1 byte 
                                                                  */
                                                                  
        CurMemLuvCore->stats.used_percentage = 100*CurMemLuvCore->stats.total_bytes_allocated /
                                               CurMemLuvCore->size;
                                               
        CurMemLuvCore->stats.max_address_reached = 0;
        
        CurMemLuvCore->stats.total_bytes_for_headers += (uint_t)(MEMLUV_ALLOC_HEADER *
                                                        sizeof(CORE_UINT_T));
        
        CurMemLuvCore->stats.total_fragmented_bytes = CurMemLuvCore->stats.total_bytes_allocated - 
                                                      CurMemLuvCore->stats.total_bytes_requested - 
                                                      CurMemLuvCore->stats.total_bytes_for_headers;
                                                      
        CurMemLuvCore->stats.total_addresses_allocated = CurMemLuvCore->stats.total_bytes_allocated / 
                                                         (sizeof(CORE_UINT_T));

#if DEBUG_REPORT_FRAGMENTATION==1
        CurMemLuvCore->stats.total_request_fragmented_bytes = MemluvReportReqFragBytes(CurMemLuvCore);    
#endif                                                         
                                                         
    }
    
    else if (CurMemLuvCore->action_status == MALLOC_FAIL) {
        
#if CHCK_MALLOC_FAIL_DUE_FRAGMENTATION==1
        /* Check if fail is caused due to fragmentation and not in lack of free space */
        if ((CurMemLuvCore->stats.total_bytes_allocated + allocated) <= CurMemLuvCore->size )
            CurMemLuvCore->action_status = MALLOC_FAIL_DUE_FRAGMENTATION;
#endif
        CurMemLuvCore->stats.total_bytes_requested += nbytes;
        //CurMemLuvCore->stats.total_bytes_allocated=0;
        //CurMemLuvCore->stats.used_percentage=100; /* Use the previous */ 
        CurMemLuvCore->stats.max_address_reached = 1;
        //CurMemLuvCore->stats.total_fragmented_bytes=0;
        //MCurMemLuvCore->stats.total_addresses_allocated
    }
#endif
}

#if SIM_WITH_GLIBC_MALLOC==0
void* CurMemluvAlloc(MemLuvCore *CurMemLuvCore, uint_t nbytes) {
#ifdef __SYNTHESIS__
/* NOTE: AP inline directive due to Vivado Error SYNCHK-75:
 * "function 'MemluvAlloc1' returns a pointer value; please inline this function or change return
 * to an argument."
 */
#pragma AP inline
#endif
    //QueryLock(CurMemLuvCore);
    var_type_t *addr=NULL, *returned;
    uint_t cand_base; 
#if HW_DEBUG_MEMLUV==1
    uint_t allocated=0;
#endif
    Dfprintf(2, CurMemLuvConf->dbg_fd, "\n/---------------- ALLOC [heap %d] -----------------\\\n", 
             CurMemLuvCore->id);
    CurMemLuvCore->alloc_rqst++; 
    CurMemLuvCore->tot_rqst++;
    Dfprintf(2, CurMemLuvConf->dbg_fd, "DEBUG: Alloc Request %u: %u bytes (Total Request %u)\n", 
             CurMemLuvCore->alloc_rqst, nbytes, CurMemLuvCore->tot_rqst);
    
    /* 0x1: Find a valid position on freelist of continuous nbytes, given a fit algorithm. 
     *      Bytes for headers are computed internally in these algorithms.
     */
    
    cand_base = MemluvFit(CurMemLuvCore, nbytes);
#if HW_DEBUG_MEMLUV==1
    allocated=CurMemLuvCore->ibitpos_pad+1-cand_base;
    Dfprintf(2, CurMemLuvConf->dbg_fd, "DEBUG: CurMemLuvCore->stats.total_bytes_allocated + "
             "allocated = %u+%u=%u\n", CurMemLuvCore->stats.total_bytes_allocated, allocated, 
             CurMemLuvCore->stats.total_bytes_allocated + allocated );
#endif
    if (CurMemLuvCore->action_status == MALLOC_SUCCESS) {
        
        /* 0x2: If malloc succceeded, make padding of address according to alingment and do 
         *      some statistics 
         */
        
        CurMemLuvCore->aligned = ReqPadding(cand_base);
        CurMemLuvCore->aligned_pad = CurMemLuvCore->aligned>>(CurMemLuvCore->log_align);
        addr = (((var_type_t*)CurMemLuvCore->base) + CurMemLuvCore->aligned_pad);
#if HW_DEBUG_MEMLUV==1
        MemluvAllocUpdateStas(CurMemLuvCore, nbytes, allocated);
        Dfprintf(2, CurMemLuvConf->dbg_fd, "DEBUG: >> [OK] Addresses: %p-%p\taligned=%u "
                 "log_align=%d\n\t[Core usage: %u%% (%u out of %u B)] [Treq. %u B] "
                 "[Addresses used %u out of %u] [Frag. %u] B\n", 
                 addr, addr +((nbytes/sizeof(cand_base))>>(CurMemLuvCore->log_align)), 
                 CurMemLuvCore->aligned, CurMemLuvCore->log_align, 
                 CurMemLuvCore->stats.used_percentage, CurMemLuvCore->stats.total_bytes_allocated, 
                 CurMemLuvCore->size, CurMemLuvCore->stats.total_bytes_requested, 
                 CurMemLuvCore->stats.total_addresses_allocated, CurMemLuvCore->depth, 
                 CurMemLuvCore->stats.total_fragmented_bytes); /* FIXME: fix the last address 
                                                                * (2nd %p) as previously mentioned 
                                                                */
#endif
    }
    else {
        // 0x3: If malloc failed, return the address of base heap
        
        addr = CurMemLuvCore->base; // NOTE: Causes memory core overwrite from address 0x00...0
#if HW_DEBUG_MEMLUV==1
        MemluvAllocUpdateStas(CurMemLuvCore, nbytes, allocated);
        Dfprintf(2, CurMemLuvConf->dbg_fd, "DEBUG: >> [FAIL] addr=%p top=%p Core usage:%u%% "
                 "[%u out of %u]\n", addr, CurMemLuvCore->top, CurMemLuvCore->stats.used_percentage, 
                 CurMemLuvCore->stats.total_bytes_allocated, CurMemLuvCore->size);
#endif
    }
    
    // 0x4: The address is increased by MEMLUV_ALLOC_HEADER positions if we use header.
    
    returned = addr + MEMLUV_ALLOC_HEADER; /* addr is increased ALIGNMENT times for every time of 
                                            * MEMLUV_ALLOC_HEADER due pointer arithmetic 
                                            */
    Dfprintf(2, CurMemLuvConf->dbg_fd, "DEBUG: Returned pointer: %p\n", returned);
#if MEMLUV_ALLOC_HEADER > 0
    
    /* 0x5: If we use header, write to the first two addresses the length of the requested 
     *      allocation and the index of the requested allocation on freelist bitmap, respectively
     */
    
    Dfprintf(2, CurMemLuvConf->dbg_fd, "DEBUG: ...and i will write CurMemLuvCore->core[%u] = %u  "
             "CurMemLuvCore->core[%u] = %u\n", cand_base>>(CurMemLuvCore->log_align), nbytes, 
             (cand_base>>CurMemLuvCore->log_align)+1, (CORE_UINT_T)cand_base);
        /* The first chunk of length sizeof(CORE_UINT_T) stores the length of the requested allocation */
        CurMemLuvCore->core[(cand_base>>CurMemLuvCore->log_align)] = (CORE_UINT_T)(nbytes); 
        /* The second chunk of length sizeof(CORE_UINT_T) stores the index of the requested 
         * allocation on freelist bitmap 
         */
        CurMemLuvCore->core[(cand_base>>CurMemLuvCore->log_align)+1] = (CORE_UINT_T)cand_base; 
#endif
    //MemluvDumpFreeList(CurMemLuvCore, THIS);
    DumpStatisticsToFile(CurMemLuvCore
#if USE_MEMLUV==1 && HW_DEBUG_MEMLUV==1 
                                        ,1
                                        ,allocated
#endif
                        );
    Dfprintf(2, CurMemLuvConf->dbg_fd, "\\-------------------------------------------------/\n");
    //QueryUnLock(CurMemLuvCore);
    
    // 0x6: Return the address in the form of a generic (void*) pointer.
    
    return ((void*)returned);
}
#else
    void * CurMemluvAlloc(MemLuvCore *CurMemLuvCore, uint_t nbytes) {
        return malloc(nbytes);
    }
#endif

void* MemluvMemcpyBytes(void* destination, void* source, uint_t bytes) {
#ifdef __SYNTHESIS__
#pragma AP inline
#endif
uint_t i;
uint8_t* d = destination;
uint8_t* s = source;
for (i = 0; i < bytes; i++)
    d[i] = s[i];
return destination;
}

void* MemluvMemcpyWords(void* destination, void* source, uint_t words) {
#ifdef __SYNTHESIS__
#pragma AP inline
#endif
uint_t i;
CORE_UINT_T* d = destination;
CORE_UINT_T* s = source;
for (i = 0; i < words; i++)
    d[i] = s[i];
return destination;
}