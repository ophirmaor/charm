/***************************************************************************
 * RCS INFORMATION:
 *
 *	$RCSfile$
 *	$Author$	$Locker$		$State$
 *	$Revision$	$Date$
 *
 ***************************************************************************
 * DESCRIPTION:
 *
 ***************************************************************************
 * REVISION HISTORY:
 *
 * $Log$
 * Revision 2.2  1995-09-08 02:38:48  gursoy
 * Cmi_dim is no more exported from machine.c. now thru a function call
 *
 * Revision 2.1  1995/07/17  17:47:17  knauff
 * Added '#include "converse.h"'
 *
 * Revision 2.0  1995/07/10  22:12:53  knauff
 * Initial revision
 *
 ***************************************************************************/
static char ident[] = "@(#)$Header$";

/* This file contains all the spanning tree functions */
#include "machine.h"
#include "converse.h"

#define MAXSPAN    4          /* The maximum permitted span on 
				 each node of the spanning tree */
#define MAXNODES   1024
#define MAXCUBEDIM  10         /* log_2 (MAXNODES) */


typedef struct spantreearray {
    int noofchildren;
    int parent;
    int *children;
} SpanTreeArray;

static SpanTreeArray *SpanArray;
static int *NodeStore;   /* used to store the nodes in the spanning 
		       tree in breadth first order */
static int numnodes;

CmiSpanTreeInit()
{
    int i, j;
    BOOLEAN visited[MAXNODES];
    int next, currentnode;
    int neighbours[MAXCUBEDIM];
    int dim;

    numnodes = CmiNumPe();
    dim      = CmiNumNeighbours(0);
    SpanArray = (SpanTreeArray *)CmiAlloc(sizeof(SpanTreeArray) * numnodes);
    NodeStore = (int *) CmiAlloc(sizeof(int) * numnodes);
    visited[0] = TRUE;
    NodeStore[0] = 0;  /* the root of the spanning tree */
    SpanArray[0].parent = -1;  /* no parent */

    for (i = 1; i < numnodes; i++)
        visited[i] = FALSE;

    for (next = 1, i = 0; i < numnodes; i++)
    {
	currentnode = NodeStore[i];
	CmiGetNodeNeighbours(currentnode, neighbours);
	SpanArray[currentnode].noofchildren = 0;
	for (j = 0; j < dim && 
	            SpanArray[currentnode].noofchildren < MAXSPAN; j++)
	{
	    if (!visited[neighbours[j]])
	    {
		NodeStore[next + SpanArray[currentnode].noofchildren] = 
								neighbours[j];
		SpanArray[currentnode].noofchildren++;
		SpanArray[neighbours[j]].parent = currentnode;
		visited[neighbours[j]] = TRUE;
		
	    }
	}
	if (SpanArray[currentnode].noofchildren != 0)
	{
	    SpanArray[currentnode].children = &NodeStore[next];
	    next += SpanArray[currentnode].noofchildren;
	}
    }

    for (i = 0; i < numnodes; i++)  /* check */
	if (!visited[i])
	   CmiError("node %d not part of spanning tree: initialization error!\n",i);
}


int CmiSpanTreeRoot()
{
    return 0;  /* node 0 is always the root of the spanning tree on the 
		  hypercubes */
}


int CmiSpanTreeParent(node)
int node;
{
    return SpanArray[node].parent;
}


CmiSpanTreeChildren(node, children)
int node, *children;
{
    int i;

    for( i = 0; i < SpanArray[node].noofchildren; i++)
         children[i] = SpanArray[node].children[i];
}


int CmiNumSpanTreeChildren(node)
int node;
{
    return SpanArray[node].noofchildren;
}




PrintSpanTree()
{
    int i,j;

    for (i = 0; i < numnodes; i++)
    {
	CmiPrintf("node: %d, parent: %d, numchildren: %d, children: ",
		 i, SpanArray[i].parent, SpanArray[i].noofchildren); 
	for (j = 0; j < SpanArray[i].noofchildren; j++)
	     CmiPrintf("%d ",SpanArray[i].children[j]);
	CmiPrintf("\n");
    }
}



CmiSendToSpanTreeLeaves(size, msg)
int size;
char * msg;
{
    int node;

    /* node 0 cannot be a leaf of a spanning tree: it is the root */
    for (node = 1; node < numnodes; node++)
    if (SpanArray[node].noofchildren == 0)  /* it is a leaf */
        CmiAsyncSend(node, size, msg, PROCESS_PID);
}
