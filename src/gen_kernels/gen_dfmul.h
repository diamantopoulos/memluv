
/**
 * @file   src/gen_dfmul.h
 * @author Dionysios Diamantopoulos(diamantd@microlab.ntua.gr)
 * @date   April 2014
 *
 * @brief  Headers for generating DFMUL accelerators for memluv evaluation.
 */

#ifndef _GEN_DFMUL_H_
#define _GEN_DFMUL_H_

/* ----------- DFMUL Kernel parameters -------- */
#define NUM_DFMUL_ACCELERATORS 0
#define TRAFFIC_LOOPS_DFMUL 2
#define RANDOM_DFMUL_SIZE 0
/* -------------------------------------------- */

#include "../../apps/CHStone_v1.8_130128/dfmul/softfloat.c"

void GenKernelDFMUL(FILE* fd);

/*
+--------------------------------------------------------------------------+
| * Test Vectors (added for CHStone)                                       |
|     a_input, b_input : input data                                        |
|     z_output : expected output data                                      |
+--------------------------------------------------------------------------+
*/
#define N_DFMUL_MAX 20
const float64 a_input_DFMUL[N_DFMUL_MAX] = {
  0x7FF0000000000000ULL,	/* inf */
  0x7FFF000000000000ULL,	/* nan */
  0x7FF0000000000000ULL,	/* inf */
  0x7FF0000000000000ULL,	/* inf */
  0x3FF0000000000000ULL,	/* 1.0 */
  0x0000000000000000ULL,	/* 0.0 */
  0x3FF0000000000000ULL,	/* 1.0 */
  0x0000000000000000ULL,	/* 0.0 */
  0x8000000000000000ULL,	/* -0.0 */
  0x3FF0000000000000ULL,	/* 1.0 */
  0x3FF0000000000000ULL,	/* 1.0 */
  0x4000000000000000ULL,	/* 2.0 */
  0x3FD0000000000000ULL,	/* 0.25 */
  0xC000000000000000ULL,	/* -2.0 */
  0xBFD0000000000000ULL,	/* -0.25 */
  0x4000000000000000ULL,	/* 2.0 */
  0xBFD0000000000000ULL,	/* -0.25 */
  0xC000000000000000ULL,	/* -2.0 */
  0x3FD0000000000000ULL,	/* 0.25 */
  0x0000000000000000ULL		/* 0.0 */
};

const float64 b_input_DFMUL[N_DFMUL_MAX] = {
  0xFFFFFFFFFFFFFFFFULL,	/* nan */
  0xFFF0000000000000ULL,	/* -inf */
  0x0000000000000000ULL,	/* nan */
  0x3FF0000000000000ULL,	/* -inf */
  0xFFFF000000000000ULL,	/* nan */
  0x7FF0000000000000ULL,	/* inf */
  0x7FF0000000000000ULL,	/* inf */
  0x3FF0000000000000ULL,	/* 1.0 */
  0x3FF0000000000000ULL,	/* 1.0 */
  0x0000000000000000ULL,	/* 0.0 */
  0x8000000000000000ULL,	/* -0.0 */
  0x3FD0000000000000ULL,	/* 0.25 */
  0x4000000000000000ULL,	/* 2.0 */
  0xBFD0000000000000ULL,	/* -0.25 */
  0xC000000000000000ULL,	/* -2.0 */
  0xBFD0000000000000ULL,	/* -0.25 */
  0x4000000000000000ULL,	/* -2.0 */
  0x3FD0000000000000ULL,	/* 0.25 */
  0xC000000000000000ULL,	/* -2.0 */
  0x0000000000000000ULL		/* 0.0 */
};

const float64 z_output_DFMUL[N_DFMUL_MAX] = {
  0xFFFFFFFFFFFFFFFFULL,	/* nan */
  0x7FFF000000000000ULL,	/* nan */
  0x7FFFFFFFFFFFFFFFULL,	/* nan */
  0x7FF0000000000000ULL,	/* inf */
  0xFFFF000000000000ULL,	/* nan */
  0x7FFFFFFFFFFFFFFFULL,	/* nan */
  0x7FF0000000000000ULL,	/* inf */
  0x0000000000000000ULL,	/* 0.0 */
  0x8000000000000000ULL,	/* -0.0 */
  0x0000000000000000ULL,	/* 0.0 */
  0x8000000000000000ULL,	/* -0.0 */
  0x3FE0000000000000ULL,	/* 0.5 */
  0x3FE0000000000000ULL,	/* 0.5 */
  0x3FE0000000000000ULL,	/* 0.5 */
  0x3FE0000000000000ULL,	/* 0.5 */
  0xBFE0000000000000ULL,	/* -0.5 */
  0xBFE0000000000000ULL,	/* -0.5 */
  0xBFE0000000000000ULL,	/* -0.5 */
  0xBFE0000000000000ULL,	/* -0.5 */
  0x0000000000000000ULL		/* 0.0 */
};


#endif