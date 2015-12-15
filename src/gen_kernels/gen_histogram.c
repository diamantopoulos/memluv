
/**
 * @file   src/gen_histogram.c
 * @author Dionysios Diamantopoulos(diamantd@microlab.ntua.gr)
 * @date   July 2014
 *
 * @brief   Generating histogram accelerators for memluv evaluation.
 */

#include "../memluv.h"
#include "../gen_memluv_wrappers.h"
#include "gen_accelerators.h"
#include "gen_histogram.h"


void GenKernelHistogram(FILE *fd) {
        int i;
        if (NUM_HISTOGRAM_ACCELERATORS != 0)
            fprintf(fd, "#include \"../src/gen_kernels/gen_histogram.h\"  \n\n");
        
        for (i = 0; i < NUM_HISTOGRAM_ACCELERATORS; i++) {
        fprintf(fd, "\n/* HISTOGRAM KERNEL %d */                                         \n", i);
        fprintf(fd, "int HISTOGRAM_k%d(void);                                            \n", i);
        fprintf(fd, "int HISTOGRAM_k%d(void) {                                           \n", i);
        fprintf(fd, "#ifdef __SYNTHESIS__                                                \n");
        fprintf(fd, "#pragma AP inline ");
        if (HISTOGRAM_HLS_INLINE== 0) 
                fprintf(fd, "off\n");
        else if (HISTOGRAM_HLS_INLINE== 2) 
                fprintf(fd, "recursive\n");
        else
                fprintf(fd, "\n");
        fprintf(fd, "#endif                                                              \n");
        fprintf(fd, "int main_result_HISTOGRAM_k%d=0, N_HISTOGRAM_k%d = N_HISTOGRAM_MAX; \n", i, i);
        fprintf(fd, "int i_HISTOGRAM_k%d;                                                \n", i);
        fprintf(fd, "unsigned char *val;                \n");

#if USE_MEMLUV == 1
                fprintf(fd, "char *fdata_HISTOGRAM_k%d; \n", i);
                fprintf(fd, "int *blue_HISTOGRAM_k%d, *green_HISTOGRAM_k%d, *red_HISTOGRAM_k%d; \n", i, i, i);
#else
                fprintf(fd, "char fdata_HISTOGRAM_k%d[N_HISTOGRAM_MAX]; \n", i);
                fprintf(fd, "int blue_HISTOGRAM_k%d[256];       \n", i);
                fprintf(fd, "int green_HISTOGRAM_k%d[256];      \n", i);
                fprintf(fd, "int red_HISTOGRAM_k%d[256];        \n", i);
#endif
        
#if USE_MEMLUV == 1
        /* Accelerator's Memory Allocation */
                fprintf(fd, "\tfdata_HISTOGRAM_k%d=(char*)MemluvAlloc((uint_t)(N_HISTOGRAM_k%d*(int_t)sizeof(char)), %d);       \n", i, i, i%NUM_HW_HEAPS);
                fprintf(fd, "\tblue_HISTOGRAM_k%d=(int*)MemluvAlloc((uint_t)(256*(int_t)sizeof(int)), %d);                      \n", i, i%NUM_HW_HEAPS);
                fprintf(fd, "\tgreen_HISTOGRAM_k%d=(int*)MemluvAlloc((uint_t)(256*(int_t)sizeof(int)), %d);                     \n", i, i%NUM_HW_HEAPS);                
                fprintf(fd, "\tred_HISTOGRAM_k%d=(int*)MemluvAlloc((uint_t)(256*(int_t)sizeof(int)), %d);                       \n", i, i%NUM_HW_HEAPS);
#endif
#if RANDOM_HISTOGRAM_SIZE == 1
        fprintf(fd, "\tunsigned short heap_lfsr_ptr;                                           \n");
        fprintf(fd, "\theap_lfsr_ptr = 0xACE1u;                                                \n");
#endif
        fprintf(fd, "\tfor (i_HISTOGRAM_k%d=0; i_HISTOGRAM_k%d < N_HISTOGRAM_k%d; i_HISTOGRAM_k%d+=3)                           \n", i, i, i ,i);
#if RANDOM_HISTOGRAM_SIZE == 1
        fprintf(fd, "\t\tfdata_HISTOGRAM_k%d[i_HISTOGRAM_k%d]=(char)RandMinMaxSyn(1, (uint_t)i_HISTOGRAM_k%d+1, &heap_lfsr_ptr, ", i, i, i);

/* The following statement is not valid, since we provide different random function for testing the static vs memluv approach.
   However it is the only way to overcome the issue that Vivado does not synthesize the circuit with static allocation + random function which uses seeed on stack.
*/
#if USE_MEMLUV == 1
        fprintf(fd, "1);        \n");
#else
        fprintf(fd, "0);        \n");
#endif
        
#else
        fprintf(fd, "\t\tfdata_HISTOGRAM_k%d[i_HISTOGRAM_k%d] = (char)i_HISTOGRAM_k%d;                                          \n", i, i, i);
#endif
        fprintf(fd, "\tfor (i_HISTOGRAM_k%d=0; i_HISTOGRAM_k%d < N_HISTOGRAM_k%d; i_HISTOGRAM_k%d+=3) {                         \n", i ,i ,i ,i);  
        fprintf(fd, "\t\tval = (unsigned char *)&(fdata_HISTOGRAM_k%d[i_HISTOGRAM_k%d]);                                        \n", i, i);
        fprintf(fd, "\t\tblue_HISTOGRAM_k%d[*val]++;                                                                            \n", i);
        
        fprintf(fd, "\t\tval = (unsigned char *)&(fdata_HISTOGRAM_k%d[i_HISTOGRAM_k%d+1]);                                      \n", i, i);
        fprintf(fd, "\t\tgreen_HISTOGRAM_k%d[*val]++;                                                                           \n", i);
      
        fprintf(fd, "\t\tval = (unsigned char *)&(fdata_HISTOGRAM_k%d[i_HISTOGRAM_k%d+2]);                                      \n", i, i);
        fprintf(fd, "\t\tred_HISTOGRAM_k%d[*val]++;                                                                             \n", i);
        fprintf(fd, "\n\t\tmain_result_HISTOGRAM_k%d+=3;                                                                        \n", i);
        fprintf(fd, "\t}                                                                                                        \n");
#if USE_MEMLUV == 1
#if FREE_INPUTS == 1
        fprintf(fd, "\tMemluvFree((void*)fdata_HISTOGRAM_k%d, (uint_t)(N_HISTOGRAM_k%d*(int_t)sizeof(char)), %d);       \n", i, i, i%NUM_HW_HEAPS);
#endif
#if FREE_OUTPUTS == 1
        fprintf(fd, "\tMemluvFree((void*)blue_HISTOGRAM_k%d, (uint_t)(256*(int_t)sizeof(int)), %d);     \n", i, i%NUM_HW_HEAPS);
        fprintf(fd, "\tMemluvFree((void*)green_HISTOGRAM_k%d, (uint_t)(256*(int_t)sizeof(int)), %d);    \n", i, i%NUM_HW_HEAPS);
        fprintf(fd, "\tMemluvFree((void*)red_HISTOGRAM_k%d, (uint_t)(256*(int_t)sizeof(int)), %d);      \n", i, i%NUM_HW_HEAPS);
#endif
#endif
        fprintf(fd, "\treturn (main_result_HISTOGRAM_k%d != N_HISTOGRAM_k%d); \n", i, i);
        fprintf(fd, "}                                                        \n");
        }
}
