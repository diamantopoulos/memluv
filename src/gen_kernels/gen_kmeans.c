
/**
 * @file   src/gen_kmeans.c
 * @author Dionysios Diamantopoulos(diamantd@microlab.ntua.gr)
 * @date   April 2014
 *
 * @brief   Generating kmeans accelerators for memluv evaluation.
 */

#include "../memluv.h"
#include "../gen_memluv_wrappers.h"
#include "gen_accelerators.h"
#include "gen_kmeans.h"


void GenKernelKmeans(FILE *fd) {
	if (NUM_KMEANS_ACCELERATORS != 0) {
		fprintf(fd, "#include \"../src/gen_kernels/gen_kmeans.h\"	\n\n");
		fprintf(fd, "typedef struct {						\n");
		fprintf(fd, "\tint num_points; // number of vectors			\n");
		fprintf(fd, "\tint dim;        // Dimension of each vector		\n");
		fprintf(fd, "\tint num_means; // number of clusters			\n");
		fprintf(fd, "\tint grid_size; // size of each dimension of vector space	\n");
		fprintf(fd, "\tint modified;						\n");
		fprintf(fd, "} SETUP_T;							\n\n");
		
		fprintf(fd, "/** generate_points()					\n");
		fprintf(fd, " *  Generate the points					\n");
		fprintf(fd, " */							\n");
		fprintf(fd, "void generate_points(int *pts, int size, SETUP_T *setup);	\n");
		fprintf(fd, "void generate_points(int *pts, int size, SETUP_T *setup) {	\n");
		fprintf(fd, "#ifdef __SYNTHESIS__					\n");
		fprintf(fd, "#pragma AP inline						\n");
		fprintf(fd, "#endif							\n");
		fprintf(fd, "\tint i, j;						\n");
#if RANDOM_KMEANS_VALUES == 1
		fprintf(fd, "\tunsigned short heap_lfsr_ptr;				\n");
		fprintf(fd, "\theap_lfsr_ptr = 0xACE1u;					\n");
#endif
		fprintf(fd, "\tfor (i=0; i<size; i++) {					\n");
		fprintf(fd, "\t\tfor (j=0; j<setup->dim; j++) {				\n");
#if RANDOM_KMEANS_VALUES == 1
		fprintf(fd, "\t\t\tpts[(i*setup->dim)+j] = (int)RandMinMaxSyn(1, (uint_t)setup->grid_size, &heap_lfsr_ptr, ");
#if USE_MEMLUV == 1
	fprintf(fd, "1);	\n");
#else
	fprintf(fd, "0);	\n");
#endif
#else
		fprintf(fd, "\t\t\tpts[(i*setup->dim)+j] = setup->grid_size+i+j;	\n");
#endif
		fprintf(fd, "\t\t}							\n");
		fprintf(fd, "\t}							\n");
		fprintf(fd, "}								\n\n");
		
		fprintf(fd, "/** get_sq_dist()																\n");
		fprintf(fd, "*  Get the squared distance between 2 points												\n");
		fprintf(fd, "*/																		\n");
		fprintf(fd, "static inline unsigned int get_sq_dist(int *v1, int v1_row, int *v2, int v2_row, SETUP_T *setup);						\n");
		fprintf(fd, "static inline unsigned int get_sq_dist(int *v1, int v1_row, int *v2, int v2_row, SETUP_T *setup) {						\n");
		fprintf(fd, "#ifdef __SYNTHESIS__															\n");
		fprintf(fd, "#pragma AP inline																\n");
		fprintf(fd, "#endif																	\n");
		fprintf(fd, "\tint i;																	\n");
		fprintf(fd, "\tunsigned int sum = 0;															\n");
		fprintf(fd, "\tfor (i = 0; i < setup->dim; i++) {													\n");
		fprintf(fd, "\t\tsum += (unsigned int)((v1[v1_row*setup->dim+i] - v2[v2_row*setup->dim+i]) * (v1[v1_row*setup->dim+i] - v2[v2_row*setup->dim+i]));	\n");
		fprintf(fd, "\t}																	\n");
		fprintf(fd, "\treturn sum;																\n");
		fprintf(fd, "}																		\n\n");

		fprintf(fd, "/** add_to_sum()								\n");
		fprintf(fd, " *	Helper function to update the total distance sum			\n");
		fprintf(fd, " */									\n");
		fprintf(fd, "void add_to_sum(int *sum, int *point, int point_row, SETUP_T *setup);	\n");
		fprintf(fd, "void add_to_sum(int *sum, int *point, int point_row, SETUP_T *setup) {	\n");
		fprintf(fd, "#ifdef __SYNTHESIS__							\n");
		fprintf(fd, "#pragma AP inline								\n");
		fprintf(fd, "#endif									\n");
		fprintf(fd, "\tint i;									\n");

		fprintf(fd, "\tfor (i = 0; i < setup->dim; i++) {					\n");
		fprintf(fd, "\t\tsum[i] += point[point_row*setup->dim+i];  				\n");  
		fprintf(fd, "\t}    									\n");
		fprintf(fd, "}										\n\n");
		
		fprintf(fd, "/** find_clusters()								\n");
		fprintf(fd, " *  Find the cluster that is most suitable for a given set of points		\n");
		fprintf(fd, " */										\n");
		fprintf(fd, "void find_clusters(int *points, int *means, int *clusters, SETUP_T *setup);	\n");
		fprintf(fd, "void find_clusters(int *points, int *means, int *clusters, SETUP_T *setup) {	\n");
		fprintf(fd, "#ifdef __SYNTHESIS__								\n");
		fprintf(fd, "#pragma AP inline									\n");
		fprintf(fd, "#endif										\n");
		fprintf(fd, "\tint i, j;									\n");
		fprintf(fd, "\tunsigned int min_dist, cur_dist;							\n");
		fprintf(fd, "\tint min_idx;									\n");
		fprintf(fd, "\tfor (i = 0; i < setup->num_points; i++) {					\n");
		fprintf(fd, "\t\tmin_dist = get_sq_dist(points, i, means, 0, setup);				\n");
		fprintf(fd, "\t\tmin_idx = 0; 									\n");
		fprintf(fd, "\t\tfor (j = 1; j < setup->num_means; j++) {					\n");
		fprintf(fd, "\t\t\tcur_dist = get_sq_dist(points, i, means, j, setup);				\n");
		fprintf(fd, "\t\t\tif (cur_dist < min_dist) {							\n");
		fprintf(fd, "\t\t\t\tmin_dist = cur_dist;							\n");
		fprintf(fd, "\t\t\t\tmin_idx = j;    								\n");
		fprintf(fd, "\t\t\t}										\n");
		fprintf(fd, "\t\t}										\n");
        	fprintf(fd, "\t\tif (clusters[i] != min_idx) {							\n");
		fprintf(fd, "\t\t\tclusters[i] = min_idx;							\n");
		fprintf(fd, "\t\t\tsetup->modified = true;							\n");
		fprintf(fd, "\t\t}										\n");
		fprintf(fd, "\t}    										\n");
		fprintf(fd, "}											\n\n");
		
		fprintf(fd, "/** calc_means()								\n");
		fprintf(fd, " *  Compute the means for the various clusters				\n");
		fprintf(fd, " */									\n");
		fprintf(fd, "void calc_means(int *points, int *means, int *clusters, SETUP_T *setup);	\n");
		fprintf(fd, "void calc_means(int *points, int *means, int *clusters, SETUP_T *setup) {	\n");
		fprintf(fd, "#ifdef __SYNTHESIS__							\n");
		fprintf(fd, "#pragma AP inline								\n");
		fprintf(fd, "#endif									\n");
		fprintf(fd, "\tint i, j, grp_size;							\n");
		//fprintf(fd, "\tint *sum;								\n");
		//fprintf(fd, "\tsum = (int *)malloc((unsigned int)setup->dim * sizeof(int));		\n");
		fprintf(fd, "\tint sum[KMEANS_DEF_DIM];							\n");
		fprintf(fd, "\tfor (i = 0; i < setup->num_means; i++) {					\n");
		fprintf(fd, "\t\tfor(j=0; j<setup->dim; j++) 						\n");
		fprintf(fd, "\t\t\tsum[j] = 0;								\n");
		fprintf(fd, "\t\tgrp_size = 0;								\n");
		fprintf(fd, "\t\tfor (j = 0; j < setup->num_points; j++) {				\n");
		fprintf(fd, "\t\t\tif (clusters[j] == i) {						\n");
		fprintf(fd, "\t\t\t\tadd_to_sum(sum, points, j, setup);					\n");
		fprintf(fd, "\t\t\t\tgrp_size++;							\n");
		fprintf(fd, "\t\t\t}    								\n");
		fprintf(fd, "\t\t}									\n");
		fprintf(fd, "\t\tfor (j = 0; j < setup->dim; j++) {					\n");
		fprintf(fd, "\t\t\tif (grp_size != 0) { 						\n");
		fprintf(fd, "\t\t\t\tmeans[i*setup->dim+j] = sum[j] / grp_size;				\n");
		fprintf(fd, "\t\t\t}									\n");
		fprintf(fd, "\t\t}   									\n");      
		fprintf(fd, "\t}									\n");
		fprintf(fd, "}										\n\n");
		
		fprintf(fd, "/** calc_final()							\n");
		fprintf(fd, " *  Helper function to return the sum of all means and dims	\n");
		fprintf(fd, " */								\n");
		fprintf(fd, "int calc_final(int *means, SETUP_T *setup);			\n");
		fprintf(fd, "int calc_final(int *means, SETUP_T *setup) {			\n");
		fprintf(fd, "#ifdef __SYNTHESIS__						\n");
		fprintf(fd, "#pragma AP inline							\n");
		fprintf(fd, "#endif								\n");
		fprintf(fd, "\tint i, j, final = 0;						\n");
		fprintf(fd, "\tfor (i = 0; i < setup->num_means; i++) {				\n");
		fprintf(fd, "\t\tfor (j = 0; j < setup->dim; j++) {				\n");
		fprintf(fd, "\t\t\tfinal += means[i*setup->dim+j];				\n");
		fprintf(fd, "\t\t}								\n");
		fprintf(fd, "\t}								\n");
		fprintf(fd, "\treturn (final);							\n");
		fprintf(fd, "}									\n\n");
	}
	int i;
	for (i = 0; i < NUM_KMEANS_ACCELERATORS; i++) {
		fprintf(fd, "\n/* KMEANS KERNEL %d */							\n", i);
		fprintf(fd, "int KMEANS_k%d(void);							\n", i);
		fprintf(fd, "int KMEANS_k%d(void) {							\n", i);
		fprintf(fd, "#ifdef __SYNTHESIS__							\n");
		fprintf(fd, "#pragma AP inline ");
		if (KMEANS_HLS_INLINE== 0) 
			fprintf(fd, "off								\n");
		else if (KMEANS_HLS_INLINE== 2) 
			fprintf(fd, "recursive								\n");
		else
			fprintf(fd, "									\n");
		fprintf(fd, "#endif									\n");
		fprintf(fd, "\tint main_result_KMEANS_k%d=0, N_KMEANS_k%d = N_KMEANS_MAX;		\n", i, i);
#if USE_MEMLUV == 1	
		fprintf(fd, "\tint *points, *means, *clusters;						\n");
#endif
		fprintf(fd, "\tint i;									\n");
#if USE_MEMLUV == 1
		fprintf(fd, "\tSETUP_T *setup = (SETUP_T *)MemluvAlloc((uint_t)sizeof(SETUP_T), %d);	\n", i%NUM_HW_HEAPS);
#else
		fprintf(fd, "\tSETUP_T setup_static; 							\n");
		fprintf(fd, "\tSETUP_T *setup = &setup_static; 						\n");
#endif
		fprintf(fd, "\tsetup->num_points = KMEANS_DEF_NUM_POINTS;				\n");
		fprintf(fd, "\tsetup->num_means = KMEANS_DEF_NUM_MEANS;					\n");
		fprintf(fd, "\tsetup->dim = KMEANS_DEF_DIM;						\n");
		fprintf(fd, "\tsetup->grid_size = KMEANS_DEF_GRID_SIZE;					\n");
#if USE_MEMLUV == 1
		//fprintf(fd, "\tpoints = (int *)malloc((unsigned int)setup->num_points * (unsigned int)setup->dim * sizeof(int));	\n");
		fprintf(fd, "\tpoints = (int *)MemluvAlloc((uint_t)((unsigned int)setup->num_points * (unsigned int)setup->dim * (uint_t)sizeof(int)), %d);	\n", i%NUM_HW_HEAPS);
#else
		fprintf(fd, "\tint points[setup->num_points*setup->dim];				\n");
#endif
		fprintf(fd, "\tgenerate_points(points, setup->num_points, setup); 			\n");
#if USE_MEMLUV == 1
		//fprintf(fd, "\tmeans = (int *)malloc((unsigned int)setup->num_means * (unsigned int)setup->dim * sizeof(int));	\n");
		fprintf(fd, "\tmeans = (int *)MemluvAlloc((uint_t)((unsigned int)setup->num_means * (unsigned int)setup->dim * (uint_t)sizeof(int)), %d);	\n", i%NUM_HW_HEAPS);
#else
		fprintf(fd, "\tint means[setup->num_means*setup->dim];					\n");
#endif
		fprintf(fd, "\tgenerate_points(means, setup->num_means, setup);				\n");
#if USE_MEMLUV == 1
		//fprintf(fd, "\tclusters = (int *)malloc(sizeof(int) * (unsigned int)setup->num_points);	\n");
		fprintf(fd, "\tclusters = (int *)MemluvAlloc((uint_t)((unsigned int)setup->num_points * (uint_t)sizeof(int)), %d);	\n", i%NUM_HW_HEAPS);
#else
		fprintf(fd, "\tint clusters[setup->num_points];						\n");
#endif
		fprintf(fd, "\tfor(i=0; i<setup->num_points; i++) 					\n");
		fprintf(fd, "\t\tclusters[i] = -1;							\n");
		fprintf(fd, "\tsetup->modified = true;							\n");
		fprintf(fd, "\twhile (setup->modified) {						\n");
		fprintf(fd, "\t\tsetup->modified = false;						\n");
		fprintf(fd, "\t\tfind_clusters(points, means, clusters, setup);				\n");
		fprintf(fd, "\t\tcalc_means(points, means, clusters, setup);				\n");
		fprintf(fd, "\t}									\n");
#if USE_MEMLUV == 1 && FREE_INPUTS == 1
		//fprintf(fd, "\tfree(points);								\n");
		fprintf(fd, "\tMemluvFree((void*)points, (uint_t)((unsigned int)setup->num_points * (unsigned int)setup->dim * (uint_t)sizeof(int)), %d);	\n", i%NUM_HW_HEAPS);
#endif
#if USE_MEMLUV == 1 && FREE_OUTPUTS == 1
		//fprintf(fd, "\tfree(clusters);								\n");
		fprintf(fd, "\tMemluvFree((void*)clusters, (uint_t)((unsigned int)setup->num_points * (uint_t)sizeof(int)), %d);			\n", i%NUM_HW_HEAPS);
#endif
		fprintf(fd, "\tmain_result_KMEANS_k%d = calc_final(means, setup);			\n", i);
		//fprintf(fd, "\tdump_matrix(means, setup->num_means, setup->dim);			\n");
#if USE_MEMLUV == 1 && FREE_INPUTS == 1
		fprintf(fd, "\tMemluvFree((void*)setup, (uint_t)(sizeof(SETUP_T)), %d);			\n", i%NUM_HW_HEAPS);
#endif
#if USE_MEMLUV == 1 && FREE_OUTPUTS == 1
		//fprintf(fd, "\tfree(means);								\n");
		fprintf(fd, "\tMemluvFree((void*)means, (uint_t)((unsigned int)setup->num_means * (unsigned int)setup->dim * (uint_t)sizeof(int)), %d);	\n", i%NUM_HW_HEAPS);
#endif
		
		fprintf(fd, "\treturn (main_result_KMEANS_k%d + N_KMEANS_k%d);				\n", i, i);
		fprintf(fd, "}										\n");
	}
}
