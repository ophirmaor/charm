/*
Threaded Charm++ "Framework Framework" Implementation header

Implements an array of migratable threads.
Provides utility routines for registering user
data, stopping, starting, and migrating threads.

Orion Sky Lawlor, olawlor@acm.org, 11/19/2001
*/
#ifndef __CHARM_TCHARM_IMPL_H
#define __CHARM_TCHARM_IMPL_H

#include "pup.h"
#include "pup_c.h"
#include "charm-api.h"
#include "tcharmc.h"
#include "cklists.h"
#include "memory-isomalloc.h"

class TCharmTraceLibList;

#include "tcharm.decl.h"

class TCharm;

// This little class holds values between a call to TCHARM_Set_* 
//   and the subsequent TCHARM_Create_*.  It should be moved
//   into a parameter to TCHARM_Create.
class TCHARM_Thread_options {
public:
	int stackSize; /* size of thread execution stack, in bytes */
	int exitWhenDone; /* flag: call CkExit when thread is finished. */
	// Fill out the default thread options:
	TCHARM_Thread_options(int doDefault);
	TCHARM_Thread_options() {}

	void sanityCheck(void);
};

class TCharmInitMsg : public CMessage_TCharmInitMsg {
 public:
	//Function to start thread with:
	CthVoidFn threadFn;
	//Initial thread parameters:
	TCHARM_Thread_options opts;
	//Array size (number of elements)
	int numElements;
	//Data to pass to thread:
	char *data;

	TCharmInitMsg(CthVoidFn threadFn_,const TCHARM_Thread_options &opts_)
		:threadFn(threadFn_), opts(opts_) {}
};

//Thread-local variables:
CtvExtern(TCharm *,_curTCharm);

CDECL {typedef void (*TCpupUserDataC)(pup_er p,void *data);};
FDECL {typedef void (*TCpupUserDataF)(pup_er p,void *data);};

class TCharm: public CBase_TCharm
{
 public:

//User's heap-allocated/global data:
	class UserData {
		void *data; //user data pointer
		bool isC;
		TCpupUserDataC cfn;
		TCpupUserDataF ffn;
	public:
		UserData(int i=0) {data=NULL; cfn=NULL; ffn=NULL;}
		UserData(TCpupUserDataC cfn_,void *data_)
			{cfn=cfn_; data=data_; isC=true;}
		class isFortran{};
		UserData(TCpupUserDataF ffn_,void *data_,isFortran tag)
			{ffn=ffn_; data=data_; isC=false;}
		inline void *getData(void) const {return data;}
		void pup(PUP::er &p);
		friend inline void operator|(PUP::er &p,UserData &d) {d.pup(p);}
	};
	//New interface for user data:
	CkVec<UserData> sud;
	
//Tiny semaphore-like pointer producer/consumer
	class TCharmSemaphore {
	public:
		int id; //User-defined identifier
		void *data; //User-defined data
		CthThread thread; //Waiting thread, or 0 if none
		
		TCharmSemaphore() { id=-1; data=NULL; thread=NULL; }
		TCharmSemaphore(int id_) { id=id_; data=NULL; thread=NULL; }
	};
	/// Short, unordered list of waiting semaphores.
	CkVec<TCharmSemaphore> sema;
	TCharmSemaphore *findSema(int id);
	TCharmSemaphore *getSema(int id);
	void freeSema(TCharmSemaphore *);
	
	/// Store data at the semaphore "id".
	///  The put can come before or after the get.
	void semaPut(int id,void *data);

	/// Retreive data from the semaphore "id", returning NULL if not there.
	void *semaPeek(int id);
	
	/// Retreive data from the semaphore "id".
	///  Blocks if the data is not immediately available.
	void *semaGets(int id);
	
	/// Retreive data from the semaphore "id".
	///  Blocks if the data is not immediately available.
	///  Consumes the data, so another put will be required for the next get.
	void *semaGet(int id);

//One-time initialization
	static void nodeInit(void);
	static void procInit(void);
 private:
	//Informational data about the current thread:
	class ThreadInfo {
	public:
		CProxy_TCharm tProxy; //Our proxy
		int thisElement; //Index of current element
		int numElements; //Number of array elements
	};

	TCharmInitMsg *initMsg; //Thread initialization data
	CthThread tid; //Our migratable thread
	friend class TCharmAPIRoutine; //So he can get to heapBlocks:
	CmiIsomallocBlockList *heapBlocks; //Migratable heap data
	CtgGlobals threadGlobals; //Global data

	bool isStopped, resumeAfterMigration, exitWhenDone;
	ThreadInfo threadInfo;
	double timeOffset; //Value to add to CkWallTimer to get my clock

	//Old interface for user data:
	enum {maxUserData=16};
	int nUd;
	UserData ud[maxUserData];

	void ResumeFromSync(void);

 public:
	TCharm(TCharmInitMsg *initMsg);
	TCharm(CkMigrateMessage *);
	~TCharm();
	
	virtual void ckJustMigrated(void);
	void migrateDelayed(int destPE);
	void atBarrier(CkReductionMsg *);
	void atExit(CkReductionMsg *);

	void clear();

	//Pup routine packs the user data and migrates the thread
	virtual void pup(PUP::er &p);

	//Start running the thread for the first time
	void run(void);

	inline double getTimeOffset(void) const { return timeOffset; }

//Client-callable routines:
	//Sleep till entire array is here
	void barrier(void);
	
	//Block, migrate to destPE, and resume
	void migrateTo(int destPE);

	//Thread finished running
	void done(void);

	//Register user data to be packed with the thread
	int add(const UserData &d);
	void *lookupUserData(int ud);
	
	inline static TCharm *get(void) {
		TCharm *c=getNULL();
#ifndef CMK_OPTIMIZE
		if (!c) ::CkAbort("TCharm has not been initialized!\n");
#endif
		return c;
	}
	inline static TCharm *getNULL(void) {return CtvAccess(_curTCharm);}
	inline CthThread getThread(void) {return tid;}
	inline const CProxy_TCharm &getProxy(void) const {return threadInfo.tProxy;}
	inline int getElement(void) const {return threadInfo.thisElement;}
	inline int getNumElements(void) const {return threadInfo.numElements;}

	//Start/stop load balancer measurements
	inline void stopTiming(void) {ckStopTiming();}
	inline void startTiming(void) {ckStartTiming();}

	//Block our thread, run the scheduler, and come back
	void schedule(void);

	//As above, but start/stop the thread itself, too.
	void stop(void); //Blocks; will not return until "start" called.
	void start(void);
	//Aliases:
	inline void suspend(void) {stop();}
	inline void resume(void) { if (isStopped) start();}

	//Go to sync, block, possibly migrate, and then resume
	void migrate(void);

	//Entering thread context: turn stuff on
	static void activateThread(void) {
		TCharm *tc=CtvAccess(_curTCharm);
		if (tc!=NULL) {
			CmiIsomallocBlockListActivate(tc->heapBlocks);
			CtgInstall(tc->threadGlobals);
		}
	}
	//Leaving this thread's context: turn stuff back off
	static void deactivateThread(void) {
		CmiIsomallocBlockListActivate(NULL);
		CtgInstall(NULL);		
	}
};


//Created in all API routines: disables/enables migratable malloc
class TCharmAPIRoutine {
 public:
	TCharmAPIRoutine() { //Entering Charm++ from user code
		//Disable migratable memory allocation while in Charm++:
		TCharm::deactivateThread();
	}
	~TCharmAPIRoutine() { //Returning to user code from Charm++:
		//Reenable migratable memory allocation
		TCharm::activateThread();
	}
};


#define TCHARMAPI(routineName) TCHARM_API_TRACE(routineName,"tcharm");

//Node setup callbacks: called at startup on each node
FDECL void FTN_NAME(TCHARM_USER_NODE_SETUP,tcharm_user_node_setup)(void);
FDECL void FTN_NAME(TCHARM_USER_SETUP,tcharm_user_setup)(void);


#endif


