#ifndef TOP_APPRENTICE_H
#define TOP_APPRENTICE_H

#include "AccountHandler.h"
#include <hash_map>
#include "ManagerBase.h"
#include "LockCommon.h"
#include "Event.h"
#include "base\bitStream.h"
#include "../Common/TopType.h"
#include "../Common/OrgBase.h"
#include <map>
#include <set>

class CtopApprentice
{
public:
	CtopApprentice();
	~CtopApprentice();

	typedef stdext::hash_map<int,int> CACHE;
	
	CACHE getApprtcMap();
	void  updateMap(int playerId, int num);

private:
	void readDB();
	CACHE m_topApprentice;
};


#endif