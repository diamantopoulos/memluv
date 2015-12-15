
/**
 * @file   src/gen_regression.c
 * @author Dionysios Diamantopoulos(diamantd@microlab.ntua.gr)
 * @date   April 2014
 *
 * @brief   Generating regression accelerators for memluv evaluation.
 */

#include "../memluv.h"
#include "../gen_memluv_wrappers.h"
#include "gen_accelerators.h"
#include "gen_regression.h"


void GenKernelRegression(FILE *fd) {
	if (NUM_REGRESSION_ACCELERATORS != 0) {
		fprintf(fd, "#include \"../src/gen_kernels/gen_regression.h\"	\n\n");
		fprintf(fd, "typedef struct {			\n");
		fprintf(fd, "char x;				\n");
		fprintf(fd, "char y;				\n");
		fprintf(fd, "} POINT_T;				\n\n");

        fprintf(fd, "long long hlsmul(char x, char y);          \n");
        fprintf(fd, "long long hlsmul(char x, char y) {         \n");
        fprintf(fd, "#ifdef __SYNTHESIS__                       \n");             
        fprintf(fd, "#pragma AP inline off                      \n");
        fprintf(fd, "#endif                                     \n");
        fprintf(fd, "\treturn(x*y);                             \n");
        fprintf(fd, "}                                          \n");
	}

	int i;
	for (i = 0; i < NUM_REGRESSION_ACCELERATORS; i++) {
	fprintf(fd, "\n/* REGRESSION KERNEL %d */						\n", i);
	fprintf(fd, "int REGRESSION_k%d(void);							\n", i);
	fprintf(fd, "int REGRESSION_k%d(void) {							\n", i);
	fprintf(fd, "#ifdef __SYNTHESIS__							\n");
	fprintf(fd, "#pragma AP inline ");
	if (REGRESSION_HLS_INLINE== 0) 
		fprintf(fd, "off								\n");
	else if (REGRESSION_HLS_INLINE== 2) 
		fprintf(fd, "recursive								\n");
	else
		fprintf(fd, "									\n");
	fprintf(fd, "#endif									\n");
	
	fprintf(fd, "\tint main_result_REGRESSION_k%d=0, N_REGRESSION_k%d = N_REGRESSION_MAX;	\n", i, i);
	
#if USE_MEMLUV == 1
	fprintf(fd, "\tPOINT_T *points;								\n");
#else
	fprintf(fd, "\tPOINT_T points[N_REGRESSION_k%d];					\n", i);
#endif
	fprintf(fd, "\tlong long n = (long long) N_REGRESSION_k%d;				\n", i);
	fprintf(fd, "\tlong long SX_ll = 0, SY_ll = 0, SXX_ll = 0, SYY_ll = 0, SXY_ll = 0;	\n");
	fprintf(fd, "\tlong i;									\n");
   
#if USE_MEMLUV == 1
	/* Accelerator's Memory Allocation */
	fprintf(fd, "\tpoints=(POINT_T*)MemluvAlloc((uint_t)(N_REGRESSION_k%d*(int_t)sizeof(POINT_T)), %d);	\n", i, i%NUM_HW_HEAPS);
#endif
#if RANDOM_REGRESSION_VALUES == 1
	fprintf(fd, "\tunsigned short heap_lfsr_ptr;							\n");
	fprintf(fd, "\theap_lfsr_ptr = 0xACE1u;								\n");
#endif
	fprintf(fd, "\tfor (i = 0; i < n; i++) {							\n");
#if RANDOM_REGRESSION_VALUES == 1
	fprintf(fd, "\t\tpoints[i].x = points[i].y = (char)RandMinMaxSyn(1, (uint_t)i+1, &heap_lfsr_ptr, ");
/* The following statement is not valid, since we provide different random function for testing the static vs memluv approach.
   However it is the only way to overcome the issue that Vivado does not synthesize the circuit with static allocation + random function which uses seeed on stack.
*/
#if USE_MEMLUV == 1
	fprintf(fd, "1);	\n");
#else
	fprintf(fd, "0);	\n");
#endif
#else
	fprintf(fd, "\t\tpoints[i].x = points[i].y = (char)i;	        				\n");
#endif
	fprintf(fd, "\t}										\n");
	// ADD UP RESULTS
	fprintf(fd, "\tfor (i = 0; i < n; i++) {							\n");
	//Compute SX, SY, SYY, SXX, SXY
	fprintf(fd, "\t\tSX_ll  += points[i].x;							        \n");
	fprintf(fd, "\t\tSXX_ll += hlsmul(points[i].x,points[i].x);					\n");
	fprintf(fd, "\t\tSY_ll  += points[i].y;			        				\n");
	fprintf(fd, "\t\tSYY_ll += hlsmul(points[i].y,points[i].y);					\n");
	fprintf(fd, "\t\tSXY_ll += hlsmul(points[i].x,points[i].y);					\n");
	fprintf(fd, "\t}										\n");

	fprintf(fd, "\tdouble a, b, xbar, ybar, r2;						        \n");
	fprintf(fd, "\tdouble SX = (double)SX_ll;							\n");
	fprintf(fd, "\tdouble SY = (double)SY_ll;							\n");
	fprintf(fd, "\tdouble SXX= (double)SXX_ll;						        \n");
	fprintf(fd, "\tdouble SYY= (double)SYY_ll;      						\n");
	fprintf(fd, "\tdouble SXY= (double)SXY_ll;	        					\n");

	fprintf(fd, "\tb = (double)((double)n*SXY - SX*SY) / ((double)n*SXX - SX*SX);				\n");
	fprintf(fd, "\ta = ((double)SY_ll - b*(double)SX_ll) /(double)n;							\n");
	fprintf(fd, "\txbar = (double)SX_ll / (double)n;							\n");
	fprintf(fd, "\tybar = (double)SY_ll / (double)n;							\n");
	fprintf(fd, "\tr2 = (double)((double)n*SXY - SX*SY) * ((double)n*SXY - SX*SY) / (((double)n*SXX - SX*SX)*((double)n*SYY - SY*SY));\n");
	fprintf(fd, "\tmain_result_REGRESSION_k%d = (int)(r2 + a + b + xbar + ybar);			\n", i);
#if USE_MEMLUV == 1
#if FREE_INPUTS == 1
	fprintf(fd, "\tMemluvFree((void*)points, (uint_t)(N_REGRESSION_k%d*(int_t)sizeof(POINT_T)), %d);	\n", i, i%NUM_HW_HEAPS);
#endif
#endif
	fprintf(fd, "\treturn (main_result_REGRESSION_k%d + N_REGRESSION_k%d);		\n", i, i);
	fprintf(fd, "}									\n");
	}
}