//
//  DropData.h
//  HelloLua
//
//  Created by th on 14-2-12.
//
//

#ifndef __HelloLua__DropData__
#define __HelloLua__DropData__


#include "Common/DataFile.h"
#include <string.h>
#include <hash_map>


enum enDropName {
	NpcGroup = 1,
	GoodsGroup,
	TrapGroup,
	DropBag,
	DropMax
};

const static char* DropName[DropMax-1]=
{
	"npc组",
	"物品组",
	"陷阱组",
	"掉落包"
};

class CDropData;
class CDropDataRes {

public:
	enum{
		DROP_BEGIN_ID=6001001,
		DROP_END_ID=  6999999
	};
	typedef std::vector<CDropData*> CROPDATAVEC;
	typedef CROPDATAVEC::iterator	CROPDATAITR;

	CDropDataRes();
	~CDropDataRes();

	typedef stdext::hash_multimap<S32,CDropData*> DROPDATAMAP;
	typedef DROPDATAMAP::iterator DROPDATAMAPITR;

	bool read();
	void close();
	void clear();
	bool getDropData(S32 DropId, CROPDATAVEC& DropVec, U32& nMaxVal);
private:
	DROPDATAMAP m_DataMap;
};

class CDropData {

public:
	CDropData():m_DropId(0),m_Type(0),m_GoodsId(0),m_Odds(0)
	{};
	~CDropData(){};

	S32 m_DropId;
	U8 m_Type;
	S32 m_GoodsId;
	F32 m_Odds;
};

extern CDropDataRes* g_DropDataMgr;

#endif /* defined(__HelloLua__DropData__) */
