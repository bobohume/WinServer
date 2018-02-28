#ifndef __FIXEDSTALLBASE_H__
#define __FIXEDSTALLBASE_H__

#include "Base/BitStream.h"
#include "Common/PacketType.h"
#include "common/PlayerStruct.h"
#include "CommLib/CommLib.h"
  
#define BUYSYSSTALLPRISE(a)    600000 + 24 * a                //购买系统商铺价格[注：公式＝300000 * (1 + m_TotalFlourish / 25000) * 2]

enum enStallStatus
{
	SS_MODIFY	= 0,					//商铺修改状态
	SS_OPEN		= 1,					//商铺开放状态
};

enum enStallShopType
{
	STALLTYPE_NONE		 = 0,			//无
	STALLTYPE_CHANDLERY	 = 1,			//杂货店
	STALLTYPE_PET		 = 2,			//灵兽店
	STALLTYPE_GEM		 = 3,			//宝石店
	STALLTYPE_EQUIP		 = 4,			//装备店
	STALLTYPE_MATERAIL	 = 5,			//材料店
};

enum enStallConstant
{
	BASEFLOURISH		= 10,			//商铺基础繁荣度
	INITSTALLS			= 50,			//初始商铺数
	INITTOTALFLOURISH	= 500,			//初始总繁荣度（初始商业指数）
	MAXTOTALFLOURISH	= 50000,		//最大总繁荣度（最大商业指数）
	TRADETAX			= 11,			//初始交易税为0.11
	DAYTAX				= 10200,		//初始每天税收
	ATTORNPRICE			= 600000,		//初始转让价格
	BUYSTALLIMPREST		= 1050000,		//购买商铺额外预付金
	INITSELLITEMS		= 24,			//初始出售物品栏个数
	INITSELLPETS		= 6,			//初始出售灵兽个数
	SHOUTINTERVAL0		= 60,			//喊话发送间隔(1分钟)
	SHOUTINTERVAL1		= 300,			//喊话发送间隔(5分钟)
	SHOUTINTERVAL2		= 600,			//喊话发送间隔(10分钟)
	BUYSTALLITEM		= 105090012,	//购买商铺固定道具行商秘籍
	SHOUTITEM0			= 105102503,	//喊话道具ID
	SHOUTITEM1			= 105102506,	//喊话道具ID
	MAXLEAVEWORDS		= 50,			//玩家留言最多保存50条
	MAXTRADERECORDS		= 50,			//玩家交易信息最多保存200条
	MAX_MONEY_NUM		= 1000000000,	//金钱最大上限
};

enum enStallOp
{
	STO_NONE,							//全部更新
	STO_NAME,							//修改摊位名称
	STO_TYPE,							//修改摊位类型
	STO_LEAVEWORDS,						//修改留言
	STO_SHOUT,							//修改广告喊话
	STO_FLOWMONEY,						//修改流动资金
	STO_ATTORNPRICE,					//修改转让价格
	STO_BEATTORN,						//修改是否转让
};

#pragma pack(push,1)

struct stFixedStall
{	
	U16		StallID;					//商铺ID
	char	StallName[MAX_NAME_LENGTH];	//商铺名字
	U32		StallerID;					//商铺铺主ID
	char	StallerName[MAX_NAME_LENGTH];//商铺铺主名称
	U32		PreStallerID;				//上位铺主ID
	U8		StallType;					//商铺类型
	U8		StallStatus;				//商铺状态
	S32		TradeTax;					//商铺交易税
	S32		StallTax;					//商铺固定税
	U32		PayTaxTime;					//上次交税时间
	bool	IsAttorn;					//是否转让
	S32		AttornPrice;				//转让价格
	U32		AttornStartTime;			//转让时间
	S32		FixedMoney;					//固定资金
	S32		FlowMoney;					//流动资金
	U8		Flourish;					//繁荣度
	char	Notice[128];				//商铺告示
	bool	IsShoutAll;					//是否全服喊话
	char	ShoutContent[128];			//喊话内容(广告语)
	U16		ShoutInterval;				//喊话间隔时间(秒)
	U32		ShoutItemID;				//喊话消耗道具编号
	U16		ShoutItemNum;				//喊话消耗道具数量
	U32		ShoutLastTime;				//上次喊话时间
	U16		BuySellItems;				//购买出售物品栏数
	U8		BuySellPets;				//购买出售灵兽栏数
	U32		BuyTime;					//购买商铺时间
	U64		DataVerion;					//数据版本号
	//初始化方法(系统回收商铺也调用此方法)
	void Init(U16 id)
	{
		StallID = id;
		sprintf_s(StallName, MAX_NAME_LENGTH, "%d号商铺", id+1);
		StallerID = PreStallerID = 0;
		strcpy_s(StallerName, MAX_NAME_LENGTH, "系统商铺");
		StallStatus = SS_MODIFY;
		TradeTax = TRADETAX;
		StallType = STALLTYPE_NONE;
		StallTax = DAYTAX;

		U32 curtime;
		_time32((__time32_t*)&curtime);

		PayTaxTime = getZeroTime(curtime);
		IsAttorn = 0;
		AttornPrice = ATTORNPRICE;
		AttornStartTime = 0;
		FixedMoney = FlowMoney = 0;
		Flourish = BASEFLOURISH;
		Notice[0] = 0;
		IsShoutAll = 0;
		ShoutContent[0] = 0;
		ShoutInterval = SHOUTINTERVAL0;
		ShoutItemID = 0;
		ShoutItemNum = 0;
		ShoutLastTime = 0;
		BuySellItems = INITSELLITEMS;
		BuySellPets = INITSELLPETS;
		BuyTime = 0;
		DataVerion = ::GetTickCount64();
	}
};

struct stFixedStallItem : stItemInfo
{
	U16 StallID;
	U16 Pos;
	S32 Price;
	S16 BuyNum;		//仅用于收购栏设定初始收购数量
	S16 LeftNum;	//仅用于收购栏当前剩余数量
	U64 DataVersion;//数据版本号
	void InitData(U16 stallid, U16 pos)
	{
		stItemInfo::InitData();
		StallID = stallid;
		Pos = pos;
		Price = 0;
		BuyNum = 0;
		LeftNum = 0;
		DataVersion = ::GetTickCount64();
	}
};

struct stFixedStallPet : stPetInfo
{
	U16 StallID;
	U16 Pos;
	S32 Price;
	U64 DataVersion;
	void InitData(U16 stallid, U16 pos)
	{
		stPetInfo::InitData();
		StallID = stallid;
		Pos = pos;
		Price = 0;
		DataVersion = ::GetTickCount64();
	}
};

struct stStallVersion
{
	U32 SellItemVer[MAX_FIXEDSTALL_SELLITEM];
	U32 BuyItemVer[MAX_FIXEDSTALL_BUYITEM];
	U32 SellPetVer[MAX_FIXEDSTALL_SELLPET];
};

//商铺列表数据项结构
struct stStallListEntry
{
	U8		StallID;					//摊位ID
	U8		Flourish;					//繁荣度
	U8		StallType;					//摊位类型
	U8		StallStatus;				//摊位状态
	bool	IsAttorn;					//是否转让
	char	StallName[MAX_NAME_LENGTH];	//摊位名字
	char	Notice[128];				//摊主留言
	char	StallerName[MAX_NAME_LENGTH];//摊主名称
	U32		StallerID;					//摊主ID
	S32		AttornPrice;				//转让价格
	U32		BuyTime;					//购买摊位时间
};

//快速留言结构
struct stStallChat
{
	U32 time;							//当前时间
	char sender[MAX_NAME_LENGTH];		//发送者
	char content[128];					//留言内容
};

//快速商铺交易记录结构
struct stStallRecord
{
	enum
	{
		TRADE_SELLITEM,					//购买物品
		TRADE_SELLPET,					//购买灵兽
		TRADE_BUYITEM,					//出售物品
	};
	U8 type;							// 交易方式
	U32 time;							// 记录发生时间
	S32 money;							// 发生金钱
	S32 tax;							// 发生多少税金
	char targetName[MAX_NAME_LENGTH];	// 对方玩家名称
	union
	{
		stItemInfo* item;				//　物品数据
		stPetInfo* pet;					//  灵兽数据
	} trade;
	
	stStallRecord() { memset(this, 0, sizeof(stStallRecord));}
	~stStallRecord()
	{
		if((type == 0 || type == 2) && trade.item) delete trade.item; 
		else if(type == 1 && trade.pet) delete trade.pet;
	}
};

#pragma pack(pop)

#endif//__FIXEDSTALLBASE_H__