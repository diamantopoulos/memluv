
/**
 * @file   unittest/include/gen_unittest.h
 * @author Dionysios Diamantopoulos(diamantd@microlab.ntua.gr)
 * @date   June 2014
 *
 * @brief  Headers for generating unit test for memluv evaluation.
 */

#ifndef _GEN_UNITTEST_H_
#define _GEN_UNITTEST_H_

/* ----------- UNITTEST parameters --------- */
#define NUM_UNITTEST_THREADS UNITTEST_KERNELS
#define NUM_UNITTEST_LOOPS 1
#define UNITTEST_HLS_INLINE 0
#define RANDOM_UNITTEST_VALUES 1
#define UNITTEST_RAND_FREE 0

#define GEN_UNITTEST_SEED 1

#define N_UNITTEST_MAX 5000
#define BLOCK_LEN 100

/* ----------------------------------------- */

void GenUnitTest(FILE *fd);

#endif
