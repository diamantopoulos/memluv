
/**
 * @file   src/util.h
 * @author Dionysios Diamantopoulos(diamantd@microlab.ntua.gr)
 * @date   March 2014
 *
 * @brief  Memluv auxiliary functions headers, i.e. timing, printing wrappers, random generators etc.
 */

#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdarg.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include "memluv.h"

void Dprintf(int8_t level, const char* format, ... );
void Dfprintf(int8_t level, FILE *stream, const char* format, ... );
void bprintf( const char* format, ... );
void Uint_tToBin(FREELIST_UINT_T n);
uint_t RandMinMax (uint_t min, uint_t max );
double lfsr113 (void);
unsigned LfsrRand(void);
unsigned LfsrRandSyn(unsigned short stack_lfsr, uint8_t mode);
uint_t RandMinMaxSyn (uint_t min, uint_t max, unsigned short *lfsr_ptr, uint8_t mode);
void DumpStatisticsToFile(MemLuvCore *CurMemLuvCore
#if USE_MEMLUV==1 && HW_DEBUG_MEMLUV==1
			  ,uint8_t action 
			  ,uint_t nbytes
#endif
			);
void TimeAllToFile(FILE *stream);
void TimeSecToFile(FILE *stream);
void TimeMicroSecToFile(FILE *stream);
int lrand(void);
int rsize(void);
int get_memory_usage_kb(long* vmrss_kb, long* vmsize_kb);

#endif