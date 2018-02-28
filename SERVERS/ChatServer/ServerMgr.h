#ifndef _SERVER_MGR_H_
#define _SERVER_MGR_H_

#include <hash_map>
#include <memory>
#include <hash_set>
#include "base/locker.h"

class GateServer
{
public:
    GateServer(void)
    {
        Clear();
    }

    void Clear(void)
    {
        socketId = 0;
        lineId   = 0;
        gateId   = 0;
    }

    bool IsValid(void) const {return 0 != socketId;}

	int socketId;
	int lineId;
	int gateId;
};

#define MAX_LINE_COUNT 256
#define MAX_GATE_COUNT 1000

class CServerMgr
{
public:
	CServerMgr();
	virtual ~CServerMgr();
    
	void addServer(int socketId,int lineId,int gateId);
	void removeServer(int socketId);

    int  GetServer(int lineId,int gateId);
  
	int  getLineId( int socketId );
	void getGates( int* gates, int& count );
	void getAllGates(std::vector<int>& gateList,int lineId);
private:
    CMyCriticalSection m_cs;

	typedef stdext::hash_map<int,GateServer*> ServerMap;
	ServerMap mServerMap;

    GateServer m_gates[MAX_LINE_COUNT][MAX_GATE_COUNT];
};

#endif /*_SERVER_MGR_H_*/