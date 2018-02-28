//#include "stdafx.h"
#include "DB_GMAccountGoldOP.h"
#include "DBContext.h"

DB_GMQueryAccountGold::DB_GMQueryAccountGold()
:AccountId(0), sid(0), socketId(0),GMAccountID(0)
{
}

int DB_GMQueryAccountGold::Execute(int ctxId, void* param)
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
				MAKE_PACKET(sendPacket, AW_GMQUERYACCOUNTGOLD_RESPOND, AccountId,SERVICE_WORLDSERVER);
				sendPacket.writeInt(sid, Base::Bit32);
				sendPacket.writeInt(GMAccountID, Base::Bit32);
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
			g_Log.WriteError("RemoteAccount GM query account gold failed.[AccountID=%d,GMAccountID=%d]", AccountId, GMAccountID);
		}
	}
	return PACKET_OK;
}
// 
// DB_GMUpdateAccountGold::DB_GMUpdateAccountGold()
// :AccountId(0), sid(0), socketId(0),GMAccountID(0),gold(0)
// {
// }
// 
// int DB_GMUpdateAccountGold::Execute(int ctxId, void* param)
// {
// 	if (DB_CONTEXT_NOMRAL == ctxId)
// 	{
// 		CDBConn* pDBConn = (CDBConn*)param;
// 		IF_ASSERT(pDBConn == NULL)
// 			return PACKET_ERROR; //TODO: 检查是否释放资源
// 
// 		try
// 		{
// 			M_SQL(pDBConn, "EXECUTE USP_GMOP_CHARGE %d,%d,%d", AccountId,gold,GMAccountID);
// 			if (pDBConn->More())
// 			{
// 				MAKE_PACKET(sendPacket, AW_GMUPDATEACCOUNTGOLD_RESPOND, AccountId,SERVICE_WORLDSERVER);
// 				sendPacket.writeInt(sid, Base::Bit32);
// 				sendPacket.writeInt(GMAccountID, Base::Bit32);//AccountGold
// 				sendPacket.writeInt(pDBConn->GetInt(), Base::Bit32);//MonthPays
// 				SEND_PACKET_SOCK(sendPacket, socketId, CRemoteAccount::getInstance()->getServerSocket());
// 			}
// 		}
// 		catch (CDBException &e)
// 		{
// 			g_Log.WriteFocus("<<ERROR>>msg=%s, dberr=%s, ecode=%d, func="__FUNCTION__, e.w_msgtext, e.m_dberrstr, e.m_dberr);
// 		}
// 		catch (...)
// 		{
// 			g_Log.WriteError("RemoteAccount update account gold  failed.[AccountID=%d,GMaccountID=%d]", AccountId, GMAccountID);
// 		}
// 	}
// 	return PACKET_OK;
// }