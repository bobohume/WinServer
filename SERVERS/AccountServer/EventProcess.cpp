#include "base/SafeString.h"
#include "DBLib/dbLib.h"
#include "Common/Script.h"
#include "Common/MemGuard.h"
#include "base/WorkQueue.h"
#include "Common/PacketType.h"
#include "WinTCP/IPacket.h"
#include "WinTCP/dtServerSocket.h"
#include "Common/CommonClient.h"
#include "AccountServer.h"
#include "EventProcess.h"
#include "PlayerMgr.h"
#include "Common/PlayerStructEx.h"
#include "DBLayer\Data\TBLAccount.h"
#include "Common\RegisterCommon.h"
#include "Common/DumpHelper.h"
#include "DBLayer/Data/TBLGmLoginCheck.h"
#include "encrypt/AuthCrypt.h"
#include "Common/Common.h"
#include "Common/CfgBuilder.h"
#include "ZLIB/zconf.h"
#include "ZLIB//zlib.h"

#ifdef _SXZ_UNITTEST
#include "../../../UnitTest/UnitTest/UnitCallback/UnitCallback.h"
#endif

using namespace std;

//#define PASS9


//static int playerLoginCount = 0;
WorldServerInfo g_worldServerInfo;
CMyCriticalSection g_worldServerInfo_cs;

struct queueItem
{
	int  socketId;
	int  accountId;
	bool flag;
	int  isAdult;
    bool isLogin;
	LOGIN_TYPE loginType;
	U8 netProvider;

	queueItem()
	{
		socketId  = 0;
		accountId = 0;
		flag      = false;
        isLogin   = false;
		loginType = LOGIN_Normal;
	}
};


int SERVER_CLASS_NAME::EventProcess(LPVOID Param)
{
	WorkQueueItemStruct *pItem = (WorkQueueItemStruct*)Param;
	switch(pItem->opCode)
	{
	case WQ_CONNECT:
		{
			char ip[20];
			memcpy(ip, pItem->Buffer, pItem->size);
			ip[pItem->size] = '\0';
			SERVER->GetPlayerManager()->AddIPMap(pItem->Id, ip);
		}
		break;
	case WQ_DISCONNECT:
		{
			SERVER->GetPlayerManager()->ReleaseSocketMap(pItem->Id, true);
		}
		break;
	case WQ_PACKET:
		{
			bool restartFlag = true;
			stPacketHead* pHead = (stPacketHead*)pItem->Buffer;

			if(ISVALID_EVENT(pHead->Message))
			{
                try
                {
					//注意:catch SE异常必须有设置translator
					//_set_se_translator( (_se_translator_function)&CDumpHelper::Exception );

                    char *pData	= (char *)(pHead) + IPacket::GetHeadSize();
                    Base::BitStream RecvPacket(pData,pItem->size-IPacket::GetHeadSize());
                    //restartFlag = !fnList[pHead->Message](pItem->Id,pHead,&RecvPacket);
					restartFlag = !TRIGGER_EVENT(pItem->Id,pHead,RecvPacket);
                }
                catch( SHException& e )
                {
                    g_Log.WriteError( "严重错误：抛出结构化异常[WQ_PACKET: MSGCODE=%s][ERRCODE=%d]",SHOWMSG_EVENT(pHead->Message), e.getCode());
                }
                catch( ... )
                {
                    g_Log.WriteError( "严重错误：未处理异常[WQ_PACKET: MSGCODE=%s]", SHOWMSG_EVENT(pHead->Message));
                }
            }

			if(restartFlag)
			{
                g_Log.WriteWarn("收到一条未处理的消息，断开这个连接 MsgId:%s Socket:%d",SHOWMSG_EVENT(pHead->Message),pItem->Id);
				SERVER->GetServerSocket()->PostEvent(dtServerSocket::OP_RESTART,pItem->Id);
			}
		}
		break;
    case WQ_CONFIGMONITOR:
        {
            char* fileName = (char*)pItem->Buffer;

            //重新载入配置文件
            if (0 == _stricmp(fileName,MONITOR_CONFIGFILE))
            {
                CFG_BUIILDER->Reload();
            }
        }
        break;
	case  WQ_TIMER:
		{
		}
		break;
	}

	return false;
}

namespace EventFn
{
	void Initialize()
	{
		REGISTER_EVENT_FUNCTION("COMMON_RegisterResponse",			 &EventFn::HandleRegisterResponse);
		REGISTER_EVENT_FUNCTION("CLIENT_ACCOUNT_LoginRequest",		 &EventFn::HandleClientLoginRequest);
		REGISTER_EVENT_FUNCTION("ACCOUNT_WORLD_ClientLoginResponse", &EventFn::HandleWorldClientLoginResponse);
		REGISTER_EVENT_FUNCTION("RA_LoginAck",						 &EventFn::HandleRemoteLoginAck);
		REGISTER_EVENT_FUNCTION("RA_RegisterAck",					 &EventFn::HandleRemoteRegisterResponse);
		REGISTER_EVENT_FUNCTION("WORLD_ACCOUNT_PlayerLogout",		 &EventFn::HandleWorldPlayerLogout);
		REGISTER_EVENT_FUNCTION("ACCOUNT_REMOTE_PlayerCountResponse",&EventFn::HandleRemotePlayerCount);
	}

	bool HandleRegisterResponse(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
	{
		//来自WorldServer对自己注册的反馈
		SERVER->OnServerStart();
		return true;
	}

	bool HandleWorldClientLoginResponse(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
	{
		int UID = Packet->readInt(Base::Bit32);
		int AccountId = Packet->readInt(Base::Bit32);
		//int netProvider = Packet->readInt(Base::Bit8);
		

		ERROR_CODE Error = (ERROR_CODE)Packet->readInt(Base::Bit16);

        g_Log.WriteLog("收到world对帐号[%d]的登录响应[%d]",AccountId,Error);

		{
			CLocker lock(SERVER->GetPlayerManager()->m_cs);

			stAccountInfo *ppInfo = SERVER->GetPlayerManager()->GetAccountMap(AccountId);
			if(ppInfo && ppInfo->UID == UID)
			{
				CMemGuard Buffer(MAX_PACKET_SIZE MEM_GUARD_PARAM);
				Base::BitStream SendPacket(Buffer.get(), MAX_PACKET_SIZE);
				stPacketHead *pSendHead = IPacket::BuildPacketHead(SendPacket, "CLIENT_ACCOUNT_LoginResponse");

				SendPacket.writeInt(Error,Base::Bit16);

				if(Error == NONE_ERROR)
				{
					SendPacket.writeInt(UID,UID_BITS);
					SendPacket.writeInt(AccountId,Base::Bit32);
				}

				SendPacket.copyFrom(Packet);
				pSendHead->PacketSize = SendPacket.getPosition()-IPacket::GetHeadSize();
				SERVER->GetServerSocket()->Send(ppInfo->socketId,SendPacket);

                //设置玩家的登入时间信息
				TBLAccount tb_account(SERVER_CLASS_NAME::GetActorDB());
				stSocketInfo* pSocketInfo = SERVER->GetPlayerManager()->GetSocketMap(ppInfo->socketId);
				DBError err = tb_account.SaveLoginInfo(AccountId, pSocketInfo ? inet_ntoa(*((in_addr*)&pSocketInfo->LoginIP)) : "",Error);
			}
		}

		return true;
	}

	bool HandleRemoteLoginAck( int _SocketHandle,stPacketHead *pHead,Base::BitStream *Packet )
	{
		char AccountName[ACCOUNT_NAME_LENGTH] = "";
		char Password[PASSWORD_LENGTH] = "";
		int SocketHandle = pHead->Id;
		int AccountId = 0;
		ERROR_CODE errorCode = Packet->readInt(Base::Bit16);
		if (errorCode == NONE_ERROR)
		{
			AccountId = Packet->readInt(Base::Bit32);
			Packet->readString(AccountName, ACCOUNT_NAME_LENGTH);
			Packet->readString(Password, PASSWORD_LENGTH);

			g_Log.WriteLog("收到RA对帐号[%d]的登录反馈", AccountId);

			errorCode = handleAccountLogin(AccountName, AccountId, DBERR_NONE, SocketHandle, 1, true, 0);
		}
		
		if (errorCode != NONE_ERROR)
		{
			CMemGuard Buffer(64 MEM_GUARD_PARAM);
			Base::BitStream SendPacket(Buffer.get(),64);
			stPacketHead *pSendHead = IPacket::BuildPacketHead(SendPacket,"CLIENT_ACCOUNT_LoginResponse");
			SendPacket.writeInt(errorCode,Base::Bit16);
			pSendHead->PacketSize = SendPacket.getPosition()-IPacket::GetHeadSize();
			SERVER->GetServerSocket()->Send(SocketHandle,SendPacket);
		}

		return true;
	}

	ERROR_CODE handleAccountLogin(char* AccountName, int& AccountId, ERROR_CODE Error, int SocketHandle, 
		int flag, bool isFirst, int oldPlayerFlag)
	{
		TBLAccount tb_account(SERVER_CLASS_NAME::GetActorDB());
		DBError err = tb_account.MakeAccount(AccountName, AccountId, flag, g_nopass9);
		if(err != DBERR_NONE)
		{
			g_Log.WriteError("数据库帐号数据错误!(name=%s, id=%d, code=%s)", AccountName, AccountId, getDBErr(err));
			return err;
		}

		if(Error == NONE_ERROR)
		{
			TBLAccount::stAccount accountInfo;

			if ( 1 == oldPlayerFlag )
			{
				err = tb_account.SetAccountStatus(AccountId);
				if(err != DBERR_NONE)
					return err;
			}
			
            err = tb_account.UpdateLoginTime(AccountId);

            if(err != DBERR_NONE)
                return err;

            err = tb_account.Load(AccountId, &accountInfo);

            if(err != DBERR_NONE)
                return err;

            /*if ( 0 == accountInfo.GMFlag )
            {
                if ( !SERVER->GetLoginFlag() )
                    return GAME_SERVER_INVALID;
            }*/

            Players actorlist;
            DBError err = tb_account.LoadPlayerList(accountInfo.AccountID, actorlist);

            if(err != DBERR_NONE)
            {
                g_Log.WriteError("tb_account.QueryID err=%s", getDBErr(err));
                return err;
            }

			if(Error == NONE_ERROR)
			{
				CLocker lock(SERVER->GetPlayerManager()->m_cs);
				stAccountInfo *ppInfo = NULL;
				if( isFirst )
				{
					SERVER->GetPlayerManager()->ReleaseAccountMap(AccountId);
					ppInfo = SERVER->GetPlayerManager()->AddAccountMap(SocketHandle,AccountId);
				}
				else
				{
					ppInfo = SERVER->GetPlayerManager()->GetAccountMap(AccountId);
				}
				if (!ppInfo)
					return UNKNOW_ERROR;

				stSocketInfo* pSocketInfo = SERVER->GetPlayerManager()->GetSocketMap(SocketHandle);

				std::string ip;
				if (0 != pSocketInfo)
					ip = inet_ntoa(*((in_addr*)&pSocketInfo->LoginIP));

				/*if ( (accountInfo.GMFlag != 0) && (accountInfo.GMFlag != 200) )
				{
					TBLGmLoginCheck tb_gmCheck(SERVER_CLASS_NAME::GetActorDB());
					DBError err = tb_gmCheck.QueryMACandIP(macAddr,ip.c_str());
					if (err != DBERR_NONE)
						return ILLEGAL_GM;
				}*/
                
                if (0 == strlen(ppInfo->AccountName))
                {
				    sStrcpy(ppInfo->AccountName, ACCOUNT_NAME_LENGTH, AccountName, ACCOUNT_NAME_LENGTH);
                }

				ppInfo->flag   = flag;
                ppInfo->GMFlag = accountInfo.GMFlag;

				char Buffer[4096];
				Base::BitStream SendPacket(Buffer,sizeof(Buffer));

				stPacketHead *pSendHead = IPacket::BuildPacketHead(SendPacket, "ACCOUNT_WORLD_ClientLoginRequest");
				SendPacket.writeFlag(isFirst);
				SendPacket.writeInt(ppInfo->UID, UID_BITS);
				SendPacket.writeInt(ppInfo->AccountId, Base::Bit32);
				SendPacket.writeInt(accountInfo.GMFlag, Base::Bit8);
				SendPacket.writeInt(accountInfo.Status, Base::Bit32);

				// 增加读取防沉迷数据
				SendPacket.writeFlag( accountInfo.IsAdult );
				//SendPacket.writeInt( accountInfo.dTotalOnlineTime, 32 );
				//SendPacket.writeInt( accountInfo.dTotalOfflineTime, 32 );
				SendPacket.writeInt( accountInfo.LoginTime, 32 );
				SendPacket.writeInt( accountInfo.LogoutTime, 32 );

				SendPacket.writeString(ip.c_str());
				SendPacket.writeString(ppInfo->AccountName);

                SendPacket.writeInt(actorlist.size(), 32);
                for (size_t i = 0; i < actorlist.size(); ++i)
                {
					actorlist[i].WriteData(&SendPacket);
                }

				pSendHead->PacketSize = SendPacket.getPosition()-IPacket::GetHeadSize();
				SERVER->GetWorldSocket()->Send(SendPacket);
			}
		}
		return Error;
	}

	bool HandleClientLoginRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
	{
        //AuthCrypt::Instance()->DecryptRecv((uint8*)pHead + IPacket::GetHeadSize(),pHead->PacketSize);

		ERROR_CODE errorCode = NONE_ERROR;
		char AccountName[ACCOUNT_NAME_LENGTH] = "";
		//char Password[PASSWORD_LENGTH] = "";
		//int  AccountId = 0;
		char gBuildVersion[128] = "";
		Packet->readString(gBuildVersion, sizeof(gBuildVersion));

		if (IS_ACCEPTABLE_BUILD_VERSION(gBuildVersion))
		{
			Packet->readString(AccountName, sizeof(AccountName));
			//_strupr_s(AccountName);
			//Packet->readString(Password,    sizeof(Password));
			g_Log.WriteLog("帐号[%s]登录帐号服务器", AccountName);
			CMemGuard Buffer(256 MEM_GUARD_PARAM);
			Base::BitStream sendPacket(Buffer.get(), 256);
			stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "RA_LoginRst", SocketHandle, SERVICE_REMOTESERVER);
			sendPacket.writeString(AccountName, sizeof(AccountName));
			//sendPacket.writeString(Password, sizeof(Password));
			pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
			SERVER_CLASS_NAME::GetInstance()->GetWorldSocket()->Send(sendPacket);
		}
		else
		{
			errorCode = VERSION_ERROR;
			g_Log.WriteWarn("版本验证错误 clientVersion=%s,err=%d",gBuildVersion,errorCode);
		}

		if (errorCode != NONE_ERROR)
		{
			char Buffer[64];
			Base::BitStream SendPacket(Buffer,sizeof(Buffer));
			stPacketHead *pSendHead = IPacket::BuildPacketHead(SendPacket,"CLIENT_ACCOUNT_LoginResponse");
			SendPacket.writeInt(errorCode,Base::Bit16);
			pSendHead->PacketSize = SendPacket.getPosition()-IPacket::GetHeadSize();
			SERVER->GetServerSocket()->Send(SocketHandle,SendPacket);
			return true;
		}

		return true;
	}

	// 响应RemoteAccount的快速注册响应
	bool HandleRemoteRegisterResponse( int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet )
	{
		int AccountId = pHead->Id;
		int AccountSocketId = pHead->DestZoneId;
		char AccountName[ACCOUNT_NAME_LENGTH] = "";
		char PassWord[PASSWORD_LENGTH] = "";

		CMemGuard Buffer(256 MEM_GUARD_PARAM);
		Base::BitStream SendPacket(Buffer.get(),256);
		stPacketHead *pSendHead = IPacket::BuildPacketHead(SendPacket,"CLIENT_ACCOUNT_RegisterAccountResponse",AccountId,SERVICE_CLIENT);
		SendPacket.writeString(AccountName, sizeof(AccountName));
		SendPacket.writeString(PassWord, sizeof(PassWord));

		pSendHead->PacketSize = SendPacket.getPosition()-IPacket::GetHeadSize();
		SERVER->GetServerSocket()->Send(AccountSocketId,SendPacket);
		return true;
	}

	bool HandleWorldPlayerLogout( int,stPacketHead *,Base::BitStream* )
	{
		// 出发一个排队中的帐号登陆
        return true;
	}

	bool HandleRemotePlayerCount( int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet )
	{
		{
			CLocker lock(g_worldServerInfo_cs);
			g_worldServerInfo.ReadData(Packet);

            float status1 = float(atoi(CFG_BUIILDER->Get("status1").c_str())) / 100.0f;
            float status2 = float(atoi(CFG_BUIILDER->Get("status2").c_str())) / 100.0f;

            if (0 == status1)
            {
                status1 = 0.25f;
            }

            if (0 == status2)
            {
                status2 = 0.50f;
            }

			g_worldServerInfo.convertPlayer(status1,status2);  //从remoteAccount获得的是状态
		}
		return true;
	}
}

