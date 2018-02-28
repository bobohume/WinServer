//#include "stdafx.h"
#include "DB_LoginPlayer.h"
#include "DBContext.h"

DB_LoginPlayer::DB_LoginPlayer()
:account(0), socketId(0), areaId(0), playerId(0), loginAward(false)
{
	strcpy_s(IP, sizeof(IP), "");
	strcpy_s(MachineUID, sizeof(MachineUID), "");
}

int DB_LoginPlayer::Execute(int ctxId, void* param)
{
	if (DB_CONTEXT_NOMRAL == ctxId)
	{
		CDBConn* pDBConn = (CDBConn*)param;
		IF_ASSERT(pDBConn == NULL)
			return PACKET_ERROR; //TODO: 检查是否释放资源

		int bonusCount = 0;
		int NetBarLv = 0;
		int accountVipLv = 0;

		if (loginAward)
		{
			if (strcmp(MachineUID, "") != 0)
			{
				//登陆次数活动和帐号vip
				try
				{
					M_SQL(pDBConn, "EXECUTE USP_LOGINPLAYER_SXZ '%s', %d", MachineUID, account);  //传入机器码和帐号id
					if(pDBConn->More())
					{
						bonusCount = pDBConn->GetInt();
						accountVipLv = pDBConn->GetInt();
					}
				}
				catch (CDBException &e)
				{
					g_Log.WriteFocus("<<ERROR>>msg=%s, dberr=%s, ecode=%d, func='%s'", e.w_msgtext, e.m_dberrstr, e.m_dberr, __FUNCTION__);
				}
				catch (...)
				{
					g_Log.WriteError("RemoteAccount handle login player error(login count bonus).");
				}
			}		
		}
		int defrayExp = 0;
		int sponsorActivity = 0;
		try
		{
			M_SQL(pDBConn, "SELECT [Experience] FROM TBL_DefrayExp WHERE AccountID = %d AND PlayerID = %d", account, playerId);
			if (pDBConn->More())
				defrayExp = pDBConn->GetInt();
			M_SQL(pDBConn, "SELECT [Sponsor] FROM TBL_Sponsor WHERE AccountID = %d", account);
			if (pDBConn->More())
			{
				SPONSOR_TYPE sponsor = (SPONSOR_TYPE)pDBConn->GetInt();
				sponsorActivity |= BIT(sponsor);
			}
		}
		catch (CDBException& e)
		{
			g_Log.WriteFocus("<<ERROR>>msg=%s, dberr=%s, ecode=%d, func='%s'", e.w_msgtext, e.m_dberrstr, e.m_dberr, __FUNCTION__);
		}
		catch (...)
		{
			g_Log.WriteError("RemoteAccount handle login player error(defray experience).");
		}

		MAKE_PACKET(sendPacket, "WORLD_REMOTE_PlayerLogin", account);
		sendPacket.writeInt(bonusCount, 32);
		sendPacket.writeInt(NetBarLv, 32);
		sendPacket.writeInt(accountVipLv, 32);
		sendPacket.writeInt(defrayExp, 32);
		sendPacket.writeInt(sponsorActivity, 32);

		SEND_PACKET_SOCK(sendPacket, socketId, CRemoteAccount::getInstance()->getServerSocket());
	}
	return PACKET_OK;
}