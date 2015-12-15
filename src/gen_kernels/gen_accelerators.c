
/**
 * @file   src/gen_accelerators.c
 * @author Dionysios Diamantopoulos(diamantd@microlab.ntua.gr)
 * @date   January 2014
 *
 * @brief   Generating accelerators for memluv evaluation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../memluv.h"
#include "../gen_memluv_wrappers.h"
#include "gen_accelerators.h"
#include "gen_dfadd.h"
#include "gen_dfmul.h"
#include "gen_regression.h"
#include "gen_pca.h"
#include "gen_mmul.h"
#include "gen_kmeans.h"
#include "gen_strmatch.h"
#include "gen_histogram.h"
#include "../../unittest/include/gen_unittest.h"

double ullong_to_double (unsigned long long x)
{
  union
  {
    double d;
    unsigned long long ll;
  } t;

  t.ll = x;
  return t.d;
}

int main(void) {
	
	FILE* fd;
	int j;
	time_t t;
	t = time(NULL);
	
	printf("\n#############################################################\n");
	printf("            GenAccelerators v1.0 - Copyleft 2013-2104\n");
	printf("#############################################################\n");
	
	fd = fopen("accelerators.c", "w+" );
	if (!fd) {
		printf("Error: Unable to open file %s.\n\n", "accelerators.c");
		exit(1);
	}
	
	fprintf(fd, "/* \n * Automated generated file on %s * Report bugs @ diamantd@microlab.ntua.gr \n*/\n\n", asctime(localtime(&t)));
	
	fprintf(fd, "#include <stdio.h>						\n");
	fprintf(fd, "#include <stdlib.h>					\n");
	fprintf(fd, "#include \"../src/gen_kernels/gen_accelerators.h\"		\n");
	fprintf(fd, "#include \"../src/memluv.h\"				\n");
	fprintf(fd, "#include \"../src/global.h\"				\n\n");
	
	/* DFADD Accelerators */
	GenKernelDFADD(fd);
	
	/* Histogram Accelerators */
	GenKernelHistogram(fd);
	
	/* Regression Accelerators */
	GenKernelRegression(fd);
	
	/* PCA Accelerators */
	GenKernelPCA(fd);
	
	/* MMUL Accelerators */
	GenKernelMMUL(fd);
	
	/* Kmeans Accelerators */
	GenKernelKmeans(fd);	
	
	/* String-match Accelerators */
	GenKernelStrmatch(fd);
        
        /* Unit-test Accelerators */
        GenUnitTest(fd);
        
	fprintf(fd, "\n /* MAIN */					\n");
	fprintf(fd, "int accelerators(					\n");
	fprintf(fd, "\t\t#if HW_DEBUG_MEMLUV==1				\n");		
	fprintf(fd, "\t\t\tstruct MemLuvStats *mlvstats			\n");
	fprintf(fd, "\t\t#else						\n");
	fprintf(fd, "\t\t\tvoid						\n");
	fprintf(fd, "\t\t#endif						\n");
	fprintf(fd, "\t\t);						\n");
	fprintf(fd, "int accelerators(					\n");
	fprintf(fd, "\t\t#if HW_DEBUG_MEMLUV==1				\n");		
	fprintf(fd, "\t\t\tstruct MemLuvStats *mlvstats			\n");
	fprintf(fd, "\t\t#endif						\n");
	fprintf(fd, "\t\t) {						\n");	
	
	fprintf(fd, "\tint main_result=0;				\n");
	for (j = 0; j < NUM_DFADD_ACCELERATORS; j++)
		fprintf(fd, "\tint main_result_DFADD_k%d=0;		\n", j);
	for (j = 0; j < NUM_HISTOGRAM_ACCELERATORS; j++)
		fprintf(fd, "\tint main_result_HISTOGRAM_k%d=0;		\n", j);
	for (j = 0; j < NUM_REGRESSION_ACCELERATORS; j++)
		fprintf(fd, "\tint main_result_REGRESSION_k%d=0;	\n", j);
	for (j = 0; j < NUM_PCA_ACCELERATORS; j++)
		fprintf(fd, "\tint main_result_PCA_k%d=0;		\n", j);
	for (j = 0; j < NUM_MMUL_ACCELERATORS; j++)
		fprintf(fd, "\tint main_result_MMUL_k%d=0;		\n", j);
	for (j = 0; j < NUM_KMEANS_ACCELERATORS; j++)
		fprintf(fd, "\tint main_result_KMEANS_k%d=0;		\n", j);
	for (j = 0; j < NUM_STRMATCH_ACCELERATORS; j++)
		fprintf(fd, "\tint main_result_STRMATCH_k%d=0;		\n", j);
        for (j = 0; j < NUM_UNITTEST_THREADS; j++)
                fprintf(fd, "\tint main_result_UNITTEST_k%d=0;          \n", j);
        
	for (j = 0; j < NUM_DFADD_ACCELERATORS; j++)
		fprintf(fd, "\tmain_result_DFADD_k%d=DFADD_k%d();		\n", j, j);
	for (j = 0; j < NUM_HISTOGRAM_ACCELERATORS; j++)
		fprintf(fd, "\tmain_result_HISTOGRAM_k%d=HISTOGRAM_k%d();	\n", j, j);
	for (j = 0; j < NUM_REGRESSION_ACCELERATORS; j++)
		fprintf(fd, "\tmain_result_REGRESSION_k%d=REGRESSION_k%d();	\n", j, j);
	for (j = 0; j < NUM_PCA_ACCELERATORS; j++)
		fprintf(fd, "\tmain_result_PCA_k%d=PCA_k%d();			\n", j, j);
	for (j = 0; j < NUM_MMUL_ACCELERATORS; j++)
		fprintf(fd, "\tmain_result_MMUL_k%d=MMUL_k%d();			\n", j, j);
	for (j = 0; j < NUM_KMEANS_ACCELERATORS; j++)
		fprintf(fd, "\tmain_result_KMEANS_k%d=KMEANS_k%d();		\n", j, j);
	for (j = 0; j < NUM_STRMATCH_ACCELERATORS; j++)
		fprintf(fd, "\tmain_result_STRMATCH_k%d=STRMATCH_k%d();		\n", j, j);
	for (j = 0; j < NUM_UNITTEST_THREADS; j++)
                fprintf(fd, "\tmain_result_UNITTEST_k%d=UNITTEST_k%d();         \n", j, j);

	fprintf(fd, "\tmain_result = ");
	for (j = 0; j < NUM_DFADD_ACCELERATORS; j++)
            (j == NUM_DFADD_ACCELERATORS-1) ?
                fprintf(fd, "main_result_DFADD_k%d; \n", j) :
                fprintf(fd, "main_result_DFADD_k%d + ", j);

        for (j = 0; j < NUM_HISTOGRAM_ACCELERATORS; j++)
	    (j == NUM_HISTOGRAM_ACCELERATORS-1) ?
                fprintf(fd, "main_result_HISTOGRAM_k%d; \n", j) :
                fprintf(fd, "main_result_HISTOGRAM_k%d + ", j);

        for (j = 0; j < NUM_REGRESSION_ACCELERATORS; j++)
		(j == NUM_REGRESSION_ACCELERATORS-1) ?
		fprintf(fd, "main_result_REGRESSION_k%d; \n", j) :
		fprintf(fd, "main_result_REGRESSION_k%d == ", j);
        
	for (j = 0; j < NUM_PCA_ACCELERATORS; j++)
		(j == NUM_PCA_ACCELERATORS-1) ? 
		fprintf(fd, "main_result_PCA_k%d; \n", j) : 
		fprintf(fd, "main_result_PCA_k%d + ", j);
        
	for (j = 0; j < NUM_MMUL_ACCELERATORS; j++)
		(j == NUM_MMUL_ACCELERATORS-1) ? 
		fprintf(fd, "main_result_MMUL_k%d; \n", j) :
		fprintf(fd, "main_result_MMUL_k%d + ", j);
        
	for (j = 0; j < NUM_KMEANS_ACCELERATORS; j++)
		(j == NUM_KMEANS_ACCELERATORS-1) ? 
		fprintf(fd, "main_result_KMEANS_k%d; \n", j) :
		fprintf(fd, "main_result_KMEANS_k%d + ", j);
        
	for (j = 0; j < NUM_STRMATCH_ACCELERATORS; j++)
		(j == NUM_STRMATCH_ACCELERATORS-1) ? 
		fprintf(fd, "main_result_STRMATCH_k%d; \n", j) :
		fprintf(fd, "main_result_STRMATCH_k%d + ", j);
        
        for (j = 0; j < NUM_UNITTEST_THREADS; j++)
                (j == NUM_UNITTEST_THREADS-1) ? 
                fprintf(fd, "main_result_UNITTEST_k%d; \n", j) :
                fprintf(fd, "main_result_UNITTEST_k%d + ", j);
        
	fprintf(fd, "#if HW_DEBUG_MEMLUV==1			\n");
	fprintf(fd, "\t*mlvstats=ReturnMemLuvStats();		\n");
	fprintf(fd, "#endif					\n");
	fprintf(fd, "\treturn main_result;			\n");
	fprintf(fd, "}						\n");

	fclose(fd);

	printf("INFO: Completely generated %d HW threads in accelerators.c\n", 
                                                                        NUM_DFADD_ACCELERATORS+
                                                                        NUM_DFMUL_ACCELERATORS+
                                                                        NUM_HISTOGRAM_ACCELERATORS+
                                                                        NUM_REGRESSION_ACCELERATORS+
                                                                        NUM_PCA_ACCELERATORS+
                                                                        NUM_MMUL_ACCELERATORS+
                                                                        NUM_KMEANS_ACCELERATORS+
                                                                        NUM_STRMATCH_ACCELERATORS+
                                                                        NUM_UNITTEST_THREADS);
        
	printf("INFO:\tDFADD:\t\t%d\n\tDFMUL:\t\t%d\n\tHISTOGRAM:\t%d\n\tREGRESSION:\t%d\n\t"
                      "PCA:\t\t%d\n\tMMUL:\t\t%d\n\tKMEANS:\t\t%d\n\tSTRMATCH:\t%d\n\tUNITTEST:\t%d\n",
                       NUM_DFADD_ACCELERATORS, NUM_DFMUL_ACCELERATORS, NUM_HISTOGRAM_ACCELERATORS, 
                       NUM_REGRESSION_ACCELERATORS, NUM_PCA_ACCELERATORS, NUM_MMUL_ACCELERATORS, 
                       NUM_KMEANS_ACCELERATORS, NUM_STRMATCH_ACCELERATORS, NUM_UNITTEST_THREADS);
	printf("#############################################################\n\n");
	return 0;
}