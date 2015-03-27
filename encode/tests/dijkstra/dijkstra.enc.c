#include <stdio.h>
#include <stdlib.h>

#include "dijkstra.h"

extern long AdjMatrix[NUM_NODES][NUM_NODES];
extern NODE rgnNodes[NUM_NODES];


/*
 * Custom types
 */
struct _QITEM
{
  long iNode;
  long iDist;
  long iPrev;
  struct _QITEM *qNext;
};
typedef struct _QITEM QITEM;


/*
 * Module-global variables
 */
static QITEM *qHead = 0;
static long g_qCount = 0;
static long ch;
static long iPrev, iNode;
static long i, iCost, iDist;

/*
 * Helper functions
 */
void enqueue (long iNode, long iDist, long iPrev)
{
    QITEM *qNew = (QITEM *) malloc(sizeof(QITEM));
    QITEM *qLast = qHead;

    if (!qNew)
    {
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
        while (qLast->qNext)
            qLast = qLast->qNext;

        qLast->qNext = qNew;
    }
    g_qCount++;
}

void dequeue (long *piNode, long *piDist, long *piPrev)
{
    if (qHead)
    {
        *piNode = qHead->iNode;
        *piDist = qHead->iDist;
        *piPrev = qHead->iPrev;
        QITEM *qSecond = qHead->qNext;
        free(qHead);
        qHead = qSecond;
        g_qCount--;
    }
}

long qcount()
{
    return(g_qCount);
}

void ___enc_dijkstra(long chStart, long chEnd)
{
    for (ch = 0; ch < NUM_NODES; ch++)
    {
        rgnNodes[ch].iDist = NONE;
        rgnNodes[ch].iPrev = NONE;
    }

    if (chStart == chEnd)
    {
        return;
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
    }
}

