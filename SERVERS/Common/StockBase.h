#ifndef STOCK_BASE_H
#define STOCK_BASE_H

#include "Base/BitStream.h"
#include "Common/PacketType.h"
#include <xfunctional>
#include <map>


//元宝挂单类型
enum enOrderType
{
	OT_BUY,						//买入元宝挂单
	OT_SELL,					//卖出元宝挂单
};

//元宝交易挂单状态
enum enOrderStatus
{
	OS_NONE,					//挂单正常进行中
	OS_FINISH,					//挂单成交完成
	OS_CLOSE,					//挂单已经被收盘操作取消
	OS_CANCEL,					//挂单已经被玩家撤单
	OS_MAX,
};

static char* szOrderStatus[OS_MAX] = 
{
	"等待交易",
	"完成交易",
	"收盘撤消",
	"玩家撤消",
};

//元宝操作类型
enum enGoldOp
{
	GO_DRAWSAVE,				//领取存入
	GO_BUYGOODS,				//商品消费
	GO_DRAWREWARD,				//分期返利奖励元宝
	GO_BUYGOLDSAVE = 10,		//元宝交易买入交易存入
	GO_SELLGOLDSAVE,			//元宝交易卖出扣除
	GO_CANCELBUYORDER,			//撤消买入元宝挂单返还金钱
	GO_CANCELSELLORDER,			//撤消卖出元宝挂单返还元宝
	GO_CLOSEORDER,				//收盘返还元宝
	GO_RELIFE,					//扣元宝死亡复活
	GO_EXCHANGE_CR,				//充值福利兑换
	GO_EXCHANGE_DR,				//领取元宝兑换
	GO_EXCHANGE_WR,				//每周福利兑换
	GO_EXCHANGE_MR,				//每月福利兑换
	GO_EXCHANGE_VR0,			//初级累积兑换
	GO_EXCHANGE_VR1,			//中级累积兑换
	GO_EXCHANGE_VR2,			//高级累积兑换
	GO_EXCHANGE_VR3,			//白银级累积兑换
	GO_EXCHANGE_VR4,			//黄金级累积兑换
	GO_EXCHANGE_VR5,			//钻石级累积兑换
    GO_CHALLENGE_INC,           //竞技场挑战次数增加
	GO_EQUIP_SUOHUN,			//装备锁魂消耗元宝
	GO_GUANXING_ZHANBU,			//观星占卜
	GO_GUANXING_OPENSLOT,		//开启星象格
    GO_ACTIVE_XXPACK,           //激活星象宝袋
    GO_ADD_XXPACK_POWER,        //星象宝袋充能
	GO_ACTIVE_PRIVILEGE,        //开通特权
	GO_PAY_AGAIN_PRIVILEGE,     //特权续费
	GO_UPGRADE_PRIVILEGE,        //特权升级
	GO_GMUPDATE,				//GM修正
	GO_TRUMP_SUOHUN,            //法宝锁魂消耗元宝
	GO_SOULEQ_SUOHOLE,          //魂装锁定宝石孔
};

//挂单操作返回
enum enGoldReason
{
	GR_PENDING,					//挂单成功(未有任何交易)
	GR_PART,					//挂单成功(有部分交易)
	GR_FINISH,					//挂单成功(全部完成交易)
	GR_FAILED,					//挂单失败(未知原因)
	GR_REDUCEGOLD,				//挂单失败(预扣元宝失败)
};

//元宝交易挂单数据结构
struct stGoldTrade
{
	U32  OrderID;				//交易流水号
	U32  AccountID;				//帐号ID
	U32  PlayerID;				//角色ID
	char PlayerName[32]; 		//角色名称
	U8	 OrderType;				//交易方式(0-买入元宝 1-卖出元宝)
	int  Price;					//单价(每100元宝价值)
	int  TradeNum;				//挂单原始数量
	int  LeftNum;				//剩余数量
	U8	 OrderStatus;			//交易状态
	U32  UpTime;				//挂单时间
	stGoldTrade() { Clear();}
	void Clear() { memset(this, 0, sizeof(stGoldTrade));}
};

//元宝交易交易历史结构
struct stGoldTradeDetail
{
	U32  OrderID;				//交易流水号
	U32  TradeNum;				//交易数量
	U32  TradeDate;				//成交时间
	stGoldTradeDetail() {OrderID = TradeNum = TradeDate = 0;}
};

typedef std::multimap<int, stGoldTrade*> MAPASC;
typedef std::multimap<int, stGoldTrade*, std::greater<int> > MAPDESC;

//元宝交易队列数据
struct stQuotation
{
	struct stPrice
	{
		U16 Num;
		int Price;
	};
	struct stLatestPrice
	{
		U16 Num;
		int Price;
		U32 TradeDate;
	};

	enum {MAX_ITEMS = 5};

	stPrice BuyPrice[MAX_ITEMS];		//排行最前5组买入元宝挂单价
	stPrice SellPrice[MAX_ITEMS];		//排行最前5组卖出元宝挂单价
	stLatestPrice LatestPrice[MAX_ITEMS];//最近5笔交易记录
	int CurrentPrice;					//最近成交价(今日最近一笔成功交易的元宝单价)
	int ClosePrice;						//最近收盘价(今早6:00清盘前最后一笔成功交易的元宝单价)
	int Pendings;						//未决的挂单数
	stQuotation() { Clear();}
	void Clear() {memset(this, 0, sizeof(stQuotation));}
};

//成交元宝交易挂单
struct stGoldTradeEx : stGoldTrade
{
	int BargainNum;		//成交数量
};

//收盘元宝交易挂单
struct stCloseGoldTrade
{
	int OrderID;	//挂单编号
	int PlayerID;	//角色ID
	int LeftValue;	//退还的剩余元宝或税收金
};

#endif//STOCK_BASE_H