
/**
 * @file   src/util.c
 * @author Dionysios Diamantopoulos(diamantd@microlab.ntua.gr)
 * @date   March 2014
 *
 * @brief  Memluv auxiliary functions, i.e. timing, printing wrappers, random generators etc.
 */

#include "util.h"

void Dprintf(int8_t level, const char* format, ... ) {
#ifndef __SYNTHESIS__
#if DEBUG_MEMLUV==1 && SIM_MINIMUM_OVERHEAD==0
	if (level<=DEBUG_MEMLUV_LEVEL) {
		va_list arglist;
		va_start( arglist, format );
		vprintf( format, arglist );
		va_end( arglist );
	}
#endif
#endif
}

void Dfprintf(int8_t level, FILE *stream, const char* format, ... ) {
#ifndef __SYNTHESIS__
#if DEBUG_MEMLUV==1 && SIM_MINIMUM_OVERHEAD==0
	if (level<=DEBUG_MEMLUV_LEVEL) {
		va_list arglist;
		va_start( arglist, format );
		vfprintf(stream, format, arglist );
		va_end( arglist );
	}
#endif
#endif
}

void bprintf( const char* format, ... ) {
#ifndef __SYNTHESIS__
#if DEBUG_MEMLUV==1 && SIM_MINIMUM_OVERHEAD==0
	va_list arglist;
	va_start( arglist, format );
	vprintf( format, arglist );
	va_end( arglist );
#endif
#endif
}

/* Print n as a binary number */
void Uint_tToBin(FREELIST_UINT_T n) {
	FREELIST_UINT_T i;
	char j=8;
	i = (FREELIST_UINT_T)(1<<(sizeof(n) * 8 - 1));
	while (i > 0) {
		if (j++%8 == 0)
			Dfprintf(2, CurMemLuvConf->dbg_fd, " ");
		if (n & i)
			Dfprintf(2, CurMemLuvConf->dbg_fd, "1");
		else
			Dfprintf(2, CurMemLuvConf->dbg_fd, "0");
		i >>= 1;
	}
}

void DumpStatisticsToFile(MemLuvCore *CurMemLuvCore 
#if USE_MEMLUV==1 && HW_DEBUG_MEMLUV==1		  
			  ,uint8_t action 
			  ,uint_t nbytes
#endif
			){
#ifndef __SYNTHESIS__
#if SIM_MINIMUM_OVERHEAD==0
	if (CurMemLuvCore->file_init==1) {
		CurMemLuvCore->file_init=0;
		if (NULL == (CurMemLuvCore->fd = fopen(CurMemLuvCore->filename, "w"))) {
			printf ("ERROR: Could not create statistics output file. Aborting...\n");
			exit (-1);
		}
		TimeAllToFile(CurMemLuvCore->fd);
		MemluvInfo(CurMemLuvCore, CurMemLuvCore->fd, THIS);
		Dfprintf(1, CurMemLuvCore->fd, "\t\t\t       \t[Core ]\t      \t[TotalBytes]\t[TotalBytes]\t[TotalBytes]\t[TotalBytes]        \n");
		Dfprintf(1, CurMemLuvCore->fd, "  [Time (us)]\t\t[Ov/wr]\t[Usage]\tTotRqst\t[allocated ]\t[requested ]\t[fragmented]\t[forHeaders]\t[Action]\t[Status]\t[TotReqFragB]\n");
		Dfprintf(1, CurMemLuvCore->fd, "----------------------------------------------------------------------------------------------------------------------------------------------------------\n");
		TimeMicroSecToFile(CurMemLuvCore->fd);
		Dfprintf(1, CurMemLuvCore->fd, "\t\t\t\t\t\t\t\t\t\t\t\tInit\n");
		if ( fclose(CurMemLuvCore->fd) != 0) {
			printf ("ERROR: Could not close statistics output file after opening. Aborting...\n");
			exit (-1);
		}
	}
	else {
		if (NULL == (CurMemLuvCore->fd = fopen(CurMemLuvCore->filename, "a"))) {
			printf ("ERROR: Could not append to statistics output file. Aborting...\n");
			exit (-1);
		}
		TimeMicroSecToFile(CurMemLuvCore->fd);
#if USE_MEMLUV==1
#if HW_DEBUG_MEMLUV==1
		Dfprintf(1, CurMemLuvCore->fd, "\t[%u]\t[%.3u%%]\t%u\t%.10u\t%.10u\t[%.10u]\t[%.10u]\t%s %u B\t%d\t\t%.10u\n", 
			CurMemLuvCore->stats.max_address_reached, CurMemLuvCore->stats.used_percentage , CurMemLuvCore->tot_rqst, 
			CurMemLuvCore->stats.total_bytes_allocated, CurMemLuvCore->stats.total_bytes_requested, CurMemLuvCore->stats.total_fragmented_bytes, CurMemLuvCore->stats.total_bytes_for_headers, (action==1)? "Malloc +" : "Free   -", nbytes, CurMemLuvCore->action_status, CurMemLuvCore->stats.total_request_fragmented_bytes);
#else
		Dfprintf(1, CurMemLuvCore->fd, "\t\t\t\t\t\t\t\t\t\t\t\tN/A\t\tHW_DEBUG_MEMLUV=0\n");
#endif
#else		
		Dfprintf(1, CurMemLuvCore->fd, "\t\t\t\t\t\t\t\t\t\t\t\tN/A\t\tUSE_MEMLUV=0\n");
#endif
		if ( fclose(CurMemLuvCore->fd) != 0) {
			printf ("ERROR: Could not close statistics output file after appending. Aborting...\n");
			exit (-1);
		}
		else
			CurMemLuvCore->fd = NULL;
	}
#endif
#endif
}

void TimeAllToFile(FILE *stream) {
  time_t now;
  struct timeval tv;
  gettimeofday(&tv, 0);
  long long microseconds = tv.tv_sec*1000000LL + tv.tv_usec; // caculate milliseconds
  time(&now);
  Dfprintf(1, stream, "@ %lld us - %s ", microseconds, ctime(&now));
}

void TimeSecToFile(FILE *stream) {
  struct timeval tv;
  gettimeofday(&tv, 0);
  Dfprintf(1, stream, "@ %f: ", (double)(tv.tv_sec+(__time_t)(((__suseconds_t)1e-6)*tv.tv_usec)));
}

static uint8_t start_time=0;
static long long start_microseconds;
void TimeMicroSecToFile(FILE *stream) {
	struct timeval te; 
	gettimeofday(&te, NULL); // get current time
	long long microseconds = te.tv_sec*1000000LL + te.tv_usec; // caculate milliseconds
	if (start_time==0) {
		start_microseconds=microseconds;
		start_time=1;
	}
	microseconds -= start_microseconds;    
	Dfprintf(1, stream, "@ %.16lld ", microseconds);
}

int lrand()
{
  static unsigned long long next = 0;
  next = next * 0x5deece66dLL + 11;
  return (int)((next >> 16) & 0x7fffffff);
}

int rsize()
{
  int rv = 8 << (lrand() % 15);
  rv = lrand() & (rv-1);
  return rv;
}

uint_t RandMinMax(uint_t min, uint_t max ) {
        if (max < min)
                bprintf("ERROR: Cannot produce a random integer in [%d,%d]", min, max);
        return ((uint_t)rand() % (max - min + 1) + min);
}

/** The version of RandMinMax above with seed as an argument.
 *  mode is used to choose between assign seed from heap (0) or stack (1).
 *  Stack -> the seed is kept on shared variable lfsr, forbits Vivado to exploit parallelization on multiple calls from different kernels.
 *  Heap -> no shared variable, leading to full parallelization.
 */
uint_t RandMinMaxSyn(uint_t min, uint_t max, unsigned short *lfsr_ptr, uint8_t mode) {
#ifdef __SYNTHESIS__
#if MEMLUV_HLS_INLINE == 0
#pragma AP inline off
#else
#pragma AP inline
#endif
#endif
        if (max < min)
                bprintf("ERROR: Cannot produce a random integer in [%d,%d]", min, max);
        return ((uint_t)LfsrRandSyn(*lfsr_ptr, mode) % (max - min + 1) + min);
}

/* An ultra-simple pseudo-random generator based on a
 * Linear feedback shift register
 * http://stackoverflow.com/questions/7602919/how-do-i-generate-random-numbers-without-rand-function
 * http://en.wikipedia.org/wiki/Linear_feedback_shift_register
 */
unsigned short heap_lfsr = 0xACE1u;
unsigned LfsrRand(void) {
    unsigned bit;
    bit  = ((heap_lfsr >> 0) ^ (heap_lfsr >> 2) ^ (heap_lfsr >> 3) ^ (heap_lfsr >> 5) ) & 1;
    return (unsigned)((heap_lfsr =  (heap_lfsr >> 1) | (unsigned short)(bit << 15)));
}

/** The version of LfsrRand above with seed as an argument. Faulty use, only for Vivado HLS purpose.  */
unsigned LfsrRandSyn(unsigned short stack_lfsr, uint8_t mode) {
#ifdef __SYNTHESIS__
#if MEMLUV_HLS_INLINE == 0
#pragma AP inline off
#else
#pragma AP inline
#endif
#endif
    unsigned bit;
    unsigned short *cur_lfsr;
    if (mode == 0)
	    cur_lfsr = &heap_lfsr;
    else
	    cur_lfsr = &stack_lfsr;
    bit  = ((*cur_lfsr >> 0) ^ (*cur_lfsr >> 2) ^ (*cur_lfsr >> 3) ^ (*cur_lfsr >> 5) ) & 1;
    return (unsigned)((*cur_lfsr =  (*cur_lfsr >> 1) | (unsigned short)(bit << 15)));
}

/*
   32-bits Random number generator U[0,1): lfsr113
   Author: Pierre L'Ecuyer,
   Source: http://www.iro.umontreal.ca/~lecuyer/myftp/papers/tausme2.ps
   ---------------------------------------------------------
*/
/**** VERY IMPORTANT **** :
  The initial seeds z1, z2, z3, z4  MUST be larger than
  1, 7, 15, and 127 respectively.
****/
#define SEED 987654321
static uint32_t z1 = SEED, z2 = SEED, z3 = SEED, z4 = SEED;
double lfsr113 (void)
{
   uint32_t b;
   b  = ((z1 << 6) ^ z1) >> 13;
   z1 = ((z1 & 4294967294U) << 18) ^ b;
   b  = ((z2 << 2) ^ z2) >> 27;
   z2 = ((z2 & 4294967288U) << 2) ^ b;
   b  = ((z3 << 13) ^ z3) >> 21;
   z3 = ((z3 & 4294967280U) << 7) ^ b;
   b  = ((z4 << 3) ^ z4) >> 12;
   z4 = ((z4 & 4294967168U) << 13) ^ b;
   return (z1 ^ z2 ^ z3 ^ z4) * 2.3283064365386963e-10;
}

/*
* Look for lines in the procfile contents like: 
* VmRSS:		 5560 kB
* VmSize:		 5560 kB
*
* Grab the number between the whitespace and the "kB"
* If 1 is returned in the end, there was a serious problem 
* (we could not find one of the memory usages)
*/
int get_memory_usage_kb(long* vmrss_kb, long* vmsize_kb)
{
	/* Get the the current process' status file from the proc filesystem */
	FILE* procfile = fopen("/proc/self/status", "r");

	size_t to_read = 8192;
	char buffer[to_read];
	if (0 == fread(buffer, sizeof(char), to_read, procfile))
		Dprintf(1, "Warning: Error opening /proc/self/status to read vm data.\n");
	fclose(procfile);

	short found_vmrss = 0;
	short found_vmsize = 0;
	char* search_result;

	/* Look through proc status contents line by line */
	char delims[] = "\n";
	char* line = strtok(buffer, delims);

	while (line != NULL && (found_vmrss == 0 || found_vmsize == 0) )
	{
		search_result = strstr(line, "VmRSS:");
		if (search_result != NULL)
		{
			sscanf(line, "%*s %ld", vmrss_kb);
			found_vmrss = 1;
		}

		search_result = strstr(line, "VmSize:");
		if (search_result != NULL)
		{
			sscanf(line, "%*s %ld", vmsize_kb);
			found_vmsize = 1;
		}

		line = strtok(NULL, delims);
	}

	return (found_vmrss == 1 && found_vmsize == 1) ? 0 : 1;
}

