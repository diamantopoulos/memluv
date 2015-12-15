
/**
 * @file   unittest/src/gen_unittest.c
 * @author Dionysios Diamantopoulos(diamantd@microlab.ntua.gr)
 * @date   June 2014
 *
 * @brief   Generating a unit test for memluv evaluation.
 */

#include "../../src/memluv.h"
#include "../../src/gen_memluv_wrappers.h"
#include "../include/gen_unittest.h"

uint8_t once_flag=0;

uint_t RandMinMaxUnitTest(uint_t min, uint_t max ) {
        if (max < min) {
                printf("ERROR: Cannot produce a random integer in [%d,%d]. Aborting...", min, max);
                exit(-1);
        }
        if (once_flag==0) {
            once_flag=1;
            srand (GEN_UNITTEST_SEED);
        }
        return ((uint_t)rand() % (max - min + 1) + min);
}

void GenUnitTest(FILE *fd) {
	
	if (NUM_UNITTEST_THREADS != 0) {
		fprintf(fd, "#include \"../unittest/include/gen_unittest.h\"	\n\n");
	}
	int i,j;
	for (i = 0; i < NUM_UNITTEST_THREADS; i++) {
            fprintf(fd, "\n/* UNITTEST KERNEL %d */                             \n", i);
            fprintf(fd, "int UNITTEST_k%d(void);                                \n", i);
            fprintf(fd, "int UNITTEST_k%d(void) {                               \n", i);
            fprintf(fd, "#ifdef __SYNTHESIS__                                   \n");
            fprintf(fd, "#pragma AP inline ");
            if (UNITTEST_HLS_INLINE== 0) 
                fprintf(fd, "off                                                \n");
            else if (UNITTEST_HLS_INLINE== 2) 
                fprintf(fd, "recursive                                          \n");
            else
                fprintf(fd, "                                                   \n");
            fprintf(fd, "#endif                                                 \n");

            fprintf(fd, "\tint i;                                               \n");
            fprintf(fd, "\tint main_result_UNITTEST_k%d = 0, matrix_len;        \n", i);

            for (j = 0; j < NUM_UNITTEST_LOOPS; j++) {
                fprintf(fd, "\tint main_result_UNITTEST_k%d_l%d = 0;            \n", i, j);
                fprintf(fd, "\tmatrix_len = %u;                                 \n", 
                        RandMinMaxUnitTest(1, N_UNITTEST_MAX));

                /* Create the matrices to store input/output */
#if USE_MEMLUV == 1
                fprintf(fd, "\tint *matrix_A_l%d=(int*)MemluvAlloc((uint_t)(matrix_len*(int_t)sizeof(int)), %d);     \n", j, i%NUM_HW_HEAPS);
                fprintf(fd, "\tint *matrix_B_l%d=(int*)MemluvAlloc((uint_t)(matrix_len*(int_t)sizeof(int)), %d);     \n", j, i%NUM_HW_HEAPS);
                fprintf(fd, "\tint *matrix_out_l%d=(int*)MemluvAlloc((uint_t)(matrix_len*(int_t)sizeof(int)), %d);   \n", j, i%NUM_HW_HEAPS);
#else
            fprintf(fd, "\tint matrix_A_l%d[matrix_len];                     \n", j);
            fprintf(fd, "\tint matrix_B_l%d[matrix_len];                     \n", j);
            fprintf(fd, "\tint matrix_out_l%d[matrix_len];                   \n", j);
#endif
	
#if RANDOM_UNITTEST_VALUES == 1
            fprintf(fd, "\tunsigned short heap_lfsr_ptr;                                \n");
            fprintf(fd, "\theap_lfsr_ptr = 0xACE1u;                                     \n");
#endif
            fprintf(fd, "\tfor(i = 0; i < matrix_len; i++) {                            \n");
            fprintf(fd, "\t\tmatrix_A_l%d[i] = (int)(i);                                \n", j);
            fprintf(fd, "\t\tmatrix_B_l%d[i] = (int)(matrix_len-i);                     \n", j);
#if RANDOM_UNITTEST_VALUES == 1
            fprintf(fd, "\t      matrix_out_l%d[i] = (int)RandMinMaxSyn(1, (uint_t)matrix_len,"
                        " &heap_lfsr_ptr, ", j);
#if USE_MEMLUV == 1
            fprintf(fd, "1);    \n");
#else
            fprintf(fd, "0);    \n");
#endif
#else
            fprintf(fd, "\t      matrix_out_l%d[i] =  matrix_len*i;                     \n", j);
#endif
            fprintf(fd, "\t}                                                            \n");
            /* The following statement is not valid, since we provide different random function for
             * testing the static vs memluv approach. However it is the only way to overcome the 
             * issue that Vivado does not synthesize the circuit with static allocation + 
             * random function which uses seeed on stack.
             */
            fprintf(fd, "\tfor(i = 0; i < matrix_len; i += BLOCK_LEN)                   \n");
            fprintf(fd, "\t\tmatrix_out_l%d[i] +=                                       \n", j);
            fprintf(fd, "\t\t( matrix_A_l%d[i] *                                        \n", j);
            fprintf(fd, "\t\tmatrix_B_l%d[i]);                                          \n", j);
#if USE_MEMLUV == 1 && FREE_INPUTS == 1
#if UNITTEST_RAND_FREE == 1
            if (RandMinMaxUnitTest(0,1) == 1) 
#endif
	    {
                fprintf(fd, "\tMemluvFree((void*)matrix_A_l%d, (uint_t)(matrix_len*(int_t)sizeof(int)), %d);     \n", j, i%NUM_HW_HEAPS);
                fprintf(fd, "\tMemluvFree((void*)matrix_B_l%d, (uint_t)(matrix_len*(int_t)sizeof(int)), %d);     \n", j, i%NUM_HW_HEAPS);
            }
#endif
            fprintf(fd, "\tfor(i = 0; i < matrix_len; i++) {                            \n");
            fprintf(fd, "\t\tmain_result_UNITTEST_k%d_l%d += matrix_out_l%d[i];  \n", i, j, j);
            fprintf(fd, "\t}                                                             \n");

#if USE_MEMLUV == 1 && FREE_OUTPUTS == 1
#if UNITTEST_RAND_FREE == 1
            if (RandMinMaxUnitTest(0,1) == 1)
#endif
                fprintf(fd, "\tMemluvFree((void*)matrix_out_l%d, (uint_t)(matrix_len*"
                            "(int_t)sizeof(int)), %d);       \n", j, i%NUM_HW_HEAPS);
#endif
            fprintf(fd, "\tmain_result_UNITTEST_k%d += main_result_UNITTEST_k%d_l%d;    \n", i, i, j);
        }
        fprintf(fd, "\treturn (main_result_UNITTEST_k%d);                               \n", i);
        fprintf(fd, "}                                                                  \n");
    }
}