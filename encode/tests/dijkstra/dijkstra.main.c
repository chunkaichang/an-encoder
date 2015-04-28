/* Adapted from Dmitry Kuvayskiy's test for his
   "AN-Transformer".
 */

#include <stdio.h>
#include <stdlib.h>

#include "../mylibs/encode.h"
#include "../mylibs/mycyc.h"
#include "../mylibs/mycheck.h"

#include "dijkstra.h"

#define log0(...) printf(__VA_ARGS__)
#define log1(...) printf(__VA_ARGS__)
#define ADJ_PRINT(...) printf(__VA_ARGS__)

extern void ___enc_dijkstra(long, long);

long AdjMatrix[NUM_NODES][NUM_NODES];
NODE rgnNodes[NUM_NODES];

/*
 * Helper functions
 */
void print_path(NODE *rgnNodes, long chNode)
{
  if (rgnNodes[chNode].iPrev != NONE)
  {
    print_path(rgnNodes, rgnNodes[chNode].iPrev);
  }
  log1(" %ld", chNode);
}

int main(int argc, char *argv[])
{
  uint64_t t1, t2, total = 0;
  unsigned i, j, k;
  FILE *fp;

  if (argc < 2)
  {
    fprintf(stderr, "Usage: dijkstra <filename>\n");
    fprintf(stderr, "Only supports matrix size is #define'd.\n");
  }

  /* open the adjacency matrix file: */
  fp = fopen (argv[1], "r");
  if (!fp) return -1;

  /* generate a symmetric adjacency matrix: */
  for (i = 0; i < NUM_NODES; i++)
  {
    for (j = 0; j <= i; j++)
    {
      fscanf(fp, "%d", &k);
      AdjMatrix[i][j] = AN_ENCODE_VALUE(k);
      AdjMatrix[j][i] = AN_ENCODE_VALUE(k);
    }
  }
  /* check symmetry: */
  for (i = 0; i < NUM_NODES; i++)
  {
    for (j = 0; j < NUM_NODES; j++)
      if (AdjMatrix[i][j] != AdjMatrix[j][i])
        return -1;
  }

  ADJ_PRINT("Adjacency matrix:\n");
  for (i = 0; i < NUM_NODES; i++)
  {
    for (j = 0; j < NUM_NODES; j++)
       ADJ_PRINT("(%02d,%02d)%4lld; ", i, j, AN_DECODE_VALUE(AdjMatrix[i][j]));
    ADJ_PRINT("\n");
  }
  ADJ_PRINT("\n");

  __cs_log(argc, argv);
  __cs_fopen(argc, argv);
  __cs_reset();
  /* find NUM_NODES shortest paths between nodes */
  j = NUM_NODES / 2;
  for (i = 0; i < NUM_NODES; i++)
  {
    j = j % NUM_NODES;

    t1 = __cyc_rdtsc();
    ___enc_dijkstra(i, j);
    t2 = __cyc_rdtsc();
    total += t2 - t1;

    if (i == j)
    {
      log0("Shortest path is 0 in cost. Just stay where you are.\n");
    }
    else
    {
      unsigned k;
      for(k = 0; k < NUM_NODES; k++)
      {
        rgnNodes[k].iDist = AN_DECODE_VALUE(rgnNodes[k].iDist);
        rgnNodes[k].iPrev = AN_DECODE_VALUE(rgnNodes[k].iPrev);
        __cs_facc(rgnNodes[k].iDist);
        __cs_acc(rgnNodes[k].iDist);
        __cs_facc(rgnNodes[k].iPrev);
        __cs_acc(rgnNodes[k].iPrev);
      }
      log1("Shortest path is %ld in cost. ", rgnNodes[j].iDist);
      log0("Path is: ");
      print_path(rgnNodes, j);
      log0("\n");
    }

    j++;
  }

  __cyc_msg(total);
  __cs_fclose();
  __cs_msg();

  return 0;
}
