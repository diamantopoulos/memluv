
/**
 * @file   src/gen_kmeans.h
 * @author Dionysios Diamantopoulos(diamantd@microlab.ntua.gr)
 * @date   April 2014
 *
 * @brief  Headers for generating kmeans accelerators for memluv evaluation.
 */

#ifndef _GEN_KMEANS_H_
#define _GEN_KMEANS_H_

/* -------- KMEANS Kernel parameters ------- */
#define NUM_KMEANS_ACCELERATORS KMEANS_KERNELS
#define N_KMEANS_MAX 5000
#define KMEANS_HLS_INLINE 0
#define RANDOM_KMEANS_VALUES 1
#define KMEANS_DEF_NUM_POINTS 200 // 20000 for scaling measurement
#define KMEANS_DEF_NUM_MEANS 3
#define KMEANS_DEF_DIM 3
#define KMEANS_DEF_GRID_SIZE 100
#define false 0
#define true 1
/* -------------------------------------------- */

void GenKernelKmeans(FILE *fd);

#endif
