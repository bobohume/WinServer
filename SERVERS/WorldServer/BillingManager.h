#ifndef __BILLINGMANAGER_H__
#define __BILLINGMANAGER_H__
/*
#include "ManagerBase.h"
#include "LockCommon.h"
#include "Event.h"
#include "base\bitStream.h"
#include "Common\BillingBase.h"

class BillingManager : public CManagerBase< BillingManager, MSG_BILLING_BEGIN, MSG_BILLING_END >
{	
public:
	BillingManager();
	~BillingManager();
	bool Update();
	int	 GetVersion() { return mVersion;}
	void ChangeVersion(int ver, GOODSVEC& goodsList);

	void ClearBoardList();
	void UpdateBoardList(BOARDVEC&);
	void HandleCWQueryBoard(int, stPacketHead*, Base::BitStream*);

	void LoadGoods(GOODSVEC& goodsList);
	bool ClearGoods();

	void HandleZWQueryGoods(int, stPacketHead*, Base::BitStream*);
	void SendWZQueryGoods(int, int);

	void HandleZWDrawGold(int, stPacketHead*, Base::BitStream*);
	void HandleAWDrawGold(int, stPacketHead*, Base::BitStream*);
	void HandleZWQueryGold(int, stPacketHead*, Base::BitStream*);
	void HandleZWReduceGold(int, stPacketHead*, Base::BitStream*);
	void ReduceGoldCallBack(int, int, int);

	void HandleZWQueryTotalDrawGold(int,stPacketHead*, Base::BitStream*);
	void SendWZQueryGold(int PlayerID, int LeftGold, int TotalDrawGold, bool IsDraw=false);
	void SendWZQueryTotalDrawGold(int PlayerId, int TotalDrawGolds);
	void HandleZWBuyGoods(int, stPacketHead*, Base::BitStream*);
	void BuyGoodsUpdatePriceCallBack(int LeftGold, int FriendID, int Op, stTradeGoods& tradegoods);
	void SendMailForPrice(int PlayerID, int FriendID, PRICEID priceId, U32 priceNum);
	
	void HandleCWQueryAccountGold(int,stPacketHead *,Base::BitStream *);
	void HandleAWQueryAccountGold(int,stPacketHead *,Base::BitStream *);

	void HandleWAGMQueryAccountGold(int sid,int AccountID,int GMAccountID);
	void HandleAWGMQueryAccountGold(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet);

/*	void HandleWAGMUpdateAccountGold(int sid,int AccountID,int GMAccountID,int gold);*//*
	void HandleAWGMUpdateAccountGold(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet);

	void SendWCQueryAccountGold(int PlayerID, int AccountGold);
#ifdef _NTJ_UNITTEST
public:
#else
private:
#endif
	int				mVersion;			//上架商品数据版本
	GOODSVEC		mGoodsList;			//上架商品列表
	BOARDVEC		mBoardList;			//广播消息字符串列表
};
*/

#endif//__BILLINGMANAGER_H__