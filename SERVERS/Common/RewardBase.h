#ifndef __REWARDBASE_H__
#define __REWARDBASE_H__

#include "Base/BitStream.h"
#include "Common/PacketType.h"
#include <vector>

//奖励类型
enum enRewardType
{
	REWARD_BINDMONEY			= 1,	//绑定金元
	REWARD_BINDGOLD				= 2,	//绑定元宝
	REWARD_ITEM					= 3,	//物品
};

#pragma pack(push, 1)

//奖励结构
struct stAccountReward
{
	U32 AccountID;			//帐号编号
	U32 RewardID;			//奖励编号
	U8  RewardType;			//奖励类型
	S32 RewardValue;		//奖励内容
	S32 RewardNum;			//奖励数量
	bool IsAuto;			//是否系统自动完成
	bool IsDraw;			//是否领取过
	U32 PlayerID;			//领取的角色编号
	stAccountReward() { Clear();}
	void Clear() { memset(this, 0, sizeof(stAccountReward));}
};

#pragma pack(pop)

#endif//__REWARDBASE_H__