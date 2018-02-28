#ifndef __HelloLua__ItemData__
#define __HelloLua__ItemData__
#include "Common/DataFile.h"
#include <string.h>
#include <hash_map>

enum enItemType {
	Item_Knife= 1,
	Item_Sword,
	Item_Gun,
	Item_Axe,
	Item_Hammer,
	Item_Firearm,
	Item_Shield,
	Item_Qi,
	Item_Food,
	Item_Charm,//符咒
	Item_Medicine,//丹药
	Item_CuCur,//葫芦
	Item_Scroll,//卷轴
	Item_Other,
	Item_Max
};

const static char* s_ItemTypeName[Item_Max-1]=
{
	"刀系",
	"剑系",
	"枪系",
	"斧系",
	"锤系",
	"火器",
	"盾",
	"器",
	"食物",
	"符咒",
	"丹药",
	"葫芦",
	"卷轴",
	"其他"
};

enum enEquipColor
{
	EQUIP_COLOR_WHITE		= 0,//白色
	EQUIP_COLOR_GREEN,//绿色
	EQUIP_COLOR_BLUE,//蓝色
	EQUIP_COLOR_PURPLE,//紫色
	EQUIP_COLOR_ORANGE,//橙色
	EQUIP_COLOR_RED,//红色
};

class CItemData;

class CItemDataRes {

public:
	enum{
		ITEM_BEGIN_ID=1000001,
		ITEM_END_ID  =1999999,
	};
	CItemDataRes();
	~CItemDataRes();

	typedef stdext::hash_map<S32,CItemData*> ITEMDATAMAP;
	typedef ITEMDATAMAP::iterator ITEMDATA_ITR;

	bool read();
	void close();
	void clear();
	CItemData* getItemData(S32 ItemId);

private:
	ITEMDATAMAP m_DataMap;
};

class CItemData {

public:
	CItemData():m_ItemId(0),m_sName(""),m_sIcon(""),m_sType(0),m_sDes(""),m_Limit(0),m_Mpro(0),
		m_AttWidth(0),m_AttLength(0),m_Sale(0),m_Skill(0),m_Timelimit(0),m_UseTimes(0),
		m_MaxDie(0),m_CD(0),m_BuffID1(0),m_Trap(0),m_Effic(0), m_BuffID2(0), m_GemHole(0), m_Buy(0)
	{};
	~CItemData(){};

	enum enLimitType
	{
		LIMIT_DIE_NO_LOST				= BIT(0),//	死亡不掉落
		LIMIT_NO_SALE					= BIT(1),//	不能出售
		LIMIT_NO_SAVE					= BIT(2),//	不能保存
		LIMIT_STRENGTHEN_FLAG			= BIT(3),//	强化标志
		LIMIT_USE_FLAG					= BIT(4),//	适用标志
		LIMIT_THROW_LOSE				= BIT(5),//	投掷不消失
	};

	enum
	{
		ITEM_WEAPON_LEVEL_BASE_BUFF         =   4800001,
		ITEM_DEFENSE_LEVEL_BASE_BUFF        =   4801001,
		ITEM_WEAPON_STRENGTHEN_BASE_BUFF    =   4810001,
		ITEM_DEFENSE_STRENGTHEN_BASE_BUFF   =   4811001,
	};

	inline bool isWeapon()
	{
		if(m_sType == Item_Knife || m_sType == Item_Sword || m_sType == Item_Gun || m_sType == Item_Axe || m_sType == Item_Hammer || m_sType == Item_Firearm)
			return true;
		else
			return false;
	}

	inline bool isDefense()
	{
		return (m_sType == Item_Shield);
	}
	
	inline bool isEquipMent()
	{
		return (isWeapon() || isDefense());
	}

	inline bool isSecondaryEquip()
	{
		return (m_sType == Item_Qi);
	}

	S32 m_ItemId;
	std::string m_sName;
	std::string m_sIcon; //图片
	U8 m_sType; //类型
	std::string m_sDes;//描述
	S32 m_Limit;//
	S32 m_Mpro;//
	S32 m_AttWidth;//
	S32 m_AttLength;//
	S32 m_Buy;
	S32 m_Sale;//
	S32 m_Skill;//
	S32 m_Timelimit;//
	S32 m_UseTimes;//
	S32 m_MaxDie;//
	S32 m_CD;//
	S32 m_BuffID1;//
	S32 m_BuffID2;
	S32 m_Trap;//
	S32 m_Effic;//调用特效id
	S32 m_GemHole;//宝石孔数目
};
extern CItemDataRes* g_ItemDataMgr;
#endif