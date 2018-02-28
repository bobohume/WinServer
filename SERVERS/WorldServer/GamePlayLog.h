#ifndef __GAMEPLAYLOG_H__
#define __GAMEPLAYLOG_H__
/*
#include "Common/ChatBase.h"
#ifndef DB_STRUCT_H
#include "common/dbStruct.h"
#endif

class GameplayLog
{
public:
	static void GL_AccountLogin(int AccountId);
	static void GL_AccountLogout(int AccountId);

	static void GL_GMAccountLogin(int GMAccountId);
	static void GL_GMAccountLogout(int GMAccountId);
	static void GL_GMAccountQuery(int GMAccountId, int AccountId);
	static void GL_GMUpdateAccountGold(int GMAccountId, int AccountId);
	static void GL_GMUpdateActorGold(int GMAccountId, int AccountId);
	static void GL_GMFreezeAccount(int GMAccountId, int AccountId);
	static void GL_GMUnfreezeAccount(int GMAccountId, int AccountId);
	static void GL_GMBanAccount(int GMAccountId, int AccountId);
	static void GL_GMUnbanAccount(int GMAccountId, int AccountId);
	static void GL_GMKickAccount(int GMAccountId, int AccountId);
	static void GL_GMBanActor(int GMAccountId, int playerId);
	static void GL_GMUnbanActor(int GMAccountId, int playerId);
	static void GL_GMFreezeActor(int GMAccountId, int playerId);
	static void GL_GMUnfreezeActor(int GMAccountId, int playerId);
	static void GL_GMKickActor(int GMAccountId, int playerId);
	static void GL_GMTransferActor(int GMAccountId, int playerId, 
				int zoneId, float x, float y, float z);
	static void GL_GMSendMail(int GMAccountId, int playerId, 
				int itemId, int itemNum, int money);
	static void GL_GMActorQuery(int GMAccountId, int playerId);
	static void GL_GMDelActorQuery(int GMAccountId, int playerId);
	static void GL_GMDelActor(int GMAccountId, int playerId);
	static void GL_GMUnDelActor(int GMAccountId, int playerId);

	static void GL_GMModifyLevel(int GMAccountId, int playerId, int oldlevel, int newlevel);
	static void GL_GMModifyEXP(int GMAccountId, int playerId, int oldexp, int newexp);
	static void GL_GMModifyGold(int GMAccountId, int playerId, int oldgold, int newgold, int oldBindMoney, int newBindMoney, int oldBindGold, int newBindGold);
	static void GL_GMActorRename(int GMAccountId, int playerId, const char* oldName, const char* newName);
	static void GL_GMPvpScoreUpdate(int GMAccountId, int playerId, int oldPvpScore, int newPvpScore);
	static void GL_GMActorClrSecondPassword(int GMAccountId, int playerId);
	static void GL_GMActorResetXianShu(int GMAccountId, int playerId);
	static void GL_GMGodShipMdf(int GMAccountId, int playerId, U64 oldexp, U64 newexp, int oldLevel, int newLevel);


	static void GL_GMDeleteMail(int GMAccountId, stMailItem& mailitem);
	static void GL_GMModifyMail(int GMAccountId, stMailItem& mailitem);
	static void GL_GMGetMission(int GMAccountId, int playerId, int missionId);
	static void GL_GMDeleteMission(int GMAccountId, int playerId, int missionId);
	static void GL_GMGetCompleteMission(int GMAccountId, int playerId, int missionId);
	static void GL_GMQueryMail(int GMAccountId, int playerId, int itemId, int itemNum, int money);

	static void GL_GMGetSkill(int GMAccountId, int playerId, int skillId);
	static void GL_GMDeleteSkill(int GMAccountId, int playerId, int skillId);
	static void GL_GMGetLivingSkill(int GMAccountId, int playerId, int skillId);
	static void GL_GMDeleteLivingSkill(int GMAccountId, int playerId, int skillId);
	
	static void GL_GMGetItem(int GMAccountId, int playerId, stItemInfo& item);
	static void GL_GMModifyItem(int GMAccountId, int playerId, stItemInfo& olditem, stItemInfo& newitem);
	static void GL_GMDelItem(int GMAccountId, int playerId, stItemInfo& item);

	static void GL_GMXingXiangMODIFY(int GMAccountId, int playerId, stItemInfo& olditem, stItemInfo& newitem,int pos);
	static void GL_GMXingXiangDEL(int GMAccountId, int playerId, stItemInfo& olditem,int pos);
	static void GL_GMModifyGodPrint(int GMAccountId, int playerId,stGodPrintInfo *oldGPinfo,stGodPrintInfo *newGPinfo);

	static void GL_GMGetPet(int GMAccountId, int playerId, stPetInfo& pet);
	static void GL_GMDeletePet(int GMAccountId, int playerId, stPetInfo& pet);
	static void GL_GMModifyPet(int GMAccountId, int playerId, stPetInfo& oldpet, stPetInfo& newpet);
	static void GL_GMGetMount(int GMAccountId, int playerId, stMountInfo& mount);
	static void GL_GMDeleteMount(int GMAccountId, int playerId, stMountInfo& mount);
	static void GL_GMModifyMount(int GMAccountId, int playerId, stMountInfo& oldmount, stMountInfo& newmount);
	static void GL_GMLingChongEdit(int GMAccountId, int playerId, stGodPetInfo *oldInfo, stGodPetInfo *newInfo);

	static void GL_GMSendMSG(int GMAccountId, int playerId, stChatMessage& msg);
	static void GL_GMSendAutoMSG(int GMAccountId, int playerId, stChatMessage& msg);
	static void GL_GMChgRole(int GMAccountId, int accountId, int gmFlag);
	static void GL_GMPlayerOp(int GMAccountId, int GMplayerId, int TarPlayerId,const char* cmd);

	static void GL_PlayerLogin(int PlayerId);
	static void GL_PlayerLogout(int PlayerId);
	static void GL_PlayerCreate(int PlayerId);
	static void GL_PlayerDelete(int PlayerId, int level);

	static void GL_PlayerTeamBreak(int leaderId, TeamMemberInfo* teamList);
	static void GL_PlayerTeamToLeader(int oldLeaderId, int newLeaderId);
	static void GL_PlayerTeamInvate(int playerid, int other);
	static void GL_PlayerTeamKick(int leaderId, int playerId);
	static void GL_PlayerTeamExit(int playerId, int leaderId);
	static void GL_PlayerTeamJoin(int playerId, int leaderId);

	static void GL_PlayerMakeSocial(int PlayerId, int targetId, int srcType, int tagType);
	static void GL_PlayerChangeSocial(int PlayerId, int targetId, int oldSrcType, int newSrcType, int oldTagType, int newTagType);
	static void GL_PlayerDeleteSocial(int PlayerId, int targetId, int srcType, int tagType);

	static void GL_GoldExchange(int PlayerId, int type, int change, int item);

	static void GL_MailDelete(stMailItem& mailitem);
	static void GL_MailSend(int PlayerId, S32 recver, stMailItem& mailitem);
	static void GL_ServerMailSend(S32 recver, stMailItem& mailitem);

	static void GL_ArenaFight(U32 PlayerId, S32 Arena, S32 Season, S32 TotalScore, S32 CurrentScore);

	static void GL_DrawGold(int PlayerID, int Gold, int AccountGold, int LeftGold);
	static void GL_BuyGoods(int PlayerID, int FriendID, int Gold, int LeftGold, U64 PriceID, int PriceNum);
	static void GL_ReduceGold(int PlayerID, U32 type, int Gold, int LeftGold);
	static void GL_TradeBuyOrder(int PlayerID, int OrderID, int SrcPrice, int BargainNum,
		int TargetID, int TargetOrderID, int TagPrice);
	static void GL_TradeSellOrder(int PlayerID, int OrderID, int SrcPrice, int BargainNum,
		int TargetID, int TargetOrderID, int TagPrice);
	static void GL_BuyOrder(int PlayerID, int OrderID, int Price, int TradeNum);
	static void GL_SellOrder(int PlayerID, int OrderID, int Price, int TradeNum);
	static void GL_CloseGoldTrade_Tax(int PlayerID, int Tax);
	static void GL_CloseGoldTrade_Gold(int PlayerID, int Gold);
	static void GL_CancelGoldTrade(int PlayerID, int OrderID, int type, int price, int num );

	static void GL_OrgLog(int playerID, const OrgLogInfo& info, enOpType op);
	static void GL_LeagueLog(int playerID, const LeagueLogInfo& info, enOpType op);

	//商铺的相关日志
	static void GL_ShopBuyShop(U32 PlayerID, U16 stallId, S32 money, U32 itemId);
	static void GL_ShopFlowMoney(U32 PlayerID, U16 stallId, S32 srcMoney, S32 tagMoney);
	static void GL_ShopFixedMoney(U32 PlayerID, U16 stallId, S32 srcMoney, S32 tagMoney);
	static void GL_ShopBuySlots(U32 PlayerID, U16 stallId, U8 type, U8 page);
	static void GL_ShopDayTax(U32 PlayerID, U32 stallId, S32 dayTax, S32 flourish, S32 totalFlourish, S32 srcMoney, S32 tagMoney);
	static void GL_ShopCostShoutItem(U32 PlayerID, U32 stallId, U32 itemId, S32 itemNum);
	static void GL_ShopDumpStall(U32 PlayerID, U32 stallId);
	static void GL_ShopSetSellItem(int PlayerID, U32 type,stFixedStallItem* item);
	static void GL_ShopSetSellPet(int PlayerID, U32 type,stFixedStallPet* pet);
	static void GL_ShopSetBuyItem(int PlayerID, U32 type,stFixedStallItem* item);
	static void GL_ShopSetShoutItem(int PlayerID, U32 type, U32 stallId, U32 itemId, S32 itemNum);
	static void GL_ShopBuySellItem(int PlayerID, U32 type, U32 stallId, U32 stallerId, S32 srcFlowMoney, 
		S32 tagFlowMoney, S32 money, S32 tax, S32 taxRate, stItemInfo* item, S32 pos, S32 price, S32 itemNum);
	static void GL_ShopBuySellPet(int PlayerID, U32 type, U32 stallId, U32 stallerId, S32 srcFlowMoney,
		S32 tagFlowMoney, S32 money, S32 tax, S32 taxRate, stPetInfo* pet, S32 pos, S32 price);
	static void GL_ShopSellBuyItem(int PlayerID, U32 type, U32 stallId, U32 stallerId, S32 srcFlowMoney,
		S32 tagFlowMoney, stItemInfo* item, S32 index, S32 srcNum, S32 leftNum, S32 price);

	static void GL_DrawSRBonus(int PlayerId, U32 SRId, S32 PerMonth, S32 gold, S32 bindgold, S32 money,
		S32 bindmoney, S32 exp);

	static void GL_AuctionAddOK(int PlayerId, U32 ItemId, stItemInfo* pItem, S32 ItemNum, S32 price);
	static void GL_AuctionAddFailed(int PlayerId, U32 ItemId, stItemInfo* pItem, S32 ItemNum, S32 price);
	static void GL_AuctionBuyOK(int PlayerId, U32 Id, S32 price);
	static void GL_AuctionBuyFailed(int PlayerId, U32 Id, S32 price);
	static void GL_AuctionCancelOK(int PlayerId, U32 Id);
	
    //判断是否需要记录日志
	static bool GL_IsRecord(int AccountId, int PlayerId, U32 LogType) { return true;}
};
*/
#endif//__GAMEPLAYLOG_H__
