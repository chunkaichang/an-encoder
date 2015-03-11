#include <stdio.h>
#include <stdlib.h>

#define NUM_NODES   100
#define NONE        9999
#define time        unsigned int

/*
 * Custom types
 */
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


/*
 * Global variables
 */
QITEM *qHead = NULL;
int AdjMatrix[NUM_NODES][NUM_NODES];
int g_qCount = 0;
NODE rgnNodes[NUM_NODES];
int ch;
int iPrev, iNode;
int i, iCost, iDist;

#define log0(...) fprintf(stdout, __VA_ARGS__)
#define log1(...) fprintf(stdout, __VA_ARGS__)

/*
 * Helper functions
 */
void print_path (NODE *rgnNodes, int chNode)
{
  if (rgnNodes[chNode].iPrev != NONE)
    {
      print_path(rgnNodes, rgnNodes[chNode].iPrev);
    }
  log1(" %d", chNode);
}

void enqueue (int iNode, int iDist, int iPrev)
{
    QITEM *qNew = (QITEM *) malloc(sizeof(QITEM));
    QITEM *qLast = qHead;

    if (!qNew)
    {
        log0("Out of memory.\n");
        exit(1);
    }
    qNew->iNode = iNode;
    qNew->iDist = iDist;
    qNew->iPrev = iPrev;
    qNew->qNext = NULL;

    if (!qLast)
    {
        qHead = qNew;
    }
    else
    {
        QITEM tmp;
        while (qLast->qNext)
        {
            tmp = *qLast;
            qLast = tmp.qNext;
        }

        qLast->qNext = qNew;
    }
    g_qCount++;
}

void dequeue (int *piNode, int *piDist, int *piPrev)
{
    QITEM *qKill = qHead;
    QITEM tmp = *qHead;

    if (qHead != 0)
    {
        *piNode = qHead->iNode;
        *piDist = qHead->iDist;
        *piPrev = qHead->iPrev;
        qHead = tmp.qNext;
        free(qKill);
        g_qCount--;
    }
}

int qcount()
{
    return(g_qCount);
}

void dijkstra(int chStart, int chEnd)
{
    for (ch = 0; ch < NUM_NODES; ch++)
    {
        rgnNodes[ch].iDist = NONE;
        rgnNodes[ch].iPrev = NONE;
    }

    if (chStart == chEnd)
    {
        log0("Shortest path is 0 in cost. Just stay where you are.\n");
    }
    else
    {
        rgnNodes[chStart].iDist = 0;
        rgnNodes[chStart].iPrev = NONE;

        enqueue (chStart, 0, NONE);

        while (qcount() > 0)
        {
            dequeue(&iNode, &iDist, &iPrev);
            for (i = 0; i < NUM_NODES; i++)
            {
                if ((iCost = AdjMatrix[iNode][i]) != NONE)
                {
                    if ((NONE == rgnNodes[i].iDist) ||
                        (rgnNodes[i].iDist > (iCost + iDist)))
                    {
                        rgnNodes[i].iDist = iDist + iCost;
                        rgnNodes[i].iPrev = iNode;
                        enqueue (i, iDist + iCost, iNode);
                    }
                }
            }
        }

        log1("Shortest path is %d in cost. ", rgnNodes[chEnd].iDist);
        log0("Path is: ");
        print_path(&rgnNodes[0], chEnd);
        log0("\n");
    }
}


/*
 * Dummy definitions of functions for pre- and post-computation.
 * Former function encodes all global variables.
 * Latter function checks and decodes all global variables.
 */
void precomputation() {}
void postcomputation() {}

/*
 * computation() is the main entry for encoding.
 * Python translator should:
 *   -- extract the AST-subtree upon finding computation()
 *   -- transform code in this subtree using one of the encodings
 *       -- encode all args
 *       -- encode all internal functions
 *       -- encode all constants
 *       -- decode and check return value
 *
 */
void computation()
{
    int i = 0;
    int j, k;

    /* finds 10 shortest paths between nodes */
    for (j = NUM_NODES/2; i < 20; i++) {
        j = j % NUM_NODES;
        dijkstra(i, j);
        j++;
    }
}


int main(int argc, char *argv[]) {
    long long time ago;    //it does nothing
    int i, j, k;
    FILE *fp;

    /* open the adjacency matrix file */
    fp = fopen ("tests/inputs/dijkstra_input.dat", "r");

    /* make a fully connected matrix */
    for (i = 0; i < NUM_NODES; i++)
    {
        for (j = 0; j < NUM_NODES; j++)
        {
            /* make it more sparce */
            fscanf(fp, "%d", &k);
            AdjMatrix[i][j] = k;
        }
    }

    precomputation();
    computation();
    postcomputation();

    fflush(stdout);
}
