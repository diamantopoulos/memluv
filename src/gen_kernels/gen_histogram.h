
/**
 * @file   src/gen_histogram.h
 * @author Dionysios Diamantopoulos(diamantd@microlab.ntua.gr)
 * @date   July 2014
 *
 * @brief  Headers for generating histogram accelerators for memluv evaluation.
 */

#ifndef _GEN_HISTOGRAM_H_
#define _GEN_HISTOGRAM_H_

/* -------- HISTOGRAM Kernel parameters ------- */
#define NUM_HISTOGRAM_ACCELERATORS HISTOGRAM_KERNELS
/**
 * Name  x(px)  y(px)   x:y     x*y(px) 
 * QQVGA 160    120     4:3     19200
 * HQVGA 240    160     3:2     38400
 * QVGA  320    240     4:3     76800
 * WQVGA 360    240     3:2     86400
 * WQVGA 400    240     5:3     96000
 * HVGA  480    320     3:2     153600
 * VGA   640    480     4:3     307200
 * WVGA  720    480     3:2     345600
 * WVGA  800    480     5:3     384000
 * FWVGA 854    480     16:9    409920
 * SVGA  800    600     4:3     480000
 * DVGA  960    640     3:2     614400
 * WSVGA 1024   576     16:9    589824
 * WSVGA 1024   600     128:75  614400  
 */
#define N_HISTOGRAM_MAX 1000 // 307200 for scaling measurement
#define HISTOGRAM_HLS_INLINE 0
#define RANDOM_HISTOGRAM_SIZE 1
/* -------------------------------------------- */

void GenKernelHistogram(FILE *fd);

#endif
