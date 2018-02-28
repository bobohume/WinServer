#ifndef __PVP_DATA__
#define __PVP_DATA__
#include "Common/DataFile.h"
#include "BaseData.h"
#include <string.h>
#include <hash_map>
#include "BASE/tVector.h"

struct CPvpData;
class CPvpDataRes : public CBaseDataResEx<CPvpData>
{
	typedef std::unordered_map<S32, CPvpData*> PVPMAP;
public:
	CPvpDataRes();
	~CPvpDataRes();

	bool read();
	std::vector<CPvpData*> GetPvpData(S32 Id);

	PVPMAP m_PvpMap;
};

struct CPvpData
{
	CPvpData():m_ID(0), m_sName(""), m_Score(0), m_CardID(0),\
		m_Agi(0), m_Int(0), m_Str(0)
	{
	};
	~CPvpData(){};

	S32	m_ID;
	std::string m_sName;
	S32 m_Score;
	S32 m_CardID;
	S32 m_Agi;
	S32 m_Int;
	S32 m_Str;
};

struct CPvpBuffData;
class CPvpBuffDataRes : public CBaseDataRes<CPvpBuffData>
{
public:
	CPvpBuffDataRes();
	~CPvpBuffDataRes();

	bool read();
	S8	 randomBuff(S32 nType);
private:
	Vector<CPvpBuffData*> m_BuffTypeVec[5];
};

struct CPvpBuffData
{
	CPvpBuffData() :m_ID(0), m_sName(""), m_BuffType(0), m_BuffID(0), m_CostType(0), \
		m_CostNum(0), m_BuffLv(0)
	{
	};
	~CPvpBuffData() {};

	S32	m_ID;
	std::string m_sName;
	S32	m_BuffLv;
	S32 m_BuffType;
	S32 m_BuffID;
	S32 m_CostType;
	S32 m_CostNum;
};

struct CTopAwardData;
class CTopAwardDataRes : public CBaseDataRes<CTopAwardData>
{
public:
	CTopAwardDataRes();
	~CTopAwardDataRes();

	bool read();
	CTopAwardData* getTopRand(S32 nRank);
};

struct CTopAwardData
{
	CTopAwardData() :m_ID(0), m_Begin(0), m_End(0)
	{
		memset(m_Award, 0, sizeof(m_Award));
		memset(m_AwardNum, 0, sizeof(m_AwardNum));
	};
	~CTopAwardData() {};

	S32	m_ID;
	S32 m_Begin;
	S32 m_End;
	S32	m_Award[4];
	S32 m_AwardNum[4];
};

extern CPvpDataRes* g_PvpDataMgr;
extern CPvpBuffDataRes* g_PvpBuffDataMgr;
extern CTopAwardDataRes* g_TopAwardDataMgr;
#endif//__PVP_DATA__