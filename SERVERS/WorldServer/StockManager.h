#ifndef __GOLDTRADEMANAGE_H__
#define __GOLDTRADEMANAGE_H__
/*
#include "Common/StockBase.h"
#include "base/bitStream.h"
#include "LockCommon.h"
#include "ManagerBase.h"
#include <deque>
#include <vector>

// ============================================================================
// 元宝交易管理
//
class GoldTradeManage : public CManagerBase< GoldTradeManage, MSG_GOLDTRADE_BEGIN, MSG_GOLDTRADE_END >
{
public:
	typedef std::deque<stQuotation::stLatestPrice*>  VECPRICE;
	typedef std::vector<stCloseGoldTrade*> VECCLOSE;
	GoldTradeManage();
	~GoldTradeManage();
	bool Initialize();
	void Update();

	void SetBreakOrder(std::vector<int>& buyList, std::vector<int>& sellList);
	void CloseOrder(U32 currentTime);
	void SetClosePrice(int closePrice, U32 nextCloseTime, VECCLOSE& taxList, VECCLOSE& goldList);

	void HandleZWCommissionRequest(int, stPacketHead*, Base::BitStream*);
	void CommissionOrder(bool IsOk, stGoldTrade* trade);
	void CommissionOrderResult(bool IsOk, stGoldTrade* pTrade, std::vector<stGoldTradeEx*> sourceList);
	void SendZoneCommission(int error, int PlayerID);

	void HandleCWCancelOrderRequest(int, stPacketHead*, Base::BitStream*);
	void CancelOrderResult(bool IsOk, stGoldTrade* pOrder);

	void HandleCWQueryOrderRequest(int, stPacketHead*, Base::BitStream*);
	void SendWCQueryOrder(int PlayerID, bool IsOk, std::vector<stGoldTrade*>& order,
		std::vector<stGoldTradeDetail*>& detail);
	void HandleCWQueryQuotation(int, stPacketHead*, Base::BitStream*);
#ifndef _NTJ_UNITTEST
private:
#else
public:
#endif
	void ClearTable();
	void SendWZLeftGold(int PlayerID, int AccountID);
	void SendWCCancelOrder(int, int, bool);
	void SendWCQueryQuotation(int);
	void AddLatestPrice(U32 tradedate, S32 price, U16 num);

#ifndef _NTJ_UNITTEST
private:
#else
public:
#endif
	MAPASC				mSellTable;			//委托卖出列表
	MAPDESC				mBuyTable;			//委托买入列表
	VECPRICE			mLatestPrice;		//最近交易记录

	U32					mNextCloseTime;		//下一次收盘时间
	S32					mCurrentPrice;		//最近成交价
	S32					mClosePrice;		//最近收盘价
};
*/
#endif//__GOLDTRADEMANAGE_H__