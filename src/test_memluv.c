
/*----------------------------------------------------------------------------
| @file   src/test_memluv.c
| @author Dionysios Diamantopoulos(diamantd@microlab.ntua.gr)
| @date   December 2013
| 
| @brief  Test Memluv functions.
*----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "memluv.h"
#include "global.h"

#define TB_UINT_T1 uint8_t
#define TB_UINT_T2 uint16_t
#define TB_UINT_T3 uint32_t
#define TB_UINT_T4 uint8_t
#define TB_UINT_T5 float32
#define TB_UINT_T6 float64

#define TB_UINT_T TB_UINT_T3


#define TB_VAL_OFFSET 25

/* Available tests:
 * 1-STRUCT
 * 2-SINGLE_ARRAYS
 * 3-DOUBLE_ARRAYS
 * 4-LINKED_LISTS
 * 5-Same as 2, but with shared subfunction calls
 * 6-Same as 2, but with distinct subfunction calls
 * 7-Simple dataflow/pipeline test (no memluv)
 * 8-String manipulation
 */
#define TEST 2

#define MAX_J 11135

#define RANDOM_SEED 0
#define RANDOM_FREE 0

/* Initializations */
#if TEST==1
struct teststruct
{
TB_UINT_T1 test1;
TB_UINT_T2 test2;
TB_UINT_T3 test3;
TB_UINT_T4 test4;
TB_UINT_T5 test5;
};
typedef struct teststruct teststruct;
#endif

/* Prototype synthesizable test function */
uint_t yadmm(uint_t j
		#if HW_DEBUG_MEMLUV==1				
			, struct MemLuvStats *mlvstats
		#endif
		);
TB_UINT_T3 subfunctionA(uint_t);
TB_UINT_T3 subfunctionB(uint_t);


TB_UINT_T3 subfunctionA(uint_t j) {
//#pragma AP inline off
//#pragma HLS dataflow
	uint_t i, finalA1=0, finalB1=0;
#if USE_MEMLUV == 1
	TB_UINT_T *A1=(TB_UINT_T*)MemluvAlloc((uint_t)(j*sizeof(TB_UINT_T)), 0);
	TB_UINT_T *B1=(TB_UINT_T*)MemluvAlloc((uint_t)(j*sizeof(TB_UINT_T)), 0);
#else
	TB_UINT_T A1[MAX_J];
	TB_UINT_T B1[MAX_J];
#endif

	for (i=0;i<j;i++) {
		A1[i]=(TB_UINT_T)i;
		//printf("TB DEBUG: A[%d] is stored in %p with value %d\n", i, &A[i], A[i]);
	}
	//TB_UINT_T3 C1=(TB_UINT_T3*)MemluvAlloc((uint_t)(2*j*sizeof(TB_UINT_T3)));
	for (i=0;i<j;i++)
		B1[i]=(TB_UINT_T)(TB_VAL_OFFSET+i);
	for (i=0;i<j;i++) {
		finalA1+=(uint_t)(A1[i]);
		finalB1+=(uint_t)(B1[i]);
	}
#if USE_MEMLUV == 1
	MemluvFree((void*)A1, (uint_t)(j*sizeof(TB_UINT_T)), 0);
	MemluvFree((void*)B1, (uint_t)(j*sizeof(TB_UINT_T)), 0);
#endif
	//printf("\nTB DEBUG: finalA1=%u finalB1=%u \n", finalA1, finalB1);
	return (TB_UINT_T3)(finalA1+finalB1);
}

TB_UINT_T3 subfunctionB(uint_t j) {
//#pragma AP inline off
//#pragma HLS dataflow
	uint_t i, finalA1=0, finalB1=0;
#if USE_MEMLUV == 1
	TB_UINT_T *A1=(TB_UINT_T*)MemluvAlloc((uint_t)(j*sizeof(TB_UINT_T)), 1);
	TB_UINT_T *B1=(TB_UINT_T*)MemluvAlloc((uint_t)(j*sizeof(TB_UINT_T)), 1);
#else
	TB_UINT_T A1[MAX_J];
	TB_UINT_T B1[MAX_J];
#endif
	
	for (i=0;i<j;i++) {
		A1[i]=(TB_UINT_T)i;
		//printf("TB DEBUG: A[%d] is stored in %p with value %d\n", i, &A[i], A[i]);
	}
	//TB_UINT_T3 C1=(TB_UINT_T3*)MemluvAlloc((uint_t)(2*j*sizeof(TB_UINT_T3)));
	for (i=0;i<j;i++)
	//	B1[i]=(TB_UINT_T3)(TB_VAL_OFFSET+i);
	for (i=0;i<j;i++) {
		finalA1+=(uint_t)(A1[i]);
		finalB1+=(uint_t)(B1[i]);
	}
#if USE_MEMLUV == 1
	MemluvFree((void*)A1, (uint_t)(j*sizeof(TB_UINT_T)), 1);
	//MemluvFree((void*)B1, (uint_t)(j*sizeof(TB_UINT_T3)), 1);
#endif
	return (TB_UINT_T)(finalA1+finalB1);
}

static int i1=0, i2=0, d=0;
void func_A(int *a, int *b);
void func_B(int *c);
void func_C(void);

void func_A(int *a, int *b) {
#ifdef __SYNTHESIS__
#pragma AP inline off
#endif
	int i;
	for (i=0; i<100; i++)
		i1 = *a + *b  + i;
}

void func_B(int *c) {
#ifdef __SYNTHESIS__	
#pragma AP inline off
#endif
	int i;
	for (i=0; i<100; i++)
		i2 = i1 + *c + i;
}

void func_C(void) {
#ifdef __SYNTHESIS__
#pragma AP inline off
#endif
	int i;
	for (i=0; i<100; i++)
		d = i2 + i;
}

/* strlen */
size_t strlen_syn(const char *s);
size_t strlen_syn(const char *s) {
#ifdef __SYNTHESIS__
#pragma AP inline
#endif
	const char *p = s;
	/* Loop over the data in s.  */
	while (*p != '\0')
		p++;
	return (size_t)(p - s);
}

 /* strcmp */
int strcmp_syn(const char *s1, const char *s2);
int strcmp_syn(const char *s1, const char *s2) {
#ifdef __SYNTHESIS__
#pragma AP inline
#endif
	unsigned char uc1, uc2;
	/* Move s1 and s2 to the first differing characters 
	in each string, or the ends of the strings if they
	are identical.  */
	while (*s1 != '\0' && *s1 == *s2) {
		s1++;
		s2++;
	}
	/* Compare the characters as unsigned char and
	return the difference.  */
	uc1 = (*(unsigned char *) s1);
	uc2 = (*(unsigned char *) s2);
	return ((uc1 < uc2) ? -1 : (uc1 > uc2));
}

uint_t yadmm(uint_t j 
		#if HW_DEBUG_MEMLUV==1				
			, struct MemLuvStats *mlvstats			
		#endif	
		) {

	TB_UINT_T3 result=0;
#if TEST==1
	struct teststruct *teststruct0;
	teststruct0=(teststruct*)MemluvAlloc((uint_t)(sizeof(teststruct)), 0);
	teststruct0->test1 = (TB_UINT_T1)j;
	teststruct0->test2 = (TB_UINT_T2)(j+1);
	teststruct0->test3 = (TB_UINT_T3)(j+2);
	teststruct0->test4 = (TB_UINT_T4)(j+3);
	teststruct0->test5 = (TB_UINT_T5)(j+4);

	MemluvFree ((void*)teststruct0, (uint_t)(sizeof(teststruct)), 0);
	
	teststruct0=(teststruct*)MemluvAlloc((uint_t)(sizeof(teststruct)), 0);
	teststruct0->test1 = (TB_UINT_T1)j;
	teststruct0->test2 = (TB_UINT_T2)(j+TB_VAL_OFFSET);
	teststruct0->test3 = (TB_UINT_T3)(j+TB_VAL_OFFSET*2);
	teststruct0->test4 = (TB_UINT_T4)(j+TB_VAL_OFFSET*4);
	teststruct0->test5 = (TB_UINT_T5)(j+TB_VAL_OFFSET*5);

	result = ((TB_UINT_T3)teststruct0->test1+
		(TB_UINT_T3)teststruct0->test2+
		(TB_UINT_T3)teststruct0->test3+
		(TB_UINT_T4)teststruct0->test4+
		(TB_UINT_T5)teststruct0->test5);
#endif


#if TEST==2
	TB_UINT_T i, finalA1=0, finalB1=0;
	//printf("TB1: index=%d\n", index);
#if USE_MEMLUV == 1
	TB_UINT_T *A1=(TB_UINT_T*)MemluvAlloc((TB_UINT_T)(j*sizeof(TB_UINT_T)), 0);
#else
	TB_UINT_T A1[MAX_J];
	TB_UINT_T B1[MAX_J];
#endif
	
	for (i=0;i<j;i++) {
		A1[i]=(TB_UINT_T3)i;
		//printf("TB DEBUG: A[%d] is stored in %p with value %d\n", i, &A[i], A[i]);
	}
#if USE_MEMLUV == 1
	TB_UINT_T *B1=(TB_UINT_T*)MemluvAlloc((TB_UINT_T)(j*sizeof(TB_UINT_T)), 0);
	//TB_UINT_T3 C1=(TB_UINT_T*)MemluvAlloc((uint_t)(2*j*sizeof(TB_UINT_T3)));
#endif
	for (i=0;i<j;i++)
		B1[i]=(TB_UINT_T)(TB_VAL_OFFSET+i);
	for (i=0;i<j;i++) {
		finalA1+=(TB_UINT_T)(A1[i]);
		finalB1+=(TB_UINT_T)(B1[i]);
	}
#if USE_MEMLUV == 1
#if RANDOM_FREE == 1
        if (RandMinMax(0,1) == 1)
#endif            
            MemluvFree ((void*)A1, (uint_t)(j*sizeof(TB_UINT_T)), 0);
            
#if RANDOM_FREE == 1
        if (RandMinMax(0,1) == 1)
#endif        
            //MemluvFree ((void*)B1, (uint_t)(j*sizeof(TB_UINT_T)), 0);
        
#endif
	printf("\nTB DEBUG: finalA1=%u finalB1=%u \n", finalA1, finalB1);
	result = (TB_UINT_T)(finalA1+finalB1);
#endif

#if TEST==3
	uint_t i, k;
	uint_t **A;
	A = (uint_t**)MemluvAlloc((uint_t)(j*sizeof(uint_t*)), 0);
	for (i=0;i<j;i++)
		A[i]=(uint_t*)MemluvAlloc((uint_t)(5*sizeof(uint_t)), 0);
	for (i=0;i<j;i++)
		for (k=0;k<5;k++) {
			printf("TB DEBUG: trying i=%u, k=%u\n", i, k);
			A[i][k] = (uint_t)(i+k);
		}
	for (i=0;i<j;i++)
		for (k=0;k<5;k++)
			result+=A[i][k];
	for (i=0;i<j;i++)
		MemluvFree((void*)A[i], (uint_t)(5*sizeof(uint_t)), 0);
	MemluvFree((void*)A, (uint_t)(j*sizeof(uint_t*)), 0);
#endif
#if HW_DEBUG_MEMLUV==1
	*mlvstats=ReturnMemLuvStats();
#endif


#if TEST==4

        node* testnode = MemluvCreateList(5);
        MemluvPrintList(testnode);
        testnode = MemluvAddToList(testnode, 2147483647, true);
        MemluvPrintList(testnode);
        result=testnode[2].val+j;
        MemluvFreeList(testnode);
#endif


#if TEST==5 || TEST==6
//#pragma HLS pipeline
//#pragma AP inline
//#pragma AP dataflow
	TB_UINT_T3 result1 = subfunctionA(j);
//#pragma HLS pipeline
//#pragma AP inline
#if TEST==5
	TB_UINT_T3 result2 = subfunctionA(j);
#else
	TB_UINT_T3 result2 = subfunctionB(j+1);
#endif
//#pragma HLS pipeline
//#pragma AP inline
	//TB_UINT_T3 result3 = subfunction(j);
	//TB_UINT_T3 result4 = subfunction(j);
	
	result = result1 + result2;// + result3 + result4;
#endif


#if TEST==7
//#pragma AP inline off
	int a=(int)j, b=1, c=1;
//#pragma HLS pipeline
//#pragma AP dataflow
	func_A(&a,&b);
	func_B(&c);
	func_C();
	
	result = (uint_t)d;
#endif
	
#if TEST==8
	uint_t i;
	const char *key1 = "Helloworld";
	char *key2;
	key2 = (char*)MemluvAlloc((uint_t)((20+1)*sizeof(char)), 0);
	for (i=0; i<20; i++)
		key2[i] = (char)(i+97);
	key2[20+1]='\0';
	printf("DEBUG: key1=%s\tstrlen_syn(key1)=%zu\tkey2=%s\tstrlen_syn(key2)=%zu\n", key1, strlen_syn(key1), key2, strlen_syn(key2));
	for (i=0; i<strlen_syn(key2); i++)
		result += (uint_t)key2[i]+j;
	MemluvFree((void*)key2, (uint_t)((20+1)*sizeof(char)), 0);
	
	if (strcmp_syn(key2, "abcdefghijklmnopqrst") == 0)
		result += 100;
	else
		result -= 100;	
	
#endif	
	
	return (result);
}


int main(void)
{
	uint_t i=4, returned, val;
	MemLuvStats mlvstats;
	MemLuvCore *mlvcore;
// 	long vmrss, vmsize;
	MemluvInit();
	mlvcore = ReturnMemLuvCore();
	MemluvInfo(NULL, stdout, ALL);
#if TEST==1
	printf("TB DEBUG: Testing Struct data type\n");
#elif TEST==2
	printf("TB DEBUG: Testing Single array data type\n");
#elif TEST==3
	printf("TB DEBUG: Testing Double array data type\n");
#endif
	//for (i=11135;i>11130;i--) {
        for (i=205;i>204;i--) {
        //for (i=100;i<205;i++) {
		printf("try i=%u ", i);

#if RANDOM_SEED==1
		val=RandMinMax(1,i);
#else
		val=i;
#endif
		returned = yadmm(val
				#if HW_DEBUG_MEMLUV==1
					, &mlvstats					
				#endif	
				);
		printf("TB DEBUG: [final=%u] ", returned);
		printf("[max_address_reached=%u] [Core Usage=%u%%] mlvdbg.result=%u [Total B allocated=%u] [Total B requested=%u] [Total B fragmented=%u]\n\n", 
			mlvstats.max_address_reached, mlvstats.used_percentage , mlvstats.result, 
			mlvstats.total_bytes_allocated, mlvstats.total_bytes_requested, mlvstats.total_fragmented_bytes);
// 		get_memory_usage_kb(&vmrss, &vmsize);
// 		printf("TB DEBUG: %2d: Current memory usage: VmRSS = %6ld KB, VmSize = %6ld KB\n", i, vmrss, vmsize);
	}
#if SIM_WITH_GLIBC_MALLOC==0
	MemluvDumpFreeList(mlvcore, ALL);
	MemluvDumpCore(mlvcore, ALL);
#endif
	MemluvEnd();
	return 0;
}
