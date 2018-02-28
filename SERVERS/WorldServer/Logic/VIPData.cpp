
#include "VIPData.h"
#include "Common/OrgBase.h"

CVIPDataRes * g_VIPDataMgr = new CVIPDataRes();

CVIPDataRes::~CVIPDataRes()
{
	clear();
}

bool CVIPDataRes::read()
{
	CDataFile file;
	RData linedata;

	if (!file.readDataFile("data/VIP.dat")) 
	{
		JUST_ASSERT("read vip data failed.");
		return false;
	}

	for (int i = 0; i < file.RecordNum; ++i)
	{
		VIPData *pVipData = new VIPData();

		file.GetData(linedata);
		DOASSERT(linedata.m_Type == DType_U8, "read vip level error");
		pVipData->m_VipLevel = linedata.m_U8;

		file.GetData(linedata);
		DOASSERT(linedata.m_Type == DType_U32, "read vip drawgold error");
		pVipData->m_DrawGold = linedata.m_U32;

		file.GetData(linedata);
		DOASSERT(linedata.m_Type == DType_U8, "read vip impose error");
		pVipData->m_ImposeTopLimit = linedata.m_U8;

		file.GetData(linedata);
		DOASSERT(linedata.m_Type == DType_U8, "read vip vim error");
		pVipData->m_VIMTopLimit = linedata.m_U8;

		file.GetData(linedata);
		DOASSERT(linedata.m_Type == DType_U8, "read vip strength error");
		pVipData->m_StrengthTopLimit = linedata.m_U8;

		file.GetData(linedata);
		DOASSERT(linedata.m_Type == DType_U8, "read vip activity toplimit error");
		pVipData->m_ActivityTopLimit = linedata.m_U8;

		file.GetData(linedata);
		DOASSERT(linedata.m_Type == DType_U8, "read vip impose error");
		pVipData->m_ImposePlus = linedata.m_U8;

		file.GetData(linedata);
		DOASSERT(linedata.m_Type == DType_U8, "read vip princenumber error");
		pVipData->m_PrinceNumberTopLimit = linedata.m_U8;

		file.GetData(linedata);
		DOASSERT(linedata.m_Type == DType_U8, "read vip apply error");
		pVipData->m_ApplyTopLimit = linedata.m_U8;

		file.GetData(linedata);
		DOASSERT(linedata.m_Type == DType_F32, "read vip vsa reduce percent error");
		pVipData->m_VSAReducePercent = linedata.m_F32;

		file.GetData(linedata);
		DOASSERT(linedata.m_Type == DType_U8, "read vip can jump battle error");
		pVipData->m_CanJumpBattleAnim = linedata.m_U8;

		file.GetData(linedata);
		DOASSERT(linedata.m_Type == DType_U32, "read vip gift id error");
		pVipData->m_VIPGiftId = linedata.m_U32;

		addData(pVipData->m_VipLevel, pVipData);
	}

	return true;
}

