#ifndef __CARD_ITEM_H__
#define __CARD_ITEM_H__

#include "Common/DataFile.h"
#include "BaseData.h"
#include <string.h>
#include <hash_map>
#include <vector>

#ifndef MACRO_SKILL_ID
#define  MACRO_SKILL_ID(series,level)  (U32)((U32)series *1000 +(U32)level)
#endif

#define MACRO_EXP_ID(color, level) (U32)((U32)color*1000 + (U32)level)

const unsigned int MAX_INIT_SKILL = 3;

struct CCardData;
class CCardDataRes:public CBaseDataRes<CCardData>
{
public:
	CCardDataRes();
	virtual ~CCardDataRes();

	bool read();
};


struct CCardData
{
	U32		m_CardID;
	std::string m_sName;
	std::string m_sDes;
	U8		m_Color;
	U32		m_Icon;

	U16		m_InitAgi;
	U16		m_InitInt;
	U16		m_InitStr;

	U16		m_InitAgiGrow;
	U16		m_InitIntGrow;
	U16		m_InitStrGrow;

	U32		m_SkillID[MAX_INIT_SKILL];

	U8		m_HireType;
	S32		m_HireVal;
	std::string m_Pos;
	U8		m_ApplyPos;
	CCardData():m_CardID(0), m_sName(""), m_sDes(""), m_Color(0), m_Icon(0), m_InitAgi(0), m_InitInt(0), m_InitStr(0), \
		m_InitAgiGrow(0), m_InitIntGrow(0), m_InitStrGrow(0), m_HireType(0), m_HireVal(0), m_Pos(""), m_ApplyPos(0)
	{
		memset(m_SkillID, 0, sizeof(m_SkillID));
	}
};

struct CExpData
{
	U32 m_Id;
	S32 m_CostMoney;
	S32 m_TotalMoney;
	CExpData() : m_Id(0), m_CostMoney(0), m_TotalMoney(0)
	{
	}
};

class CExpDataRes : public CBaseDataRes<CExpData>
{
public:
	CExpDataRes();
	virtual ~CExpDataRes();

	bool read();
};

struct CApplyData
{
	S32 m_Id;
	std::string m_Des;
	U8	m_Type[2];
	CApplyData() : m_Id(0), m_Des("")
	{
		memset(m_Type, 0, sizeof(m_Type));
	}
};

class CApplyDataRes : public CBaseDataRes<CApplyData>
{
public:
	CApplyDataRes();
	virtual ~CApplyDataRes();

	bool read();
};

struct CLvLimitData
{
	S32 m_Id;
	std::string m_Des;
	S32 m_LimitLv;
	S32 m_ItemId[3];
	S32 m_AddV;
	CLvLimitData() : m_Id(0), m_Des(""), m_LimitLv(0), m_AddV(0)
	{
		memset(m_ItemId, 0, sizeof(m_ItemId));
	}
};

class CLvLimitDataRes : public CBaseDataRes<CLvLimitData>
{
public:
	CLvLimitDataRes();
	virtual ~CLvLimitDataRes();

	bool read();
};

enum FateType
{
	FATE_TYPE_ALL = 1,
	FATE_TYPE_ONE = 2,
};

struct CFateData
{
	S32 m_Id;
	std::string m_Name;
	S8  m_Type;
	S8  m_AddType;
	S32 m_AddVal;
	std::vector<S32> m_CardVec;
	CFateData() : m_Id(0), m_Name(""), m_Type(0), m_AddType(0), m_AddVal(0)
	{
	}
};

class CFateDataRes : public CBaseDataRes<CFateData>
{
public:
	CFateDataRes();
	virtual ~CFateDataRes();

	bool read();
	std::vector<CFateData*>& getFateVec();
private:
	std::vector<CFateData*> m_DataVec;
};
extern CExpDataRes* g_ExpDataMgr;
extern CCardDataRes* g_CardDataMgr;
extern CApplyDataRes* g_ApplyDataMgr;
extern CLvLimitDataRes* g_LvLimitDataMgr;
extern CFateDataRes* g_FateDataMgr;
#endif // __CARD_ITEM_H__
