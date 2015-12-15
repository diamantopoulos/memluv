
/**
 * @file   src/test_accelerators.c
 * @author Dionysios Diamantopoulos(diamantd@microlab.ntua.gr)
 * @date   January 2014
 *
 * @brief  Test accelerators with Memluv functions.
 */

#include <stdio.h>
#include <stdlib.h>
#include "memluv.h"
#include "global.h"

#include "../build/accelerators.c"

long long wtime(void);

long long wtime() {
#ifndef __SYNTHESIS__
    struct timeval tv;
    gettimeofday(&tv, 0);
    return (long long)(tv.tv_sec*1000000LL + tv.tv_usec);    // caculate milliseconds
    //return (double)(tv.tv_sec+1e-6*tv.tv_usec); // caculate seconds
#else
    return(0);
#endif
}

int main(void)
{
	int returned;
        long long start=0;
        long long algo_exec_time=0;
        start = wtime();
	MemLuvCore *mlvcore;
	struct MemLuvStats mlvstats;
	mlvcore = ReturnMemLuvCore();
	long vmrss, vmsize;
	MemluvInit();
	MemluvInfo(NULL, stdout, ALL);
        algo_exec_time = wtime();
	returned = accelerators(
				#if HW_DEBUG_MEMLUV==1
					&mlvstats
				#endif
				);
        algo_exec_time = wtime() - algo_exec_time;
	printf("TB DEBUG: [final=%u] ", returned);
	printf("[max_address_reached=%u] [Core Usage=%u%%] mlvdbg.result=%u [Total B allocated=%u] [Total B requested=%u] [Total B fragmented=%u]\n\n", 
		mlvstats.max_address_reached, mlvstats.used_percentage , mlvstats.result, 
		mlvstats.total_bytes_allocated, mlvstats.total_bytes_requested, mlvstats.total_fragmented_bytes);
	get_memory_usage_kb(&vmrss, &vmsize);
	printf("TB DEBUG: Current memory usage: VmRSS = %6ld KB, VmSize = %6ld KB\n", vmrss, vmsize);
        printf("INFO: [Elapsed time: %lld us]  [Internal time: %lld us]\n\n", wtime() - start, algo_exec_time);
#if SIM_WITH_GLIBC_MALLOC==0
	MemluvDumpFreeList(mlvcore, ALL);
	MemluvDumpCore(mlvcore, ALL);
#endif

	return 0;
}
