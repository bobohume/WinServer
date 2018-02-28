//
//  ItemData.cpp
//  HelloLua
//
//  Created by th on 14-2-10.
//
//

#include "ItemData.h"
#include "Common/OrgBase.h"
#include "CommLib/CommLib.h"

CItemDataRes *g_ItemDataMgr=new CItemDataRes();

CItemDataRes::CItemDataRes()
{

}
CItemDataRes::~CItemDataRes()
{
	clear();
}

bool CItemDataRes::read()
{
	CDataFile file;
	RData LineData;

	if(!file.readDataFile("Data_1_Item.dat"))
	{
		JUST_ASSERT("read itemData error");
		return  false;
	}


	for(int i = 0; i < file.RecordNum; ++i)
	{
		CItemData* pdata = new CItemData();

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_S32, "read itemdata ID error");
		pdata->m_ItemId = LineData.m_S32;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_string, "read itemdata name error");
		pdata->m_sName = LineData.m_string;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_string, "read itemdata icon error");
		pdata->m_sIcon= LineData.m_string;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_enum16, "read itemdata type error");
		pdata->m_sType = LineData.m_Enum16;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_string, "read itemdata des error");
		pdata->m_sDes = LineData.m_string;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_S32, "read itemdata limit error");
		pdata->m_Limit = LineData.m_S32;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_S32, "read itemdata Mpro error");
		pdata->m_Mpro = LineData.m_S32;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_S32, "read itemdata attwidth error");
		pdata->m_AttWidth = LineData.m_S32;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_S32, "read itemdata attlength error");
		pdata->m_AttLength = LineData.m_S32;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_S32, "read itemdata sale error");
		pdata->m_Sale = LineData.m_S32;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_S32, "read itemdata skill error");
		pdata->m_Skill = LineData.m_S32;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_S32, "read itemdata timelimit error");
		pdata->m_Timelimit = LineData.m_S32;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_S32, "read itemdata usetimes error");
		pdata->m_UseTimes = LineData.m_S32;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_S32, "read itemdata Maxdie error");
		pdata->m_MaxDie = LineData.m_S32;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_S32, "read itemdata cD error");
		pdata->m_CD = LineData.m_S32;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_S32, "read itemdata buffID1 error");
		pdata->m_BuffID1 = LineData.m_S32;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_S32, "read itemdata buffID2 error");
		pdata->m_BuffID2 = LineData.m_S32;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_S32, "read itemdata trap error");
		pdata->m_Trap = LineData.m_S32;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_S32, "read itemdata effic error");
		pdata->m_Effic = LineData.m_S32;

		file.GetData(LineData);
		ASSERT(LineData.m_Type == DType_S32, "read itemdata ±¦Ê¯¿×ÊýÁ¿ error");
		pdata->m_GemHole = LineData.m_S32;

		m_DataMap.insert(ITEMDATAMAP::value_type(pdata->m_ItemId,pdata));
	}
	return true;
}

CItemData* CItemDataRes::getItemData(S32 ItemId)
{
	if(ItemId<ITEM_BEGIN_ID || ItemId>ITEM_END_ID)
		return NULL;
	ITEMDATA_ITR itr=m_DataMap.find(ItemId);

	if (itr!=m_DataMap.end()) {
		if (itr->second)
			return itr->second;
		else
			return NULL;

	}
	return NULL;
}

void CItemDataRes::clear()
{
	for (ITEMDATA_ITR itr=m_DataMap.begin(); itr!=m_DataMap.end(); ++itr) {
		SAFE_DELETE(itr->second);
	}
	m_DataMap.clear();
}

void CItemDataRes::close()
{
	clear();
}