#include <stdio.h>
#include <stdlib.h>
#include <mcheck.h>
#include "../../../../src/memluv.h"
#include "../../../../src/global.h"
#include "dijkstra.h"
// cat memluv_stats_heap-0.txt | tail -n +20 | awk '{ print $2" "$6}' | sed '1d; n; d'> xaxa.txt  // remove even lines
// cat memluv_stats_heap-0.txt | tail -n +20 | awk '{ print $2" "$6}' | sed '1d; n; d'> xaxa.txt  // remove odd lines
// cat memluv_stats_heap-0.txt | tail -n +20 | awk '{ print $2" "$6}' > xaxa.txt        

QITEM *qHead = NULL;

int AdjMatrix[NUM_NODES][NUM_NODES];

int g_qCount = 0;
NODE rgnNodes[NUM_NODES];
int iPrev, iNode;
int iCost, iDist;

/* Measurements related variables */
uint_t TOTAL_MOVES_ENQ=0, TOTAL_MOVES_DEQ=0;


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

void print_path (NODE *LrgnNodes, int chNode)
{
/* NOTE: For some reason, rgnNodes are not indexed well on HLS, causing error in recursive call. */
#ifndef __SYNTHESIS__
  if (LrgnNodes[chNode].iPrev != NONE)
    {
      print_path(LrgnNodes, LrgnNodes[chNode].iPrev);
    }
  printf (" %d", chNode);
  fflush(stdout);
#else
  printf ("<Unknown: rgnNodes are not indexed well on HLS, causing error in recursive call>\n"); 
#endif
}

void MemluvPrintDijkstraList(QITEMsyn* nd) {
#if SIM_MINIMUM_OVERHEAD==0
#ifndef __SYNTHESIS__
    QITEMsyn *ptr = &nd[qHeadSyn];

    Dprintf(3, " -------Printing syn list Start------- \n");
//     printf(" DBG: qHeadSyn=%u  ptr->index=%u \n", qHeadSyn, ptr->index);
    while(ptr->index != 0)
    {
        Dprintf(3, "Node id %u:  ptr->next=%u   DATA: [iNode|iDist|iPrev]=[%4d|%4d|%4d]\n", ptr->index,  ptr->next,  ptr->iNode,  ptr->iDist,  ptr->iPrev);
        ptr = &nd[ptr->next];
    }
    Dprintf(3, " -------Printing syn list End------- \n\n");

    return;
#endif
#endif
}

void PrintDijkstraList(void) {
#if SIM_MINIMUM_OVERHEAD==0
    Dprintf(3, " -------Printing x86 list Start------- \n");
    QITEM* qTmp=qHead;
    if (qTmp==NULL)
        Dprintf(3, "Node g_qCount=%d  Empty queue\n", g_qCount);
    while (qTmp) {
        Dprintf(3, "Node g_qCount=%d  DATA: [iNode|iDist|iPrev]=[%4d|%4d|%4d]\n", g_qCount, qTmp->iNode,  qTmp->iDist,  qTmp->iPrev);
        qTmp=qTmp->qNext;        
    }
    Dprintf(3, " -------Printing x86 list End------- \n\n");
#endif
}

QITEMsyn * enqueue_syn (QITEMsyn *array_list, int LiNode, int LiDist, int LiPrev) {
#ifdef __SYNTHESIS__                                        
#pragma AP inline                                               
#endif
  int new_alloc_list_flag=0;
    
  Dprintf(3, " qHeadSyn =%u\n", qHeadSyn);
  if (qHeadSyn==0) {
    Dprintf(3, " 1st call to enqueue_syn, creating Dijkstra array List\n");
    array_list = MemluvCreateDijkstraList(LiNode, LiDist, LiPrev);
    new_alloc_list_flag=1;
  }
  else {
     
    qCurrSyn++;
    array_list[qCurrSyn-1].next=qCurrSyn;
    array_list[qCurrSyn].index=qCurrSyn;
    array_list[qCurrSyn].next = 0;
  
    array_list[qCurrSyn].iNode = LiNode;
    array_list[qCurrSyn].iDist = LiDist;
    array_list[qCurrSyn].iPrev = LiPrev;
  }
#if HLS_LIST_EMULATION_MODE==1
#elif HLS_LIST_EMULATION_MODE==2
    uint_t words_to_copy;
    if (new_alloc_list_flag!=1) {
        QITEMsyn* prev_pointer=array_list;
        if (++TOTAL_MOVES_ENQ%EMULATION_HEAP_ACTIVITY_RATIO==0) {
            MemluvFree((void*)array_list, (uint_t)((qCurrSyn)*sizeof(QITEMsyn)), 0);
#ifndef __SYNTHESIS__
            /* BUG: Reallocation is not supported on this level of hierarchy as of 2014.2 */
            array_list = (QITEMsyn *)MemluvAlloc((uint_t)((qCurrSyn+1)*sizeof(QITEMsyn)), 0);
#endif
        }
        Dprintf(3, "DEBUG: prev_pointer=%p array_list=%p\n", prev_pointer, array_list);
        if ((prev_pointer!=array_list) || (FORCE_MEMCPY_ON_REALLOC==1)) { /* If the reallocation occurs to another chunk of memory */
            words_to_copy= (uint_t)(qCurrSyn*sizeof(QITEMsyn)/sizeof(CORE_UINT_T));
            Dprintf(3, "DEBUG: words_to_copy=%d\n",  words_to_copy);  
            array_list=MemluvMemcpyWords(array_list, prev_pointer, words_to_copy);
        }
    }
#else
#endif
    
  g_qCount++;
  //               ASSERT(g_qCount);
  MemluvPrintDijkstraList(array_list);
  return array_list;
}


QITEMsyn * enqueue_x86 (QITEMsyn *array_list, int LiNode, int LiDist, int LiPrev)
{
    int loops2=0;
#if DIJKSTRA_USE_MEMLUV==1
    QITEM *qNew = (QITEM *) MemluvAlloc((uint_t)sizeof(QITEM), 0); 
#else
    QITEM *qNew = (QITEM *) malloc(sizeof(QITEM));
#endif
  QITEM *qLast = qHead;
  
  if (!qNew) 
    {
      fprintf(stderr, "Out of memory.\n");
      exit(1);
    }
  qNew->iNode = LiNode;
  qNew->iDist = LiDist;
  qNew->iPrev = LiPrev;
  qNew->qNext = NULL;
  
  if (!qLast) 
    {
      qHead = qNew;
    }
  else
    {
      while (qLast->qNext) {
          qLast = qLast->qNext;
         Dprintf(3, "Entered black area -%d:  q->iNode = %d  q->iDist = %d q->iPrev = %d;\n",loops2++, qLast->iNode, qLast->iDist, qLast->iPrev); 
      }
      qLast->qNext = qNew;
    }
  g_qCount++;
  //               ASSERT(g_qCount);
  PrintDijkstraList();
  return array_list;
}

QITEMsyn * enqueue (QITEMsyn *array_list, int LiNode, int LiDist, int LiPrev) {
#ifdef __SYNTHESIS__                                        
#pragma AP inline                                               
#endif
    Dprintf(3, "Enqueuing with qCurrSyn=%d\t", qCurrSyn);
#if DIJKSTRA_USE_MEMLUV_SYN==1   
   return enqueue_syn(array_list, LiNode, LiDist, LiPrev);
#else
   return enqueue_x86(array_list, LiNode, LiDist, LiPrev);
#endif
}

QITEMsyn * dequeue_x86 (QITEMsyn *array_list, int *piNode, int *piDist, int *piPrev)
{
  QITEM *qKill = qHead;
  
  if (qHead)
    {
      //                 ASSERT(g_qCount);
      *piNode = qHead->iNode;
      *piDist = qHead->iDist;
      *piPrev = qHead->iPrev;
      qHead = qHead->qNext;
#if DIJKSTRA_USE_MEMLUV==1      
      MemluvFree((void*)qKill, (uint_t)sizeof(QITEM), 0); 
#else
      free(qKill);
#endif
      g_qCount--;
    }
    PrintDijkstraList();
    return array_list;
}

QITEMsyn * dequeue_syn (QITEMsyn *array_list, int *piNode, int *piDist, int *piPrev)
{
#ifdef __SYNTHESIS__                                        
#pragma AP inline                                               
#endif
  int i;
  if (qHeadSyn)
    {
      //                 ASSERT(g_qCount);

      
      *piNode = array_list[qHeadSyn].iNode;
      *piDist = array_list[qHeadSyn].iDist;
      *piPrev = array_list[qHeadSyn].iPrev;
      
      /* Copying data TODO:Consider memcpy here! */
    for (i = 2; i < g_qCount+1; i++) {
        Dprintf(3, "Copying data...iter=%d\n", i);
        array_list[i-1].index=array_list[i].index-1;
        if (array_list[i].next==0)
            array_list[i-1].next=array_list[i].next;
        else
            array_list[i-1].next=array_list[i].next-1;
        
        array_list[i-1].iNode=array_list[i].iNode;
        array_list[i-1].iDist=array_list[i].iDist;
        array_list[i-1].iPrev=array_list[i].iPrev;
    }
    Dprintf(3, "qCurrSyn=%u qHeadSyn=%u\n", qCurrSyn, qHeadSyn);
    
//#if HLS_LIST_EMULATION_MODE==1
    //printf("Trying MemluvDestroyDijkstraList with qCurrSyn=%d qHeadSyn=%d\n", qCurrSyn, qHeadSyn);
    if ((qCurrSyn==qHeadSyn)==1) {
        qHeadSyn--;
        MemluvDestroyDijkstraList(array_list);
    }
    else {
#if HLS_LIST_EMULATION_MODE==1
#elif HLS_LIST_EMULATION_MODE==2
    uint_t words_to_copy;
        QITEMsyn* prev_pointer=array_list;
        if (++TOTAL_MOVES_DEQ%EMULATION_HEAP_ACTIVITY_RATIO==0) {
            MemluvFree((void*)array_list, (uint_t)((qCurrSyn+1)*sizeof(QITEMsyn)), 0);
#ifndef __SYNTHESIS__
            /* BUG: Reallocation is not supported on this level of hierarchy as of 2014.2 */
            array_list = (QITEMsyn *)MemluvAlloc((uint_t)((qCurrSyn)*sizeof(QITEMsyn)), 0);
#endif        
        }
        Dprintf(3, "DEBUG: prev_pointer=%p array_list=%p\n", prev_pointer, array_list);
        if ((prev_pointer!=array_list) || (FORCE_MEMCPY_ON_REALLOC==1)) { /* If the reallocation occurs to another chunk of memory */
            words_to_copy=(uint_t)(qCurrSyn*sizeof(QITEMsyn)/sizeof(CORE_UINT_T));
            Dprintf(3, "DEBUG: words_to_copy=%d\n",  words_to_copy);  
            array_list=MemluvMemcpyWords(array_list, prev_pointer, words_to_copy);
        }
#else
#endif       
    }   
    qCurrSyn--;
    g_qCount--;
    }
    MemluvPrintDijkstraList(array_list);
    return array_list;
}

QITEMsyn * dequeue (QITEMsyn *array_list, int *piNode, int *piDist, int *piPrev) {
#ifdef __SYNTHESIS__                                        
#pragma AP inline                                               
#endif
Dprintf(3, "Dequeuing with qCurrSyn=%d\t", qCurrSyn);
#if DIJKSTRA_USE_MEMLUV_SYN==1   
    return dequeue_syn(array_list, piNode, piDist, piPrev);
#else
    return dequeue_x86(array_list, piNode, piDist, piPrev);
#endif
}


int qcount (void)
{
  return(g_qCount);
}

QITEMsyn * MemluvCreateDijkstraList(int LiNode, int LiDist, int LiPrev) {
#ifdef __SYNTHESIS__                                        
#pragma AP inline                                               
#endif
    qHeadSyn = 0;
    qCurrSyn = 0;
    QITEMsyn *array_list = (QITEMsyn *)MemluvAlloc(2*sizeof(QITEMsyn), 0);

    /* First creating NULL-emulation node */
    array_list[qCurrSyn].index = qCurrSyn;
    array_list[qCurrSyn].next = 0;
    qHeadSyn = array_list[qCurrSyn].index;
    qCurrSyn++;

    /* Then creating the first node of the list */
    array_list[qCurrSyn].index = qCurrSyn;
    array_list[qCurrSyn].next = 0;
    qHeadSyn = array_list[qCurrSyn].index;
  
    array_list[qHeadSyn].iNode = LiNode;
    array_list[qHeadSyn].iDist = LiDist;
    array_list[qHeadSyn].iPrev = LiPrev;
    
    Dprintf(3, "TB-DEBUG-IN: New list headnode on addr %p, indexed on position %u\n", (void*)(array_list+qCurrSyn), array_list[qHeadSyn].index);
    return array_list;
}

uint_t dequeue_runs=0;
uint_t enqueue_runs=0;

void MemluvDestroyDijkstraList(QITEMsyn* Darray_list){
#ifdef __SYNTHESIS__                                        
#pragma AP inline                                               
#endif
    Dprintf(3, "Destroying DijkstraList\n");
    MemluvFree((void*)Darray_list, (uint_t)(2*sizeof(QITEMsyn)), 0);
}
int loops=0;
int dijkstra(int chStart, int chEnd, int *gpio0);    
int dijkstra(int chStart, int chEnd, int *gpio0) 
{
/* 120 cycles latency for initialization */  
#ifdef __SYNTHESIS__                                        
#pragma AP inline                                               
#endif
// dequeue_runs=0;
// enqueue_runs=0;
#ifndef __SYNTHESIS__
    QITEMsyn * array_list=NULL;
#else
    QITEMsyn * array_list;
#endif
    *gpio0=0;
//   QITEMsyn *array_list;
  int i, ch;
  for (ch = 0; ch < NUM_NODES; ch++)
    {
      rgnNodes[ch].iDist = NONE;
      rgnNodes[ch].iPrev = NONE;
    }

  if (chStart == chEnd) 
    {
      printf("Shortest path is 0 in cost. Just stay where you are.\n");
    }
  else
    {
      rgnNodes[chStart].iDist = 0;
      rgnNodes[chStart].iPrev = NONE;
     *gpio0=10;  
     array_list = enqueue (array_list, chStart, 0, NONE); /* 18 cycles latency*/
     enqueue_runs++;
     *gpio0=20; 
      while (qcount() > 0)
	{
          *gpio0=30;
	  array_list = dequeue (array_list, &iNode, &iDist, &iPrev); /* 12 cycles latency*/
          *gpio0=40;
          dequeue_runs++;
	  for (i = 0; i < NUM_NODES; i++)
	    { /* 8 cycles latency without nested enqueue */
	      if ((iCost = AdjMatrix[iNode][i]) != NONE)
		{
		  if ((NONE == rgnNodes[i].iDist) || 
		      (rgnNodes[i].iDist > (iCost + iDist)))
		    {
		      rgnNodes[i].iDist = iDist + iCost;
		      rgnNodes[i].iPrev = iNode;
//                       printf("LOOP %d\n", loops++);
                      *gpio0=*gpio0+1;
#ifndef __SYNTHESIS__
                      /* BUG: Nested call is not supported on this level of hierarchy as of 2014.2 */
		      array_list = enqueue (array_list, i, iDist + iCost, iNode);
#endif
                      enqueue_runs++;
		    }
		}
	    }
	}
    }
    return (rgnNodes[chEnd].iDist);
}

int main(int argc, char *argv[]) {
//   mtrace();
  int i,j,k,spath;
  int gpio0;
  long long start=0, hw_time, hw_cycles;
  long long algo_exec_time=0;
  start = wtime();
  FILE *fp;
  printf("======================================================\n");
  printf("INFO: Running Dijkstra alg. with NUM_NODES   = %d\n", NUM_NODES);
  printf("INFO: x86 unmodified SW version from Mibench : %s\n", (DIJKSTRA_USE_MEMLUV||DIJKSTRA_USE_MEMLUV_SYN)==0?"ON":"OFF");
  printf("INFO: =++= using MemLuv instead of glibc     : %s\n", (DIJKSTRA_USE_MEMLUV)==1?"ON":"OFF"); 
  printf("INFO: =++= using MemLuv & HLS linked lists   : %s\n", (DIJKSTRA_USE_MEMLUV_SYN)==1?"ON":"OFF"); 
  printf("INFO: Minimum Overhead Simulation Run        : %s\n", (SIM_MINIMUM_OVERHEAD)==1?"ON":"OFF"); 
  printf("INFO: Emulation heap activity ratio          = %d\n", EMULATION_HEAP_ACTIVITY_RATIO); 
  printf("INFO: Force memcpy on realloc                : %s\n", (FORCE_MEMCPY_ON_REALLOC)==1?"ON":"OFF");  
  printf("======================================================\n");
#if DIJKSTRA_USE_MEMLUV==1 || DIJKSTRA_USE_MEMLUV_SYN==1
  //MemLuvCore *mlvcore;
  MemluvInit();
  //mlvcore = ReturnMemLuvCore();
  MemluvInfo(NULL, stdout, ALL);
#endif
  
  if (argc<2) {
    fprintf(stderr, "Usage: dijkstra <filename>\n");
    fprintf(stderr, "Only supports matrix size is #define'd.\n");
  }

  /* open the adjacency matrix file */
  fp = fopen (argv[1],"r");
  if (fp==NULL) {
      fprintf(stderr, "Cannot find file %s. Aborting...\n\n", argv[1]);
      exit(-1);
  }

  /* make a fully connected matrix */
  for (i=0;i<NUM_NODES;i++) {
    for (j=0;j<NUM_NODES;j++) {
      /* make it more sparce */
      fscanf(fp,"%d",&k);
			AdjMatrix[i][j]= k;
    }
  }
  
  algo_exec_time = wtime();
  /* finds 10 shortest paths between nodes */
  for (i=0,j=NUM_NODES/2;i<PATHS_TO_FIND;i++,j++) {
			j=j%NUM_NODES;
      printf("Finding shortest path between nodes %d to %d\n", i, j);                 
      spath=dijkstra(i,j, &gpio0);
      printf("Shortest path is %d in cost. gpio0=%d ", spath, gpio0);
      printf("Path is: ");
      print_path(rgnNodes, j);
      printf("\n");
  }
  algo_exec_time = wtime() - algo_exec_time;
  
#if DIJKSTRA_USE_MEMLUV==1  || DIJKSTRA_USE_MEMLUV_SYN==1 
  MemluvEnd();
#endif
  //printf("INFO: enqueue_runs=%u dequeue_runs=%u\n", enqueue_runs, dequeue_runs);
  hw_cycles=10*(120*PATHS_TO_FIND+enqueue_runs*(18+8)+dequeue_runs*12);
  hw_time=hw_cycles*1/HW_FREQ;
  printf("INFO: [Elapsed time: %lld us]  [Internal time: %lld us]  [HW time: %lld us]\n\n", wtime() - start, algo_exec_time, hw_time);
//   muntrace();
  exit(0);
  

}
