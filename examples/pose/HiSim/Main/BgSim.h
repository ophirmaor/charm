using namespace std;
#include <stdlib.h>
#include <math.h>
#include <map>
#include <vector>
#include <string>
#include <deque>
#include <algorithm>
#include <functional>
#include <utility>
#include <limits.h>
#include "util.h"
#include "InitNetwork.h"

//#define DEBUG

#ifdef DEBUG
#define print_praveen CkPrintf
#else
#define print_praveen //
#endif

#define SIMPLE_SEND 0
#define BROADCAST 1
#define INTERNAL_DELAY 1
#define ARBITRATION_DELAY 1
#define REQUEST_DELAY 1
#define ACK_DELAY 1
#define START_LATENCY 200
#define IDLE -1
#define MESH3D 1
#define HYPERCUBE 2

enum {
	X_POS = 0,
	Y_POS,
	Z_POS,
	X_NEG,
	Y_NEG,
	Z_NEG,
	INJECTION_PORT
};

extern unsigned int netLength;
extern unsigned int netHeight;
extern unsigned int netWidth;
extern Config config;

#define maxP(a,b) ((a)>(b)?(a):(b))
#define minP(a,b) ((a)<(b)?(a):(b))

class Position {
	public :
	int x,y,z;

	Position() {}
	Position(int a,int b,int c): x(a),y(b),z(c){};

	void init(int id) {
		z = id / (netLength * netHeight);
		id = id - z*(netLength * netHeight);
		y = id / (netLength);
		x = id - y*netLength;
	}

	void initnew(int a,int b,int c) { x = a; y = b; z = c; }

	Position operator +(int axis) {
		Position ret = *this;
		switch(axis) {  // Use -1 to signify an invalid.node
			case X_POS: if(netLength != 1) ret.x = (x+1)%netLength; break;
			case Y_POS: if(netHeight != 1) ret.y = (y+1)%netHeight; break;
			case Z_POS: if(netWidth != 1) ret.z = (z+1)%netWidth; break;
			case X_NEG: if(netLength != 1) ret.x = (x+netLength-1)%netLength; break;
			case Y_NEG: if(netHeight != 1) ret.y = (y+netHeight-1)%netHeight; break;
			case Z_NEG: if(netWidth != 1) ret.z = (z+netWidth-1)%netWidth; break;
		}
		return ret;
	}

	int getId() { return(x+y*netLength+z*netLength*netHeight); }

	void getNeighbours(int *next) {
		Position tmp = *this,tmp2;
		for(int i=0;i<6;i++) {
			tmp2 = (tmp+i);
			next[i]  = tmp2.getId();
		}
	}

	Position & operator=(const Position& obj) {
		x = obj.x; y = obj.y; z = obj.z;
		return *this;
	}
	int operator != (const Position& obj) {
		if((x != obj.x) || (y != obj.y) || (z != obj.z)) return 1;
		return 0;
	}
};

class RoutingInformation {
	public:
	int dst;
	int datalen;
};

class NicMsg {
	public:
	RoutingInformation routeInfo;
	int src;
	int pktId;
	int msgId;
	int totalLen;   // Total length of message. is constant across packets of a message
	int destNodeCode;
	int index;
	int destTID;
	int recvTime;
	int origovt;

	NicMsg(){}
	NicMsg & operator=(const NicMsg &obj)
	{
		eventMsg::operator=(obj);
		src = obj.src;  routeInfo = obj.routeInfo;
		pktId = obj.pktId; msgId = obj.msgId;  
		totalLen = obj.totalLen;  origovt = obj.origovt;
		destNodeCode = obj.destNodeCode;
		index = obj.index;
		destTID = obj.destTID; 
		recvTime = obj.recvTime; 
		return *this;
	}
};

class Header {
        public:
	RoutingInformation routeInfo;
        int src;
        int pktId;
        int msgId;
        int totalLen,nextId,prevId;
        int portId;
        int vcid,prev_vcid;
        Header(){}

        Header & operator=(const Header &obj)
        {
                src = obj.src; routeInfo = obj.routeInfo;
                pktId = obj.pktId; msgId = obj.msgId; 
                totalLen = obj.totalLen; prevId = obj.prevId;
                portId = obj.portId; nextId = obj.nextId;
                vcid = obj.vcid; prev_vcid = obj.prev_vcid;
                return *this;
        }

        Header & operator=(const NicMsg &obj)
        {
		src = obj.src;  routeInfo = obj.routeInfo;
		pktId = obj.pktId; msgId = obj.msgId;  
		totalLen = obj.totalLen;  
                return *this;
        }

        void dump()
        {
                CkPrintf("HEADER src %d dst %d pktid %d portId %d msgId %d \n",
                                src,routeInfo.dst,pktId,portId,msgId);
        }

        bool operator==(const Header &obj) const {
                if(src==obj.src && routeInfo.dst==obj.routeInfo.dst && pktId==obj.pktId && msgId==obj.msgId)  return true;  else  return false;
        }

        bool operator < (const Header &obj) const {
                if(msgId < obj.msgId) return true; else
                if((msgId == obj.msgId) && (pktId < obj.pktId)) return true;
                else return false;
        }
};

class Packet {
	public:
	Header hdr;
	Packet(){}
	Packet & operator=(const Packet &obj)
	{
		eventMsg::operator=(obj);
		hdr = obj.hdr;
		return *this;
	}

        void dump() { hdr.dump(); }

};
	
class MsgStore {
        public:
        int src;
        int pktId;
        int msgId;
        int totalLen;
        int recvTime;
        int destNodeCode;
        int index;
        int destTID;
	int origovt;

        MsgStore(){}

        MsgStore & operator=(const MsgStore &obj)
        {
                src = obj.src; pktId = obj.pktId; msgId = obj.msgId;  origovt = obj.origovt;
                totalLen = obj.totalLen; destNodeCode = obj.destNodeCode;
                index = obj.index; destTID = obj.destTID; recvTime = obj.recvTime; 
                return *this;
        }

        MsgStore & operator=(const NicMsg &obj)
        {
                src = obj.src; pktId = obj.pktId; msgId = obj.msgId;  origovt = obj.origovt;
                totalLen = obj.totalLen; 
                destNodeCode = obj.destNodeCode; index = obj.index;
                destTID = obj.destTID; recvTime = obj.recvTime; 
	}

        bool operator==(const MsgStore &obj) const {
                if(src==obj.src && pktId==obj.pktId && msgId==obj.msgId)  return true;  else  return false;
        }
};

class flowStart {
        public:
	int nextId;
        int vcid;
	int prev_vcid;
	int datalen;
        flowStart(){}
        flowStart & operator=(const flowStart &obj)
        {
        eventMsg::operator=(obj);
        vcid = obj.vcid;
	prev_vcid = obj.prev_vcid;
	nextId = obj.nextId;
	datalen = obj.datalen;
        return *this;
        }
};

class remoteMsgId
{
	public:
	int msgId;
	int nodeId;

	remoteMsgId(){}
	remoteMsgId(int m,int s):msgId(m),nodeId(s){}
	remoteMsgId & operator=(const remoteMsgId &obj)
	{	msgId = obj.msgId;	nodeId = obj.nodeId;  return *this;  }

	bool operator<(const remoteMsgId &obj) const {
		if(nodeId < obj.nodeId) return true; else 
		if((nodeId == obj.nodeId) && (msgId < obj.msgId)) return true;
		else return false;
	}
	bool operator==(const remoteMsgId &obj) const {
		if(msgId==obj.msgId  && nodeId==obj.nodeId) return true;  else  return false;
	}
};


class remotePktId
{
	public:
	int pktId;
	int len;

	remotePktId(){}
	remotePktId(int p,int l):pktId(p),len(l){}
	remotePktId & operator=(const remotePktId &obj)
	{	pktId = obj.pktId; len = obj.len; return *this;  }

	bool operator<(const remotePktId &obj) const {
		if(pktId < obj.pktId)  return true;   else  return false;
	}
	bool operator==(const remotePktId &obj) const {
		if(pktId==obj.pktId  && len==obj.len)	return true;   else   return false;
	}
};

class NetInterfaceMsg {
	public:
	int id;
	int startId;
	int numP;
	
	NetInterfaceMsg(){}
	NetInterfaceMsg(int i,int start,int np):
		id(i),startId(start),numP(np){}

	~NetInterfaceMsg(){}	

	NetInterfaceMsg& operator = (const NetInterfaceMsg& obj) {
		eventMsg::operator=(obj);
		id = obj.id;
		startId = obj.startId;
		numP = obj.numP;
		return *this;
	}
		
};

class InputBufferMsg {
	public:
	int id;
	int portid;
	int nodeid;
	int numP;
	int startChannelId;
	
	InputBufferMsg(){}
	InputBufferMsg(int i,int pid,int nid,int np,int sid):
		id(i),portid(pid),nodeid(nid),numP(np),startChannelId(sid){}

	~InputBufferMsg(){}	

	InputBufferMsg& operator = (const InputBufferMsg& obj) {
		eventMsg::operator=(obj);
		id = obj.id;
		portid = obj.portid;
		nodeid = obj.nodeid;
		numP = obj.numP;
		startChannelId = obj.startChannelId;
		return *this;
	}
};		

class ChannelMsg {
	public:
	int id;
	int portid;
	int nodeid;
	int numP;
	
	ChannelMsg(){}
	ChannelMsg(int i,int pid, int nid,int nump):
		id(i),portid(pid),nodeid(nid),numP(nump){}

	~ChannelMsg(){}	

	ChannelMsg& operator = (const ChannelMsg& obj) {
		eventMsg::operator=(obj);
		id = obj.id;
		portid = obj.portid;
		nodeid = obj.nodeid;
		numP = obj.numP;
		return *this;
	}
};		

class SwitchMsg {
	public:
	int id;
	SwitchMsg(){}
	SwitchMsg(int i):id(i){}
	~SwitchMsg(){}
	SwitchMsg &operator = (const SwitchMsg &obj) {
		eventMsg::operator=(obj);
		id = obj.id;
		return *this;
	}
};

#define NO_VC_AVAILABLE -1

class NicConsts {
	public:
	int id;
	Position pos;
	int startId,numP;
};

#ifndef compile
#include "../Topology/MainTopology.h"

class Switch {
	public:
	map <int,int> Bufsize;	 // Downstream buffer sizes .. Note this is not current switch buffer sizes.
	map <int,vector <Header> > inBuffer;  // Save the packets when path is stalled
	map <int,int> mapVc; // This is mainly a way to keep a way of updating credits for inputVc based on outputVc
	map <int,int> requested;  // Used to do head of line blocking

	int id;
        unsigned  char InputRoundRobin,RequestRoundRobin,AssignVCRoundRobin;
        Topology *topology;
        InputVcSelection *inputVcSelect;
        OutputVcSelection *outputVcSelect;
		
	Switch(){}
	Switch(SwitchMsg *m);

	void recvPacket(Packet *);
	void recvPacket_anti(Packet *){restore(this);}
	void recvPacket_commit(Packet *){}
	void updateCredits(flowStart *);
	void updateCredits_anti(flowStart *){restore(this);}
	void updateCredits_commit(flowStart *){}
	void checkNextPacketInVc(flowStart *);
	void checkNextPacketInVc_anti(flowStart *){restore(this);}
	void checkNextPacketInVc_commit(flowStart *){}
	void sendPacket(Packet *,const int &,const int &,const int &);
	
	~Switch(){}

	Switch& operator=(const Switch& obj) {
	rep::operator=(obj);
	Bufsize = obj.Bufsize; InputRoundRobin = obj.InputRoundRobin; RequestRoundRobin = obj.RequestRoundRobin;  
	inBuffer = obj.inBuffer; AssignVCRoundRobin = obj.AssignVCRoundRobin; topology = obj.topology; mapVc = obj.mapVc;
	requested = obj.requested;
	return *this;
	}
	bool operator==(const Switch& obj) {
	return(id==obj.id);
	}
};



// Should take care of contention when multiple ports are sending data to nic ...
class NetInterface {
	public:
	map <remoteMsgId,int > pktMap;
	map <remoteMsgId,MsgStore> storeBuf;
	int numRecvd,roundRobin;
	int prevIntervalStart,counter;
	Topology *topology;
	NicConsts *nicConsts;

	NetInterface(){}
	NetInterface(NetInterfaceMsg *niMsg);

	void recvMsg(NicMsg *);
	void recvMsg_anti(NicMsg *){restore(this);}
	void recvMsg_commit(NicMsg *){}
	void recvPacket(Packet *);
	void recvPacket_anti(Packet *){restore(this);}
	void recvPacket_commit(Packet *){}
	void storeMsgInAdvance(NicMsg *);
	void storeMsgInAdvance_anti(NicMsg *){restore(this);}
	void storeMsgInAdvance_commit(NicMsg *){}
	~NetInterface(){}

	NetInterface& operator=(const NetInterface& obj) {
	rep::operator=(obj);
	pktMap = obj.pktMap; numRecvd = obj.numRecvd;  nicConsts = obj.nicConsts; storeBuf = obj.storeBuf;
	prevIntervalStart = obj.prevIntervalStart; counter = obj.counter; topology = obj.topology;roundRobin = obj.roundRobin;
	return *this;
	}

	bool operator==(const NetInterface& obj) {
	return(nicConsts->id==((obj.nicConsts)->id));
	}
};

class Request
{
        public:
        int nextId;
        int datalen; 
	int vcid;
        Request(){}
        Request(int n,int d):nextId(n),datalen(d){}
        Request & operator=(const Request &obj)
        {
	       nextId = obj.nextId; 
		datalen = obj.datalen;  vcid = obj.vcid; }
        bool operator == (const Request &obj) const {
               if((nextId == obj.nextId) && (vcid == obj.vcid) && (datalen == obj.datalen)) return true; else return false;
        }
};

class InputBufferConsts {
	public:
	int id;
	Position pos,*next;
	int nodeid;	
	int numP,startChannelId;
	int portid;	
};
	
class InputBuffer {
	public:
	InputBufferConsts *ibuf;
	unsigned  char InputRoundRobin,RequestRoundRobin,AssignVCRoundRobin;
        map <int,int> Bufsize;
        map <int,int> requested;
        map <int,vector <Header> >  inBuffer;
        map <int,vector <Request> > requestQ;
	Topology *topology;
	InputVcSelection *inputVcSelect;
	OutputVcSelection *outputVcSelect;

	InputBuffer(){}
	InputBuffer(InputBufferMsg *m);

	void recvPacket(Packet *);
	void recvPacket_anti(Packet *){restore(this);}
	void recvPacket_commit(Packet *){}
	void restartBufferFlow(flowStart *);
	void restartBufferFlow_anti(flowStart *){restore(this);}
	void restartBufferFlow_commit(flowStart *){}
	void recvRequest(Packet *);
	void recvRequest_anti(Packet *){restore(this);}
	void recvRequest_commit(Packet *){}
	void checkVC(flowStart *);
	void checkVC_anti(flowStart *){restore(this);}
	void checkVC_commit(flowStart *){}

	~InputBuffer(){}

	InputBuffer& operator=(const InputBuffer& obj) {
	rep::operator=(obj);
	ibuf = obj.ibuf;
	Bufsize = obj.Bufsize; InputRoundRobin = obj.InputRoundRobin; RequestRoundRobin = obj.RequestRoundRobin; requested = obj.requested; 
	inBuffer = obj.inBuffer; requestQ = obj.requestQ; AssignVCRoundRobin = obj.AssignVCRoundRobin; topology = obj.topology;
	return *this;
	}
	bool operator==(const InputBuffer& obj) {
	return(ibuf->id==(obj.ibuf)->id);
	}
};

#endif

class Channel {
	public:
	int id;
	int prevIntervalStart,counter,portid,nodeid,numP;

	Channel() {}
	~Channel(){}
	Channel(ChannelMsg *m);
	void recvPacket(Packet *);
	void recvPacket_anti(Packet *){restore(this);}
	void recvPacket_commit(Packet *){}

	Channel& operator=(const Channel& obj) {
	rep::operator=(obj);
	id=obj.id;  portid = obj.portid; 
	prevIntervalStart = obj.prevIntervalStart; counter = obj.counter;nodeid =obj.nodeid;numP = obj.numP;
	return *this;
	}

	bool operator==(const Channel& obj) {
	return(id==obj.id);
	}
};
