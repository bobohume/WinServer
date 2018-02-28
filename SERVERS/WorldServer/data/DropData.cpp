//
//  DropData.cpp
//  HelloLua
//
//  Created by th on 14-2-12.
//
//

#include "DropData.h"
#include "Common/OrgBase.h"
#include "CommLib/CommLib.h"

CDropDataRes * g_DropDataMgr=new CDropDataRes();

CDropDataRes::CDropDataRes()
{

}
CDropDataRes::~CDropDataRes()
{
	clear();
}

bool CDropDataRes::read()
{
	CDataFile file;
	RData lineData;
	if (!file.readDataFile("data/Data_6_Drop.dat")) {
		JUST_ASSERT("read dropdata error");
		return false;
	}
	for (int i=0;i<file.RecordNum; ++i) {
		CDropData* pData=new CDropData();

		file.GetData(lineData);
		DOASSERT(lineData.m_Type==DType_S32,"read dropdata id error");
		pData->m_DropId=lineData.m_S32;

		file.GetData(lineData);
		DOASSERT(lineData.m_Type==DType_enum8,"read dropdata type error");
		pData->m_Type=lineData.m_Enum8;

		file.GetData(lineData);
		DOASSERT(lineData.m_Type==DType_S32,"read dropdata goodsid error");
		pData->m_GoodsId=lineData.m_S32;

		file.GetData(lineData);
		DOASSERT(lineData.m_Type==DType_F32,"read dropdata odds error");
		pData->m_Odds=lineData.m_F32;

		m_DataMap.insert(DROPDATAMAP::value_type(pData->m_DropId,pData));
	}
	return true;
}

void CDropDataRes::clear()
{
	for (DROPDATAMAPITR itr=m_DataMap.begin(); itr!=m_DataMap.end(); ++itr) 
	{
		SAFE_DELETE(itr->second);
	}

	m_DataMap.clear();
}

void CDropDataRes::close()
{
	clear();
}

bool CDropDataRes::getDropData(S32 DropId, CROPDATAVEC& DropVec,  U32& nMaxVal)
{
	DropVec.clear();

	if (DropId<DROP_BEGIN_ID || DropId>DROP_END_ID) {
		return false;
	}

	DROPDATAMAPITR LOWItr = m_DataMap.lower_bound(DropId);  
	DROPDATAMAPITR UPItr  = m_DataMap.upper_bound(DropId);
	
	for(DROPDATAMAPITR itr = LOWItr; itr != UPItr; ++itr)
	{
		if(itr->second->m_Type != GoodsGroup && itr->second->m_Type != DropBag)
			continue;

		DropVec.push_back(itr->second);
		nMaxVal += itr->second->m_Odds * 1000;
	}

	return true;
}
