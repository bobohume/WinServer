#ifndef ROOM_MGR_H
#define ROOM_MGR_H
#include "base/bitStream.h"
#include "common/PacketType.h"
#include "../LockCommon.h"
#include "../ManagerBase.h"
#include <hash_map>
#include "ChessTable.h"

typedef stdext::hash_map<U64, ITable*> TABLE_MAP;
typedef TABLE_MAP::iterator			   TABLE_ITR;
typedef stdext::hash_map<U32, U64>	   PLAYER_MAP;
typedef PLAYER_MAP::iterator		   PLAYER_ITR;
typedef stdext::hash_map<U64, bool>	   TABLE_MAP_EX;
typedef TABLE_MAP_EX::iterator		   TABLE_MAP_ITR;

class CRoomMgr
{
public:
	CRoomMgr();
	~CRoomMgr();

public:
	//创建棋牌桌子
	enRoomError CreateChessTable(U32 nPlayerId, U64 &nTableId, U32 nGameType, U32 nCreateType, S64 nScore, char* HeadImgUrl, char* nickname);

	//加入棋牌桌子
	//enRoomError JoinChessTable(U32 nPlayerId, U64 nTableId);

	//桌子准备
	enRoomError ReadyChessTable(U32 nPlayerId, U64 nTableId, U32& nSlot, char* HeadImgUrl, char* nickname);

	//桌子准备
	enRoomError RestartChessTable(U32 nPlayerId, U64 nTableId);

	//离开棋牌桌子
	enRoomError LeaveChessTable(U32 nPlayerId, U64 nTableId);

	//解散棋牌桌子
	enRoomError DisbandChessTable(U32 nPlayerId, U64 nTableId, U8 nAgreeFlag);

	//查找桌子
	CChessTable* FindChessTable(U64 nTableId);

	U64  AssignId();
	static CRoomMgr* Instance();

	bool HandleCreateChessTable(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet);
	bool HandleJoinChessTable(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet);
	bool HandleReadyChessTable(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet);
	bool HandleRestartChessTable(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet);
	bool HandleLeaveChessTable(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet);
	bool HandleDisbandChessTable(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet);
	bool HandleUserOutCard(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet);
	bool HandleUserOperateCard(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet);

	//定时器事件
	virtual void TimeProcess(U32 value);
	//获取table 跟slot
	CChessTable* FindChessTableByPlayerId(U32 nPlayerId);
private:
	TABLE_MAP m_TableMap;//游戏桌子信息
	PLAYER_MAP m_PlayerMap;//玩家信息
	TABLE_MAP_EX m_TableMapEx;//系列号信息 
	U64 m_ID;
};
#define ROOMMGR CRoomMgr::Instance()
#endif//ROOM_MGR_H
