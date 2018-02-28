//
//  ItemData.h
//  HelloLua
//
//  Created by th on 14-2-10.
//
//

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

class CItemData;
class CItemDataRes {

public:
	enum{
		ITEM_BEGIN_ID=1000001,
		ITEM_END_ID=1405008,
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
		m_MaxDie(0),m_CD(0),m_BuffID1(0),m_Trap(0),m_Effic(0), m_BuffID2(0), m_GemHole(0)
	{};
	~CItemData(){};
	S32 m_ItemId;
	std::string m_sName;
	std::string m_sIcon; //图片
	U8 m_sType; //类型
	std::string m_sDes;//描述
	S32 m_Limit;//
	S32 m_Mpro;//
	S32 m_AttWidth;//
	S32 m_AttLength;//
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
#endif /* defined(__HelloLua__ItemData__) */