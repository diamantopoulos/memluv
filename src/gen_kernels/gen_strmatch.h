
/**
 * @file   src/gen_strmatch.h
 * @author Dionysios Diamantopoulos(diamantd@microlab.ntua.gr)
 * @date   April 2014
 *
 * @brief  Headers for generating string match accelerators for memluv evaluation.
 */

#ifndef _GEN_STRMATCH_H_
#define _GEN_STRMATCH_H_

/* -------- STRMATCH Kernel parameters ------- */
#define NUM_STRMATCH_ACCELERATORS STRMATCH_KERNELS
#define N_STRMATCH_MAX 5000
#define STRMATCH_HLS_INLINE 0
#define RANDOM_STRMATCH_VALUES 1
#define STRMATCH_DEF_CHARS_PER_LINE 40 // 1024 for scaling measurement
#define STRMATCH_DEF_LINES_PER_FILE 40 // 300 for scaling measurement
#define STRMATCH_DEF_SHOW_KEYS 0
#define STRMATCH_DEF_MAX_REC_LEN 40 // 10 for scaling measurement
/* -------------------------------------------- */

void GenKernelStrmatch(FILE *fd);

#endif