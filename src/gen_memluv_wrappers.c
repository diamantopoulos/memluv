
/**
 * @file   src/gen_malloc_wrappers.h
 * @author Dionysios Diamantopoulos(diamantd@microlab.ntua.gr)
 * @date   January 2014
 *
 * @brief  Generating HW heaps and malloc wrappers for memluv.
 */

#include <time.h>
#include "gen_memluv_wrappers.h"
#include "memluv.h"


void GenMemluvWrappers(void) {
	FILE* fd;
	int i;
	time_t t;
	t = time(NULL);
	
	fd = fopen("memluv_wrappers.c", "w+" );
	if (!fd) {
		printf("Error: Unable to open file %s.\n\n", "memluv_wrappers.c");
		exit(1);
	}
	
	fprintf(fd, "/* \n * Automated generated file on %s * Report bugs @ diamantd@microlab.ntua.gr \n*/\n\n", asctime(localtime(&t)));
	
	/* HW heaps */
	for (i = 0; i < NUM_HW_HEAPS; i++) {
		fprintf(fd, "static MemLuvCore MemLuvCore%d = {											\n", i);
		fprintf(fd, "// Static buffer for new allocations										\n");
		fprintf(fd, ".core = {0},													\n");
		fprintf(fd, ".id = %d,														\n", i);
		fprintf(fd, ".freelist_depth = MEMLUV_DEPTH*sizeof(CORE_UINT_T)/(sizeof(FREELIST_UINT_T)*8),					\n");
		fprintf(fd, ".freelist_total_bits = MEMLUV_DEPTH*sizeof(CORE_UINT_T)/(sizeof(FREELIST_UINT_T)*8)*sizeof(FREELIST_UINT_T)*8,	\n");
		fprintf(fd, ".freelist = {0},     // 1bit represents 1 distinct memory location							\n");
		fprintf(fd, ".size = sizeof(MemLuvCore%d.core),											\n", i);
		fprintf(fd, ".depth = MEMLUV_DEPTH,												\n");
		fprintf(fd, ".base = (var_type_t*)MemLuvCore%d.core,										\n", i);
		fprintf(fd, ".log_align = LOG_ALIGN,												\n");
		fprintf(fd, ".alloc_rqst = 0,													\n");
		fprintf(fd, ".free_rqst = 0,													\n");
		fprintf(fd, ".tot_rqst = 0,													\n");
		fprintf(fd, ".action_status = 0													\n");
		fprintf(fd, "#ifndef __SYNTHESIS__												\n");
		fprintf(fd, ", .fd = NULL,													\n");
		fprintf(fd, ".filename = \"./memluv_stats_heap-%d.txt\",									\n", i);
		fprintf(fd, ".file_init = 1													\n");
		fprintf(fd, "#endif														\n");
		fprintf(fd, "#if HW_DEBUG_MEMLUV==1												\n");
		fprintf(fd, ", .stats = {0}													\n");
		fprintf(fd, "#endif														\n");
		fprintf(fd, "#if MEMLUV_FIRST_FIT_CACHE_LAST_BIN										\n");
		fprintf(fd, ", .ff_cache_bin_in=0													\n");
		fprintf(fd, "#endif														\n");
		
		fprintf(fd, "};															\n\n");
	}
	
	/* MemluvAlloc wrapper */
	fprintf(fd, "void* MemluvAlloc(uint_t nbytes, uint_t heap_id) {		\n");
	fprintf(fd, "#ifdef __SYNTHESIS__					\n");
	fprintf(fd, "#pragma AP inline						\n");
	fprintf(fd, "#endif							\n");
	fprintf(fd, "\tswitch ( heap_id ) {					\n");
	for (i = 1; i < NUM_HW_HEAPS; i++) {
		fprintf(fd, "\tcase %d:							\n", i);
		fprintf(fd, "\t	return ((void*)CurMemluvAlloc(&MemLuvCore%d, nbytes));	\n", i);
		fprintf(fd, "\t	break;							\n");
	}
	fprintf(fd, "\tdefault:							\n");
	fprintf(fd, "\t	return ((void*)CurMemluvAlloc(&MemLuvCore0, nbytes));	\n");
	fprintf(fd, "\t	break;							\n");
	fprintf(fd, "\t}							\n");
	fprintf(fd, "}								\n\n");

	/* MemluvFree wrapper */
	fprintf(fd, "void MemluvFree(CORE_UINT_T *ptr, uint_t nbytes, uint_t heap_id) {		\n");
	fprintf(fd, "#ifdef __SYNTHESIS__							\n");
	fprintf(fd, "#if MEMLUV_HLS_INLINE == 1 || MEMLUV_ALLOC_HEADER > 0			\n");
	fprintf(fd, "#pragma AP inline								\n");
	fprintf(fd, "#else									\n");
	fprintf(fd, "#pragma AP inline off							\n");
	fprintf(fd, "#endif									\n");
	fprintf(fd, "#endif									\n");
	fprintf(fd, "\tswitch ( heap_id ) {							\n");
	for (i = 1; i < NUM_HW_HEAPS; i++) {
		fprintf(fd, "\tcase %d:							\n", i);
		fprintf(fd, "\t\tCurMemluvFree(&MemLuvCore%d, ptr, nbytes);		\n", i);
		fprintf(fd, "\t\tbreak;							\n");
	}
	fprintf(fd, "\tdefault:							\n");
	fprintf(fd, "\t\tCurMemluvFree(&MemLuvCore0, ptr, nbytes);		\n");
	fprintf(fd, "\t\tbreak;							\n");
	fprintf(fd, "\t}								\n\n");
	fprintf(fd, "}								\n\n");
		
	/* MemLuvCoreArray */
	fprintf(fd, "#ifndef __SYNTHESIS__\n");
	fprintf(fd, "static MemLuvCore * MemLuvCoreArray[%d] = {", NUM_HW_HEAPS);
	for (i = 0; i < NUM_HW_HEAPS; i++)
		(i == NUM_HW_HEAPS - 1) ? fprintf(fd, "&MemLuvCore%d};\n", i) : fprintf(fd, "&MemLuvCore%d,", i);
	fprintf(fd, "#endif\n");
	fclose(fd);

	printf("INFO: Completely generated %d HW heaps in memluv_wrappers.c\n", NUM_HW_HEAPS);
	printf("#############################################################\n\n");	
}

int main(void) {

	printf("\n#############################################################\n");
	printf("            GenMemluvWrappers v1.0 - Copyleft 2014\n");
	printf("#############################################################\n");
	GenMemluvWrappers();
	return 0;

}