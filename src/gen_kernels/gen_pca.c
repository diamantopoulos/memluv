
/**
 * @file   src/gen_pca.c
 * @author Dionysios Diamantopoulos(diamantd@microlab.ntua.gr)
 * @date   April 2014
 *
 * @brief   Generating PCA accelerators for memluv evaluation.
 */

#include "../memluv.h"
#include "../gen_memluv_wrappers.h"
#include "gen_accelerators.h"
#include "gen_pca.h"


void GenKernelPCA(FILE *fd) {

	if (NUM_PCA_ACCELERATORS != 0) {
		fprintf(fd, "#include \"../src/gen_kernels/gen_pca.h\"	\n\n");

		fprintf(fd, "/** generate_points()							\n");
		fprintf(fd, "*  Create the values in the matrix						\n");
		fprintf(fd, "*/										\n");
		fprintf(fd, "void generate_points(int *pts, int rows, int cols, int grid_size); 	\n");  
		fprintf(fd, "void generate_points(int *pts, int rows, int cols, int grid_size) { 	\n");
		fprintf(fd, "#ifdef __SYNTHESIS__							\n");
		fprintf(fd, "#pragma AP inline								\n");
		fprintf(fd, "#endif									\n");
		fprintf(fd, "   int i, j;								\n");
#if RANDOM_PCA_VALUES == 1
		fprintf(fd, "\tunsigned short heap_lfsr_ptr;						\n");
		fprintf(fd, "\theap_lfsr_ptr = 0xACE1u;							\n");
#endif
		fprintf(fd, "   for (i=0; i<rows; i++) {						\n");
		fprintf(fd, "      for (j=0; j<cols; j++) {						\n");
#if RANDOM_PCA_VALUES == 1
		fprintf(fd, "\t\tpts[i*cols+j] = (int)RandMinMaxSyn(1, (uint_t)grid_size, &heap_lfsr_ptr, ");
		/* The following statement is not valid, since we provide different random function for testing the static vs memluv approach.
		However it is the only way to overcome the issue that Vivado does not synthesize the circuit with static allocation + random function which uses seeed on stack.
		*/
#if USE_MEMLUV == 1
		fprintf(fd, "1);	\n");
#else
		fprintf(fd, "0);	\n");
#endif
#else
		fprintf(fd, "\t\tpts[i*cols+j] = (int)(i*cols+j+grid_size);");
#endif
		fprintf(fd, "      }							\n");
		fprintf(fd, "   }							\n");
		fprintf(fd, "}								\n\n");

		fprintf(fd, "/** calc_mean()							\n");
		fprintf(fd, "*  Compute the mean for each row					\n");
		fprintf(fd, "*/									\n");
		fprintf(fd, "void calc_mean(int *matrix, int *mean, int rows, int cols);	\n");
		fprintf(fd, "void calc_mean(int *matrix, int *mean, int rows, int cols) {	\n");
		fprintf(fd, "#ifdef __SYNTHESIS__						\n");
		fprintf(fd, "#pragma AP inline							\n");
		fprintf(fd, "#endif								\n");
		fprintf(fd, "   int i, j;							\n");
		fprintf(fd, "   int sum = 0;							\n");

		fprintf(fd, "   for (i = 0; i < rows; i++) {					\n");
		fprintf(fd, "      sum = 0;							\n");
		fprintf(fd, "      for (j = 0; j < cols; j++) {					\n");
		fprintf(fd, "         sum += matrix[i*cols+j];					\n");
		fprintf(fd, "      }								\n");
		fprintf(fd, "      mean[i] = sum / cols;   					\n");
		fprintf(fd, "   }								\n");
		fprintf(fd, "}									\n\n");

		fprintf(fd, "/** calc_cov()										\n");
		fprintf(fd, "*  Calculate the covariance								\n");
		fprintf(fd, "*/												\n");
		fprintf(fd, "void calc_cov(int *matrix, int *mean, int *cov, int rows, int cols);			\n");
		fprintf(fd, "void calc_cov(int *matrix, int *mean, int *cov, int rows, int cols) {			\n");
		fprintf(fd, "#ifdef __SYNTHESIS__									\n");
		fprintf(fd, "#pragma AP inline										\n");
		fprintf(fd, "#endif											\n");
		fprintf(fd, "   int i, j, k;										\n");
		fprintf(fd, "   int sum;										\n");
   
		fprintf(fd, "   for (i = 0; i < rows; i++) {								\n");
		fprintf(fd, "      for (j = i; j < rows; j++) {								\n");
		fprintf(fd, "         sum = 0;										\n");
		fprintf(fd, "         for (k = 0; k < cols; k++) {							\n");
		fprintf(fd, "            sum = sum + ((matrix[i*cols+k] - mean[i]) * (matrix[j*cols+k] - mean[j]));	\n");
		fprintf(fd, "         }											\n");
		fprintf(fd, "         cov[i*rows+j] = cov[j*rows+i] = sum/(cols-1);					\n");
		fprintf(fd, "      }   											\n");
		fprintf(fd, "   }   											\n");
		fprintf(fd, "}												\n\n");
	}
	
	int i;
	for (i = 0; i < NUM_PCA_ACCELERATORS; i++) {
	fprintf(fd, "/* PCA KERNEL %d */						\n", i);
	fprintf(fd, "int PCA_k%d(void);							\n", i);
	fprintf(fd, "int PCA_k%d(void) {						\n", i);
	fprintf(fd, "#ifdef __SYNTHESIS__						\n");
	fprintf(fd, "#pragma AP inline ");
	if (PCA_HLS_INLINE== 0) 
		fprintf(fd, "off							\n");
	else if (PCA_HLS_INLINE== 2) 
		fprintf(fd, "recursive							\n");
	else
		fprintf(fd, "								\n");
	fprintf(fd, "#endif								\n");
	
	fprintf(fd, "\tint i,j,main_result_PCA_k%d=0, N_PCA_k%d = N_PCA_MAX;		\n", i, i);
	
	fprintf(fd, "\tint num_rows = PCA_DEF_NUM_ROWS;					\n");
	fprintf(fd, "\tint num_cols = PCA_DEF_NUM_COLS;					\n");
	fprintf(fd, "\tint grid_size = PCA_DEF_GRID_SIZE;				\n");

	 // Create the matrix to store the points
#if USE_MEMLUV == 1
	fprintf(fd, "\tint *matrix=(int*)MemluvAlloc((uint_t)(num_rows*num_cols*(int_t)sizeof(int)), %d);	\n", i, i%NUM_HW_HEAPS);
#else
	fprintf(fd, "\tint matrix[PCA_DEF_NUM_ROWS*PCA_DEF_NUM_COLS];			\n");
#endif
	//Generate random values for all the points in the matrix
	fprintf(fd, "\tgenerate_points(matrix, num_rows, num_cols, grid_size);		\n");
	// Allocate Memory to store the mean and the covariance matrix
#if USE_MEMLUV == 1
	fprintf(fd, "\tint *mean=(int*)MemluvAlloc((uint_t)(num_rows*(int_t)sizeof(int)), %d);	\n", i, i%NUM_HW_HEAPS);
	fprintf(fd, "\tint *cov=(int*)MemluvAlloc((uint_t)(num_rows*num_rows*(int_t)sizeof(int)), %d);	\n", i, i%NUM_HW_HEAPS);
#else
	fprintf(fd, "\tint mean[PCA_DEF_NUM_ROWS];					\n");
	fprintf(fd, "\tint cov[PCA_DEF_NUM_ROWS*PCA_DEF_NUM_COLS];			\n");
#endif
	// Compute the mean and the covariance
	fprintf(fd, "\tcalc_mean(matrix, mean, num_rows, num_cols);			\n");
	fprintf(fd, "\tcalc_cov(matrix, mean, cov, num_rows, num_cols);			\n");
	
#if USE_MEMLUV == 1
#if FREE_INPUTS == 1
	fprintf(fd, "\tMemluvFree((void*)matrix, (uint_t)(num_rows*num_cols*(int_t)sizeof(int)), %d);	\n", i, i%NUM_HW_HEAPS);
#endif
#if FREE_OUTPUTS == 1
	fprintf(fd, "\tMemluvFree((void*)mean, (uint_t)(num_rows*(int_t)sizeof(int)), %d);	\n", i, i%NUM_HW_HEAPS);
#endif
#endif

	fprintf(fd, "\tfor (i=0; i<num_rows; i++)					\n");
	fprintf(fd, "\t\tfor (j=0; j<num_rows; j++)					\n");
	fprintf(fd, "\t\t\tmain_result_PCA_k%d += cov[i*num_rows+j];			\n", i);


#if USE_MEMLUV == 1
#if FREE_OUTPUTS == 1
	fprintf(fd, "\tMemluvFree((void*)cov, (uint_t)(num_rows*num_rows*(int_t)sizeof(int)), %d);	\n", i, i%NUM_HW_HEAPS);
#endif
#endif
	fprintf(fd, "\treturn (main_result_PCA_k%d + N_PCA_k%d);		\n", i, i);
	fprintf(fd, "}									\n");
	}
}