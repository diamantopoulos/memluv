
/**
 * @file   src/gen_regression.h
 * @author Dionysios Diamantopoulos(diamantd@microlab.ntua.gr)
 * @date   April 2014
 *
 * @brief  Headers for generating regression accelerators for memluv evaluation.
 */

#ifndef _GEN_REGRESSION_H_
#define _GEN_REGRESSION_H_

/* -------- REGRESSION Kernel parameters ------- */
#define NUM_REGRESSION_ACCELERATORS REGRESSION_KERNELS
/* In original phoenix implementation the datasets were:
 * key_file_50MB.txt  -> 5522430 lines (1 string/line)
 * key_file_100MB.txt -> 10396203 lines
 * key_file_500MB.txt -> 51981019 lines
 * NOTE: The benchmark process sequentially the data by taking the first characters (bytes) of 
 *       every word.
 */
#define N_REGRESSION_MAX 5000  // 100000 for scaling measurement
#define REGRESSION_HLS_INLINE 0
#define RANDOM_REGRESSION_VALUES 1
/* -------------------------------------------- */

void GenKernelRegression(FILE *fd);

#endif