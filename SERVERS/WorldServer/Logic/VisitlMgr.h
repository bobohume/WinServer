#ifndef __VISIT_MGR_H
#define __VISIT_MGR_H
#include "VisitlBase.h"
#include "BaseMgr.h"
#include "Base/tVector.h"

const U32 VISITCITYCD = 30 * 60;

class CVisitMgr : public CBaseManagerEx<CityRef, S32>
{
	typedef CBaseManagerEx<CityRef, S32>   Parent;
public:
	CVisitMgr();
	virtual ~CVisitMgr();

	static CVisitMgr* Instance();

	CityRef CreateData(U32 nPlayerID, S32 nID);
	bool AddData(U32 nPlayerID, S32 nCityID, S32 nLevel, S32 nSupport, S32 nDevelopFlag);

	enOpError rollDice(U32 nPlayerID, std::pair<S32, S32>& DropInfo);
	enOpError VisitCity(U32 nPlayerID, S32 nVal, std::pair<S32, S32>& DropInfo, bool bCostItem = false);
	enOpError DevelopCity(U32 nPlayerID, S32 nType);

	//---------Notify Operator---------//
	void UpdateToClient(U32 nPlayerID, CityRef pData);
	void SendInitToClient(U32 nPlayerID);

	bool HandleRollDiceRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleRollFixedDiceRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleDevelopCityRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	//---------DB Operator---------//
	DB_Execution* LoadDB(U32 nPlayerID);

	bool UpdateDB(U32 nPlayerID, CityRef pData);
	bool InsertDB(U32 nPlayerID, CityRef pData);
	bool DeleteDB(U32 nPlayerID, S32 ID);

	static void Export(struct lua_State* L);
};

#define VISITMGR CVisitMgr::Instance()
#endif//__VISIT_MGR_H