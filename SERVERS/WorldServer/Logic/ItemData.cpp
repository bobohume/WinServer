#include "ItemData.h"
#include "Common/OrgBase.h"
#include "Common/mRandom.h"
#include "CommLib/CommLib.h"

CItemDataRes *g_ItemDataMgr=new CItemDataRes();
CComposeDataRes* g_ComposeDataMgr = new CComposeDataRes();
CDropDataRes*	g_DropDataMgr = new CDropDataRes();
CGiftDataRes*	g_GiftDataMgr = new CGiftDataRes();
CShopDataRes*   g_ShopDataMgr = new CShopDataRes();
CRandomShopRes*  g_RandomShopDataMgr = new CRandomShopRes();
CEquipStrengthenRes* g_EquipStrengthenDataMgr = new CEquipStrengthenRes();

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

	if(!file.readDataFile("data/Item.dat"))
	{
		JUST_ASSERT("read itemData error");
		return  false;
	}

	for(int i = 0; i < file.RecordNum; ++i)
	{
		CItemData* pdata = new CItemData();

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read itemdata ID error");
		pdata->m_ItemId = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_string, "read itemdata name error");
		pdata->m_sName = LineData.m_string;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_string, "read itemdata des error");
		pdata->m_sDes= LineData.m_string;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_string, "read itemdata getdes error");
		pdata->m_sGetDes = LineData.m_string;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read itemdata res error");
		pdata->m_ResID = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read itemdata color error");
		pdata->m_Color = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read itemdata type error");
		pdata->m_Type = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read itemdata equipFlag error");
		pdata->m_EquipFlag = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read itemdata useflag error");
		pdata->m_UseFlag = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read itemdata useval error");
		pdata->m_UseVal = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read itemdata skill error");
		pdata->m_SkillId = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read itemdata maxdie error");
		pdata->m_MaxDie = LineData.m_U16;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read itemdata sortval error");
		pdata->m_SortVal = LineData.m_U16;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read itemdata usetarget error");
		pdata->m_UseTaget = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read itemdata effect error");
		pdata->m_Effect = LineData.m_U8;

		addData(pdata->m_ItemId, pdata);
	}
	return true;
}

//------------合成--------------//
CComposeDataRes::CComposeDataRes()
{
}
CComposeDataRes::~CComposeDataRes()
{
	clear();
}

bool CComposeDataRes::read()
{
	CDataFile file;
	RData LineData;

	if (!file.readDataFile("data/Produce.dat"))
	{
		JUST_ASSERT("read ProduceData error");
		return  false;
	}

	for (int i = 0; i < file.RecordNum; ++i)
	{
		CComposeData* pdata = new CComposeData();

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read producedata ID error");
		pdata->m_ID = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read producedata composeid error");
		pdata->m_ComposeItemID = LineData.m_U32;

		for (int i = 0; i < MAX_COMPOSECOST_NUM; ++i)
		{
			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_U32, "read producedata costitemid error");
			pdata->m_CostItemID[i] = LineData.m_U32;

			file.GetData(LineData);
			DOASSERT(LineData.m_Type == DType_U16, "read itemdata costitemnum error");
			pdata->m_CostItemNum[i] = LineData.m_U16;
		}

		addData(pdata->m_ID, pdata);
	}
	return true;
}

//------------掉落--------------//
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
	RData LineData;

	if (!file.readDataFile("data/Chest.dat"))
	{
		JUST_ASSERT("read Chestdata error");
		return  false;
	}

	for (int i = 0; i < file.RecordNum; ++i)
	{
		CDropData* pdata = new CDropData();

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read Chestdata dropid error");
		pdata->m_DropId = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read Chestdata itemid error");
		pdata->m_ItemId = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read Chestdata itemnum error");
		pdata->m_ItemNum = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read Chestdata randval error");
		pdata->m_RandVal = LineData.m_U16;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_string, "read Chestdata randval error");
		pdata->m_Des = LineData.m_string;

		addData(pdata->m_DropId, pdata);
	}

	return true;
}

CDropData*	CDropDataRes::getData(S32 Id, S32 randVal)
{
	for (auto itr = m_DataMap.lower_bound(Id); itr != m_DataMap.upper_bound(Id); ++itr)
	{
		if (itr->second && randVal <= itr->second->m_RandVal)
			return itr->second;
	}
	return NULL;
}

S32	 CDropDataRes::getData(S32 Id)
{
	S32 iRandVal = 0;
	for (auto itr = m_DataMap.lower_bound(Id); itr != m_DataMap.upper_bound(Id); ++itr)
	{
		if (itr->second && iRandVal <= itr->second->m_RandVal)
			iRandVal = itr->second->m_RandVal;
	}
	return iRandVal;
}

std::vector<CDropData*> CDropDataRes::getDataEx(S32 Id)
{
	std::vector<CDropData*> DropVec;
	for (auto itr = m_DataMap.lower_bound(Id); itr != m_DataMap.upper_bound(Id); ++itr)
	{
		if (itr->second)
			DropVec.push_back(itr->second);
	}
	return DropVec;
}

//------------礼包--------------//
CGiftDataRes::CGiftDataRes()
{
}
CGiftDataRes::~CGiftDataRes()
{
	clear();
}

bool CGiftDataRes::read()
{
	CDataFile file;
	RData LineData;

	if (!file.readDataFile("data/GiftPack.dat"))
	{
		JUST_ASSERT("read Chestdata error");
		return  false;
	}

	for (int i = 0; i < file.RecordNum; ++i)
	{
		CGiftData* pdata = new CGiftData();

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read GiftPack giftid error");
		pdata->m_GitfId = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read GiftPack itemid error");
		pdata->m_ItemId = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read GiftPack quantity error");
		pdata->m_Quantity = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_string, "read GiftPack des error");
		pdata->m_Des = LineData.m_string;

		addData(pdata->m_GitfId, pdata);
	}

	return true;
}

std::vector<CGiftData*> CGiftDataRes::getData(S32 Id)
{
	std::vector<CGiftData*> GiftVec;
	for (auto itr = m_DataMap.lower_bound(Id); itr != m_DataMap.upper_bound(Id); ++itr)
	{
		if(itr->second)
			GiftVec.push_back(itr->second);
	}
	return GiftVec;
}

//------------商城--------------//
CShopDataRes::CShopDataRes()
{
}
CShopDataRes::~CShopDataRes()
{
	clear();
}

bool CShopDataRes::read()
{
	CDataFile file;
	RData LineData;

	if (!file.readDataFile("data/Shop.dat"))
	{
		JUST_ASSERT("read Shopdat error");
		return  false;
	}

	for (int i = 0; i < file.RecordNum; ++i)
	{
		CShopData* pdata = new CShopData();

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read Shopdat giftid error");
		pdata->m_ShopId = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read Shopdat itemid error");
		pdata->m_ItemId = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read Shopdat shoptype error");
		pdata->m_ShopType = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read Shopdat saletype error");
		pdata->m_SaleType = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read Shopdat sale error");
		pdata->m_Sale = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read Shopdat discounttype error");
		pdata->m_DiscountType = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read Shopdat discount error");
		pdata->m_Discount = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read Shopdat LmtScale error");
		pdata->m_LmtScale = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read Shopdat BuyTimes error");
		pdata->m_BuyTimes = LineData.m_U8;

		addData(pdata->m_ShopId, pdata);
	}

	return true;
}

//--------------普通商城--------------//
CRandomShopRes::CRandomShopRes() : m_MaxRand(0)
{
}

CRandomShopRes::~CRandomShopRes()
{
	clear();
}

bool CRandomShopRes::read()
{
	CDataFile file;
	RData LineData;

	if (!file.readDataFile("data/Exchange.dat"))
	{
		JUST_ASSERT("read Exchangedat error");
		return  false;
	}

	for (int i = 0; i < file.RecordNum; ++i)
	{
		CRandomShopData* pdata = new CRandomShopData();

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read Exchangedat id error");
		pdata->m_Id = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read Exchangedat itemid error");
		pdata->m_ItemId = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read Exchangedat itemnum error");
		pdata->m_ItemNum = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read Exchangedat scaletype error");
		pdata->m_SaleType = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read Exchangedat sale error");
		pdata->m_Sale = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U16, "read Exchangedat randVal error");
		pdata->m_RandVal = LineData.m_U16;

		m_MaxRand += pdata->m_RandVal;

		if (pdata->m_SaleType == 1)
		{
			m_RandVec.push_back(std::make_pair(pdata->m_Id, m_MaxRand));
		}
		
		addData(pdata->m_Id, pdata);
	}

	return true;
}

Vector<int> CRandomShopRes::GetRandomShop()
{
	Vector<int> AllShopVec;
	Vector<int> RandomShopVec;
	for (auto itr = m_DataMap.begin(); itr != m_DataMap.end(); ++itr) {
		AllShopVec.push_back(itr->first);
	}

	do 
	{
		S32 nRand = gRandGen.randI(0, AllShopVec.size() - 1);
		RandomShopVec.push_back(AllShopVec[nRand]);
		AllShopVec.erase(nRand);
	} while (RandomShopVec.size() < 9);
	
	return RandomShopVec;
}

Vector<int> CRandomShopRes::GetRandomShopEx() {
	Vector<int> RandomShopVec;

	auto nRandVal = 0;
	do{
		nRandVal = gRandGen.randI(0, m_MaxRand);
		for (auto itr = m_RandVec.begin(); itr != m_RandVec.end(); ++itr) {
			if (nRandVal <= (*itr).second) {
				RandomShopVec.push_back((*itr).first);
				break;
			}
		}
	} while (RandomShopVec.size() < 9);

	return RandomShopVec;
}

//------------强化--------------//
CEquipStrengthenRes::CEquipStrengthenRes()
{
}
CEquipStrengthenRes::~CEquipStrengthenRes()
{
	clear();
}

bool CEquipStrengthenRes::read()
{
	CDataFile file;
	RData LineData;

	if (!file.readDataFile("data/Upgrade.dat"))
	{
		JUST_ASSERT("read Upgradedat error");
		return  false;
	}

	for (int i = 0; i < file.RecordNum; ++i)
	{
		CEquipStrengthenData* pdata = new CEquipStrengthenData();

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read Upgradedat m_ItemId error");
		pdata->m_ItemId = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read Upgradedat m_Level error");
		pdata->m_Level = LineData.m_U8;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read Upgradedat m_LevelExp error");
		pdata->m_LevelExp = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read Upgradedat m_TotalExp error");
		pdata->m_TotalExp = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read Upgradedat m_Exp error");
		pdata->m_Exp = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U32, "read Upgradedat m_SkillSeries error");
		pdata->m_SkillSeries = LineData.m_U32;

		file.GetData(LineData);
		DOASSERT(LineData.m_Type == DType_U8, "read Upgradedat m_SkillLv error");
		pdata->m_SkillLv = LineData.m_U8;

		addData(pdata->m_ItemId, pdata);
	}

	return true;
}

CEquipStrengthenData*	CEquipStrengthenRes::getData(S32 Id, S32 Level)
{
	for (auto itr = m_DataMap.lower_bound(Id); itr != m_DataMap.upper_bound(Id); ++itr)
	{
		if (itr->second && Level == itr->second->m_Level)
			return itr->second;
	}
	return NULL;
}
