
/**
 * @file   src/gen_malloc_wrappers.h
 * @author Dionysios Diamantopoulos(diamantd@microlab.ntua.gr)
 * @date   January 2014
 *
 * @brief  Headers for generating HW heaps and malloc wrappers for memluv.
 */

#ifndef _GEN_MALLOC_WRAPPERS_H_
#define _GEN_MALLOC_WRAPPERS_H_


/** The GenMemluvWrappers() function generates malloc/free wrappers and static HW heaps according to 1
 *  @param void         No parameter value, just do the silent job
 *  @retval void        No returned value.
 */
void GenMemluvWrappers(void);

#endif

#define NUM_HW_HEAPS 2
