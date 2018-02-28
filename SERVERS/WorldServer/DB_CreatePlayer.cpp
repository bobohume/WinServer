#include "stdafx.h"
#include "DB_CreatePlayer.h"
#include "DBLayer/common/DBUtility.h"
#include "DBLayer/data/TBLPlayer.h"
#include "DBContext.h"
#include "WorldServer.h"
#include "ServerMgr.h"
#include "PlayerMgr.h"
#include "GamePlayLog.h"
#include "wintcp/dtServerSocket.h"
#include "wintcp/asyncSocket.h"

DB_CreatePlayer::DB_CreatePlayer(void)
{
    m_error         = 0;
    m_playerPos     = 0;
    m_accountStatus = 0;
    m_isOpenedTime  = 0;
}

int DB_CreatePlayer::Execute(int ctxId,void* param)
{
    if (!m_spData)
        return PACKET_ERROR;

    int playerId = m_spData->PlayerId;

    if(DB_CONTEXT_NOMRAL == ctxId)
    {
        CDBConn* pDBConn = (CDBConn*)param;
        assert(0 != pDBConn);

        TBLPlayer tb_player(0);
        tb_player.AttachConn(pDBConn);
        DBError err = tb_player.Create_Simple(m_spData.get());

        if(err != DBERR_NONE)
        {
            g_Log.WriteError("创建角色数据失败[AccountId = %d, PlayerName=%s]", m_spData->AccountId, m_spData->PlayerName);
            m_error = PLAYER_NAME_ERROR;
        }
        else
        {
            if (!m_isOpenedTime)
            {
                //设置帐号标识,存在预创建角色
                TBLAccount tblAccount(0);
                tblAccount.AttachConn(pDBConn);
                tblAccount.SetAccountStatus(m_spData->AccountId,m_accountStatus | EAS_PRECREATEREWARD);
            }
        }
        
        PostLogicThread(this);
        return PACKET_NOTREMOVE;
    }
    else
    {
        AccountRef spAccount = SERVER->GetPlayerManager()->GetAccount(m_spData->AccountId);

		if(!spAccount)
		{
			return PACKET_OK;
		}

        if (0 == m_error)
        {
            spAccount->m_newPlayerId = playerId;
            
            //添加角色数据	
            spAccount->CurPlayerPos = m_playerPos;
			PlayerInfo info;
			info.playerID = playerId;
            spAccount->m_playerIdList[m_playerPos]  = info;

            if (!m_isOpenedTime)
            {
                spAccount->status |= EAS_PRECREATEREWARD;
            }
        }
        else
        {
            spAccount->PlayerNum--;

            g_Log.WriteLog("通知RA创建角色");

            //通知远程服务器删除角色
            char buf[128];
            Base::BitStream remotePacket(buf, sizeof(buf));
            stPacketHead* pSendHead = IPacket::BuildPacketHead(remotePacket, "WORLD_REMOTE_DeletePlayer", playerId);
            SERVER->GetRemoteAccountSocket()->Send(remotePacket);
        }
        
        //发送反馈给客户端
        char buf[MAX_PACKET_SIZE];
        Base::BitStream sendPacket(buf,sizeof(buf));
        stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, "CLIENT_WORLD_CreatePlayerResponse", m_spData->AccountId, SERVICE_CLIENT);
		//---------ylb web-------//
		sendPacket.writeInt(m_error, Base::Bit16);
		sendPacket.writeInt(spAccount->UID, UID_BITS);
		sendPacket.writeInt(playerId, Base::Bit32);
        /*sendPacket.writeInt(spAccount->UID, UID_BITS);
        sendPacket.writeInt(m_error, Base::Bit16);
		sendPacket.writeInt(playerId, Base::Bit32);*/

        SERVER->GetServerSocket()->Send(spAccount->GetGateSocketId(), sendPacket);
    }

    return PACKET_OK;
}