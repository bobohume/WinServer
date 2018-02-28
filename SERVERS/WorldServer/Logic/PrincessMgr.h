#ifndef __PRINCESS_MGR_H
#define __PRINCESS_MGR_H
#include "PrincessBase.h"
#include "BattleMgr.h"
#include "Base/tVector.h"
#include "BuffData.h"


const U32 TRUNCARDCD = 30 * 60;

struct stTurnCard
{
	std::string selUID;
	Vector<std::string> UVec;
	S32 chooseTime;
	bool InitFlag;

	stTurnCard() : selUID(""), chooseTime(0), InitFlag(false)
	{
	}

	void Clear()
	{
		selUID = "";
		UVec.clear();
		chooseTime = 0;
		InitFlag = false;
	}
};

class CPrincessMgr : public CBaseManagerEx<PrincessRef, std::string>
{
	typedef CBaseManagerEx<PrincessRef, std::string>   Parent;
	typedef stdext::hash_map<U32, stTurnCard>		   TPLAYERMAP;
	typedef TPLAYERMAP::iterator					   TPLAYERITR;
public:
	CPrincessMgr();
	virtual ~CPrincessMgr();

	static CPrincessMgr* Instance();
	bool AddSupport(U32 nPlayerID, U32 nID);
	PrincessRef CreateData(U32 nPlayerID, U32 nID);
	bool CanAddData(U32 nPlayerID, U32 nID);
	bool AddData(U32 nPlayerID, U32 nID, S32 nSupport = 0, bool bNotify = true);

	bool CaculateStats(U32 nPlayerID, std::string UID, Stats& stats);//计算单个属性
	void RemovePlayer(U32 nPlayerID);
	void DeleteData(U32 nPlayerID, std::string nID);

	//获取征服中满足招募值得妃子
	bool FindPrincessIdsBySupport(U32 nSupport, std::vector<U32>& UIDs);
	S32		 GetPrincessNum(U32 nPlayerID);
	enOpError TurnCard(U32 nPlayerID);//翻牌
	enOpError ChoosePrincess(U32 nPlayerID, U32 nType);
	enOpError SupportPrincess(U32 nPlayerID, std::string UID);
	void	  AddPrincessAbility(PrincessRef pData, U32 nType);
	enOpError AddPrincessPos(U32 nPlayerID, std::string UID, S32 nPrincessPos, S32 nPrePrincessPos = 0);
	enOpError RemovePrincessPos(U32 nPlayerID, std::string UID);
	enOpError DispatchPrincess(U32 nPlayerID, std::string UID, U32 nType);
	enOpError SKillLvUpPrincess(U32 nPlayerID, std::string UID);
	enOpError LearnRitePrincesss(U32 nPlayerID, std::string CUID, std::string UID, S32 nPos);
	enOpError AwardItemPrincess(U32 nPlayerID, std::string UID, U32 nItemID);
	enOpError PlantFlowerPrincess(U32 nPlayerID, std::string UID, S32 nFlowerID, S32 nPos);
	enOpError FinishPrincess(U32 nPlayerID, S32 nType, S32 nSlot, S32& nPreVal, S32& nAftVal);
	enOpError AddMaxSlot(U32 nPlayerID, S32 nType);
	enOpError AddPrinceAbility(U32 nPlayerID, std::string UID, S32 nAddVal);
	void	  UpdatePrincess(U32 nPlayerID);
	//---------Notify Operator---------//
	void UpdateToClient(U32 nPlayerID, PrincessRef pData);
	void SendInitToClient(U32 nPlayerID);

	//---------DB Operator---------//
	DB_Execution* LoadDB(U32 nPlayerID);
	PrincessRef SyncLoadDB(U32 nPlayerID, std::string UID);

	bool UpdateDB(U32 nPlayerID, PrincessRef pData);
	bool InsertDB(U32 nPlayerID, PrincessRef pData);
	bool DeleteDB(U32 nPlayerID, std::string UID);
	bool UpdatePrincessTime(U32 nPlayerID, PrincessRef pData);

	bool HandleClientSupportRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleClientAwardRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleClientTurnCardRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleClientChooseRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleClientBestowedRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleClientDispatchRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleClientLearnRiteRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleClientPlantFlowerRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleClientFinishRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleClientAddMaxSlotRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);

	static void Export(struct lua_State* L);
private:
	TPLAYERMAP m_TPlayerMap;//翻牌队列
};

#define PRINCESSMGR CPrincessMgr::Instance()
#endif//__PRINCESS_MGR_H