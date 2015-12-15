
/**
 * @file   src/gen_pca.h
 * @author Dionysios Diamantopoulos(diamantd@microlab.ntua.gr)
 * @date   April 2014
 *
 * @brief  Headers for generating PCA accelerators for memluv evaluation.
 */

#ifndef _GEN_PCA_H_
#define _GEN_PCA_H_

/* -------- PCA Kernel parameters ------- */
#define NUM_PCA_ACCELERATORS PCA_KERNELS
#define PCA_DEF_GRID_SIZE 100  // all values in the matrix are from 0 to this value 
#define PCA_DEF_NUM_ROWS 60  // 250 for scaling measurement
#define PCA_DEF_NUM_COLS 60  // 250 for scaling measurement
#define PCA_HLS_INLINE 0
#define RANDOM_PCA_VALUES 1
#define N_PCA_MAX PCA_DEF_GRID_SIZE
/* -------------------------------------------- */

void GenKernelPCA(FILE *fd);

#endif