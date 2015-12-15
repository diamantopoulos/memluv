
/**
 * @file   src/freelist.c
 * @author Dionysios Diamantopoulos(diamantd@microlab.ntua.gr)
 * @date   December 2013
 *
 * @brief  Memluv feelist oriented functions.
 */

#include "freelist.h"

void MemluvSetFreelist(FREELIST_UINT_T A[ ], var_type_t start_pos, var_type_t end_pos) {
#ifdef __SYNTHESIS__
#if MEMLUV_HLS_INLINE == 0
#pragma AP inline off
#else
#pragma AP inline
#endif
#endif
#if FREELIST_HANDLE_POLICY == 0
	var_type_t i;
	for (i=start_pos; i<=end_pos; i++) {
		MemluvSetBitFreelist(A, i);
	}
#elif FREELIST_HANDLE_POLICY == 1
	var_type_t i;
	FREELIST_UINT_T j=0;
	FREELIST_UINT_T pos;
	FREELIST_UINT_T flag;
	FREELIST_UINT_T reg_val=0;
	for (i=start_pos; i<=end_pos; i++) {
		j = (FREELIST_UINT_T)(i/(sizeof(FREELIST_UINT_T)*8));
		pos = i%(sizeof(FREELIST_UINT_T)*8);
		flag = 1;  					// flag = 0000.......00001
		flag = (FREELIST_UINT_T)(flag << pos);		// flag = 0000...010...000   (shifted i positions)
		reg_val = reg_val | flag;			// Set the bit at the i-th position in A[j]
		Dprintf(3, "DEBUG: Inside MemluvSetFreelist: bin=%u, reg=%u, pos=%u, flag=%u, reg_val=%u\n", i, j, pos, flag, reg_val);
		if (pos==FREELIST_WIDTH-1) {
			A[j] = A[j] | reg_val; // Only (i%FREELIST_WIDTH) writes to freelist
			reg_val=0;
		}
	}
	A[j] = A[j] | reg_val; // plus one last write to freelist
#elif FREELIST_HANDLE_POLICY == 2
	var_type_t start_reg, end_reg;
	var_type_t start_pos_in_reg, end_pos_in_reg;
	var_type_t i;
	FREELIST_UINT_T mask, zero=0; 
	FREELIST_UINT_TD flag=1; /* Double the size of FREELIST_UINT_T, since we need to calculate the number 2^FREELIST_WIDTH */
	start_reg = (FREELIST_UINT_T)(start_pos/(sizeof(FREELIST_UINT_T)*8));
	end_reg = (FREELIST_UINT_T)(end_pos/(sizeof(FREELIST_UINT_T)*8));
	start_pos_in_reg = start_pos%(sizeof(FREELIST_UINT_T)*8);
	end_pos_in_reg = end_pos%(sizeof(FREELIST_UINT_T)*8);
	Dprintf(3, "DEBUG: Inside MemluvSetFreelist: start_pos=%u, end_pos=%u, start_reg=%u, end_reg=%u, start_pos_in_reg=%u, end_pos_in_reg=%u\n", start_pos, end_pos, start_reg, end_reg, start_pos_in_reg, end_pos_in_reg);
	switch ( end_reg - start_reg ) {
	case 0:
		// power of 2 with shift: i.e. 2^30 == 1 << 30 == 1073741824
		mask = (FREELIST_UINT_T)(((flag<<(end_pos_in_reg+1))-1) - ((flag<<start_pos_in_reg)-1));
		Dprintf(3, "DEBUG: mask=%u\n", mask);
		A[start_reg] = A[start_reg] | mask;
		break;
	default:
		mask = (FREELIST_UINT_T)((~zero) - ((flag<<start_pos_in_reg)-1));
		Dprintf(3, "DEBUG: start mask=%u\n", mask);
		A[start_reg] = A[start_reg] | mask;
		for (i=start_reg+1; i<end_reg; i++) {
			Dprintf(3, "DEBUG: Marking entire reg %u\n", i);
			A[i] = (FREELIST_UINT_T)(~zero);
		}
		mask = (FREELIST_UINT_T)(((flag<<(end_pos_in_reg+1))-1));
		Dprintf(3, "DEBUG: end mask=%u\n", mask);	
		A[end_reg] = A[end_reg] | mask;
		break;
	}
#else
	Dprintf(0, "Unsupported FREELIST_HANDLE_POLICY on stc/memluv.h. Aborting...\n\n");
	exit(-1);
#endif
}


void MemluvClearFreelist(MemLuvCore* CurMemLuvCore, var_type_t start_pos, var_type_t end_pos) {
#ifdef __SYNTHESIS__
#if MEMLUV_HLS_INLINE == 0
#pragma AP inline off
#else
#pragma AP inline
#endif
#endif
#if FREELIST_HANDLE_POLICY == 0
	var_type_t i;
	for (i=start_pos; i<=end_pos; i++) {
#if CHCK_FREELIST_BEFORE_FREE==1
		if (MemluvTestBitFreelist(CurMemLuvCore->freelist, i) == 0 ) {
			CurMemLuvCore->action_status = FREE_FAIL_UNREGISTERED_BIT;
			Dfprintf(2, CurMemLuvConf->dbg_fd, "\nDEBUG: Useless free request on freelist position %u", i);
		}
#endif
		MemluvClearBitFreelist(CurMemLuvCore->freelist, i);
	}
#elif FREELIST_HANDLE_POLICY == 1
	var_type_t i;
	FREELIST_UINT_T j=0;
	FREELIST_UINT_T pos;
	FREELIST_UINT_T flag;
 	FREELIST_UINT_T reg_val=(FREELIST_UINT_T)(~0);
	for (i=start_pos; i<=end_pos; i++) {
		j = (FREELIST_UINT_T)(i/(sizeof(FREELIST_UINT_T)*8));
		pos = i%(sizeof(FREELIST_UINT_T)*8);
		flag = 1;  					// flag = 0000.......00001
		flag = (FREELIST_UINT_T)(flag << pos);		// flag = 0000...010...000   (shifted i positions)
		flag = (FREELIST_UINT_T)(~flag);		// flag = 1111...101...111
		reg_val = reg_val & flag;			// Set the bit at the i-th position in A[j]
		Dprintf(3, "DEBUG: Inside MemluvSetFreelist: bin=%u, reg=%u, pos=%u, flag=%u, reg_val=%u\n", i, j, pos, flag, reg_val);
		if (pos==FREELIST_WIDTH-1) {
			CurMemLuvCore->freelist[j] = CurMemLuvCore->freelist[j] & reg_val; // Only (i%FREELIST_WIDTH) writes to freelist
			reg_val=(FREELIST_UINT_T)(~0);
		}
	}
 	CurMemLuvCore->freelist[j] = CurMemLuvCore->freelist[j] & reg_val; // plus one last write to freelist
#elif FREELIST_HANDLE_POLICY == 2
	var_type_t start_reg, end_reg;
	var_type_t start_pos_in_reg, end_pos_in_reg;
	var_type_t i;
	FREELIST_UINT_T mask, zero=0; 
	FREELIST_UINT_TD flag=1; /* Double the size of FREELIST_UINT_T, since we need to calculate the number 2^FREELIST_WIDTH */
	start_reg = (FREELIST_UINT_T)(start_pos/(sizeof(FREELIST_UINT_T)*8));
	end_reg = (FREELIST_UINT_T)(end_pos/(sizeof(FREELIST_UINT_T)*8));
	start_pos_in_reg = start_pos%(sizeof(FREELIST_UINT_T)*8);
	end_pos_in_reg = end_pos%(sizeof(FREELIST_UINT_T)*8);
	Dprintf(3, "DEBUG: Inside MemluvSetFreelist: start_pos=%u, end_pos=%u, start_reg=%u, end_reg=%u, start_pos_in_reg=%u, end_pos_in_reg=%u\n", start_pos, end_pos, start_reg, end_reg, start_pos_in_reg, end_pos_in_reg);
	switch ( end_reg - start_reg ) {
	case 0:
		// power of 2 with shift: i.e. 2^30 == 1 << 30 == 1073741824
		mask = (FREELIST_UINT_T)(~(((flag<<(end_pos_in_reg+1))-1) - ((flag<<start_pos_in_reg)-1)));
		Dprintf(3, "DEBUG: mask=%u\n", mask);
		CurMemLuvCore->freelist[start_reg] = CurMemLuvCore->freelist[start_reg] & mask;
		break;
	default:
		mask = (FREELIST_UINT_T)(~(((~zero)) - ((flag<<start_pos_in_reg)-1)));
		Dprintf(3, "DEBUG: start mask=%u\n", mask);
		CurMemLuvCore->freelist[start_reg] = CurMemLuvCore->freelist[start_reg] & mask;
		for (i=start_reg+1; i<end_reg; i++) {
			Dprintf(3, "DEBUG: Erasing entire reg %u\n", i);
			CurMemLuvCore->freelist[i] = zero;
		}
		mask = (FREELIST_UINT_T)(~(((flag<<(end_pos_in_reg+1))-1)));
		Dprintf(3, "DEBUG: end mask=%u\n", mask);	
		CurMemLuvCore->freelist[end_reg] = CurMemLuvCore->freelist[end_reg] & mask;
		break;
	}
#else
	Dprintf(0, "Unsupported FREELIST_HANDLE_POLICY on stc/memluv.h. Aborting...\n\n");
	exit(-1);
#endif
}

void MemluvSetBitFreelist( FREELIST_UINT_T A[ ],  var_type_t k ) {
#ifdef __SYNTHESIS__
#if MEMLUV_HLS_INLINE == 0
#pragma AP inline off
#else
#pragma AP inline
#endif
#endif   
   FREELIST_UINT_T i =  (FREELIST_UINT_T)(k/(sizeof(FREELIST_UINT_T)*8));
   FREELIST_UINT_T pos = k%(sizeof(FREELIST_UINT_T)*8);
   FREELIST_UINT_T flag = 1; 			 // flag = 0000.......00001
   flag = (FREELIST_UINT_T)(flag << pos);	 // flag = 0000...010...000   (shifted k positions)
   A[i] = A[i] | flag;    			 // Set the bit at the k-th position in A[i]
}

void MemluvClearBitFreelist( FREELIST_UINT_T A[ ],  var_type_t k ) {
#ifdef __SYNTHESIS__
#if MEMLUV_HLS_INLINE == 0
#pragma AP inline off
#else
#pragma AP inline
#endif
#endif
   FREELIST_UINT_T i = (FREELIST_UINT_T)(k/(sizeof(FREELIST_UINT_T)*8));
   FREELIST_UINT_T pos = k%(sizeof(FREELIST_UINT_T)*8);
   FREELIST_UINT_T flag = 1;  			// flag = 0000.......00001
   flag = (FREELIST_UINT_T)(flag << pos);	// flag = 0000...010...000   (shifted k positions)
   flag = (FREELIST_UINT_T)(~flag);		// flag = 1111...101...111
   A[i] = A[i] & flag;     // RESET the bit at the k-th position in A[i]
}

uint8_t MemluvTestBitFreelist( FREELIST_UINT_T A[ ],  var_type_t k ) {
#ifdef __SYNTHESIS__
#if MEMLUV_HLS_INLINE == 0
#pragma AP inline off
#else
#pragma AP inline
#endif
#endif
   FREELIST_UINT_T i = (FREELIST_UINT_T)(k/(sizeof(FREELIST_UINT_T)*8));
   FREELIST_UINT_T pos = k%(sizeof(FREELIST_UINT_T)*8);
   FREELIST_UINT_T flag = 1;			// flag = 0000.......00001
   flag = (FREELIST_UINT_T)(flag << pos);	// flag = 0000...010...000   (shifted k positions)
   if ( A[i] & flag )				// Test the bit at the k-th position in A[i]
      return 1;
   else
      return 0;
}