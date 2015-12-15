
/**
 * @file   src/freelist.h
 * @author Dionysios Diamantopoulos(diamantd@microlab.ntua.gr)
 * @date   December 2013
 *
 * @brief  Memluv prototype fynctions for feelist procedures.
 */

#ifndef _FREELIST_H_
#define _FREELIST_H_

#include "memluv.h"

/** The MemluvSetFreelist() function fills the freelist with ones on positions indexed by start_pos 
 *  to end_pos. This function shall do the job bit-per-bit or with mask registers based on 
 *  FREELIST_HANDLE_POLICY in memluv.h.
 * @param A		The freelist of current HW heap.
 * @param start_pos	The start index in freelist.
 * @param end_pos	The end index in freelist.
 * @retval void		Nothing is returned.
 */
void MemluvSetFreelist(FREELIST_UINT_T A[ ], var_type_t start_pos, var_type_t end_pos);

/** The MemluvClearFreelist() function fills the freelist with zeros on positions indexed by start_pos 
 *  to end_pos. This function shall do the job bit-per-bit or with mask registers based on 
 *  FREELIST_HANDLE_POLICY in memluv.h.
 * @param A		The freelist of current HW heap.
 * @param start_pos	The start index in freelist.
 * @param end_pos	The end index in freelist.
 * @retval void		Nothing is returned.
 */
void MemluvClearFreelist(MemLuvCore* CurMemLuvCore, var_type_t start_pos, var_type_t end_pos);

/** The MemluvSetBitFreelist() function sets to one the index k in the freelist.
 *  This function computes the i-register (division) and j-position in the register (mod) based
 *  on k-index.
 * @param A		The freelist of current HW heap.
 * @param start_pos	The k-index in freelist.
 * @retval void		Nothing is returned.
 */
void MemluvSetBitFreelist( FREELIST_UINT_T A[ ],  var_type_t k );

/** The MemluvClearBitFreelist() function sets to zero the index k in the freelist.
 *  This function computes the i-register (division) and j-position in the register (mod) based
 *  on k-index.
 * @param A		The freelist of current HW heap.
 * @param start_pos	The k-index in freelist.
 * @retval void		Nothing is returned.
 */
void MemluvClearBitFreelist( FREELIST_UINT_T A[ ],  var_type_t k );

/** The MemluvTestBitFreelist() function checks the value of index k in the freelist.
 *  This function computes the i-register (division) and j-position in the register (mod) based
 *  on k-index and applies an AND to a j-shifted mask to find the value.
 * @param A		The freelist of current HW heap.
 * @param start_pos	The k-index in freelist.
 * @retval val		The value (0 or 1) on k-index.
 */
uint8_t MemluvTestBitFreelist( FREELIST_UINT_T A[ ],  var_type_t k );

#endif