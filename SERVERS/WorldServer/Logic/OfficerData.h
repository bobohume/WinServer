#ifndef __OFFICER_DATA__
#define __OFFICER_DATA__
#include "Common/DataFile.h"
#include "BaseData.h"
#include <string.h>
#include <hash_map>

struct COfficerData;
class COfficerDataRes : public CBaseDataRes<COfficerData>
{
public:
	COfficerDataRes();
	~COfficerDataRes();

	bool read();
};

struct COfficerData
{
	COfficerData():m_OfficerId(0), m_sDes(""), m_OfficerType(0), m_OfficerVal(0),\
		m_SkillID(0), m_CountryFlag(0)
	{};
	~COfficerData(){};


	U8	m_OfficerId;
	std::string	m_sDes;
	U8	m_OfficerType;
	U32 m_OfficerVal;
	U32 m_SkillID;
	U8  m_CountryFlag;
};
extern COfficerDataRes* g_OfficerDataMgr;
#endif//__OFFICER_DATA__