#ifndef PVP_MANAGER_H
#define PVP_MANAGER_H

#include <unordered_map>
#include "WINTCP/IPacket.h"
#include "Common/PlayerStruct.h"
#include "BaseMgr.h"
#include <set>
#include <deque>
#include <map>
#include "PvpBase.h"
#include "PveAI.h"
/************************************************************************/
/* 战斗管理对象
/************************************************************************/
class CPveManager
{
	typedef std::map<std::pair<S32, S32>, PveInfoRef>	 DATA_MAP;
	typedef std::unordered_map<S32, DATA_MAP>			 PLAYER_MAP;
	typedef std::unordered_map<S64, DATA_VEC>			 BATTLE_MAP;
	typedef std::unordered_map<S64, PveAIRef>			 AI_MAP;
	typedef std::unordered_map<S64, PvpTableRef>		 TABLE_MAP;
	typedef TABLE_MAP::iterator							 TABLE_ITR;
	typedef PLAYER_MAP::iterator						 PLAYER_ITR;
	typedef DATA_MAP::iterator							 DATA_ITR;
public:
	CPveManager();
	~CPveManager();

	static CPveManager* Instance();

	bool CanJoinPve(S64 nPlayerID);
	bool CanOpenPve(S64 nPlayerID);
	bool CanUpdate(PvpTableRef pTable);

	enOpError JoinBattle(S64 nPlayerID, S8 nType);
	enOpError MatchBattle(S64 nPlayerID, S8 nType);
	enOpError AvengeBattle(S32 nPlayerID, S8 nType, S32 nTargetID, std::string UID);

	enOpError BattleBegin(S64 nPlayerID, S8 nType);
	enOpError BattleReady(S64 nPlayerID, S8 nType);
	enOpError BattleEndness(S64 nPlayerID, S8 nType);
	enOpError Battle(PvpTableRef pTable, S8 nType);
	enOpError Battle(S64 nPlayerID, S32 nAction, S8 nType);
	enOpError BattleCleaning(S32 nWiner, S32 nLoser, S64 nPlayerID, PvpTableRef pTable, S8 bEnd, std::string UID);
	enOpError AddBuff(S64 nPlayerID, S32 nBuffID, S8 nType, PvpInfoRef pBattleData[2]);
	DATA_VEC& GetBattle(S64 nPlayerID, S8 nType);
	S8	 GetBattleType(S32 nPlayerID);

	//获取余下可战斗的最大次数
	S8 GetRemainingMaxPVPTimes(U32 nPlayerID);
	//添加PVP战斗次数,不做任何上限检测
	enOpError AddPVPTimes(U32 nPlayerID, U16 times);

	void Update(U32 nTime);
	void BattleUpdate(PvpTableRef pData, S8 Type);
	static void Export(struct lua_State* L);

	//------------------------------//
	void AddPveInfo(S32 nPlayerID, S8 nType, S32 nTargetID, S32 nCardNum, S32 nScore, S32 nAddScore, S32 nBattleVal, S32 Unquie = 0);
	void AddData(S32 nPlayerID, S8 nType, PveInfoRef pData);
	void DeleteData(S32 nPlayerID, S8 nType, S32 nTargetID, S32 Unquie = 0);
	PveInfoRef GetData(S32 nPlayerID, S8 nType, S32 nTargetID, S32 Unqiue);
	void RemovePlayer(S32 nPlayerID);

	//----------通知客户端-----------//
	void UpdateToClient(U32 nPlayerID, PveInfoRef pData);
	void UpdateDelToClient(U32 nPlayerID, S32 nTargetID);
	void SendInitToClient(U32 nPlayerID);

	//---------DB Operator---------//
	DB_Execution* LoadDB(U32 nPlayerID);
	bool UpdateDB(U32 nPlayerID, PveInfoRef pData);
	bool InsertDB(U32 nPlayerID, PveInfoRef pData);
	bool DeleteDB(U32 nPlayerID, S8 nType, S32 nTargetID);
	bool ClearDB();
	
	bool SendPveToClient(U32 nPlayerID, S8 nType, char* Msg, U32 nError);
	bool SendPveEndToClient(U32 nPlayerID, S32 nScore, S32 nBattleNum, S32 nAddAch, S8 bEnd);
	bool UpdatePveToClient(U32 nPlayerID, PvpInfoRef pData[2], S32 nAction[2]);
	bool HandleOpenPveRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandlePveBeginRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandlePveReadyRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandlePveEndRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandlePveBattleRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandlePveAvengeRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandlePveAddBuffRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandlePveSetJumpBattle(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
private:
	//检测是否可以战斗，如果可以战斗则
	bool PVPBattleCD(U32 nPlayerID);
	//刷新cd
	bool PVPCheckAndFlushBattleCD(U32 nPlayerID);
	//获取战斗次数
	U32 PVPTimes(U32 nPlayerID);
	//使用一次战斗次数,打一个玩家减一次
	bool PVPOnce(U32 nPlayerID);
	//随机化
	void RandDEQData(DATA_VEC &Deq);
	//是否可以跳过战斗过程
	inline bool JumpBattle(const PvpTableRef pTable);
private:
	enOpError JoinBattle(U32 nPlayerID, S8 type, U32 nPlayerID1);
	PLAYER_MAP m_PlayerMap[2];//unqiue
	BATTLE_MAP m_BattleMap[3];//0-1,消息,2正常
	AI_MAP	   m_AIMap[3];
	TABLE_MAP  m_TableMap[3];
};
#define PVEMGR CPveManager::Instance()
#endif//PVP_MANAGER_H
