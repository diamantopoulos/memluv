
/**
 * @file   src/gen_dfadd.h
 * @author Dionysios Diamantopoulos(diamantd@microlab.ntua.gr)
 * @date   April 2014
 *
 * @brief  Generator of DFADD accelerators for memluv evaluation.
 */

#include <stdio.h>
#include "../../apps/CHStone_v1.8_130128/dfadd/softfloat.c"
#define READ_DFADD_HEAP


#include "../memluv.h"
#include "../gen_memluv_wrappers.h"
#include "gen_accelerators.h"
#include "gen_dfadd.h"


void GenKernelDFADD(FILE* fd) {
int i, iter;
if (NUM_DFADD_ACCELERATORS > 0) {
    fprintf(fd, "#include \"../apps/CHStone_v1.8_130128/dfadd/softfloat.c\"     \n");
#if DFADD_FEED_AND_COMPARE_FROM_WITH_ROM == 1
    fprintf(fd, "#define READ_DFADD_HEAP                                        \n");
#else
    fprintf(fd, "#define N_DFADD_MAX N_DFADD_REAL_MAX                           \n");   
#endif
    fprintf(fd, "#include \"../src/gen_kernels/gen_dfadd.h\"                    \n\n");
}
for (i = 0; i < NUM_DFADD_ACCELERATORS; i++) {
	fprintf(fd, "\n/* DFADD KERNEL %d */                                    \n", i);
	fprintf(fd, "int DFADD_k%d(void);                                       \n", i);
	fprintf(fd, "int DFADD_k%d(void) {                                      \n", i);
	fprintf(fd, "#ifdef __SYNTHESIS__                                       \n");
	fprintf(fd, "#pragma AP inline ");
	if (DFADD_HLS_INLINE== 0) 
		fprintf(fd, "off\n");
	else if (DFADD_HLS_INLINE== 2) 
		fprintf(fd, "recursive\n");
	else
		fprintf(fd, "\n");
	fprintf(fd, "#endif                                                     \n");
	fprintf(fd, "int main_result_DFADD_k%d=0, loops_DFADD_k%d, N_DFADD_k%d; \n", i, i, i);
//	fprintf(fd, "#pragma AP dataflow interval=3                             \n");
	fprintf(fd, "int i_DFADD_k%d;                                           \n", i);
	fprintf(fd, "float64 x1_k%d, x2_k%d, y_k%d, result_k%d;                 \n", i, i, i, i);
	iter=0;
#if USE_MEMLUV == 1
	for (iter=0; iter< DFADD_ITERATIONS_PER_KERNEL; iter++)
		fprintf(fd, "float64 *a_input_DFADD_k%d_iter%d, *b_input_DFADD_k%d_iter%d,"
                            " *z_output_DFADD_k%d_iter%d;	\n", i, iter, i, iter, i, iter);
#else
	for (iter=0; iter< DFADD_ITERATIONS_PER_KERNEL; iter++)
		fprintf(fd, "float64 a_input_DFADD_k%d_iter%d[N_DFADD_MAX],"
                            " b_input_DFADD_k%d_iter%d[N_DFADD_MAX],"
                            " z_output_DFADD_k%d_iter%d[N_DFADD_MAX]; \n",
                i, iter, i, iter, i, iter);
#endif
#if RANDOM_DFADD_SIZE == 1
        fprintf(fd, "unsigned short heap_lfsr_ptr;                              \n");
        fprintf(fd, "heap_lfsr_ptr = 0xACE1u;                                   \n");
#endif
        fprintf(fd, "loops_DFADD_k%d = 0;                                       \n", i);
	fprintf(fd, "while (loops_DFADD_k%d++ < TRAFFIC_LOOPS_DFADD) {          \n", i);
//	fprintf(fd, "#pragma HLS UNROLL                                         \n");
#if RANDOM_DFADD_SIZE == 1
	fprintf(fd, "\tN_DFADD_k%d=(int)RandMinMaxSyn(1, N_DFADD_MAX, &heap_lfsr_ptr, ", i);
//#if USE_MEMLUV == 1
//       fprintf(fd, "1);        \n");
//#else
        fprintf(fd, "0);        \n");
//#endif
#else
	fprintf(fd, "\tN_DFADD_k%d=N_DFADD_MAX; \n", i);
#endif		
	for (iter=0; iter< DFADD_ITERATIONS_PER_KERNEL; iter++) {
#if USE_MEMLUV == 1
	/* Accelerator's Memory Allocation */
		fprintf(fd, "\ta_input_DFADD_k%d_iter%d="
                            "(float64*)MemluvAlloc((uint_t)(N_DFADD_k%d*(int_t)sizeof(float64)), %d);\n",
                        i, iter, i, i);
		fprintf(fd, "\tb_input_DFADD_k%d_iter%d="
                            "(float64*)MemluvAlloc((uint_t)(N_DFADD_k%d*(int_t)sizeof(float64)), %d);\n",
                        i, iter, i, i);
		fprintf(fd, "\tz_output_DFADD_k%d_iter%d="
                            "(float64*)MemluvAlloc((uint_t)(N_DFADD_k%d*(int_t)sizeof(float64)), %d);\n",
                        i, iter, i, i);
#if DFADD_SEQUENTIAL_PROCESSING == 1
	}
#endif
#endif
	/* Accelerator's Data Initialization */
	fprintf(fd, "\tfor (i_DFADD_k%d = 0; i_DFADD_k%d < N_DFADD_k%d; i_DFADD_k%d++) {\n",
                i, i, i, i);
#if DFADD_SEQUENTIAL_PROCESSING == 1
	for (iter=0; iter< DFADD_ITERATIONS_PER_KERNEL; iter++) {
#endif
		fprintf(fd, "\t\ta_input_DFADD_k%d_iter%d[i_DFADD_k%d] = (float64)(", i, iter, i);
		(DFADD_FEED_AND_COMPARE_FROM_WITH_ROM == 0) ? fprintf(fd, "i_DFADD_k%d); \n", i) : 
		fprintf(fd, "a_input_DFADD_k%d_iter%d[i_DFADD_k%d]); \n", iter, i, i);
		fprintf(fd, "\t\tb_input_DFADD_k%d_iter%d[i_DFADD_k%d] = (float64)(", i, iter, i);
		(DFADD_FEED_AND_COMPARE_FROM_WITH_ROM == 0) ? fprintf(fd, "i_DFADD_k%d*2); \n", i) :
		fprintf(fd, "b_input_DFADD_k%d_iter%d[i_DFADD_k%d]); \n", iter, i, i);
		fprintf(fd, "\t\tz_output_DFADD_k%d_iter%d[i_DFADD_k%d] = (float64)(", i, iter, i);
		(DFADD_FEED_AND_COMPARE_FROM_WITH_ROM == 0) ? 
		fprintf(fd, "i_DFADD_k%d*2 + i_DFADD_k%d); \n", i, i) : 
		fprintf(fd, "z_output_DFADD_k%d_iter%d[i_DFADD_k%d]); \n", iter, i, i);
#if DFADD_SEQUENTIAL_PROCESSING == 1
	}
#endif
	fprintf(fd, "\t}                                                \n");
		
	/* Accelerator's Processing */
#if DFADD_SEQUENTIAL_PROCESSING == 1
	for (iter=0; iter< DFADD_ITERATIONS_PER_KERNEL; iter++) {
#endif
		fprintf(fd, "\tfor (i_DFADD_k%d = 0; i_DFADD_k%d < N_DFADD_k%d; i_DFADD_k%d++) {\n",
                        i, i, i, i);
		fprintf(fd, "\t\tx1_k%d = a_input_DFADD_k%d_iter%d[i_DFADD_k%d];\n",
                        i, i, iter, i);
		fprintf(fd, "\t\tx2_k%d = b_input_DFADD_k%d_iter%d[i_DFADD_k%d];\n",
                        i, i, iter, i);
		fprintf(fd, "\t\tresult_k%d = float64_add (x1_k%d, x2_k%d);\n",
                        i, i, i);
		fprintf(fd, "\t\ty_k%d = z_output_DFADD_k%d_iter%d[i_DFADD_k%d];\n",
                        i, i, iter, i);
		fprintf(fd, "\t\tmain_result_DFADD_k%d += (result_k%d != y_k%d);\n",
                        i, i, i);
		fprintf(fd, "\t}                                                \n");
#if DFADD_SEQUENTIAL_PROCESSING == 1
	}
#endif
#if USE_MEMLUV == 1
#if DFADD_SEQUENTIAL_PROCESSING == 1
	for (iter=0; iter< DFADD_ITERATIONS_PER_KERNEL; iter++) {
#endif
#if FREE_INPUTS == 1
		fprintf(fd, "\tMemluvFree ((void*)a_input_DFADD_k%d_iter%d,"
                            " (uint_t)(N_DFADD_k%d*(int_t)sizeof(float64)), %d);        \n", 
                        i, iter, i, i);
		fprintf(fd, "\tMemluvFree ((void*)b_input_DFADD_k%d_iter%d,"
                            " (uint_t)(N_DFADD_k%d*(int_t)sizeof(float64)), %d);        \n",
                        i, iter, i, i);
#endif
#if FREE_OUTPUTS == 1
		fprintf(fd, "\tMemluvFree ((void*)z_output_DFADD_k%d_iter%d,"
                            " (uint_t)(N_DFADD_k%d*(int_t)sizeof(float64)), %d);\n",
                        i, iter, i, i);
#endif
#endif
	}
	fprintf(fd, "}                                  \n");
	fprintf(fd, "\treturn (main_result_DFADD_k%d);  \n", i);
	fprintf(fd, "}                                  \n");
	}
}