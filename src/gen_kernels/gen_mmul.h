
/**
 * @file   src/gen_mmul.h
 * @author Dionysios Diamantopoulos(diamantd@microlab.ntua.gr)
 * @date   April 2014
 *
 * @brief  Headers for generating Matrix multiplication accelerators for memluv evaluation.
 */

#ifndef _GEN_MMUL_H_
#define _GEN_MMUL_H_

/* ----------- MMUL Kernel parameters --------- */
#define NUM_MMUL_ACCELERATORS MMUL_KERNELS
#define MMUL_HLS_INLINE 0
#define RANDOM_MMUL_VALUES 1
#define N_MMUL_MAX 50 // 100 for scaling measurement
#define BLOCK_LEN 100

/* -------------------------------------------- */

void GenKernelMMUL(FILE *fd);

#endif
