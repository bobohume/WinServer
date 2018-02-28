#include "stdafx.h"
#include "DBContext.h"
#include "DB_LoadPlayer.h"
#include "Common/PlayerStruct.h"
#include "DBLayer/common/DBUtility.h"
#include "data/TBLBase.h"
#include "WorldServer.h"
#include "PlayerMgr.h"
#include "AccountHandler.h"
#include "ServerMgr.h"
#include "WINTCP/dtServerSocket.h"

DB_LoadPlayer::DB_LoadPlayer(void)
{
	m_playerId = 0;
	m_error = 0;
	m_pPlayer.Clear();
}

int DB_LoadPlayer::Execute(int ctxId,void* param)
{
	if(ctxId == DB_CONTEXT_NOMRAL)
	{
		do 
		{
			CDBConn* pDBConn = (CDBConn*)param;
			assert(0 != pDBConn);

			DBError err = DBERR_UNKNOWERR;
			int iColSize	=	0;
			U32 nPlayerId	=	0;

			//读取玩家基本信息
			std::stringstream s;
			s << "SELECT AccountID,PlayerID,PlayerName,Sex,[Level],Family,FirstClass,ZoneId,PlayerFlag,\
				 LastLoginTime,LastLogoutTime,UnuseField0,UnuseField1,UnuseField2 FROM Tbl_Player WHERE PlayerID = "
			  << m_playerId;
			pDBConn->SQLExt(s.str().c_str());
			if(pDBConn->More())
			{
				m_pPlayer.BaseData.AccountId		= pDBConn->GetInt();
				m_pPlayer.BaseData.PlayerId			= pDBConn->GetInt();
				strcpy_s(m_pPlayer.BaseData.PlayerName, MAX_NAME_LENGTH, pDBConn->GetString());
				m_pPlayer.BaseData.Sex				= pDBConn->GetInt();
				m_pPlayer.BaseData.Level			= pDBConn->GetInt();
				m_pPlayer.BaseData.Family			= pDBConn->GetInt();
				m_pPlayer.BaseData.FirstClass		= pDBConn->GetInt();
				m_pPlayer.BaseData.ZoneId			= pDBConn->GetInt();
				m_pPlayer.BaseData.PlayerFlag		= pDBConn->GetInt();
				m_pPlayer.BaseData.LastLoginTime	= pDBConn->GetTime();
				m_pPlayer.BaseData.LastLogoutTime	= pDBConn->GetTime();
				m_pPlayer.BaseData.UnuseField[0]	= pDBConn->GetInt();
				m_pPlayer.BaseData.UnuseField[1]	= pDBConn->GetInt();
				m_pPlayer.BaseData.UnuseField[2]	= pDBConn->GetInt();
			}
			else
			{
				m_error = DB_LOAD_TBL_PALYER_ERROR;
				g_Log.WriteError("读取玩家 [%d] 基本数据结构错误，错误类型 [%d] ", m_playerId, m_error);
				break;
			}

			//读取玩家basicInfo信息
			iColSize = 0;
			s.clear();
			s.str("");
			s << "SELECT * FROM Tbl_PlayerBasicInfo WHERE PlayerID = "
				 << m_playerId;
			pDBConn->SQLExt(s.str().c_str());
			if(pDBConn->More())
			{
				nPlayerId									= pDBConn->GetInt();
				m_pPlayer.MainData.PartData.Money			= pDBConn->GetInt();
				m_pPlayer.MainData.PartData.Gold			= pDBConn->GetInt();
				m_pPlayer.MainData.PartData.GateId			= pDBConn->GetInt();
				m_pPlayer.MainData.PartData.HP				= pDBConn->GetInt();
				m_pPlayer.MainData.PartData.STR				= pDBConn->GetInt();
				m_pPlayer.MainData.PartData.Exp				= pDBConn->GetBigInt();
				m_pPlayer.MainData.PartData.BagSize			= pDBConn->GetInt();
				m_pPlayer.MainData.PartData.BankSize		= pDBConn->GetInt();
				m_pPlayer.MainData.PartData.BindMoney		= pDBConn->GetInt();
				m_pPlayer.MainData.PartData.UnuseField[0]	= pDBConn->GetInt();
				m_pPlayer.MainData.PartData.UnuseField[1]	= pDBConn->GetInt();
				m_pPlayer.MainData.PartData.UnuseField[2]	= pDBConn->GetInt();
			}
			else
			{
				m_error = DB_LOAD_TBL_PALYERBASICINFO_ERROR;
				g_Log.WriteError("读取玩家 [%d] 基本数据结构错误，错误类型 [%d] ", m_playerId, m_error);
				break;
			}

			//读取玩家buff信息
			iColSize = 0;
			s.clear();
			s.str("");
			s << "SELECT * FROM Tbl_Buff WHERE PlayerID = "
				<< m_playerId;
			pDBConn->SQLExt(s.str().c_str());
			if(pDBConn->More())
			{
				nPlayerId											= pDBConn->GetInt();
				m_pPlayer.DispData.BuffInfo[iColSize].buffId		= pDBConn->GetInt();
				m_pPlayer.DispData.BuffInfo[iColSize].count			= pDBConn->GetInt();
				m_pPlayer.DispData.BuffInfo[iColSize].effTimes		= pDBConn->GetInt();
				++iColSize;
			}

			//读取玩家装备信息
			iColSize = 0;
			s.clear();
			s.str("");
			s << "SELECT * FROM Tbl_Equip WHERE PlayerID = "
				<< m_playerId;
			pDBConn->SQLExt(s.str().c_str());
			while(pDBConn->More())
			{
				std::string EquipUID								= pDBConn->GetString();
				nPlayerId											= pDBConn->GetInt();
				U32 nItemId											= pDBConn->GetInt();
				iColSize											= 0;//EQUIPMGR->getEquipPose(nItemId);
				dStrcpy(m_pPlayer.MainData.EquipInfo[iColSize].UID, MAX_UUID_LENGTH, EquipUID.c_str());
				m_pPlayer.MainData.EquipInfo[iColSize].ItemID		= nItemId;
				m_pPlayer.MainData.EquipInfo[iColSize].Quantity		= pDBConn->GetInt();
				m_pPlayer.MainData.EquipInfo[iColSize].RemainUseTimes	= pDBConn->GetInt();
				m_pPlayer.MainData.EquipInfo[iColSize].Quality		= pDBConn->GetInt();
				m_pPlayer.MainData.EquipInfo[iColSize].CurWear		= pDBConn->GetInt();
				m_pPlayer.MainData.EquipInfo[iColSize].Exp			= pDBConn->GetInt();
				m_pPlayer.MainData.EquipInfo[iColSize].Level		= pDBConn->GetInt();
				m_pPlayer.MainData.EquipInfo[iColSize].ActiveFlag	= pDBConn->GetInt();
				m_pPlayer.MainData.EquipInfo[iColSize].EquipStrengthens	= pDBConn->GetInt();
				m_pPlayer.MainData.EquipInfo[iColSize].EmbedSlot[0]	= pDBConn->GetInt();
				m_pPlayer.MainData.EquipInfo[iColSize].EmbedSlot[1]	= pDBConn->GetInt();
				m_pPlayer.MainData.EquipInfo[iColSize].EmbedSlot[2]	= pDBConn->GetInt();
				m_pPlayer.MainData.EquipInfo[iColSize].EmbedSlot[3]	= pDBConn->GetInt();
				m_pPlayer.MainData.EquipInfo[iColSize].EmbedSlot[4]	= pDBConn->GetInt();
				m_pPlayer.MainData.EquipInfo[iColSize].EmbedSlot[5]	= pDBConn->GetInt();
				m_pPlayer.MainData.EquipInfo[iColSize].EmbedSlot[6]	= pDBConn->GetInt();
				m_pPlayer.MainData.EquipInfo[iColSize].EmbedSlot[7]	= pDBConn->GetInt();
				m_pPlayer.MainData.EquipInfo[iColSize].EmbedSlot[8]	= pDBConn->GetInt();
				m_pPlayer.MainData.EquipInfo[iColSize].EmbedSlot[9]	= pDBConn->GetInt();
				iColSize++;
			}

			//读取玩家物品信息
			iColSize = 0;
			s.clear();
			s.str("");
			s << "SELECT * FROM Tbl_Item WHERE PlayerID = "
				<< m_playerId;
			pDBConn->SQLExt(s.str().c_str());
			while(pDBConn->More())
			{
				dStrcpy(m_pPlayer.MainData.InventoryInfo[iColSize].UID, MAX_UUID_LENGTH, pDBConn->GetString());
				nPlayerId											= pDBConn->GetInt();
				m_pPlayer.MainData.InventoryInfo[iColSize].ItemID		= pDBConn->GetInt();
				m_pPlayer.MainData.InventoryInfo[iColSize].Quantity		= pDBConn->GetInt();
				m_pPlayer.MainData.InventoryInfo[iColSize].RemainUseTimes	= pDBConn->GetInt();
				m_pPlayer.MainData.InventoryInfo[iColSize].Quality		= pDBConn->GetInt();
				m_pPlayer.MainData.InventoryInfo[iColSize].CurWear		= pDBConn->GetInt();
				m_pPlayer.MainData.InventoryInfo[iColSize].Exp			= pDBConn->GetInt();
				m_pPlayer.MainData.InventoryInfo[iColSize].Level			= pDBConn->GetInt();
				m_pPlayer.MainData.InventoryInfo[iColSize].ActiveFlag	= pDBConn->GetInt();
				m_pPlayer.MainData.InventoryInfo[iColSize].EquipStrengthens	= pDBConn->GetInt();
				m_pPlayer.MainData.InventoryInfo[iColSize].EmbedSlot[0]	= pDBConn->GetInt();
				m_pPlayer.MainData.InventoryInfo[iColSize].EmbedSlot[1]	= pDBConn->GetInt();
				m_pPlayer.MainData.InventoryInfo[iColSize].EmbedSlot[2]	= pDBConn->GetInt();
				m_pPlayer.MainData.InventoryInfo[iColSize].EmbedSlot[3]	= pDBConn->GetInt();
				m_pPlayer.MainData.InventoryInfo[iColSize].EmbedSlot[4]	= pDBConn->GetInt();
				m_pPlayer.MainData.InventoryInfo[iColSize].EmbedSlot[5]	= pDBConn->GetInt();
				m_pPlayer.MainData.InventoryInfo[iColSize].EmbedSlot[6]	= pDBConn->GetInt();
				m_pPlayer.MainData.InventoryInfo[iColSize].EmbedSlot[7]	= pDBConn->GetInt();
				m_pPlayer.MainData.InventoryInfo[iColSize].EmbedSlot[8]	= pDBConn->GetInt();
				m_pPlayer.MainData.InventoryInfo[iColSize].EmbedSlot[9]	= pDBConn->GetInt();
				iColSize++;
			}

			//读取仓库物品信息
			iColSize = 0;
			s.clear();
			s.str("");
			s << "SELECT * FROM Tbl_Bank WHERE PlayerID = "
				<< m_playerId;
			pDBConn->SQLExt(s.str().c_str());
			while(pDBConn->More())
			{
				dStrcpy(m_pPlayer.MainData.BankInfo[iColSize].UID, MAX_UUID_LENGTH, pDBConn->GetString());
				nPlayerId											= pDBConn->GetInt();
				m_pPlayer.MainData.BankInfo[iColSize].ItemID		= pDBConn->GetInt();
				m_pPlayer.MainData.BankInfo[iColSize].Quantity		= pDBConn->GetInt();
				m_pPlayer.MainData.BankInfo[iColSize].RemainUseTimes	= pDBConn->GetInt();
				m_pPlayer.MainData.BankInfo[iColSize].Quality		= pDBConn->GetInt();
				m_pPlayer.MainData.BankInfo[iColSize].CurWear		= pDBConn->GetInt();
				m_pPlayer.MainData.BankInfo[iColSize].Exp			= pDBConn->GetInt();
				m_pPlayer.MainData.BankInfo[iColSize].Level			= pDBConn->GetInt();
				m_pPlayer.MainData.BankInfo[iColSize].ActiveFlag	= pDBConn->GetInt();
				m_pPlayer.MainData.BankInfo[iColSize].EquipStrengthens	= pDBConn->GetInt();
				m_pPlayer.MainData.BankInfo[iColSize].EmbedSlot[0]	= pDBConn->GetInt();
				m_pPlayer.MainData.BankInfo[iColSize].EmbedSlot[1]	= pDBConn->GetInt();
				m_pPlayer.MainData.BankInfo[iColSize].EmbedSlot[2]	= pDBConn->GetInt();
				m_pPlayer.MainData.BankInfo[iColSize].EmbedSlot[3]	= pDBConn->GetInt();
				m_pPlayer.MainData.BankInfo[iColSize].EmbedSlot[4]	= pDBConn->GetInt();
				m_pPlayer.MainData.BankInfo[iColSize].EmbedSlot[5]	= pDBConn->GetInt();
				m_pPlayer.MainData.BankInfo[iColSize].EmbedSlot[6]	= pDBConn->GetInt();
				m_pPlayer.MainData.BankInfo[iColSize].EmbedSlot[7]	= pDBConn->GetInt();
				m_pPlayer.MainData.BankInfo[iColSize].EmbedSlot[8]	= pDBConn->GetInt();
				m_pPlayer.MainData.BankInfo[iColSize].EmbedSlot[9]	= pDBConn->GetInt();
				iColSize++;
			}

			//读取玩家宠物信息
			iColSize = 0;
			s.clear();
			s.str("");
			s << "SELECT * FROM Tbl_Pet WHERE PlayerID = "
				<< m_playerId;
			pDBConn->SQLExt(s.str().c_str());
			while(pDBConn->More())
			{
				dStrcpy(m_pPlayer.MainData.PetInfo[iColSize].UID, MAX_UUID_LENGTH, pDBConn->GetString());
				nPlayerId											= pDBConn->GetInt();
				m_pPlayer.MainData.PetInfo[iColSize].petDataId		= pDBConn->GetInt();
				m_pPlayer.MainData.PetInfo[iColSize].curHP			= pDBConn->GetInt();
				m_pPlayer.MainData.PetInfo[iColSize].exp			= pDBConn->GetInt();
				m_pPlayer.MainData.PetInfo[iColSize].level			= pDBConn->GetInt();
				iColSize++;
			}

			//读取玩家活动信息
			iColSize = 0;
			s.clear();
			s.str("");
			s << "SELECT * FROM Tbl_TimeSet WHERE PlayerID = "
				<< m_playerId;
			pDBConn->SQLExt(s.str().c_str());
			while(pDBConn->More())
			{
				m_pPlayer.DispData.TimeSet[iColSize].id			= pDBConn->GetInt();
				nPlayerId										= pDBConn->GetInt();
				m_pPlayer.DispData.TimeSet[iColSize].flag1		= pDBConn->GetInt();
				m_pPlayer.DispData.TimeSet[iColSize].flag2		= pDBConn->GetInt();
				m_pPlayer.DispData.TimeSet[iColSize].expiretime	= pDBConn->GetInt();
				m_pPlayer.DispData.TimeSet[iColSize].finishFlag	= pDBConn->GetInt();
				++iColSize;
			}

			PostLogicThread(this);
			return PACKET_NOTREMOVE;
		} while (false);
	}
	else
	{
		if(m_error == DB_LOAD_NONE)
		{
			AccountRef  pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(m_playerId);
			if(pAccount)
			{
				pAccount->SetPlayerId(m_playerId);
				//获取完数据库
				{
					char buf[64]; 
					Base::BitStream sendPacket(buf,sizeof(buf));
					stPacketHead *pSendHead = IPacket::BuildPacketHead(sendPacket,"CLIENT_GAME_LoginResponse",pAccount->GetAccountId(),SERVICE_CLIENT);
					sendPacket.writeInt(NONE_ERROR,Base::Bit16);

					//int gateSocketId = SERVER->GetServerManager()->GetServerSocket(SERVICE_GATESERVER, 1, 1);
					pSendHead->PacketSize = sendPacket.getPosition()-IPacket::GetHeadSize();
					SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(),sendPacket);
				}
			}
		}

	}

	return PACKET_OK;
}