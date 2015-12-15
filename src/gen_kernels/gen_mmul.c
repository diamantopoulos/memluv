
/**
 * @file   src/gen_mmul.c
 * @author Dionysios Diamantopoulos(diamantd@microlab.ntua.gr)
 * @date   April 2014
 *
 * @brief   Generating Matrix multiplication accelerators for memluv evaluation.
 */

#include "../memluv.h"
#include "../gen_memluv_wrappers.h"
#include "gen_accelerators.h"
#include "gen_mmul.h"


void GenKernelMMUL(FILE *fd) {
	
	if (NUM_MMUL_ACCELERATORS != 0) {
		fprintf(fd, "#include \"../src/gen_kernels/gen_mmul.h\"	\n\n");
	}
	int i;
	for (i = 0; i < NUM_MMUL_ACCELERATORS; i++) {
	fprintf(fd, "\n/* MMUL KERNEL %d */						\n", i);
	fprintf(fd, "int MMUL_k%d(void);						\n", i);
	fprintf(fd, "int MMUL_k%d(void) {						\n", i);
	fprintf(fd, "#ifdef __SYNTHESIS__						\n");
	fprintf(fd, "#pragma AP inline ");
	if (MMUL_HLS_INLINE== 0) 
		fprintf(fd, "off							\n");
	else if (MMUL_HLS_INLINE== 2) 
		fprintf(fd, "recursive							\n");
	else
		fprintf(fd, "								\n");
	fprintf(fd, "#endif								\n");
	
	fprintf(fd, "\tint main_result_MMUL_k%d = 0, matrix_len = N_MMUL_MAX;		\n", i);
	fprintf(fd, "\tint i, j, k,a, b, c, end_i, end_j, end_k;			\n");

	
	 /* Create the matrices to store input/output */
#if USE_MEMLUV == 1
	fprintf(fd, "\tint *matrix_A=(int*)MemluvAlloc((uint_t)(matrix_len*matrix_len*(int_t)sizeof(int)), %d);		\n", i%NUM_HW_HEAPS);
	fprintf(fd, "\tint *matrix_B=(int*)MemluvAlloc((uint_t)(matrix_len*matrix_len*(int_t)sizeof(int)), %d);		\n", i%NUM_HW_HEAPS);
	fprintf(fd, "\tint *matrix_out=(int*)MemluvAlloc((uint_t)(matrix_len*matrix_len*(int_t)sizeof(int)), %d);	\n", i%NUM_HW_HEAPS);
#else
	fprintf(fd, "\tint matrix_A[matrix_len*matrix_len];								\n");
	fprintf(fd, "\tint matrix_B[matrix_len*matrix_len];								\n");
	fprintf(fd, "\tint matrix_out[matrix_len*matrix_len];								\n");
#endif
	
#if RANDOM_MMUL_VALUES == 1
	fprintf(fd, "\tunsigned short heap_lfsr_ptr;					\n");
	fprintf(fd, "\theap_lfsr_ptr = 0xACE1u;						\n");
#endif
	fprintf(fd, "\tfor(i = 0; i < matrix_len; i++) {						\n");
	fprintf(fd, "\t   for(j = 0; j < matrix_len; j++) {						\n");
	fprintf(fd, "\t      matrix_A[(matrix_len)*i + j] = (int)(i+j);					\n");
	fprintf(fd, "\t      matrix_B[(matrix_len)*i + j] = (int)(matrix_len-j);			\n");
#if RANDOM_MMUL_VALUES == 1
	fprintf(fd, "\t      matrix_out[(matrix_len)*i + j] = (int)RandMinMaxSyn(1, (uint_t)matrix_len, &heap_lfsr_ptr, ");
#if USE_MEMLUV == 1
	fprintf(fd, "1);	\n");
#else
	fprintf(fd, "0);	\n");
#endif
#else
	fprintf(fd, "\t      matrix_out[(matrix_len)*i + j] =  matrix_len*i + j;			\n");
#endif
	fprintf(fd, "\t   }										\n");
	fprintf(fd, "\t}										\n");
	/* The following statement is not valid, since we provide different random function for testing the static vs memluv approach.
	However it is the only way to overcome the issue that Vivado does not synthesize the circuit with static allocation + random function which uses seeed on stack.
	*/
	fprintf(fd, "\tfor(i = 0; i < matrix_len; i += BLOCK_LEN)					\n");
	fprintf(fd, "\t\tfor(j = 0; j < matrix_len; j += BLOCK_LEN)					\n");
	fprintf(fd, "\t\t\tfor(k = 0; k < matrix_len; k += BLOCK_LEN) {					\n");
	fprintf(fd, "\t\t\t\tend_i = i + BLOCK_LEN; end_j = j + BLOCK_LEN; end_k = k + BLOCK_LEN;	\n");
	fprintf(fd, "\t\t\t\tfor (a = i; a < end_i && a < matrix_len; a++)				\n");
	fprintf(fd, "\t\t\t\t\tfor (b = j; b < end_j && b < matrix_len; b++)				\n");
	fprintf(fd, "\t\t\t\t\t\tfor (c = k; c < end_k && c < matrix_len; c++) {			\n");
	fprintf(fd, "\t\t\t\t\t\t\tmatrix_out[(matrix_len)*a + b] += 				        \n");
	fprintf(fd, "\t\t\t\t\t\t\t( matrix_A[ (matrix_len)*a + c] * 			                \n");
	fprintf(fd, "\t\t\t\t\t\t\tmatrix_B[ (matrix_len)*c + b]);	                                \n");
	fprintf(fd, "\t\t\t\t\t\t}									\n");
	fprintf(fd, "\t\t\t\t\t}									\n");
#if USE_MEMLUV == 1 && FREE_INPUTS == 1
	fprintf(fd, "\tMemluvFree((void*)matrix_A, (uint_t)(matrix_len*matrix_len*(int_t)sizeof(int)), %d);	\n", i%NUM_HW_HEAPS);
	fprintf(fd, "\tMemluvFree((void*)matrix_B, (uint_t)(matrix_len*matrix_len*(int_t)sizeof(int)), %d);	\n", i%NUM_HW_HEAPS);
#endif
	fprintf(fd, "\tfor(i = 0; i < matrix_len; i++) {						\n");
	fprintf(fd, "\t\tfor(j = 0; j < matrix_len; j++) {						\n");
	fprintf(fd, "\t\t\tmain_result_MMUL_k%d += matrix_out[(matrix_len)*i + j];			\n", i);
	fprintf(fd, "\t\t}										\n");
	fprintf(fd, "\t}										\n");

#if USE_MEMLUV == 1 && FREE_OUTPUTS == 1
	fprintf(fd, "\tMemluvFree((void*)matrix_out, (uint_t)(matrix_len*matrix_len*(int_t)sizeof(int)), %d);	\n", i%NUM_HW_HEAPS);
#endif
	fprintf(fd, "\treturn (main_result_MMUL_k%d + matrix_len);						\n", i);
	fprintf(fd, "}												\n");
	}
}