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
 * Revision 2.1  1996-11-08 22:22:48  brunner
 * Put _main in for HP-UX CC compilation.  It is ignored according to the
 * CMK_USE_HP_MAIN_FIX flag.
 *
 * Revision 2.0  1995/09/06 17:20:37  sanjeev
 * *** empty log message ***
 *
 * Revision 2.2  1995/09/05  21:53:30  sanjeev
 * removed many Charm++ fns
 *
 * Revision 2.1  1995/07/25  00:31:02  jyelon
 * *** empty log message ***
 *
 * Revision 2.0  1995/06/02  17:35:06  brunner
 * Reorganized directory structure
 *
 * Revision 1.2  1994/12/01  23:54:52  sanjeev
 * interop stuff
 *
 * Revision 1.1  1994/11/03  17:38:34  brunner
 * Initial revision
 *
 ***************************************************************************/
static char ident[] = "@(#)$Header$";

#include "converse.h"

#if CMK_USE_HP_MAIN_FIX
void _main(argc,argv)
int argc;
char *argv[];
{
}
#endif

void CPlus_ChareExit()
{
}


void CPlus_GetAccMsgPtr()
{}

void CPlus_CallMonoInit()
{}

void CPlus_SetAccId()
{}

void CPlus_SetMonoId()
{}

void CPlus_CallCombineFn()
{}

void CPlus_CallAccInit()
{}

void CPlus_GetMonoMsgPtr()
{}

void CPlus_CallUpdateFn()
{}


void CPlus_SetMainChareID()
{}

/* projections.c calls this. For Charm++ files it is generated by translator*/
void _CKmain_DumpNames() 
{}

