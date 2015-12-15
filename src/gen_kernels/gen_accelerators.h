
/**
 * @file   src/gen_accelerators.h
 * @author Dionysios Diamantopoulos(diamantd@microlab.ntua.gr)
 * @date   January 2014
 *
 * @brief  Headers for generating accelerators for memluv evaluation.
 */

#ifndef _GEN_ACCELERATORS_H_
#define _GEN_ACCELERATORS_H_

#define USE_MEMLUV 1

#define DFADD_KERNELS 0

#define HISTOGRAM_KERNELS 2

#define REGRESSION_KERNELS 0

#define PCA_KERNELS 0

#define MMUL_KERNELS 0

#define KMEANS_KERNELS 0

#define STRMATCH_KERNELS 0

#define UNITTEST_KERNELS 0

#define FREE_INPUTS 1
#define FREE_OUTPUTS 1

double ullong_to_double (unsigned long long x);

#endif

/* -------------------------------------------- */

