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
 * Revision 2.14  1996-11-08 22:22:46  brunner
 * Put _main in for HP-UX CC compilation.  It is ignored according to the
 * CMK_USE_HP_MAIN_FIX flag.
 *
 * Revision 2.13  1996/07/15 21:03:09  jyelon
 * Changed conv-mach flags from #ifdef to #if
 *
 * Revision 2.12  1996/06/28 21:28:09  jyelon
 * Added special code for simulator version.
 *
 * Revision 2.11  1995/09/19 23:10:24  jyelon
 * added function pointer to 'StartCharm' arglist.
 *
 * Revision 2.10  1995/09/19  17:56:25  sanjeev
 * moved Charm's module inits from user_main to InitializeCharm
 *
 * Revision 2.9  1995/07/19  22:15:24  jyelon
 * *** empty log message ***
 *
 * Revision 2.8  1995/07/12  20:59:58  brunner
 * Added argv[0] to perfModuleInit() call, so performance data files
 * can use the prgram name in the log file name.
 *
 * Revision 2.7  1995/07/10  22:30:49  brunner
 * Added call to perfModuleInit() for CPV macros
 *
 * Revision 2.6  1995/07/03  17:55:55  gursoy
 * changed charm_main to user_main
 *
 * Revision 2.5  1995/06/18  21:55:06  sanjeev
 * removed loop from charm_main, put in CsdScheduler()
 *
 * Revision 2.4  1995/06/13  17:00:16  jyelon
 * *** empty log message ***
 *
 * Revision 2.3  1995/06/13  14:33:55  gursoy
 * *** empty log message ***
 *
 * Revision 2.2  1995/06/09  16:37:40  gursoy
 * Csv accesses modified
 *
 * Revision 1.4  1995/04/13  20:54:18  sanjeev
 * Changed Mc to Cmi
 *
 * Revision 1.3  1995/04/02  00:48:53  sanjeev
 * changes for separating Converse
 *
 * Revision 1.2  1995/03/17  23:38:21  sanjeev
 * changes for better message format
 *
 * Revision 1.1  1994/11/18  20:38:11  narain
 * Initial revision
 *
 ***************************************************************************/
static char ident[] = "@(#)$Header$";

#include "converse.h"


#if CMK_DEFAULT_MAIN_USES_COMMON_CODE

CpvExtern(int, numHeapEntries);
CpvExtern(int, numCondChkArryElts);
CpvExtern(int, CsdStopFlag);

user_main(argc, argv)
int argc;
char *argv[];
{
  ConverseInit(argv);

  InitializeCharm(argv) ;
  StartCharm(argv, (void *)0);

  CpvAccess(CsdStopFlag)=0;

  CsdScheduler(-1) ;

  EndCharm();
  ConverseExit() ;
}

#endif

#if CMK_DEFAULT_MAIN_USES_SIMULATOR_CODE

CpvExtern(int, numHeapEntries);
CpvExtern(int, numCondChkArryElts);
CpvExtern(int, CsdStopFlag);
CsvExtern(int, CsdStopCount);

void defaultmainModuleInit()
{
}

user_main(argc, argv)
int argc;
char *argv[];
{
  int i;

  for(i=0; i<CmiNumPes(); i++) 
  {
        CmiUniContextSwitch(i); 
        InitializeCharm(argv);
  }

  for(i=0; i<CmiNumPes(); i++) {CmiUniContextSwitch(i); ConverseInit(argv); }

  for(i=0; i<CmiNumPes(); i++) {
          CmiUniContextSwitch(i); 
          StartCharm(argv, (void *)0); 
  }

  for(i=0; i<CmiNumPes(); i++) {CmiUniContextSwitch(i);CpvAccess(CsdStopFlag)=0;}
  CsvAccess(CsdStopCount) = CmiNumPes();
  CmiUniContextSwitch(0);
  CsdUniScheduler(-1) ;

  for(i=0; i<CmiNumPes(); i++) { CmiUniContextSwitch(i); CkEndCharm();}
  for(i=0; i<CmiNumPes(); i++) {CmiUniContextSwitch(i);CpvAccess(CsdStopFlag)=0;}
  CsvAccess(CsdStopCount) = CmiNumPes();
  CmiUniContextSwitch(0);
  CsdUniScheduler(-1) ;

  for(i=0; i<CmiNumPes(); i++) { CmiUniContextSwitch(i); ConverseExit() ;}
}

#endif

