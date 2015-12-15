
/**
 * @file   src/memluv.h
 * @author Dionysios Diamantopoulos(diamantd@microlab.ntua.gr)
 * @date   December 2013
 *
 * @brief  Memluv header, including defines and configurations.
 */

#ifndef _MEMLUV_H_
#define _MEMLUV_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "MemluvConfig.h"
#include "gen_kernels/gen_accelerators.h"

/* --------------------------------------- *
 * Used defined options below, please edit *
 * --------------------------------------- */

/** The depth of internal buffer, i.e. the number of addresses 
 *  FIXME: It seems that depth 64, 128 causes Vivado to exploit internal error
 *  FIXME: Any arbitary size not power of 2, causes internal computation errors on some statistics. 
 *         When no statistics are used in control path, the functionality remains solid.
 */
#define MEMLUV_DEPTH 2048

/** Print debug messages of mem allocation functions, on SW only execution */
#define DEBUG_MEMLUV 1

/** The level of debugging. DEBUG_MEMLUV should be 1 
 *  0->None, 1-> Light Debug, 2-> Medium Debug , 3-> Insane Debug
 */
#define DEBUG_MEMLUV_LEVEL 1

/** Instatiate a HW debug monitor. Introduces extra HW resources overhead */
#define HW_DEBUG_MEMLUV 1

/** Replace memluv allocator with system's allocator, i.e. glibc calloc */
#define SIM_WITH_GLIBC_MALLOC 0

/** The statistics/debug messages are supressed for timing measurements.
 *  FIXME: When enabled, a lot of 'warning: unused parameter' are reported since the arguments of
 *         functions are never used on functions body. I have to pass the #if-#endif preprocessor
 *         macros also to function arguments. (responsible flag on gcc: -Wunused-parameter).
 *  NOTE:  On hls this define has no impact since it is only applied to non-synthesizable code.
 */
#define SIM_MINIMUM_OVERHEAD 0

/** The type of the internal memory core. Curently uint and float are supported, no way to support
 *  them simultaneously 
 *  FIXME: It seems that Vivado 2013.3 could not support pointer comparison between different types 
 *         (uint/float), while this is fixed in 2013.4. However the float 64-bit type is not 
 *         properly alleviated by 2013.4. Have to generate a web case.
 *  1-> uint  2-> float (deprecated in Vivado 2013.4)
 */
#define CORE_TYPE 1

/** The width of internal memory core. i.e. the legnth of every line of internal memory. 
 *  Supported 8,16,32,64.
 *  Whatever the internal size, the requested memory  type may be of any length/type. 
 *  LLVM take cares of data (un)packing, whatever the endianess.
 *  FIXME: It seems that size 8,16 cuases Vivado (2013.3) internal error HLS-102, ocasionally 
 *         (i.e. with multiple calls to free, using full statistics)
 *  On 2013.4 [SYNCHK-11] ... unsynthesizable type ... (possible cause(s): structure variable 
 *  cannot be decomposed due to unsupported type conversion or memory copy operation).
 */
#define CORE_WIDTH 32

/** The type of most internal pointers, i.e. base, addr, next etc. */
#define uint_t uint32_t
#define int_t int32_t

/** The width of free list, i.e. the legnth of every line of bitmap array. Supported 8,16,32,64 */
#define FREELIST_WIDTH 32

/** The policy of handling the freelist set/clear processes. 
 *  0 -> Set/Clear per bit - One write to freelist for every bit (many cycles)
 *  1 -> Set/Clear per register (and per bit for non-alinged requests), 
 *       the mask is bit-computed (many cycles) but write happens as many times as the number of 
 *       registers are affected, plus bit-process writes for non-aligned requests.
 *  2 -> Set/Clear per register. The mask is computed directly (in 6 cycles with seq. exec.) and
 *       we succeed the minimum number of writes to freelist - one per affected register.
 */
#define FREELIST_HANDLE_POLICY 2

/** The fitting algorithm of MemLuv. 
 *  0 -> First fit algorithm.
 */
#define MEMLUV_FIT_ALGORITHM 0

/** The policy of first fit algorithm of MemLuv (applicable only when 
 *  MEMLUV_FIT_ALGORITHM==0). 
 *  0 -> Search bit-by-bit starting from position 0 to bitmap length. 
 *       On first succesfull empty bin, the next search starts from the next bit.
 *       If it fails the next search starts from the next bit.
 *  1 -> Search bit-by-bit starting from position 0 to bitmap length. 
 *       On first succesfull empty bin, the freelist registers are checked with 
 *       masks. If it fails the next search starts from the next bit.
 */
#define MEMLUV_FIRST_FIT_POLICY 1

/** A special feature available for first fit algorithm (applicable only when 
 *  MEMLUV_FIT_ALGORITHM==0), that keeps a cached copy of last free bin. Since 
 *  first fit always searches for free bins starting from position 0, this 
 *  feature may increase the performance, regarding the apllication.
 *  0 -> Not enabled
 *  1 -> Enabled
 *  FIXME: Currently this option holds a bug: No-check is performed on the 
 *         sequence of malloc/free thus the cache bin may be to high freelist 
 *         position while there are empty bins on lower free bin positions.
 *         However we can enable it for simple sequential (all-malloc all-free) 
 *         testcases where performance is sinificanlty improved.
 */
#define MEMLUV_FIRST_FIT_CACHE_LAST_BIN 1

/** 1,2,4,8 bytes address alingment. Optimally should be log2(CORE_WIDTH) */
#define ALIGNMENT 4

/** The number of sizeof(CORE_UINT_T) bytes reserved at start of every allocation to store metadata.
 *  0,2 are supported so far for x86. Using inline, Vivado 2014.1 proccess it, while 2014.2 not on x86-64. 
 *  0 -> No header is used
 *  2 -> Header of two fields is used: On any address, the 1st previous address holds the index in 
 *       freelist and the 2nd previous address holds the size of allocation in bytes. i.e. :
 *       0x0 -> [size_in_bytes]
 *       0x1 -> [index_in_freelist]
 *       0x2 -> [1st_data_stored]...
 *       Note: The number of bits of CORE_UINT_T dictates the maximum allocation/free of 
 *             2^(sizeof(CORE_UINT_T)x8) bytes regardless the core size 
 *             (MEMLUV_DEPTHxsizeof(CORE_UINT_T) bytes).
 * <URGENT>FIXME:       It seems that on MEMLUV_ALLOC_HEADER=0, one can free more bytes from a 
 *                      pointer than being allocated (through MemLuvFree 2nd argument), resulting 
 *                      in counters underflow and unexpected results, while the status is still 
 *                      FREE_SUCCESS (id:4)
 */
#define MEMLUV_ALLOC_HEADER 0

/** 1 -> Inline all logic inside alloc/free. Results in a fast, yet resource hungry circuit.
 *       Can the target FPGA accept the overhead? 
 *  0 -> Results to inlining of the main alloc/free wrappers, with limited logic inside them 
 *       (computation intensive logic has been moved to shared functions/enities) 
 */
#define MEMLUV_HLS_INLINE 1

/** FIXME: Deprecated! I have to remove it. */
#define MEMLUV_HLS_DFADD_INLINE 0

/** Force checking if requested free pointer is a valid address for current hw heap */
#define CHCK_FREE_OUT_OF_BOUNDS 0

/** Force checking of freelist value prior to marking zero bits. In case a request free position 
 *  is not '1', a FREE_FAIL_UNREGISTERED_BIT enum variable is asserted on memluv_action_status_t 
 */
#define CHCK_FREELIST_BEFORE_FREE 0

/** Force checking if malloc fails due to fragmentation issues, although there are available bytes 
 *  for the requested size, but not continusoly. HW_DEBUG_MEMLUV should be '1'
 */
#define CHCK_MALLOC_FAIL_DUE_FRAGMENTATION 0

/** The percentage of lines of memory core dump which will be printed on MemluvDumpCore() 
 *  and MemluvDumpFreeList() calls.
 *  Valid integer values (no check is performed): 0-100 
 */
#define MEMELUV_DUMP_SHOW_PERCENTAGE 100

/** The output stream in which all messages are reported.
 *  0 -> stdout (standard output)
 *  1 -> file (./memluv_run_<pid>.log)
 */
#define DEBUG_PRINT_STREAM 0

/** Enabling the report of fragmentation. FIXME: Have to exclude it for synthesis - too slow on real
 *  HW (working with freelist sequential parsing.)
 *  TODO: The way it works, is valid only for FirstFit with sequential alloc-free.
 */
#define DEBUG_REPORT_FRAGMENTATION 0

/* -------------------------------------------------------------------------------------------- *
 *                              Automated stuff below, don't edit                               *
 * -------------------------------------------------------------------------------------------- *
 */

/** Qick'n'Dirty way to define __SYNTHESIS__ only for Vivado and NOT define on gcc compilation.
    Vivado is supposed to define internally the __SYNTHESIS__ macro. However this is a simple 
    solution as long as Xilinx really implement it in sim/co-sim procedures.
    The define is forced by ADD_DEFINITIONS( -DNO_DEFINE__SYNTHESIS__ ) on src/CMakeLists.txt
*/
#ifndef NO_DEFINE__SYNTHESIS__
#ifndef __SYNTHESIS__
#define __SYNTHESIS__
#endif
#endif

/*              0         1               2                    3                       4          5                   6                            7               */
typedef enum {IDLE, MALLOC_SUCCESS, MALLOC_FAIL, MALLOC_FAIL_DUE_FRAGMENTATION, FREE_SUCCESS, FREE_FAIL, FREE_FAIL_UNREGISTERED_BIT, FREE_FAIL_POINTER_OUT_OF_BOUNDS} memluv_action_status_t;
typedef enum {ALL, THIS} memluv_struct_handle_t;

/*----------------------------------------------------------------------------
| Software IEC/IEEE floating-point types.
*----------------------------------------------------------------------------*/
typedef unsigned int float32;
typedef unsigned long long float64;

/** rounds up to the nearest multiple of ALIGNMENT 
 *  limitation: works only with power of 2 size
 */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

#define SIZE_T_SIZE (ALIGN(sizeof(uint_t)))

#if CORE_TYPE == 1
	#if CORE_WIDTH == 8
		#define CORE_UINT_T uint8_t
	#elif CORE_WIDTH == 16
		#define CORE_UINT_T uint16_t
	#elif CORE_WIDTH == 32
		#define CORE_UINT_T uint32_t
	#elif CORE_WIDTH == 64
		#define CORE_UINT_T uint64_t
	#else
		#define CORE_UINT_T UNSUPPORTED_CORE_WIDTH
	#endif
	#define var_type_t uint_t
#elif CORE_TYPE == 2
	#if CORE_WIDTH == 32
		#define CORE_UINT_T float32
		#define var_type_t float32
	#elif CORE_WIDTH == 64
		#define CORE_UINT_T float64
		#define var_type_t float64
	#else
		#define CORE_UINT_T UNSUPPORTED_CORE_WIDTH
	#endif
#else
	#define CORE_UINT_T UNSUPPORTED_CORE_TYPE
#endif

#if FREELIST_WIDTH  == 8
	#define FREELIST_UINT_T uint8_t
	#define FREELIST_UINT_TD uint16_t
#elif FREELIST_WIDTH == 16
	#define FREELIST_UINT_T uint16_t
	#define FREELIST_UINT_TD uint32_t
#elif FREELIST_WIDTH == 32
	#define FREELIST_UINT_T uint32_t
	#define FREELIST_UINT_TD uint64_t
#elif FREELIST_WIDTH == 64
	#define FREELIST_UINT_T uint64_t
	/*FIXME: Not native type with higher width of uint64_t, thus FREELIST_UINT_T=uint64_t is 
         *       not recommended 
         */
	#define FREELIST_UINT_TD uint64_t
#else
	#define FREELIST_UINT_T UNSUPPORTED_FREELIST_WIDTH
#endif

/** log2(ALIGNMENT), dummy but avoid either gcc warning or extra registers for look-up table, or 
 *  log cordic module :) 
 */
#if ALIGNMENT  == 8
	#define LOG_ALIGN 3
#elif ALIGNMENT  == 4
	#define LOG_ALIGN 2
#elif ALIGNMENT  == 2
	#define LOG_ALIGN 1
#elif ALIGNMENT  == 1
	#define LOG_ALIGN 0
#else
	#define LOG_ALIGN UNSUPPORTED_ALIGNMENT
#endif

typedef struct MemLuvConf MemLuvConf;
typedef struct MemLuvStats MemLuvStats;
typedef struct MemLuvCore MemLuvCore;

void MemLuvDebugUpdate(MemLuvCore *CurMemLuvCore, uint_t arg, uint_t value);
void MemluvInfo(MemLuvCore *CurMemLuvCore, FILE* stream, memluv_struct_handle_t handle);
void MemluvDumpCore(MemLuvCore *CurMemLuvCore, memluv_struct_handle_t handle);
void MemluvInit(void);
void MemluvInitCore(MemLuvCore *CurMemLuvCore);
void MemluvEnd(void);
void MemluvEndCore(MemLuvCore *CurMemLuvCore);
void MemluvDumpFreeList(MemLuvCore *CurMemLuvCore, memluv_struct_handle_t handle);
uint_t ReqPadding(uint_t size);
void QueryLock(
		#if HW_DEBUG_MEMLUV==1
			MemLuvCore *CurMemLuvCore
		#else
			void
		#endif
		);
void QueryUnLock(
		#if HW_DEBUG_MEMLUV==1
			MemLuvCore *CurMemLuvCore
		#else
			void
		#endif
		);
uint_t MemluvReportReqFragBytes(MemLuvCore *CurMemLuvCore);

/** A pointer to MemLuvConf0, declared in global.c. It is initialized through MemluvInit().*/
MemLuvConf *CurMemLuvConf;

/* The main functions prototypes are included in separated files */
#include "malloc.h"
#include "free.h"
#include "freelist.h"
#include "util.h"

#endif
