//#include "stdafx.h"
#include "DB_GetItemBonus.h"
#include "DBContext.h"

DB_GetItemBonus::DB_GetItemBonus()
:iPlayerId(0), iUId(0), iBonusType(0), iAccountId(0), socketId(0)
{
}

int DB_GetItemBonus::Execute(int ctxId, void* param)
{
	if (DB_CONTEXT_NOMRAL == ctxId)
	{
		CDBConn* pDBConn = (CDBConn*)param;
		IF_ASSERT(pDBConn == NULL)
			return PACKET_ERROR; //TODO: 检查是否释放资源

		try
		{
			char buf[256];
			Base::BitStream sendPacket(buf, sizeof(buf));
			stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, REMOTEACCOUNT_ZONE_GETBONUS_RESPONSE, iPlayerId, SERVICE_WORLDSERVER);

			M_SQL(pDBConn, "EXECUTE USP_GetBonusItem %d,%d,%d", iUId, iAccountId, iPlayerId);
			if (pDBConn->More())
			{
				int iResult = pDBConn->GetInt();
				sendPacket.writeInt(iResult, Base::Bit8);

				int iItemId = pDBConn->GetInt();
				int iQuantity = pDBConn->GetInt();

				if (iResult > 0)
				{
					if(1 == iResult)
					{
						g_Log.WriteWarn("RemoteAccount礼包已经领取过了 account=%d, playerId=%d, Result=%d, uId=%d, itemId=%d, quantity=%d",
							iAccountId, iPlayerId, iResult, iUId, iItemId, iQuantity);
					}
					else if(2 == iResult)
					{
						g_Log.WriteWarn("RemoteAccount不存在该条礼包记录 account=%d, playerId=%d, Result=%d, uId=%d, itemId=%d, quantity=%d",
							iAccountId, iPlayerId, iResult, iUId, iItemId, iQuantity);
					}
					else if (3 == iResult)
					{
						g_Log.WriteWarn("RemoteAccount领取角色不匹配 account=%d, playerId=%d, Result=%d, uId=%d, itemId=%d, quantity=%d",
							iAccountId, iPlayerId, iResult, iUId, iItemId, iQuantity);
					}
				}
				else
				{
					g_Log.WriteLog("RemoteAccount礼包领取成功 account=%d, playerId=%d, uId=%d, itemId=%d, quantity=%d",
						iAccountId, iPlayerId, iUId, iItemId, iQuantity);
				}

				sendPacket.writeInt(iUId, Base::Bit32);
				sendPacket.writeInt(iItemId, Base::Bit32);
				sendPacket.writeInt(iQuantity, Base::Bit16);
				sendPacket.writeInt(iBonusType, Base::Bit8);
			}

			pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
			CRemoteAccount::getInstance()->getServerSocket()->Send(socketId, sendPacket);
		}
		catch (CDBException &e)
		{
			g_Log.WriteFocus("<<ERROR>>msg=%s, dberr=%s, ecode=%d, func="__FUNCTION__, e.w_msgtext, e.m_dberrstr, e.m_dberr);
		}
		catch (...)
		{
			g_Log.WriteError("RemoteAccount handle GetItemBonus error account=%d,playerId=%d,itemId=%d",iAccountId,iPlayerId,iUId);
		}
	}
	return PACKET_OK;
}