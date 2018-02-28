#ifndef ROOM_BASE_H
#define ROOM_BASE_H
#include "BASE/types.h"
#include <map>

class ITable
{
public:
	explicit ITable() : m_Id(0), m_Type(0)
	{
	}

	virtual ~ITable()
	{
	}

	virtual bool InitTable()  = 0;
	virtual void ResetTable() = 0;
	virtual U32	 GetMaxPlayers() = 0;
	virtual bool HasDisband() = 0;

	void SetId(U64 Id)	{m_Id = Id;}
	U64	 GetId()		{return m_Id;}

	bool HasPlayer(U16 sId) {return m_PlayerMap[sId];}
	U32  GetPlayer(U16 sId) {return m_PlayerMap[sId];}
	U16  GetPlayerSlot(U32 nPlayerID)
	{
		for(int i = 0; i < GetMaxPlayers(); ++i)
		{
			if(m_PlayerMap[i] == nPlayerID)
			{
				return i;
			}
		}
		return GetMaxPlayers();
	}

	U32  GetPlayerNum()	
	{
		U32 nNum = 0;
		for(int i = 0; i < GetMaxPlayers(); ++i)
		{
			if(m_PlayerMap[i])
			{
				++nNum;
			}
		}
		return nNum;
	}
	bool InsertPlayer(U32 nPlayerId, U16 sId)
	{
		if(sId >= GetMaxPlayers())
			return false;

		m_PlayerMap[sId] = nPlayerId;
		return true;
	}
	bool DelPlayer(U32 nPlayerId)
	{
		for(int i = 0; i < GetMaxPlayers(); ++i)
		{
			if(m_PlayerMap[i] == nPlayerId)
			{
				m_PlayerMap[i] = 0; 
				return true;
			}
		}
		return false;
	}
	void ClearPlayer()
	{
		for(int i = 0; i <= GetMaxPlayers(); ++i)
		{
			m_PlayerMap[i] = 0;
		}
	}

	bool HasJoinPlayer(U16 sId) {return m_PlayerJoinMap[sId];}
	U32  GetJoinPlayer(U16 sId) {return m_PlayerJoinMap[sId];}
	U16  GetJoinPlayerSlot(U32 nPlayerID)
	{
		for(int i = 0; i < 10; ++i)
		{
			if(m_PlayerJoinMap[i] == nPlayerID)
			{
				return i;
			}
		}
		return 10;
	}

	U32  GetJoinPlayerNum()	
	{
		U32 nNum = 0;
		for(int i = 0; i < GetMaxPlayers(); ++i)
		{
			if(m_PlayerJoinMap[i])
			{
				++nNum;
			}
		}
		return nNum;
	}
	bool InsertJoinPlayer(U32 nPlayerId, U16 sId)
	{
		if(sId >= 10)
			return false;

		m_PlayerJoinMap[sId] = nPlayerId;
		return true;
	}
	bool DelJoinPlayer(U32 nPlayerId)
	{
		for(int i = 0; i < GetMaxPlayers(); ++i)
		{
			if(m_PlayerJoinMap[i] == nPlayerId)
			{
				m_PlayerJoinMap[i] = 0; 
				return true;
			}
		}
		return false;
	}
	void ClearJoinPlayer()
	{
		for(int i = 0; i <= GetMaxPlayers(); ++i)
		{
			m_PlayerJoinMap[i] = 0;
		}
	}
private:
	U64 m_Id;
	U32 m_Type;
	std::map<U16, U32> m_PlayerMap;		//桌子slot对应玩家	
	std::map<U16, U32> m_PlayerJoinMap;		//桌子slot对应玩家
};

enum enRoomError
{
	RoomError_None,
	RoomError_Unknown,			// 未知
	RoomError_OutRoom,			// 不在房间
	RoomError_InRoom,			// 已在房间
	RoomError_NoExist,			// 房间不存在
	RoomError_MaxPlayer,		// 房间在玩人数超上限
	RoomError_HasPlayer,		// 该房间已经有玩家在准备中
	RoomError_Playing,			// 房间在游戏中
	RoomError_Count,
};
#endif