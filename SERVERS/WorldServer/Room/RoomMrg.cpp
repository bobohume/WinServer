#include "RoomMgr.h"
#include "CommLib/CommLib.h"
#include "WINTCP/MessageCode.h"
#include "../WorldServer.h"
#include "../PlayerMgr.h"
#include "WINTCP/dtServerSocket.h"
#include "Common/MemGuard.h"
#include "Common/mRandom.h"
#include "ChessGX.h"

CRoomMgr::CRoomMgr() : m_ID(100000)
{
	REGISTER_EVENT_METHOD("CW_CreateChessTable",  this, &CRoomMgr::HandleCreateChessTable);
	REGISTER_EVENT_METHOD("CW_JoinChessTable",    this, &CRoomMgr::HandleJoinChessTable);
	REGISTER_EVENT_METHOD("CW_ReadyChessTable",   this, &CRoomMgr::HandleReadyChessTable);
	REGISTER_EVENT_METHOD("CW_RestartChessTable", this, &CRoomMgr::HandleRestartChessTable);
	REGISTER_EVENT_METHOD("CW_LeaveChessTable",   this, &CRoomMgr::HandleLeaveChessTable);
	REGISTER_EVENT_METHOD("CW_DisbandChessTable", this, &CRoomMgr::HandleDisbandChessTable);
	REGISTER_EVENT_METHOD("CW_UserOutCard",		  this, &CRoomMgr::HandleUserOutCard);
	REGISTER_EVENT_METHOD("CW_UserOperateCard",   this, &CRoomMgr::HandleUserOperateCard);
	SERVER->GetWorkQueue()->GetTimerMgr().notify(this, &CRoomMgr::TimeProcess, 10 * 1000);
	//for(int i = 0; i < 1000; ++i)
	//{
	//	CChessGX* pTable = new CChessGX();
	//	//pTable->SetPrivateInfo(GAME_TYPE_136, (BIT(CChessTable::GAME_TYPE_ZZ_QIANGGANGHU) | BIT(CChessTable::GAME_TYPE_ZZ_SUOHU)));
	//	pTable->SetTableScore(30);

	//	U64 id = AssignId();
	//	pTable->SetId(id);
	//	m_TableMap.insert(TABLE_MAP::value_type(id, pTable));
	//	pTable->OnEventGameStart();
	//}
}

CRoomMgr::~CRoomMgr()
{
	SERVER->GetWorkQueue()->GetTimerMgr().remove(this, &CRoomMgr::TimeProcess);
}

void CRoomMgr::TimeProcess(U32 value)
{
	for(TABLE_ITR itr = m_TableMap.begin(); itr != m_TableMap.end();)
	{
		//删除房间
		if(itr->second && itr->second->HasDisband())
		{
			for(U32 i = 0; i < itr->second->GetMaxPlayers(); ++i)
			{
				U32 nPlayerId = itr->second->GetPlayer(i);
				if(nPlayerId)
				{
					m_PlayerMap.erase(nPlayerId);
				}
			}
			itr->second->ClearPlayer();
			itr->second->ClearJoinPlayer();
			m_TableMapEx.erase(itr->first);
			itr = m_TableMap.erase(itr);
		}
		else
		{
			++itr;
		}
	}

}

enRoomError CRoomMgr::CreateChessTable(U32 nPlayerId, U64 &nTableId, U32 nGameType, U32 nCreateType, S64 nScore, char* HeadImgUrl, char* nickname)
{
	if(m_PlayerMap.find(nPlayerId) != m_PlayerMap.end())
	{
		return RoomError_InRoom;
	}

	CChessGX* pNewTable = new CChessGX();
	U64 id = AssignId();
	nTableId = id;
	pNewTable->SetId(id);
	pNewTable->InsertPlayer(nPlayerId, 0);
	pNewTable->InsertJoinPlayer(nPlayerId, 0);
	if(nGameType)
		pNewTable->AddGameRule(BIT(CChessGX::GAME_TYPE_ZZ_MOBAO));
	m_TableMap.insert(TABLE_MAP::value_type(id, pNewTable));
	m_PlayerMap.insert(PLAYER_MAP::value_type(nPlayerId, id));
	//索胡
	if(nCreateType == 0)
	{
		pNewTable->AddGameRule(BIT(CChessTable::GAME_TYPE_ZZ_SUOHU));
		pNewTable->SetTableScore(nScore);
	}
	else if(nCreateType == 1)//长胡(8局)
	{
		pNewTable->AddGameRule(BIT(CChessTable::GAME_TYPE_ZZ_CHANGHU));
		pNewTable->SetTableNum(8);
	}
	else//长胡(16局)
	{
		pNewTable->AddGameRule(BIT(CChessTable::GAME_TYPE_ZZ_CHANGHU));
		pNewTable->SetTableNum(16);
	}
	pNewTable->m_strUserUrlImg[0] = HeadImgUrl;
	pNewTable->m_strUserNickName[0] = nickname;

	//pNewTable->OnEventGameStart();
	return RoomError_None;
}

//enRoomError CRoomMgr::JoinChessTable(U32 nPlayerId, U64 nTableId)
//{
//	TABLE_ITR iter = m_TableMap.find(nTableId);
//	if(iter == m_TableMap.end())
//	{
//		return RoomError_NoExist;
//	}
//
//	if(m_PlayerMap.find(nPlayerId) != m_PlayerMap.end())
//	{
//		return RoomError_InRoom;
//	}
//
//	CChessTable* pTable = dynamic_cast<CChessTable*>(iter->second);
//	if(!pTable)
//		return RoomError_Unknown;
//	
//	if(pTable->GetPlayerNum() >= pTable->GetMaxPlayers())
//		return RoomError_MaxPlayer;
//
//	if(pTable->GetGameStatus() != GS_MJ_FREE)
//		return RoomError_Playing;
//
//	return RoomError_None;
//}

enRoomError CRoomMgr::ReadyChessTable(U32 nPlayerId, U64 nTableId, U32& nSlot, char* HeadImgUrl, char* nickname)
{
	TABLE_ITR iter = m_TableMap.find(nTableId);
	if(iter == m_TableMap.end())
	{
		return RoomError_NoExist;
	}

	if(m_PlayerMap.find(nPlayerId) != m_PlayerMap.end())
	{
		return RoomError_InRoom;
	}

	CChessTable* pTable = dynamic_cast<CChessTable*>(iter->second);
	if(!pTable || pTable->HasDisband())
		return RoomError_Unknown;

	if((pTable->GetPlayerNum() >= pTable->GetMaxPlayers()) /*|| (nSlot >= pTable->GetMaxPlayers())*/)
		return RoomError_MaxPlayer;

	//if(pTable->HasPlayer(nSlot))
	//	return RoomError_HasPlayer;

	if(pTable->GetGameStatus() != GS_MJ_FREE)
		return RoomError_Playing;

	for(int i = 0; i < GAME_PLAYER; i++)
	{
		if(!pTable->HasPlayer(i))
		{
			pTable->InsertPlayer(nPlayerId, i);
			m_PlayerMap.insert(PLAYER_MAP::value_type(nPlayerId, nTableId));
			nSlot = i;
			pTable->m_strUserUrlImg[nSlot] = HeadImgUrl;
			pTable->m_strUserNickName[nSlot] = nickname;
			AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(pTable->GetPlayer(i));

			CMemGuard Buffer(32 MEM_GUARD_PARAM);
			Base::BitStream sendPacket(Buffer.get(), 32);
			stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, "WC_sendChairId", pAccount->GetAccountId(), SERVICE_CLIENT, nSlot);
			pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
			SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
			break;
		}

	}
	
	
	if(pTable->GetPlayerNum() == pTable->GetMaxPlayers())
	{
		pTable->ResetTable();
		pTable->OnEventGameStart();
		for(int i = 0; i < GAME_PLAYER; ++i)
		{
			AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(pTable->GetPlayer(i));

			CMemGuard Buffer(32 MEM_GUARD_PARAM);
			Base::BitStream sendPacket(Buffer.get(), 32);
			stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, "WC_StartGame", pAccount->GetAccountId(), SERVICE_CLIENT, 1);
			pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
			SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
		}

	}

	for(int i = 0; i < GAME_PLAYER; ++i)
	{
		AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(pTable->GetJoinPlayer(i));
		if (NULL == pAccount || pTable->GetJoinPlayer(i) == nPlayerId )
			continue;

		CMemGuard Buffer(5120 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 5120);
		stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, "WC_ReadyChessTable", pAccount->GetAccountId(), SERVICE_CLIENT, RoomError_None, i);
		sendPacket.writeInt(pTable->GetGameStatus(), Base::Bit32);
		sendPacket.writeInt(pTable->GetPlayer(0),Base::Bit32);
		sendPacket.writeInt(pTable->GetPlayer(1),Base::Bit32);
		sendPacket.writeInt(pTable->GetPlayer(2),Base::Bit32);
		sendPacket.writeInt(pTable->GetPlayer(3),Base::Bit32);
		sendPacket.writeString(pTable->m_strUserUrlImg[0].c_str(), 1024);
		sendPacket.writeString(pTable->m_strUserNickName[0].c_str(), 64);
		sendPacket.writeString(pTable->m_strUserUrlImg[1].c_str(), 1024);
		sendPacket.writeString(pTable->m_strUserNickName[1].c_str(), 64);
		sendPacket.writeString(pTable->m_strUserUrlImg[2].c_str(), 1024);
		sendPacket.writeString(pTable->m_strUserNickName[2].c_str(), 64);
		sendPacket.writeString(pTable->m_strUserUrlImg[3].c_str(), 1024);
		sendPacket.writeString(pTable->m_strUserNickName[3].c_str(), 64);

		pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}

	return RoomError_None;
}

enRoomError CRoomMgr::RestartChessTable(U32 nPlayerId, U64 nTableId)
{
	TABLE_ITR iter = m_TableMap.find(nTableId);
	if(iter == m_TableMap.end())
	{
		return RoomError_NoExist;
	}

	if(m_PlayerMap.find(nPlayerId) == m_PlayerMap.end())
	{
		return RoomError_OutRoom;
	}

	CChessTable* pTable = dynamic_cast<CChessTable*>(iter->second);
	if(!pTable)
		return RoomError_Unknown;

	if(pTable->GetPlayerSlot(nPlayerId) >= pTable->GetMaxPlayers())
		return RoomError_OutRoom;

	if(pTable->GetGameStatus() != GS_MJ_FREE)
		return RoomError_Playing;

	if(pTable->ReadyGame(pTable->GetPlayerSlot(nPlayerId)))
	{
		pTable->ResetTable();
		pTable->OnEventGameStart();
	}
	return RoomError_None;
}

enRoomError CRoomMgr::LeaveChessTable(U32 nPlayerId, U64 nTableId)
{
	TABLE_ITR iter = m_TableMap.find(nTableId);
	if(iter == m_TableMap.end())
	{
		return RoomError_NoExist;
	}
	
	CChessTable* pTable = dynamic_cast<CChessTable*>(iter->second);
	if(!pTable)
		return RoomError_Unknown;

	if(pTable->hasRule(CChessTable::GAME_TYPE_ZZ_START))
		return RoomError_Playing;

	if(pTable->GetPlayer(0) == nPlayerId)
	{
		pTable->AddGameRule(BIT(CChessTable::GAME_TYPE_ZZ_DISBAND));
		pTable->DisbandGameSendtoPlayer(pTable);
		return RoomError_None;
	}
	pTable->DelJoinPlayer(nPlayerId);
	pTable->DelPlayer(nPlayerId);
	if(m_PlayerMap.find(nPlayerId) != m_PlayerMap.end())
	{
		m_PlayerMap.erase(nPlayerId);
	}
	
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerId);
	CMemGuard Buffer(32 MEM_GUARD_PARAM);
	Base::BitStream sendPacket(Buffer.get(), 32);
	stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, "WC_LeaveChessTable", pAccount->GetAccountId(), SERVICE_CLIENT, RoomError_None);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
	SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	return RoomError_None;
}

enRoomError CRoomMgr::DisbandChessTable(U32 nPlayerId, U64 nTableId, U8 nAgreeFlag)
{
	TABLE_ITR iter = m_TableMap.find(nTableId);
	if(iter == m_TableMap.end())
	{
		return RoomError_NoExist;
	}

	CChessTable* pTable = dynamic_cast<CChessTable*>(iter->second);
	if(!pTable)
		return RoomError_Unknown;

	U16 nSlot = pTable->GetPlayerSlot(nPlayerId);
	if(nSlot >= pTable->GetMaxPlayers())
		return RoomError_OutRoom;

	if(pTable->DisbandGame(nSlot,nAgreeFlag) == 1)
	{
		//解散
		pTable->AddGameRule(BIT(CChessTable::GAME_TYPE_ZZ_DISBAND));
		for(int i = 0; i < pTable->GetMaxPlayers(); ++i)
		{

			U32 nPlayerId = pTable->GetPlayer(i);
			if(nPlayerId)
			{
				AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerId);
				if(pAccount)
				{
					CMemGuard Buffer(256 MEM_GUARD_PARAM);
					Base::BitStream sendPacket(Buffer.get(), 256);
					stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, "WC_RefuseDisbandChessTable", pAccount->GetAccountId(), SERVICE_CLIENT,1);
					for(int j = 0; j < pTable->GetMaxPlayers(); ++j)
					{
						for(int k = 0; k < 3; ++k)
						{
							sendPacket.writeInt(pTable->m_cbChiHuNum[j][k],  Base::Bit8);
						}
						S64 score = pTable->getTableScore(j);
						sendPacket.writeBits(sizeof(score) << 3, &score);
					}
					
					pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
					SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
				}
			}

		}
	}
	else if(pTable->DisbandGame(nSlot,nAgreeFlag) == 2)
	{
		for(int i = 0; i < pTable->GetMaxPlayers(); ++i)
		{

			U32 nPlayerId = pTable->GetPlayer(i);
			if(nPlayerId)
			{
				AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerId);
				if(pAccount)
				{
					CMemGuard Buffer(64 MEM_GUARD_PARAM);
					Base::BitStream sendPacket(Buffer.get(), 64);
					stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, "WC_RefuseDisbandChessTable", pAccount->GetAccountId(), SERVICE_CLIENT,0);

					pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
					SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
				}
			}

		}

		pTable->ClearDisbandGame(); 
	}
	else
	{
		for(int i = 0; i < pTable->GetMaxPlayers(); ++i)
		{
			//if(!pTable->AgreeDisband(i))
			//{
				U32 nPlayerId = pTable->GetPlayer(i);
				if(nPlayerId)
				{
					AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerId);
					if(pAccount)
					{
						CMemGuard Buffer(64 MEM_GUARD_PARAM);
						Base::BitStream sendPacket(Buffer.get(), 64);
						stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, "WC_DisbandChessTable", pAccount->GetAccountId(), SERVICE_CLIENT);
						for(int j = 0; j < pTable->GetMaxPlayers(); ++j)
						{
							sendPacket.writeInt(pTable->AgreeDisband(j),  Base::Bit8);
						}
						pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
						SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
					}
				}
			//}
		}
	}

	return RoomError_None;
}

CChessTable* CRoomMgr::FindChessTable(U64 nTableId)
{
	TABLE_ITR iter = m_TableMap.find(nTableId);
	if(iter != m_TableMap.end())
	{
		return dynamic_cast<CChessTable*>(iter->second);
	}
	return NULL;
}

CChessTable* CRoomMgr::FindChessTableByPlayerId(U32 nPlayerId)
{
	PLAYER_ITR iter = m_PlayerMap.find(nPlayerId);
	if(iter != m_PlayerMap.end())
	{
		return FindChessTable(iter->second);
	}
	return NULL;
}

CRoomMgr* CRoomMgr::Instance()
{
	static CRoomMgr s_RoomMgr;
	return &s_RoomMgr;
}


U64 CRoomMgr::AssignId()
{
	while(true)
	{
		m_ID = gRandGen.randI(100000, 999999);
		if(m_TableMapEx.find(m_ID) == m_TableMapEx.end())
		{
			m_TableMapEx.insert(TABLE_MAP_EX::value_type(m_ID, true));
			break;
		}
	}
	return (m_ID);
}

bool CRoomMgr::HandleCreateChessTable(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet)
{
	U32 nPlayerId = pHead->Id;
	U64 nTabletId = 0;
	U32 nGmaeType = pHead->DestZoneId;
	U32 nCreateType = pHead->SrcZoneId;//(0:索胡，1：长胡（8）， 2：长胡（16）)
	S32 nScore = 0;
	Packet->readBits(sizeof(nScore) << 3, &nScore);
	char HeadImgUrl[1024] = "";
	char nickname[64] = "";
	Packet->readString(HeadImgUrl, 1024);
	Packet->readString(nickname, 64);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerId);
	if(pAccount)
	{
		enRoomError eError = CreateChessTable(nPlayerId, nTabletId, nGmaeType, nCreateType, nScore,HeadImgUrl,nickname);
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, "WC_CreateChessTable", pAccount->GetAccountId(), SERVICE_CLIENT, eError);
		if(sendPacket.writeFlag(eError == RoomError_None))
		{
			sendPacket.writeBits(sizeof(nTabletId) << 3, &nTabletId);
		}
		pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CRoomMgr::HandleJoinChessTable(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet)
{
	U32 nPlayerId = pHead->Id;
	U64 nTabletId = 0;
	Packet->readBits(sizeof(nTabletId) << 3, &nTabletId);

	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerId);
	if(pAccount)
	{
		CChessTable* pTable = FindChessTable(nTabletId);
		if(pTable)
		{
			if(!pTable->HasDisband())
			{
				U32 count = pTable->GetJoinPlayerNum();
				if(count > 4)
					return false;
				for(int i = 0; i < GAME_PLAYER; i++)
				{
					if(!pTable->HasJoinPlayer(i))
					{
						pTable->InsertJoinPlayer(nPlayerId,i);
						break;
					}
				}
				
				CMemGuard Buffer(256 MEM_GUARD_PARAM);
				Base::BitStream sendPacket(Buffer.get(), 256);
				stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, "WC_JoinChessTable", pAccount->GetAccountId(), SERVICE_CLIENT);
				sendPacket.writeInt(pTable->GetGameStatus(), Base::Bit32);
				sendPacket.writeInt(pTable->GetPlayer(0),Base::Bit32);
				sendPacket.writeInt(pTable->GetPlayer(1),Base::Bit32);
				sendPacket.writeInt(pTable->GetPlayer(2),Base::Bit32);
				sendPacket.writeInt(pTable->GetPlayer(3),Base::Bit32);
				U32 nGameType = 0;
				if(pTable->hasRule(CChessGX::GAME_TYPE_ZZ_MOBAO))
				{
					nGameType = 1;
				}
				sendPacket.writeInt(nGameType, Base::Bit32);
				U32 nCreateType = 0;
				S64 nScore = 0;
				U32 nTableNum = 0;
				if(pTable->hasRule(CChessTable::GAME_TYPE_ZZ_CHANGHU))
				{
					nCreateType = 1;
				}
				if(pTable->hasRule(CChessTable::GAME_TYPE_ZZ_SUOHU))
				{
					nCreateType = 0;
				}

				nScore = pTable->getTableScore(0);
				nTableNum = pTable->GetTableNum();

				sendPacket.writeInt(nCreateType, Base::Bit32);
				sendPacket.writeInt(nTableNum, Base::Bit32);
				sendPacket.writeBits(sizeof(nScore) << 3, &nScore);

				pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
				SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
			}
		}
	}
	return true;
}

bool CRoomMgr::HandleReadyChessTable(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet)
{
	U32 nPlayerId = pHead->Id;
	U32 nSlot = 0;//pHead->DestZoneId;
	U64 nTabletId = 0;
	Packet->readBits(sizeof(nTabletId) << 3, &nTabletId);
	char HeadImgUrl[1024] = "";
	char nickname[64] = "";
	Packet->readString(HeadImgUrl, 1024);
	Packet->readString(nickname, 64);

	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerId);
	if(pAccount)
	{
		enRoomError eError = ReadyChessTable(nPlayerId, nTabletId, nSlot,HeadImgUrl,nickname);
		if(eError == RoomError_None)
		{
			CChessTable* pTable = FindChessTable(nTabletId);

			CMemGuard Buffer(5120 MEM_GUARD_PARAM);
			Base::BitStream sendPacket(Buffer.get(), 5120);
			stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, "WC_ReadyChessTable", pAccount->GetAccountId(), SERVICE_CLIENT, eError, nSlot);

			sendPacket.writeInt(pTable->GetGameStatus(), Base::Bit32);
			sendPacket.writeInt(pTable->GetPlayer(0),Base::Bit32);
			sendPacket.writeInt(pTable->GetPlayer(1),Base::Bit32);
			sendPacket.writeInt(pTable->GetPlayer(2),Base::Bit32);
			sendPacket.writeInt(pTable->GetPlayer(3),Base::Bit32);
			sendPacket.writeString(pTable->m_strUserUrlImg[0].c_str(), 1024);
			sendPacket.writeString(pTable->m_strUserNickName[0].c_str(), 64);
			sendPacket.writeString(pTable->m_strUserUrlImg[1].c_str(), 1024);
			sendPacket.writeString(pTable->m_strUserNickName[1].c_str(), 64);
			sendPacket.writeString(pTable->m_strUserUrlImg[2].c_str(), 1024);
			sendPacket.writeString(pTable->m_strUserNickName[2].c_str(), 64);
			sendPacket.writeString(pTable->m_strUserUrlImg[3].c_str(), 1024);
			sendPacket.writeString(pTable->m_strUserNickName[3].c_str(), 64);
			pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
			SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
		}

	}
	return true;
}

bool CRoomMgr::HandleRestartChessTable(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet)
{
	U32 nPlayerId = pHead->Id; 
	U64 nTabletId = 0;
	Packet->readBits(sizeof(nTabletId) << 3, &nTabletId);

	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerId);
	if(pAccount)
	{
		enRoomError eError = RestartChessTable(nPlayerId, nTabletId);
		CMemGuard Buffer(32 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 32);
		stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, "WC_RestartChessTable", pAccount->GetAccountId(), SERVICE_CLIENT, eError);
		pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CRoomMgr::HandleLeaveChessTable(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet)
{
	U32 nPlayerId = pHead->Id;
	U64 nTabletId = 0;
	Packet->readBits(sizeof(nTabletId) << 3, &nTabletId);

	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerId);
	if(pAccount)
	{
		enRoomError eError = LeaveChessTable(nPlayerId, nTabletId);

	}
	return true;
}

bool CRoomMgr::HandleDisbandChessTable(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet)
{
	U32 nPlayerId = pHead->Id;
	U64 nTabletId = 0;
	Packet->readBits(sizeof(nTabletId) << 3, &nTabletId);
	U8 nAgreeFlag = Packet->readInt(Base::Bit8);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerId);
	if(pAccount)
	{
		enRoomError eError = DisbandChessTable(nPlayerId, nTabletId, nAgreeFlag);
	}
	return true;
}

bool CRoomMgr::HandleUserOutCard(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet)
{
	U32 nPlayerId = pHead->Id;
	U64 nTabletId = 0;
	Packet->readBits(sizeof(nTabletId) << 3, &nTabletId);
	U8  cbCardData = Packet->readInt(Base::Bit8);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerId);
	if(pAccount)
	{
		CChessTable* pNewTable = FindChessTable(nTabletId);
		//桌子有效
		if(pNewTable)
		{
			//玩家有效,并且是当前用户
			U16 nSlot = pNewTable->GetPlayerSlot(nPlayerId);
			if(nSlot < pNewTable->GetMaxPlayers() && pNewTable->IsCurrentUser(nSlot))
			{
				//验证牌的有效性
				if(pNewTable->IsUserVaildCard(nSlot, cbCardData))
				{
					pNewTable->OnUserOutCard(nSlot, cbCardData);
					CMemGuard Buffer(32 MEM_GUARD_PARAM);
					Base::BitStream sendPacket(Buffer.get(), 32);
					stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, "WC_UserOutCard", pAccount->GetAccountId(), SERVICE_CLIENT, cbCardData);
					pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
					SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
				}
			}
		}
	}
	return true;
}

bool CRoomMgr::HandleUserOperateCard(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet)
{
	U32 nPlayerId = pHead->Id;
	U64 nTabletId = 0;
	Packet->readBits(sizeof(nTabletId) << 3, &nTabletId);
	U8  cbOperateCode = Packet->readInt(Base::Bit8);
	U8  cbOperateCard = Packet->readInt(Base::Bit8);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerId);
	if(pAccount)
	{
		CChessTable* pNewTable = FindChessTable(nTabletId);
		//桌子有效
		if(pNewTable)
		{
			//玩家有效,并且是当前用户
			U16 nSlot = pNewTable->GetPlayerSlot(nPlayerId);
			if(nSlot < pNewTable->GetMaxPlayers())
			{
				//验证牌的有效性
				//if(pNewTable->IsUserVaildCard(nSlot, cbOperateCard))
				{
					pNewTable->OnUserOperateCard(nSlot, cbOperateCode, cbOperateCard);
					CMemGuard Buffer(32 MEM_GUARD_PARAM);
					Base::BitStream sendPacket(Buffer.get(), 32);
					stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, "WC_UserOperateCard", pAccount->GetAccountId(), SERVICE_CLIENT, cbOperateCode, cbOperateCard);
					pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
					SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
				}
			}
		}
	}
	return true;
}
