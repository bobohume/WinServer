#ifndef __COPY_DATA_H__
#define __COPY_DATA_H__
#include "Common/DataFile.h"
#include "BaseData.h"
#include <string.h>
#include <hash_map>

struct CGateData
{
	U32 m_GateID;
	U32 m_CopyID;

	U32 m_Troops;
	U32 m_SoldierHp;
	U32 m_SoldierAtk;

	U32 m_AddMoney;
	U32 m_AddAchievement;
	U32	m_AddLand;

	// no import
	U8  m_BattleBack;

	CGateData():m_GateID(0), m_CopyID(0), m_Troops(0), m_SoldierHp(0), \
		m_SoldierAtk(0), m_AddMoney(0), m_AddAchievement(0), m_AddLand(0), \
		m_BattleBack(0)
	{
	}
};

struct CCopyData
{
	typedef stdext::hash_map<U32, CGateData*>			GATE_MAP;
	typedef GATE_MAP::iterator							GATE_ITR;

	CCopyData():m_CopyID(0), m_sName(""), m_Land(0), m_NpcIcon(0), \
		m_Troops(0), m_SoldierHp(0), m_SoldierAtk(0), m_Rebellion(0)
	{
		memset(m_AwardItem, 0, sizeof(m_AwardItem));
		memset(m_AwardNum, 0, sizeof(m_AwardNum));
		memset(m_RebellionItem, 0, sizeof(m_RebellionItem));
	}

	~CCopyData();

	U32 m_CopyID;
	std::string m_sName;
	U32 m_Land;
	S32	m_Troops;
	S16 m_SoldierHp;
	S16	m_SoldierAtk;
	S8	m_GateNum;
	S32	m_NpcIcon;
	S32 m_AwardItem[4];
	S16 m_AwardNum[4];
	S32 m_Rebellion;
	S32 m_RebellionItem[4];
	GATE_MAP m_GateMap;
};

class CCopyDataRes : public CBaseDataRes<CCopyData>
{
public:
	CCopyDataRes();
	virtual ~CCopyDataRes();

	bool read();
	CGateData* getGateData(S32 GateId);
	U32 getNextGateData(S32 GateId);
};

extern CCopyDataRes* g_CopyDataMgr;
#endif//__COPY_DATA_H__