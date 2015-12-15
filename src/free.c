
/**
 * @file   src/free.c
 * @author Dionysios Diamantopoulos(diamantd@microlab.ntua.gr)
 * @date   December 2013
 *
 * @brief   Memluv free-related functions.
 */

#include "memluv.h"
#include "util.h"
#include "free.h"

uint_t MemluvCalcPtrDistanceFromBase(MemLuvCore *CurMemLuvCore, CORE_UINT_T *ptr) {
#ifdef __SYNTHESIS__
#if MEMLUV_HLS_INLINE == 0
#pragma AP inline off
#else
#pragma AP inline
#endif
#endif
	/* TODO: Many cycles are lost here. I have to find a synthesizable faster way. At least I have to check loop unrolling */
	uint_t i, distance=0;
	if (CurMemLuvCore->action_status == FREE_SUCCESS) {
		for (i=0; i<CurMemLuvCore->depth; i++) { /* Divition with 8 since every byte of core is denoted by 1 bit in freelist. FIXME: Major bug here! */
			Dfprintf(3, CurMemLuvConf->dbg_fd, "%.3d: Comparing %.8p > %.8p", i, ptr, &CurMemLuvCore->core[i]);
			/* FIXME: The following comparison is not always synthesizable! 
			* With a)internal core size 8 or 16, b)multiple calls of free, c) full statistics, an HLS-102 internal error comes up.
			*/
			if (ptr > &CurMemLuvCore->core[i]) { /* While this statement is synthesized, the difference cannot be directly computed to a variable */
				distance+=(uint_t)sizeof(CORE_UINT_T);
				Dfprintf(3, CurMemLuvConf->dbg_fd, " Yes\n");
			}
			else {
				Dfprintf(3,CurMemLuvConf->dbg_fd, " No\n");
				i+=CurMemLuvCore->size/(uint_t)sizeof(CORE_UINT_T); /* instead of break which is synthesizable but causes vivado internal error */
			}
		}
	}
	return distance;
}


#if HW_DEBUG_MEMLUV==1
void MemluvFreeUpdateStas(MemLuvCore *CurMemLuvCore, uint_t nbytes) {
#ifdef __SYNTHESIS__
#if MEMLUV_HLS_INLINE == 0
#pragma AP inline off
#else
#pragma AP inline
#endif
#endif
	if (CurMemLuvCore->action_status == FREE_SUCCESS) {
		CurMemLuvCore->stats.total_bytes_allocated-=CurMemLuvCore->ibitpos_pad; /* Since 1bit in freelist bitmap represents 1 byte */
		CurMemLuvCore->stats.total_bytes_requested-=nbytes;
		CurMemLuvCore->stats.used_percentage=100*CurMemLuvCore->stats.total_bytes_allocated/CurMemLuvCore->size;
		CurMemLuvCore->stats.total_addresses_allocated=CurMemLuvCore->stats.total_bytes_allocated/sizeof(CORE_UINT_T);
		CurMemLuvCore->stats.total_bytes_for_headers-=(uint_t)(MEMLUV_ALLOC_HEADER*sizeof(CORE_UINT_T));
		CurMemLuvCore->stats.total_fragmented_bytes=CurMemLuvCore->stats.total_bytes_allocated-CurMemLuvCore->stats.total_bytes_requested-CurMemLuvCore->stats.total_bytes_for_headers;
#if DEBUG_REPORT_FRAGMENTATION==1
                CurMemLuvCore->stats.total_request_fragmented_bytes = MemluvReportReqFragBytes(CurMemLuvCore);    
#endif                
	}
	else if ((CurMemLuvCore->action_status == FREE_FAIL) || (CurMemLuvCore->action_status == FREE_FAIL_POINTER_OUT_OF_BOUNDS)) {
			Dfprintf(1, CurMemLuvConf->dbg_fd, "ERROR code %d on free\n", CurMemLuvCore->action_status);
		/* FIXME: Still no check on free fail */
		;
	}
}
#endif


void CurMemluvFreeBody(MemLuvCore *CurMemLuvCore, CORE_UINT_T *ptr, uint_t data_alloc_length, uint_t index) {
#ifdef __SYNTHESIS__
#if MEMLUV_HLS_INLINE == 0
#pragma AP inline off
#else
#pragma AP inline
#endif
#endif
	uint_t ptr_index_in_freelist=0, end_index_in_freelist=0, true_alloc_length;
	
#if CHCK_FREE_OUT_OF_BOUNDS==1
	if ((ptr < &CurMemLuvCore->core[0]) || (ptr > &CurMemLuvCore->core[CurMemLuvCore->depth])) {
		Dfprintf(2, CurMemLuvConf->dbg_fd, "\nDEBUG: FAIL free request from pointer %p (out of bounds [%p-%p]", (void*)ptr, (void*)&CurMemLuvCore->core[0], (void*)&CurMemLuvCore->core[CurMemLuvCore->depth]);
		CurMemLuvCore->action_status = FREE_FAIL_POINTER_OUT_OF_BOUNDS;
	}
	else
#endif
	{
#if MEMLUV_ALLOC_HEADER > 0
		ptr_index_in_freelist = index;
		true_alloc_length = data_alloc_length + (uint_t)(MEMLUV_ALLOC_HEADER*sizeof(CORE_UINT_T));
		Dfprintf(3, CurMemLuvConf->dbg_fd, "DEBUG: Length is stored in %p with data_alloc_length=%u, true_alloc_length=%u\n", ptr-2, data_alloc_length, true_alloc_length);
		Dfprintf(3, CurMemLuvConf->dbg_fd, "DEBUG: Index  is stored in %p with index=%u", ptr-1, index);
#if HW_DEBUG_MEMLUV==1
		MemLuvDebugUpdate(CurMemLuvCore, 2, (uint_t)index);
#endif
#else
		ptr_index_in_freelist = (uint_t)index;
		true_alloc_length = data_alloc_length + (uint_t)(MEMLUV_ALLOC_HEADER*ALIGNMENT);
		Dfprintf(3, CurMemLuvConf->dbg_fd, "DEBUG: ptr=%p, data_alloc_length=%u true_alloc_length=%u and index=%u core[%u]=%u", ptr, data_alloc_length, true_alloc_length, index, index/sizeof(CORE_UINT_T), CurMemLuvCore->core[index/sizeof(CORE_UINT_T)]);
#if HW_DEBUG_MEMLUV==1
		MemLuvDebugUpdate(CurMemLuvCore, 2, CurMemLuvCore->stats.lock);
#endif
#endif
		end_index_in_freelist = ReqPadding(ptr_index_in_freelist + true_alloc_length)-1;
		Dfprintf(2, CurMemLuvConf->dbg_fd, "\nDEBUG: Unmarking reserved freelist bits %u upto %u \n", ptr_index_in_freelist, end_index_in_freelist);
		MemluvClearFreelist(CurMemLuvCore, ptr_index_in_freelist, end_index_in_freelist);
#if MEMLUV_FIT_ALGORITHM==0 && MEMLUV_FIRST_FIT_CACHE_LAST_BIN==1
		Dfprintf(3, CurMemLuvConf->dbg_fd, "DEBUG: In free CurMemLuvCore->ff_cache_bin_in=%u, ptr_index_in_freelist=%u\n",CurMemLuvCore->ff_cache_bin_in, ptr_index_in_freelist);
		if (ptr_index_in_freelist < CurMemLuvCore->ff_cache_bin_in)
			CurMemLuvCore->ff_cache_bin_in = ptr_index_in_freelist;
		Dfprintf(3, CurMemLuvConf->dbg_fd, "DEBUG: In free CurMemLuvCore->ff_cache_bin_in=%u, ptr_index_in_freelist=%u\n",CurMemLuvCore->ff_cache_bin_in, ptr_index_in_freelist);
#endif
		CurMemLuvCore->ibitpos_pad = end_index_in_freelist+1-ptr_index_in_freelist;
#if HW_DEBUG_MEMLUV==1
		MemluvFreeUpdateStas(CurMemLuvCore, data_alloc_length);
		Dfprintf(2, CurMemLuvConf->dbg_fd, "DEBUG: >> [OK] [Core Usage: %u%% (%u out of %u)] [Addresses used: %u out of %u] [Frag. %u B]\n", 
			CurMemLuvCore->stats.used_percentage, CurMemLuvCore->stats.total_bytes_allocated, CurMemLuvCore->size, 
			CurMemLuvCore->stats.total_addresses_allocated, CurMemLuvCore->depth, CurMemLuvCore->stats.total_fragmented_bytes);
#endif
		Dfprintf(2, CurMemLuvConf->dbg_fd, "\\-------------------------------------------------/\n");
		//MemluvDumpFreeList(CurMemLuvCore, THIS);
		DumpStatisticsToFile(CurMemLuvCore
					#if USE_MEMLUV==1 && HW_DEBUG_MEMLUV==1 
						,2
						,CurMemLuvCore->ibitpos_pad
					#endif
					);
		//QueryUnLock(CurMemLuvCore);
	}
}


/* MemluvFree wrapper is automated generated by gen_memluv_wrappers.c, included in global.c */
#if SIM_WITH_GLIBC_MALLOC==0
void CurMemluvFree(MemLuvCore *CurMemLuvCore, CORE_UINT_T *ptr, uint_t nbytes ) {
#ifdef __SYNTHESIS__							
#if MEMLUV_HLS_INLINE == 1 || MEMLUV_ALLOC_HEADER > 0			
#pragma AP inline							
#else									
#pragma AP inline off								
#endif									
#endif
	//QueryLock(CurMemLuvCore);
	uint_t  data_alloc_length, index;

	Dfprintf(2, CurMemLuvConf->dbg_fd, "\n/----------------- FREE [heap %d] -----------------\\\n", CurMemLuvCore->id);
	CurMemLuvCore->free_rqst++;
	CurMemLuvCore->tot_rqst++;
	Dfprintf(2, CurMemLuvConf->dbg_fd, "DEBUG: Free Request %u: %u bytes from pointer %p (Total Request %u)\n", CurMemLuvCore->free_rqst, nbytes, ptr, CurMemLuvCore->tot_rqst);

	/* Always mark a succesfull free unless check is forced */
	CurMemLuvCore->action_status = FREE_SUCCESS;
#if MEMLUV_ALLOC_HEADER > 0
	//data_alloc_lengthp=(uint_t*)ptr - 2; /* The 2nd previous address holds the length of this allocation */   /* minus here is synthesizable only in inline directive*/
	data_alloc_length=CurMemLuvCore->core[(uint_t)(ptr-CurMemLuvCore->base)-2];
	index=CurMemLuvCore->core[(uint_t)(ptr-CurMemLuvCore->base)-1];
	/* FIXME: was uint8_t: This means that I cannot allocate more than 2^8 chunks at once, If I change pointer type I have to take care for endiannes */
#else
	data_alloc_length=nbytes;
	index = MemluvCalcPtrDistanceFromBase(CurMemLuvCore, ptr);
	Dfprintf(2, CurMemLuvConf->dbg_fd, "\nDEBUG: Found matching pointer with freelist index=%u\n", index);
#endif
	CurMemluvFreeBody(CurMemLuvCore, ptr, data_alloc_length, index);
}
#else
void CurMemluvFree(MemLuvCore *CurMemLuvCore, CORE_UINT_T *ptr, uint_t nbytes) {
	/* Glibc malloc stores the size of each allocation to metadata */
	free ((void*)ptr);
}
#endif