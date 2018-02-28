//#include "stdafx.h"
#include "DB_QueryGold.h"
#include "DBContext.h"

DB_QueryGold::DB_QueryGold()
:AccountId(0), PlayerId(0), socketId(0)
{
}

int DB_QueryGold::Execute(int ctxId, void* param)
{
	if (DB_CONTEXT_NOMRAL == ctxId)
	{
		CDBConn* pDBConn = (CDBConn*)param;
		IF_ASSERT(pDBConn == NULL)
			return PACKET_ERROR; //TODO: 检查是否释放资源

		try
		{
			M_SQL(pDBConn, "EXECUTE USP_QUERYGOLD_SXZ %d", AccountId);
			if (pDBConn->More())
			{
				MAKE_PACKET(sendPacket, AW_QUERYACCOUNTGOLD_RESPOND, AccountId);
				sendPacket.writeInt(PlayerId, Base::Bit32);
				sendPacket.writeInt(pDBConn->GetInt(), Base::Bit32);//AccountGold
				SEND_PACKET_SOCK(sendPacket, socketId, CRemoteAccount::getInstance()->getServerSocket());
			}
		}
		catch (CDBException &e)
		{
			g_Log.WriteFocus("<<ERROR>>msg=%s, dberr=%s, ecode=%d, func="__FUNCTION__, e.w_msgtext, e.m_dberrstr, e.m_dberr);
		}
		catch (...)
		{
			g_Log.WriteError("RemoteAccount query player gold failed.[AccountID=%d,PlayerID=%d]", AccountId, PlayerId);
		}
	}
	return PACKET_OK;
}

DB_QueryMonthPays::DB_QueryMonthPays()
:AccountId(0), PlayerId(0),socketId(0)
{
	isDraw = false;
	PlayerName[0] = 0;
}

int DB_QueryMonthPays::Execute(int ctxId, void* param)
{
	if (DB_CONTEXT_NOMRAL == ctxId)
	{
		CDBConn* pDBConn = (CDBConn*)param;
		IF_ASSERT(pDBConn == NULL)
			return PACKET_ERROR; //TODO: 检查是否释放资源

		try
		{
			M_SQL(pDBConn, "EXECUTE USP_QUERYMONTHPAYS_SXZ %d", AccountId);
			if (pDBConn->More())
			{
				MAKE_PACKET(sendPacket, AW_QUERYMONTHPAYS_RESPOND, AccountId);
				sendPacket.writeFlag(isDraw);
				sendPacket.writeInt(PlayerId, Base::Bit32);
				sendPacket.writeInt(pDBConn->GetInt(), Base::Bit32);//AccountGold
				sendPacket.writeInt(pDBConn->GetInt(), Base::Bit32);//MonthPays
				sendPacket.writeString(PlayerName, 32);
				SEND_PACKET_SOCK(sendPacket, socketId, CRemoteAccount::getInstance()->getServerSocket());
			}
		}
		catch (CDBException &e)
		{
			g_Log.WriteFocus("<<ERROR>>msg=%s, dberr=%s, ecode=%d, func="__FUNCTION__, e.w_msgtext, e.m_dberrstr, e.m_dberr);
		}
		catch (...)
		{
			g_Log.WriteError("RemoteAccount query player monthpays failed.[AccountID=%d,PlayerID=%d]", AccountId, PlayerId);
		}
	}
	return PACKET_OK;
}