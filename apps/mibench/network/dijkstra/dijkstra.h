


#define NUM_NODES                       100
#define PATHS_TO_FIND                   10
#define NONE                            999

#define DIJKSTRA_USE_MEMLUV 0
#define DIJKSTRA_USE_MEMLUV_SYN 0
#define HLS_LIST_EMULATION_MODE 2
#define FORCE_MEMCPY_ON_REALLOC 1

#define EMULATION_HEAP_ACTIVITY_RATIO 1

#define HW_FREQ 150

struct _NODE
{
  int iDist;
  int iPrev;
};
typedef struct _NODE NODE;

struct _QITEM
{
  int iNode;
  int iDist;
  int iPrev;
  struct _QITEM *qNext;
};
typedef struct _QITEM QITEM;

/*--------- Synthesizable-related data-----------*/
struct _QITEMsyn
{
    /* -- Extra variables for holding list metadata -- */
    uint_t index;
    uint_t next;
    /* -- Actual list's variables -- */
    int iNode;
    int iDist;
    int iPrev;
};
typedef struct _QITEMsyn QITEMsyn;

// QITEMsyn *array_list;
uint_t qHeadSyn = 0;
static uint_t qCurrSyn = 0;

/*-----------------------------------------------*/

long long wtime(void);
void print_path (NODE *LrgnNodes, int chNode);
void MemluvPrintDijkstraList(QITEMsyn* nd);
void PrintDijkstraList(void);
QITEMsyn * MemluvCreateDijkstraList(int LiNode, int LiDist, int LiPrev);
QITEMsyn * enqueue_syn (QITEMsyn *array_list, int LiNode, int LiDist, int LiPrev);
QITEMsyn * enqueue_x86 (QITEMsyn *array_list, int LiNode, int LiDist, int LiPrev);
QITEMsyn * enqueue (QITEMsyn *array_list, int LiNode, int LiDist, int LiPrev);
QITEMsyn * dequeue_x86 (QITEMsyn *array_list, int *piNode, int *piDist, int *piPrev);
QITEMsyn * dequeue_syn (QITEMsyn *array_list, int *piNode, int *piDist, int *piPrev);
QITEMsyn * dequeue (QITEMsyn *array_list, int *piNode, int *piDist, int *piPrev);
int qcount (void);
void MemluvDestroyDijkstraList(QITEMsyn* Darray_list);
