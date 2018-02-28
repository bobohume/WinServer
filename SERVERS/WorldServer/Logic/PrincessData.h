#ifndef __PRINCESS_DATA__
#define __PRINCESS_DATA__
#include "BASE/tVector.h"
#include "Common/DataFile.h"
#include "BaseData.h"
#include <string.h>
#include <hash_map>

struct CPrincessData;
class CPrincessDataRes : public CBaseDataRes<CPrincessData>
{
public:
	CPrincessDataRes();
	~CPrincessDataRes();

	bool GetUIDsByHireVal(U32 HireVal, std::vector<U32>& UIDs);

	bool read();

private:
	std::unordered_map<U32, std::vector<U32>> HireMap;
};

struct CPrincessData
{
	CPrincessData():m_ID(0), m_sName(""), m_sDes(""), m_ResID(0),\
		m_InitCharm(0), m_HireType(0), m_HireVal(0), m_Pos(""), m_Pos1("")
	{
		memset(m_SkillID, 0, sizeof(m_SkillID));
	};
	~CPrincessData(){};

	S32	m_ID;
	std::string m_sName;
	std::string	m_sDes;
	U32 m_ResID;
	U16 m_InitCharm;
	U32 m_SkillID[3];
	U8  m_HireType;
	U32 m_HireVal;
	std::string m_Pos;
	std::string m_Pos1;
};

struct CFlowerData;
class CFlowerDataRes : public CBaseDataRes<CFlowerData>
{
public:
	CFlowerDataRes();
	~CFlowerDataRes();

	bool read();
};

struct CFlowerData
{
	CFlowerData() : m_ItemID(0), m_Charm(0), m_FlowerTime(0)
	{
	}
	~CFlowerData() {}
	S32 m_ItemID;
	S32 m_Charm;
	S32 m_FlowerTime;
};

struct CPrincessRobotData;
class CPrincessRobotDataRes : public CBaseDataResEx<CPrincessRobotData>
{
	typedef Vector<CPrincessRobotData*> ROBOTVEC;
public:
	CPrincessRobotDataRes();
	~CPrincessRobotDataRes();
	CPrincessRobotData* RandRobot();
	bool read();
private:
	ROBOTVEC m_RobotVec;
};

struct CPrincessRobotData
{
	CPrincessRobotData() : m_Id(0), m_PrincessId(0), Battle(0),
		Culture(0), Rite(0), Charm(0)
	{
	}
	~CPrincessRobotData() {}
	S32 m_Id;
	S32 m_PrincessId;
	S32 Battle;
	S32 Culture;
	S32 Rite;
	S32 Charm;
};

extern CPrincessDataRes* g_PrincessDataMgr;
extern CFlowerDataRes*   g_FlowerDataMgr;
extern CPrincessRobotDataRes* g_PrincessRobotDataMgr;
#endif//__PRINCESS_DATA__