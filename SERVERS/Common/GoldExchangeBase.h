#ifndef __GOLDEXCHANGEBASE_H__
#define __GOLDEXCHANGEBASE_H__

#include "Base/BitStream.h"
#include "Common/PacketType.h"
#include <vector>


//更新元宝兑换数据标志
enum enExchangeType
{
	GEF_CR,					//充值返利兑换次数
	GEF_DR,					//提取赠送兑换次数
	GEF_WR,					//月大额兑换次数
	GEF_MR,					//月小额兑换次数
	GEF_VR0,				//一级兑换次数
	GEF_VR1,				//二级兑换次数
	GEF_VR2,				//三级兑换次数
	GEF_VR3,				//四级兑换次数
	GEF_VR4,				//五级兑换次数
	GEF_VR5,				//六级兑换次数
	GEF_MAX,				//兑换类型总数
};

//分期活动状态
enum enPeriodStatus
{
	APS_EXPIRE,				//已结束
	APS_DOING,				//进行中
	APS_NOOPEN,				//未开始
};

#pragma pack(push, 1)

// ----------------------------------------------------------------------------
// 月兑换配置数据结构
struct stGoldConfig
{
	S32 Change[GEF_MAX];
	U32 ItemID[GEF_MAX];
	U32 Times[GEF_MAX];
	S32 Condition[GEF_MAX];
	U32 Person[GEF_MAX];
	stGoldConfig() { Clear();}
	void Clear() {memset(this, 0, sizeof(stGoldConfig));}
};

// ----------------------------------------------------------------------------
// 月兑换数据结构
struct stGoldExchange
{
	U32 AccountID;			//帐号ID
	U32 PlayerID;			//角色ID
	U32 YearMonth;			//兑换年月
	U32 AccountGold;		//可提取帐号元宝数
	U32 CurrentGold;		//当前已提取元宝数
	U32 MonthPays;			//月充值次数
	U32 MonthGolds;			//月累积提取元宝数
	U32 Exchanges[GEF_MAX]; //月兑换次数
	stGoldExchange() { Clear();}
	void Clear() { memset(this, 0, sizeof(stGoldExchange));}
};

// ----------------------------------------------------------------------------
// 分期返利有效期间配置数据结构
struct stSR_Period
{
	U8  Status;				//活动状态(0-已结束 1-进行中 2-未开始)
	U8	IsDefault;			//是否缺省显示的活动
	U8  IsRewardGold;		//是否奖励金元和元宝
	U32 SRId;				//分期返利活动ID
	U32 StartDate;			//分期返利活动开始日期
	U32 EndDate;			//分期返利活动结束日期
	U32 DrawDate1st;		//分期返利活动第一次领取日期
	U32 DrawDate2nd;		//分期返利活动第二次领取日期
	U32 DrawDate3rd;		//分期返利活动第三次领取日期
	stSR_Period() { memset(this, 0, sizeof(stSR_Period));}
};

// ----------------------------------------------------------------------------
// 分期返利奖励配置数据结构
struct stSR_Bonus
{
	U8  PerStag;			//分期返利的第几阶段
	U8	PerMonth;			//分期返利的第几个月
	U32 SRId;				//分期返利活动ID
	S32 Gold;				//返利元宝比率
	S32 BindGold;			//返利绑定元宝比率
	S32 Money;				//返利金元比率
	S32 BindMoney;			//返利绑定金元比率
	S32 Exp;				//返利经验修为比率
	stSR_Bonus()
	{ 
		SRId = Gold = BindGold = Money = BindMoney = Exp = 0;
		PerMonth = PerStag = 0;
	}
};

struct stSR_Bonus_Data
{
	U8	IsDraw;				//是否领取过(0-未领取 1-已领取 2-不可领取)
	S32 Gold;				//返利元宝
	S32 BindGold;			//返利绑定元宝
	S32 Money;				//返利金元
	S32 BindMoney;			//返利绑定金元
	S32 Exp;				//返利经验修为
	stSR_Bonus_Data()
	{
		IsDraw = 2;
		Gold = BindGold = Money = BindMoney = Exp = 0;
	}
};

// ----------------------------------------------------------------------------
// 角色的分期返利奖励数据结构
struct stSR_History
{
	U32 SRId;				//分期返利活动ID
	S32 TotalDrawGolds;		//活动期间提取的元宝数
	stSR_Bonus_Data Data[3];//分期三个月的奖励数据
	stSR_History()
	{
		SRId = TotalDrawGolds = 0;
	}

	void Clear() { memset(this, 0, sizeof(stSR_History));}
};

#pragma pack(pop)

#endif//__GOLDEXCHANGEBASE_H__