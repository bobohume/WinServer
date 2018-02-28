#pragma once

#include "VIPBase.h"
#include "BaseMgr.h"

class CVIPMgr : public CBaseManager<stVIPBase>
{
public:
	CVIPMgr();
	~CVIPMgr();

	static CVIPMgr* Instance();
	//VIP累计充值
	bool HandleRecharge(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	//VIP礼包领取，每个VIP等级礼包只能领取一次
	bool HandleClientVipGiftRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);

	//获取精力上限加成
	U32 GetVipVIMTopLimit(U32 nPlayerId) const;
	//获取体力上限加成
	U32 GetVipStrengthTopLimit(U32 nPlayerId) const;
	//获取活力上限加成
	U32 GetVipActivityTopLimit(U32 nPlayerId) const;
	//获取精力、体力、活力回复时间减少
	F32 GetVipVSAReducePercent(U32 nPlayerId) const;

	//奏章上限
	U32 GetVipApplyTopLimit(U32 nPlayerId) const;
	//征收令上限
	U32 GetImposeTopLimit(U32 nPlayerId) const;
	//皇子席位上限
	U32 GetPriceNumTopLimit(U32 nPlayerId) const;
	//增收
	U32 GetResIncreasePercent(U32 nPlayerId) const;

	//是否可以跳过战斗动画
	bool CanJumpBattleAnim(U32 nPlayerId) const;

	//VIP征收初始值
	void VIPInitAddImpose(U32 nPlayerId);

	static void Export(struct lua_State* L);
private:
	//VIP Buff 包括 征收令上限、征收加成、皇子席位加成、奏章上限、精体活时间减少、跳过战斗动画
	bool AddVipBuff(U32 nPlayerId, U32 OldVipLevel, U32 CurrentVipLevel);
	//如果用户还未达到此等级则返回0
	U32 GetVipLvByGiftId(U32 nPlayerId, U32 VipGiftId);
private:
	const U8 MAXVIPLEVEL;
};

#define VIPMGR CVIPMgr::Instance()