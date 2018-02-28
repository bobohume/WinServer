#ifndef __RANDOMNAME_DATA__
#define __RANDOMNAME_DATA__
#include "BASE/tVector.h"
#include "Common/DataFile.h"
#include "BaseData.h"
#include <string.h>
#include <hash_map>

struct CRandomNameData;
class CRandomNameDataRes : public CBaseDataRes<CRandomNameData>
{
	typedef CBaseDataRes<CRandomNameData> Parent;
	typedef Vector<CRandomNameData*> RANDNAME_VEC;
public:
	CRandomNameDataRes();
	~CRandomNameDataRes();

	void clear();
	bool read();
	std::string randname();
private:
	RANDNAME_VEC m_FNameVec;
	RANDNAME_VEC m_SNameVec;
};

struct CRandomNameData
{
	CRandomNameData() :m_Id(0), m_Name(""), m_Type(0)
	{
	}

	~CRandomNameData() {};

	S32 m_Id;
	S32 m_Type;
	std::string m_Name;
};

extern std::string RandName();
extern CRandomNameDataRes* g_RandomNameDataMgr;
#endif//__RANDOMNAME_DATA__