//#include "stdafx.h"
#include "DB_QueryBonus.h"
#include "DBContext.h"
#include "Common\BonusBase.h"
#include "quicklz\quicklz.h"

DB_QueryBonus::DB_QueryBonus()
:playerId(0), iBonusType(0), iAccountId(0), socketId(0)
{
}

int DB_QueryBonus::Execute(int ctxId, void* param)
{
	if (DB_CONTEXT_NOMRAL == ctxId)
	{
		CDBConn* pDBConn = (CDBConn*)param;
		IF_ASSERT(pDBConn == NULL)
			return PACKET_ERROR; //TODO: 检查是否释放资源

		try
		{
			M_SQL(pDBConn, "EXECUTE UPS_QueryItemList %d,%d,%d", iAccountId, playerId, iBonusType);

			int iIndex = 0;

			char* pBuf = new char[MAX_PACKET_SIZE];
			char* pData = new char[MAX_PACKET_SIZE];
			char* zipBuf = new char[MAX_PACKET_SIZE];

			Base::BitStream sendPacket(pBuf, MAX_PACKET_SIZE);
			Base::BitStream dataStream(pData, MAX_PACKET_SIZE);

			while (pDBConn->More()) //执行SQL语句并取出结果集
			{
				stBonusItem stItem;
				stItem.uId = pDBConn->GetInt();
				stItem.itemId = pDBConn->GetInt();
				stItem.quantity = pDBConn->GetInt();
				stItem.type = iBonusType;
				if (pDBConn->GetInt() > 0)
					stItem.receiveFlag = RECEIVE_END;
				else
					stItem.receiveFlag = RECEIVE_CAN;

				strcpy_s(stItem.szDesc,sizeof(stItem.szDesc), pDBConn->GetString());
				strcat_s(stItem.szDesc,sizeof(stItem.szDesc), pDBConn->GetString());

				iIndex++;
				stItem.WriteData(&dataStream);
				//限定当记录数为MAX_ITEMBONUS_COUNT(48)的整数倍时，必须发一次包
				//因为MAX_PACKET_SIZE=32*1024 = 32768
				//而客户端解包的缓存大小 = sizeof(stBonusItem) * MAX_ITEMBONUS_COUNT = 528*48 = 25344
				//理论上一次可以发的记录数 = 32768 / 528 = 62
				//之所以定义为MAX_ITEMBONUS_COUNT = 48，应该是处于策划的设定，故以前只能显示最多48个礼包，每页12个，即4页
				//现在需要扩展到任意页，但一次发包最大应该为48个礼包的数据
				if(iIndex % MAX_ITEMBONUS_COUNT == 0) 
				{
					sendPacket.setPosition(0);
					stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, REMOTEACCOUNT_ZONE_QUERYBONUS_RESPONSE, playerId, SERVICE_ZONESERVER);
					int zipSize = MAX_PACKET_SIZE;
					int iret = SXZ_COMPRESS(zipBuf, &zipSize, pData, dataStream.getPosition());
					if (sendPacket.writeFlag(!iret)) //解压是否成功
					{
						sendPacket.writeInt(zipSize, Base::Bit32);
						sendPacket.writeBits(zipSize << 3, zipBuf); //这里左移3，即乘以8。即读取的位数。
						sendPacket.writeInt(iIndex, Base::Bit16); //最大序号：65535
					}

					pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
					CRemoteAccount::getInstance()->getServerSocket()->Send(socketId, sendPacket);
					dataStream.setPosition(0);
					//清空(为下一次发包做准备)
					memset(pBuf,0, sizeof(char) * MAX_PACKET_SIZE);
					memset(pData,0, sizeof(char) * MAX_PACKET_SIZE);
					memset(zipBuf,0, sizeof(char) * MAX_PACKET_SIZE);
					sendPacket.clear();
					dataStream.clear();
				}
			}
			//当查询完数据库记录后
			if(iIndex > 0 && iIndex % MAX_ITEMBONUS_COUNT != 0)
			{
				sendPacket.setPosition(0);
				stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, REMOTEACCOUNT_ZONE_QUERYBONUS_RESPONSE, playerId, SERVICE_ZONESERVER);
				dataStream.writeInt(iIndex, Base::Bit8);
				int zipSize = MAX_PACKET_SIZE;
				int iret = SXZ_COMPRESS(zipBuf, (size_t *)&zipSize, pData, dataStream.getPosition());
				if (sendPacket.writeFlag(!iret)) //解压是否成功
				{
					sendPacket.writeInt(zipSize, Base::Bit32);
					sendPacket.writeBits(zipSize << 3, zipBuf);
					sendPacket.writeInt(iIndex, Base::Bit16);
				}
				pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
				CRemoteAccount::getInstance()->getServerSocket()->Send(socketId, sendPacket);
			}

			delete[] pBuf;
			delete[] pData;
			delete[] zipBuf;
		}
		catch (CDBException &e)
		{
			g_Log.WriteFocus("<<ERROR>>msg=%s, dberr=%s, ecode=%d, func="__FUNCTION__, e.w_msgtext, e.m_dberrstr, e.m_dberr);
		}
		catch (...)
		{
			g_Log.WriteError("RemoteAccount handle QueryBonus error account=%d,playerId=%d,iType=%d",iAccountId,playerId,iBonusType);
		}
	}
	return PACKET_OK;
}