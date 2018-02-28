//#include "stdafx.h"
#include "DB_PurchaseRecord.h"
#include "DBContext.h"
#include "CommLib\DBThreadManager.h"
#include "Common\MemGuard.h"
#include "..\RemoteAccount.h"

DB_PurchaseRecord::DB_PurchaseRecord() : playerId(0), quantity(0), transaction_id(""), product_id(""), item_id(""),\
	purchase_date(""),  mError(0), nGold(0), nMoney(0), nCopperCoil(0), nSilverCoil(0), nGoldCoil(0)
{
}

int DB_PurchaseRecord::Execute(int ctxId, void* param)
{
	if (DB_CONTEXT_NOMRAL == ctxId)
	{
		CDBConn* pDBConn = (CDBConn*)param;
		IF_ASSERT(pDBConn == NULL)
			return PACKET_ERROR; //TODO: 检查是否释放资源

		try
		{

			M_SQL(pDBConn, "EXEC USP_VertifyRecord %d, %d, '%s', '%s', '%s', '%s'", playerId, quantity, transaction_id.c_str(), product_id.c_str(), item_id.c_str(), purchase_date.c_str());
			if (pDBConn->More())
			{
				mError		 = pDBConn->GetInt();
				nGold		 = pDBConn->GetInt();
				nMoney		 = pDBConn->GetInt();
				nCopperCoil  = pDBConn->GetInt();
				nSilverCoil  = pDBConn->GetInt();
				nGoldCoil	 = pDBConn->GetInt();
				transaction_id = pDBConn->GetString();
				PostLogicThread(this);
				return PACKET_NOTREMOVE;
			}
			else
			{
				g_Log.WriteError("DB_PurchaseRecord failed.[player=%d]", playerId);
			}
		}
		catch (CDBException &e)
		{
			g_Log.WriteFocus("<<ERROR>>msg=%s, dberr=%s, ecode=%d, func='%s'", e.w_msgtext, e.m_dberrstr, e.m_dberr, __FUNCTION__);
		}
		catch (...)
		{
			g_Log.WriteError("DB_PurchaseRecord error.[player=%D]", playerId);
		}
	}
	else
	{
		if(mError == 0)
		{
			CMemGuard Buffer(1024 MEM_GUARD_PARAM);
			Base::BitStream sendPacket(Buffer.get(), 1024);
			stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, "RW_PURCHASE_RESPONSE", playerId, SERVICE_WORLDSERVER);
			sendPacket.writeInt(nGold, Base::Bit32);
			sendPacket.writeInt(nMoney, Base::Bit32);
			sendPacket.writeInt(nCopperCoil, Base::Bit32);
			sendPacket.writeInt(nSilverCoil, Base::Bit32);
			sendPacket.writeInt(nGoldCoil, Base::Bit32);
			pSendHead->PacketSize = sendPacket.getPosition() - IPacket::GetHeadSize();
			int socketHandle = SERVER->GetWorldSocket(SERVER->GetAreaaId());
			if(socketHandle > 0)
				SERVER->getServerSocket()->Send(socketHandle, sendPacket);
		}
	}
	return PACKET_OK;
}