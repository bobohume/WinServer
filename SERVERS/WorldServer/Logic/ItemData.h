#ifndef __HelloLua__ItemData__
#define __HelloLua__ItemData__
#include "Common/DataFile.h"
#include "BaseData.h"
#include <string.h>
#include <hash_map>
#include <vector>
#include "BASE/tVector.h"

const unsigned int MAX_COMPOSECOST_NUM = 2;
enum enEquipColor
{
	EQUIP_COLOR_WHITE		= 1,//白色
	EQUIP_COLOR_BLUE,//蓝色
	EQUIP_COLOR_PURPLE,//紫色
	EQUIP_COLOR_ORANGE,//橙色
};

struct CItemData;

class CItemDataRes : public CBaseDataRes<CItemData>
{
public:
	CItemDataRes();
	~CItemDataRes();

	bool read();
};

struct CItemData 
{
	CItemData():m_ItemId(0),m_sName(""),m_sDes(""),m_ResID(0),m_Color(EQUIP_COLOR_WHITE),
		m_Type(0),m_UseFlag(0), m_UseVal(0),m_SkillId(0),m_MaxDie(0),m_SortVal(0), m_EquipFlag(0),
		m_UseTaget(0), m_Effect(0), m_sGetDes("")
	{};
	~CItemData(){};

	inline bool isEquipMent()	{ return (m_Type == 11 || m_Type == 12 || m_Type == 13); }
	inline bool isEquipStrengthenMaterial() { return m_Type == 18; }
	inline bool isAddGold()		{ return m_Type == 1; }
	inline bool isAddMoney()	{ return m_Type == 2; }
	inline bool isAddOre()		{ return m_Type == 3; }
	inline bool isAddTroops()	{ return m_Type == 4; }
	inline bool isAddAchieve()	{ return m_Type == 5; }
	inline bool isAddVim()		{ return m_Type == 9; }
	inline bool isAddStrength() { return m_Type == 10; }
	inline bool isAddImpose()	{ return m_Type == 15; }
	inline bool isDrop()		{ return m_Type == 16; }
	inline bool isGift()		{ return m_Type == 17; }
	inline bool CanUse()		{ return m_UseFlag == 0; }
	inline bool IsAccCard() {
		return 600106 == m_ItemId || 600107 == m_ItemId || 600108 == m_ItemId || 600109 == m_ItemId;
	}

	inline bool IsAddPVPTimesCard() {
		return 600069 == m_ItemId;
	}

	U32 m_ItemId;
	std::string m_sName;
	std::string m_sDes;//描述
	std::string m_sGetDes;
	U32 m_ResID;
	U8	m_Color;
	U8	m_Type; //类型
	U8	m_EquipFlag;//宝物类型
	U8  m_UseFlag;//
	U32	m_UseVal;//
	U32 m_SkillId;//
	U16 m_MaxDie;//
	U16	m_SortVal;
	U8  m_UseTaget;
	U8  m_Effect;
};

//------------合成--------------//
struct CComposeData
{
	CComposeData() : m_ID(0), m_ComposeItemID(0)
	{
		memset(m_CostItemID, 0, sizeof(m_CostItemID));
		memset(m_CostItemNum, 0, sizeof(m_CostItemNum));
	}
	~CComposeData() {}
	U8 	m_ID;
	U32	m_ComposeItemID;
	U32 m_CostItemID[MAX_COMPOSECOST_NUM];
	S32 m_CostItemNum[MAX_COMPOSECOST_NUM];
};

class CComposeDataRes : public CBaseDataRes<CComposeData>
{
public:
	CComposeDataRes();
	~CComposeDataRes();

	bool read();
};

struct CDropData
{
	S32	m_DropId;
	S32 m_ItemId;
	S32 m_ItemNum;
	S32 m_RandVal;
	std::string m_Des;
	CDropData() : m_DropId(0), m_ItemId(0), m_RandVal(0), m_Des(""), m_ItemNum(0)
	{}
};

class CDropDataRes : public CBaseDataResEx<CDropData>
{
public:
	CDropDataRes();
	~CDropDataRes();

	bool read();
	CDropData*	getData(S32 Id, S32 randVal);
	S32			getData(S32 Id);
	std::vector<CDropData*> getDataEx(S32 Id);
};

struct CGiftData
{
	S32 m_GitfId;
	S32 m_ItemId;
	S32 m_Quantity;
	std::string m_Des;
	CGiftData() : m_GitfId(0), m_ItemId(0), m_Quantity(0), m_Des("")
	{}
};

class CGiftDataRes : public CBaseDataResEx<CGiftData>
{
public:
	CGiftDataRes();
	~CGiftDataRes();

	bool read();

	std::vector<CGiftData*> getData(S32 Id);
};

struct CShopData
{
	S32 m_ShopId;
	S32 m_ItemId;
	S8	m_ShopType;
	S8  m_SaleType;
	S32 m_Sale;
	S8  m_DiscountType;
	S8  m_Discount;
	S8  m_LmtScale;
	S32 m_BuyTimes;

	CShopData() : m_ShopId(0), m_ItemId(0), m_ShopType(0), m_SaleType(0), m_Sale(0), \
		m_DiscountType(0), m_Discount(0), m_LmtScale(0), m_BuyTimes(0)
	{}
};

class CShopDataRes : public CBaseDataRes<CShopData>
{
public:
	CShopDataRes();
	~CShopDataRes();

	bool read();
};

struct CRandomShopData
{
	S32 m_Id;
	S32 m_ItemId;
	S32	m_ItemNum;
	S8  m_SaleType;
	S32 m_Sale;
	S32 m_RandVal;

	CRandomShopData() : m_Id(0), m_ItemId(0), m_ItemNum(0), m_SaleType(0), m_Sale(0), m_RandVal(0)
	{}
};

class CRandomShopRes : public CBaseDataRes<CRandomShopData>
{
public:
	CRandomShopRes();
	~CRandomShopRes();

	bool read();
	Vector<int> GetRandomShop();//不重复的商品
	Vector<int> GetRandomShopEx();//重复的商品，随机几率
	std::vector<std::pair<int, int>> m_RandVec;
	S32 m_MaxRand;
};

struct CEquipStrengthenData
{
	S32 m_ItemId;
	S32	m_Level;
	S32 m_LevelExp;
	S32 m_TotalExp;
	S32 m_Exp;
	S32 m_SkillSeries;
	S32 m_SkillLv;

	CEquipStrengthenData() : m_ItemId(0), m_Level(0), m_LevelExp(0), m_TotalExp(0), m_Exp(0), m_SkillSeries(0),
		m_SkillLv(0)
	{}
};

class CEquipStrengthenRes : public CBaseDataResEx<CEquipStrengthenData>
{
public:
	CEquipStrengthenRes();
	~CEquipStrengthenRes();

	bool read();

	CEquipStrengthenData* getData(S32 Id, S32 Level);
};

extern CItemDataRes*	g_ItemDataMgr;
extern CComposeDataRes* g_ComposeDataMgr;
extern CDropDataRes*	g_DropDataMgr;
extern CGiftDataRes*	g_GiftDataMgr;
extern CShopDataRes*	g_ShopDataMgr;
extern CRandomShopRes*	g_RandomShopDataMgr;
extern CEquipStrengthenRes* g_EquipStrengthenDataMgr;
#endif