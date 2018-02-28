#include "RandomNameData.h"
#include "Common/OrgBase.h"
#include "CommLib/CommLib.h"
#include "BuffData.h"
#include "Common/mRandom.h"

CRandomNameDataRes* g_RandomNameDataMgr = new CRandomNameDataRes();

std::string RandName() 
{
	static bool s_bInit = false;
	if (!s_bInit) {
		g_RandomNameDataMgr->read();
		s_bInit = true;
	}
	return g_RandomNameDataMgr->randname();
}

CRandomNameDataRes::CRandomNameDataRes()
{

}

CRandomNameDataRes::~CRandomNameDataRes()
{
	clear();
}

void CRandomNameDataRes::clear(){
	m_FNameVec.clear();
	m_SNameVec.clear();
	Parent::clear();
}

std::string CRandomNameDataRes::randname() {
	std::ostringstream oStr;
	S32 nRandVal = gRandGen.randI(0, m_FNameVec.size() - 1);
	oStr << m_FNameVec[nRandVal]->m_Name;
	nRandVal = gRandGen.randI(0, m_SNameVec.size() - 1);
	oStr << m_SNameVec[nRandVal]->m_Name;
	return oStr.str();
}

bool CRandomNameDataRes::read()
{
	CDataFile file;
	RData LineData;

	if(!file.readDataFile("data/RandomName.dat"))
	{
		JUST_ASSERT("read RandomName error");
		return  false;
	}

	for(int i = 0; i < file.RecordNum; ++i)
	{
		CRandomNameData* pdata = new CRandomNameData();

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read RandomName id error");
		pdata->m_Id = LineData.m_U16;


		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read RandomName type error");
		pdata->m_Type = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_string, "read RandomName name error");
		pdata->m_Name = LineData.m_string;
		addData(pdata->m_Id, pdata);
		if (pdata->m_Type == 1)
		{
			m_FNameVec.push_back(pdata);
		}
		else {
			m_SNameVec.push_back(pdata);
		}
	}
	return true;
}