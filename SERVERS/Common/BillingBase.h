#ifndef __BILLINGBASE_H__
#define __BILLINGBASE_H__

#include "Base/BitStream.h"
#include "Common/PacketType.h"
#include <vector>

//商城类型
enum enStoreType
{
	STORE_IMPORTALSTONE			= 1,	//元宝商城
	STORE_DEITYSTONE			= 2,	//绑定元宝商城
};

const struct {U32 id,start,end;} sgGoldBonus[] = {
	{105103049, 1326297600, 1556969599},
};

#define PRICEID  U64
#define MAINTYPE_NEWGOODS		1		//推荐主类固定为第1类
#define SUBTYPE_INTIME			101		//推荐主类固定即时促销子类为101
#define SUBTYPE_NEWGOODS		102		//推荐主类固定新品子类为102

#define MAINTYPE_HOTITEM		7		//推荐栏固定为第7类
#define SUBTYPE_INVENTORY		701		//推荐栏主类固定包裹栏子类为701
#define SUBTYPE_ANSWERHOT		706		//推荐栏主类固定答题栏子类为706

#define MAYDAY_START			615000000	//劳动节起始时间
#define MAYDAY_END				731000000	//劳动节结束时间

enum enPROMBonus
{
	PROM40_3=105101195,		//40级3个的推广员奖励
	PROM50_3=105101196,		//50级3个的推广员奖励
	PROM60_3=105101197,		//60级3个的推广员奖励
	PROM50_4=105101198,		//50级4个的推广员奖励
	PROM60_4=105100442,		//60级4个的推广员奖励
	PROM70_4=105100576,		//70级4个的推广员奖励
	PROM70_5=105100580,		//70级5个的推广员奖励
	PROM75_4=105101199,		//75级4个的推广员奖励
	PROM75_5=105100443,		//75级5个的推广员奖励
	PROM80_4=105100581,		//80级4个的推广员奖励
	PROM80_5=105100524,		//80级5个的推广员奖励
};

#pragma pack(push, 1)

//商城商品结构
struct stGoods
{
	static const U32 MAX_ITEMS = 20;
	U8	StoreType;			//商城类型
	U8  GoodsMainType;		//商品大类
	U16	GoodsSubType;		//商品小类
	U8	ItemCount;			//道具个数
	U8	ItemNum[MAX_ITEMS];	//道具数量
	U32 GoodsID;			//商品编号
	U32 ItemID[MAX_ITEMS];	//道具ID
	S32 Price;				//商品价格
	U32 OnSale;				//上架时间(例如：2011年1月1日12时30分,为1101011230)
	U32 OffSale;			//下架时间

	stGoods() { Clear();}
	void Clear() { memset(this, 0, sizeof(stGoods));}
	int Count() { return ItemCount;}
};

// ----------------------------------------------------------------------------
// 商城上架商品结构
struct stGoodsPrice
{
	static const U32 MAX_ITEMS = 20;
	PRICEID	PriceID;		//上架商品编号
	U32 GoodsID;			//商品编号
	char GoodsName[32];		//商品名称
	U8	StoreType;			//商城类型
	U8  GoodsMainType;		//商品大类
	U16	GoodsSubType;		//商品小类
	U32 ShowItemID;			//用于展示的道具ID
	U8	PriceType;			//商品货币类型
	S32 Price;				//实际购买单价
	U8	ItemCount;			//道具个数
	U32 ItemID[MAX_ITEMS];	//道具ID
	U8	ItemNum[MAX_ITEMS];	//道具数量
	U32 DiscountID;			//折扣配置编号
	U8	DiscountRate;		//折扣值
	U32 OnSale;				//上架时间
	U32 OffSale;			//下架时间
	U32 StartTime;			//折扣开始时间
	U32 EndTime;			//折扣结束时间
	stGoodsPrice() { Clear();}
	void Clear() { memset(this, 0, sizeof(stGoodsPrice));}
	int Count() { return ItemCount;}
};

//商城折扣配置结构
struct stDiscount
{
	U32 DiscountID;			//折扣配置编号
	U32 GoodsID;			//商品编号
	U8  DiscountRate;		//折扣值(0无折扣 1-9代表1折-9折)
	U32 StartTime;			//折扣开始时间
	U32 EndTime;			//折扣结束时间
};

//商城推荐配置结构
struct stRecommend
{
	static const U32 MAX_GOODS = 5;
	U32 RecommendID;		//推荐配置编号(小于1000的是特定推荐编号)
	PRICEID PriceID[MAX_GOODS];	//推荐商品编号列表
	stRecommend() { Clear();}
	void Clear() { memset(this, 0, sizeof(stRecommend));}
};

//商城交易记录数据结构
struct stTradeGoods
{
	U32	TradeID;			//交易流水号
	U32	AccountID;			//帐号ID
	U32	PlayerID;			//角色ID
	PRICEID	PriceID;		//上架商品编号
	U32	PriceNum;			//购买商品数量
	S8	PriceType;			//货币(0-元宝 1-绑定元宝)
	U32	Price;				//单价
	U32	AccountGold;		//帐号剩余元宝数
	U32	AreaID;				//所在消费服
	U32	TradeDate;			//交易发生时间
	stTradeGoods() { Clear();}
	void Clear() { memset(this, 0, sizeof(stTradeGoods));}
};

// 商城广播消息数据结构 
struct stMallBoard
{
	char Content[250];		//广播消息字符串内容
	int  MsgId;				//广播消息流水ID
	int	 UpdValue;			//更新标志，以递增数值来确认变化
	stMallBoard():MsgId(0),UpdValue(0)
	{
		Content[0] = '\0';
	}
};

struct stPROMItemBonus
{
	static const unsigned int MAX_LEVEL = 6;
	static const unsigned int MAX_ITEM = 11;
	S8	DrawFlag[MAX_ITEM];		//物品奖励是否领取(0-不可领取 1-可领取 2-已领取)
	int	Levels[MAX_LEVEL];		//达到等级的好友人数（80级，75级,70级，60级，50级，40级)
	stPROMItemBonus() {	memset(this, 0, sizeof(stPROMItemBonus));}
};

#pragma pack(pop)

typedef std::vector<stGoodsPrice*>	GOODSVEC;
typedef std::vector<stRecommend*>	RECOMMENDVEC;
typedef std::vector<stMallBoard*>	BOARDVEC;

#endif//__BILLINGBASE_H__