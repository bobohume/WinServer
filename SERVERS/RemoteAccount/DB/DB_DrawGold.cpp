//#include "stdafx.h"
#include "DB_DrawGold.h"
#include "DBContext.h"

DB_DrawGold::DB_DrawGold()
:AccountId(0), PlayerId(0), TakeGold(0), socketId(0)
{
}

int DB_DrawGold::Execute(int ctxId, void* param)
{
	if (DB_CONTEXT_NOMRAL == ctxId)
	{
		CDBConn* pDBConn = (CDBConn*)param;
		IF_ASSERT(pDBConn == NULL)
			return PACKET_ERROR; //TODO: 检查是否释放资源

		try
		{
			M_SQL(pDBConn, "EXECUTE USP_COSTGOLD_SXZ %d,%d", AccountId, TakeGold);
			if (pDBConn->More() && pDBConn->GetInt())
			{
				MAKE_PACKET(sendPacket, AW_DRAWGOLD_RESPOND, AccountId);
				sendPacket.writeInt(PlayerId, Base::Bit32);
				sendPacket.writeInt(TakeGold, Base::Bit32);
				sendPacket.writeInt(pDBConn->GetInt(), Base::Bit32);//LeftGold
				SEND_PACKET_SOCK(sendPacket, socketId, CRemoteAccount::getInstance()->getServerSocket());
			}
		}
		catch (CDBException &e)
		{
			g_Log.WriteFocus("<<ERROR>>msg=%s, dberr=%s, ecode=%d, func="__FUNCTION__, e.w_msgtext, e.m_dberrstr, e.m_dberr);
			return PACKET_ERROR;
		}
		catch (...)
		{
			g_Log.WriteError("RemoteAccount draw player gold failed.[AccountID=%d, PlayerID=%d, takeGold=%d]",
				AccountId, PlayerId, TakeGold);
		}
	}
	return PACKET_OK;
}