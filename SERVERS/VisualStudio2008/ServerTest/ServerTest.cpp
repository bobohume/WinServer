// ServerTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Common/GdiUtil.h"
#include "CommLib/ShareMemoryKey.h"
#include "CommLib/ShareMemoryUserPool.h"
#include "Common/PlayerStructEx.h"
#include "common/cfgbuilder.h"
#include "Common/CommonServer.h"
#include "Common/CommonClient.h"
#include "base/bitstream.h"
#include "CommLib/PacketHandle.h"
#include "CommLib/PacketProcess.h"
#include "Commlib/SimpleTimer.h"
#include "zlib/zlib.h"
#include "common/chatbase.h"

#include <vector>
#include <iostream>

std::string RemoteCount;
std::string RemoteNetIP;
std::string RemoteNetPort;
std::string MaxRemoteCount;
std::string RemoteAccountIP;
std::string RemoteAccountPort;
std::string DB_ActorServer;
std::string DB_ActorUserId;
std::string DB_ActorPassword;
std::string DB_ActorName;

std::string DB_BillingServer;
std::string DB_BillingUserId;
std::string DB_BillingPassword;
std::string DB_BillingName;

std::string LogServerIP;
std::string LogServerPort;
std::string AreaId;
std::string MaxDBConnection;

CAsyncSocket2* m_pChatClient[10000] = {0};
dtServerSocket	*			m_pServerService = 0;

ChatPlayerInfo m_chatplayers[20000];

#define TEST_WORLDDB

void TestDataAgent(void)
{
	SMDataPool<stPlayerStruct,U32,3000> pool;
	typedef SMDataPool<stPlayerStruct,U32,3000>::POOL_DATA TEST_DATA;

	while(!pool.Init(SMKEY_DATAAGENT_PLAYER))
	{
		Sleep(1);
	}

	std::cout << "初始化成功!" << std::endl;
	
	TEST_DATA* datas[100] = {0};

	while(true)
	{
		for (int i = 0; i < 100; ++i)
		{
			int randVal = rand() % 100;
			TEST_DATA* pData = datas[randVal];

			if (0 == pData)
			{
				if ((rand() % 1000) > 500)
				{
					pData = pool.Alloc(randVal,0);

					if (0 == pData)
					{
						std::cout << "无法分配数据" << std::endl;
					}
					else
					{
						pData->Attach(new stPlayerStruct);
						stPlayerStruct* playerData = pData->Get();
						playerData->BaseData.PlayerId = randVal;

						datas[randVal] = pData;
					}
				}
			}
			else
			{
				if ((rand() % 1000) > 300)
				{
					stPlayerStruct* playerData = pData->Get();
					playerData->MainData.PartData.Level++;
					pData->Save();
				}
				else
				{
					stPlayerStruct* playerData = pData->Get();
					pData->Attach(0);
					delete playerData;
					pool.Free(pData);
					datas[randVal] = 0;
				}
			}
		}


		
		pool.Update();

		Sleep(2000);
	}
}

#ifdef TEST_WORLDDB

PacketProcess	g_pktProcess;

int g_worlddbsocket = 0;

class UserPacketProcess : public PacketHandle
{
	typedef PacketHandle Parent;

	int m_ConnectTimes;
public:
	UserPacketProcess(void)
	{
		m_ConnectTimes = 0;
		m_pktProcess   = &g_pktProcess;
	}

	virtual void HandleClientLogin()
	{
		g_worlddbsocket = m_pSocket->GetClientId();

		Parent::HandleClientLogin();
	}

	virtual bool HandleGamePacket(stPacketHead *pHead,int DataSize)
	{
		return true;
	}
};


class WorldPacketProcess : public CommonPacket
{
	typedef CommonPacket Parent;

protected:
	void HandleClientLogin();
	void HandleClientLogout();
	bool HandleGamePacket(stPacketHead *pHead,int DataSize);
};

_inline void WorldPacketProcess::HandleClientLogin()
{
	
	Parent::HandleClientLogin();
}

_inline void WorldPacketProcess::HandleClientLogout()
{
	
	Parent::HandleClientLogout();
}

_inline bool WorldPacketProcess::HandleGamePacket(stPacketHead *pHead,int iSize)
{
    if (pHead->Message == CLIENT_CHAT_SendMessageRequest)
    {
        char Buffer[MAX_PACKET_SIZE];
        Base::BitStream SendPacket(Buffer,MAX_PACKET_SIZE);
        stPacketHead *pSendHead = IPacket::BuildPacketHead(SendPacket,CLIENT_CHAT_SendMessageRequest, 0, SERVICE_CHATSERVER );

        SendPacket.writeBits((32)* 8,Buffer);

        pSendHead->PacketSize = SendPacket.getPosition() - IPacket::GetHeadSize();

        Send(SendPacket);
    }

	return true;
}

void _TestWorldDB1()
{
	
	stServerParam Param;
	//开放给帐号服务器和网关服务器的服务
	m_pServerService	=	new CommonServer<WorldPacketProcess>;  //CommonServer<>(Create_UserPacketProcess);
	Param.MinClients	=	100;
	Param.MaxClients	=	100;
	m_pServerService->Initialize( RemoteNetIP.c_str(), atoi( RemoteNetPort.c_str() ),&Param);
	m_pServerService->Start();
	m_pServerService->SetMaxReceiveBufferSize(MAX_PLAYER_STRUCT_SIZE);
	m_pServerService->SetMaxSendBufferSize(MAX_PLAYER_STRUCT_SIZE);

	stPlayerStruct playerData;

	char buf[MAX_PLAYER_STRUCT_SIZE] = {0};
	char rawbuf[MAX_PLAYER_STRUCT_SIZE] = {0};	

	int rawSize = 0;
	bool needSend = true;

	U32 playerSize = sizeof(playerData);

	stdext::hash_map<U32,U32> versionMap;
		
	while(true)
	{
		g_pktProcess.run(0);

		DWORD tickQz = GetTickCount();
		
		if (needSend)
		{
			char* pTestBuf = (char*)&playerData;

			for (U32 i = 0; i < playerSize; ++i)
			{
				pTestBuf[i] = rand() % 5;
			}

			for (int i = 0; i < 100; ++i)
			{
				Base::BitStream dataStream(buf,MAX_PLAYER_STRUCT_SIZE);
				playerData.BaseData.PlayerId = rand() % 30000;
				playerData.BaseData.SaveUID  = ++versionMap[playerData.BaseData.PlayerId];

				playerData.WriteData(&dataStream);

				int size = dataStream.getPosition();
				rawSize = sizeof(rawbuf);
				int error = SXZ_COMPRESS(rawbuf, &rawSize,buf,size);

				//发送玩家数据

				char pktbuf[MAX_PLAYER_STRUCT_SIZE];
				Base::BitStream SendPacket(pktbuf,MAX_PLAYER_STRUCT_SIZE);
				stPacketHead* pSendHead = IPacket::BuildPacketHead(SendPacket,WS_SAVEPLAYER_REQUEST,playerData.BaseData.PlayerId ,SERVICE_WORLDSERVER);

				SendPacket.writeInt(0,Base::Bit32);
				SendPacket.writeInt(0,Base::Bit32);
				SendPacket.writeInt(0,Base::Bit32);
				SendPacket.writeInt(playerData.BaseData.PlayerId,Base::Bit32);
				SendPacket.writeInt(rawSize,Base::Bit32);

				SendPacket.writeBits(rawSize * 8,rawbuf);

				pSendHead->PacketSize = SendPacket.getPosition() - sizeof( stPacketHead );
				m_pServerService->Send(g_worlddbsocket,SendPacket);
			}
			
			for (int i = 0; i < 100; ++i)
			{
				char pktbuf[256];
				Base::BitStream SendPacket(pktbuf,256);
				U32 id = rand() % 30000;
				stPacketHead* pSendHead = IPacket::BuildPacketHead(SendPacket,WW_LOADPLAYER_REQUEST,id ,SERVICE_WORLDSERVER);

				SendPacket.writeInt(id,Base::Bit32);
				pSendHead->PacketSize = SendPacket.getPosition() - sizeof( stPacketHead );
				m_pServerService->Send(g_worlddbsocket,SendPacket);
			}
			//needSend = false;
		}
	

		Sleep(0);
	}
}

#endif

void TestIdCode()
{
    int i = 0;
    DWORD startTick = GetTickCount();
    
    while(i++ < 100000)
    {
        static char CharSet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        static int CharSetCount = 36;

        int length = 90 * 20;

        srand( ::GetTickCount() );

        char str1[5] = { 0, };
        char str2[5] = { 0, };
        char str3[5] = { 0, };
        char str4[5] = { 0, };

        char* strSelect[4];
        strSelect[0] = str1;
        strSelect[1] = str2;
        strSelect[2] = str3;
        strSelect[3] = str4;

        for( int j = 0; j < 4; j++ )
        {
            str1[j] = CharSet[int( ( float( rand() ) / RAND_MAX ) * ( CharSetCount - 1 ) )];
        }

        for( int j = 0; j < 4; j++ )
        {
            str2[j] = CharSet[int( ( float( rand() ) / RAND_MAX ) * ( CharSetCount - 1 ) )];
        }

        for( int j = 0; j < 4; j++ )
        {
            str3[j] = CharSet[int( ( float( rand() ) / RAND_MAX ) * ( CharSetCount - 1 ) )];
        }

        for( int j = 0; j < 4; j++ )
        {
            str4[j] = CharSet[int( ( float( rand() ) / RAND_MAX ) * ( CharSetCount - 1 ) )];
        }

        U32 mIdCodeSelect = int( ( float( rand() ) / RAND_MAX ) * 4 );

        if( mIdCodeSelect == 4 )
            mIdCodeSelect = 3;

        char* p = GdiUtil::genImage( strSelect[mIdCodeSelect], 90, 20 );

        delete []p;

        std::cout << i << std::endl;
    }

    DWORD endTick = GetTickCount();

    std::cout << float(endTick - startTick) / (float)i << std::endl;
}

long connectedCount = 0;

class ChatClientPacketProcess : public CommonPacket
{
	typedef CommonPacket Parent;

	int		m_ConnectTimes;
public:
	ChatClientPacketProcess()
	{
		m_ConnectTimes = 0;
        m_startTime = time(0);
        initSend = false;
	}

	void OnConnect(int Error=0);
	void OnDisconnect();

    int nNetLineId;
    int nNetGateId;
    time_t m_startTime;

    bool initSend;

    void SendMax(void)
    {
        char Buffer[MAX_PACKET_SIZE];
        Base::BitStream SendPacket(Buffer,MAX_PACKET_SIZE);
        stPacketHead *pSendHead = IPacket::BuildPacketHead(SendPacket,CLIENT_CHAT_SendMessageRequest, 0, SERVICE_CHATSERVER );

        SendPacket.writeBits((256)* 8,Buffer);

        pSendHead->PacketSize = SendPacket.getPosition() - IPacket::GetHeadSize();

        Send(SendPacket);
    }
protected:
	bool HandleGamePacket(stPacketHead *pHead,int iSize);
	void HandleClientLogin();
	void HandleClientLogout();
};

_inline void ChatClientPacketProcess::OnConnect(int Error)
{
	if(!Error)
		SendClientConnect();
}

_inline void ChatClientPacketProcess::OnDisconnect()
{
	Parent::OnDisconnect();
}

_inline void ChatClientPacketProcess::HandleClientLogin()
{
    InterlockedIncrement(&connectedCount);

	m_ConnectTimes++;

	const char* szIp = "";
	int port = 0;

	RegisterServer(SERVICE_GATESERVER,nNetLineId,nNetGateId,m_ConnectTimes, szIp, port );

	Parent::HandleClientLogin();
}

_inline void ChatClientPacketProcess::HandleClientLogout()
{
	Parent::HandleClientLogout();
}

_inline bool ChatClientPacketProcess::HandleGamePacket(stPacketHead *pHead,int iSize)
{
	//SERVER->GetWorkQueue()->PostEvent(-2,pHead,iSize,true);
	return true;
}

void Send_Chat_Message(ChatPlayerInfo* pSrc,ChatPlayerInfo* pDst,char cMessageType, const char* pMessage)
{
	char Buffer[MAX_PACKET_SIZE];
	Base::BitStream SendPacket(Buffer,MAX_PACKET_SIZE);
    stPacketHead *pSendHead = IPacket::BuildPacketHead(SendPacket,CLIENT_CHAT_SendMessageRequest, pSrc->playerId, SERVICE_CHATSERVER );
	stChatMessage chatMessage;
	chatMessage.btMessageType = cMessageType;
    chatMessage.nRecver = pDst->playerId;
    chatMessage.nSenderLineID = pSrc->lineId;
    dStrcpy(chatMessage.szSenderName,sizeof(chatMessage.szSenderName),pSrc->playerName);

	dStrcpy( chatMessage.szMessage, sizeof(chatMessage.szMessage), pMessage );
	stChatMessage::PackMessage( chatMessage, SendPacket );
	pSendHead->PacketSize = SendPacket.getPosition() - IPacket::GetHeadSize();

    if( m_pChatClient[pSrc->lineId]->GetState() != ISocket::SSF_DEAD )
    {
        m_pChatClient[pSrc->lineId]->Send(SendPacket);
    }
}

void TestChatServer()
{
    std::string ChatServerIP = "192.168.0.65";
    std::string ChatServerPort = "22000";
    
    //连接聊天服务器
    for (size_t i = 0; i < 250; ++i)
    {
        m_pChatClient[i] =	new CommonClient<ChatClientPacketProcess>;	//CommonClient<>(Create_WorldClientPacketProcess)
        m_pChatClient[i]->Initialize(ChatServerIP.c_str(), atoi( ChatServerPort.c_str() ));
        ChatClientPacketProcess* packet = (ChatClientPacketProcess*)m_pChatClient[i]->GetPacket();
    
        packet->nNetLineId = i + 1;
        packet->nNetGateId = 100 + i;

        m_pChatClient[i]->SetConnectType(ISocket::SERVER_CONNECT);
        m_pChatClient[i]->Start();
    }

    //启动模拟的world服务器
    stServerParam Param;

    m_pServerService	=	new CommonServer<WorldPacketProcess>;  //CommonServer<>(Create_UserPacketProcess);
	Param.MinClients	=	1;
	Param.MaxClients	=	10000;
	m_pServerService->Initialize("192.168.0.65",41000,&Param);
	m_pServerService->SetMaxReceiveBufferSize(MAX_PLAYER_STRUCT_SIZE);
	m_pServerService->SetMaxSendBufferSize(MAX_PLAYER_STRUCT_SIZE);
    m_pServerService->Start();

    for (int i = 0; i < 20000; ++i)
    {
        m_chatplayers[i].playerId  = i + 1;
        m_chatplayers[i].accountId = i + 2;

        char name[256] = {0};
        sprintf_s(name,sizeof(name),"char_%d",i);
        strcpy_s(m_chatplayers[i].playerName,sizeof(m_chatplayers[i].playerName),name);

        m_chatplayers[i].lineId = rand()% 250;
        m_chatplayers[i].gateId = m_chatplayers[i].lineId + 99;
    }

    SimpleTimer<1> m_timer;
    SimpleTimer<1> m_timer2;
    
    while(true)
    {
        time_t curTime = time(0);

        //管理和chat的gate连接
        for (size_t i = 0; i < 250; ++i)
        {
            if( m_pChatClient[i] )
            {
                if( m_pChatClient[i]->GetState() == ISocket::SSF_DEAD )
                {
                    m_pChatClient[i]->Restart();
                }

                ChatClientPacketProcess* packet = (ChatClientPacketProcess*)m_pChatClient[i]->GetPacket();

                if (curTime - packet->m_startTime > i + 5 + rand() % 10)
                {
                    //m_pChatClient[i]->Restart();
                    //packet->m_startTime = curTime;
                }
            }
        }

        //注册和反注册玩家
        if( m_pServerService->GetState() != ISocket::SSF_DEAD )
        {
            if (m_timer.CheckTimer())
            {
                for (int iRand = 0; iRand < 1000; ++iRand)
                {
                    int i = rand() % 20000;
                    ChatPlayerInfo& info = m_chatplayers[i];
                    
                    if (rand() % 1000 > 50)
                    {
                        char buf[256];
                        Base::BitStream sendPacket(buf,sizeof(buf));
                        stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket,WORLD_CHAT_AddPlayer,info.playerId,SERVICE_CHATSERVER,info.accountId);

                        sendPacket.writeBits(8 * sizeof(ChatPlayerInfo),&info);

                        pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
                        m_pServerService->Send(sendPacket);
                    }
                    else
                    {
                        char buf[64];
                        Base::BitStream sendPacket(buf,sizeof(buf));
                        stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket,WORLD_CHAT_RemovePlayer,info.accountId,SERVICE_CHATSERVER);
                        pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);

                        m_pServerService->Send(sendPacket);
                    }
                }
            }
        }

        if (m_timer2.CheckTimer())
        {
            //发送聊天消息
            for (int iRand = 0; iRand < 10000; ++iRand)
            {
                int i = rand() % 20000;
                ChatPlayerInfo& src = m_chatplayers[i];
                i = rand() % 20000;
                ChatPlayerInfo& dst = m_chatplayers[i];

                if(src.playerId == dst.playerId)
                    continue;

                Send_Chat_Message(&src,&dst,CHAT_MSG_TYPE_P2P,"ssssssssssssssssssssssssss");
            }

            for (int iRand = 0; iRand < 100; ++iRand)
            {
                int i = rand() % 250;
                MAKE_PACKET(sendPacket,WORLD_CHAT_SendMessageToLine,i,SERVICE_CHATSERVER );
                
                stChatMessage msg;
                dStrcpy(msg.szMessage, sizeof(msg.szMessage),"sdqwdwqdwqdd1212ew12e21");

                stChatMessage::PackMessage(msg,sendPacket);

                if( m_pChatClient[i]->GetState() != ISocket::SSF_DEAD )
                {
                    m_pChatClient[i]->Send(sendPacket);
                }
            }

            for (int iRand = 0; iRand < 100; ++iRand)
            {
                int i = rand() % 250;
                MAKE_PACKET(sendPacket,WORLD_CHAT_SendMessageToWorld,i,SERVICE_CHATSERVER );
                
                stChatMessage msg;
                dStrcpy(msg.szMessage, sizeof(msg.szMessage),"2222222222222DADQDWQDQWDWQQ");

                stChatMessage::PackMessage(msg,sendPacket);

                if( m_pChatClient[i]->GetState() != ISocket::SSF_DEAD )
                {
                    m_pChatClient[i]->Send(sendPacket);
                }
            }

            for (int iRand = 0; iRand < 100; ++iRand)
            {
                int i = rand() % 250;

                char Buffer[MAX_PACKET_SIZE];
                Base::BitStream SendPacket(Buffer,MAX_PACKET_SIZE);
                stPacketHead *pSendHead = IPacket::BuildPacketHead(SendPacket,CLIENT_CHAT_SendMessageRequest, m_chatplayers[i].playerId, SERVICE_CHATSERVER,2);

                stChatMessage chatMessage;
                chatMessage.btMessageType = CHAT_MSG_TYPE_NEARBY;
                chatMessage.nSenderLineID = m_chatplayers[i].lineId;
                dStrcpy(chatMessage.szSenderName,sizeof(chatMessage.szSenderName),m_chatplayers[i].playerName);
                dStrcpy( chatMessage.szMessage, sizeof(chatMessage.szMessage), "sssssssssscq21221" );

                int count = 100;
                SendPacket.writeInt(count,16);

                for (int i = 0; i < count; ++i)
                {
                    int pos = rand() % 250;

                    SendPacket.writeInt(m_chatplayers[i].playerId,32);
                }

                stChatMessage::PackMessage( chatMessage, SendPacket );
                pSendHead->PacketSize = SendPacket.getPosition() - IPacket::GetHeadSize();

                if( m_pChatClient[m_chatplayers[i].lineId]->GetState() != ISocket::SSF_DEAD )
                {
                    m_pChatClient[m_chatplayers[i].lineId]->Send(SendPacket);
                }
            }
        }
    }
}

#define CLIENT_COUNT 1

void TestNetIO()
{
    std::string ChatServerIP = "192.168.0.65";
    std::string ChatServerPort = "22000";

    //连接聊天服务器
    for (size_t i = 0; i < CLIENT_COUNT; ++i)
    {
        m_pChatClient[i] =	new CommonClient<ChatClientPacketProcess>;	//CommonClient<>(Create_WorldClientPacketProcess)
        m_pChatClient[i]->Initialize(ChatServerIP.c_str(), atoi( ChatServerPort.c_str() ));
        ChatClientPacketProcess* packet = (ChatClientPacketProcess*)m_pChatClient[i]->GetPacket();
    
        packet->nNetLineId = 1;
        packet->nNetGateId = i;

        m_pChatClient[i]->SetConnectType(ISocket::SERVER_CONNECT);
        m_pChatClient[i]->Start();

        Sleep(1);
    }

    //启动模拟的world服务器
    stServerParam Param;

    m_pServerService	=	new CommonServer<WorldPacketProcess>;  //CommonServer<>(Create_UserPacketProcess);
	Param.MinClients	=	1000;
	Param.MaxClients	=	3000;
    Param.Name[0] = 0;

	m_pServerService->Initialize("192.168.0.65",22000,&Param);
	m_pServerService->SetMaxReceiveBufferSize(MAX_PLAYER_STRUCT_SIZE);
	m_pServerService->SetMaxSendBufferSize(MAX_PLAYER_STRUCT_SIZE);
    m_pServerService->Start();

    while (connectedCount != CLIENT_COUNT)
    {
        for (size_t i = 0; i < CLIENT_COUNT; ++i)
        {
            if( m_pChatClient[i] )
            {
                if( m_pChatClient[i]->GetState() == ISocket::SSF_DEAD )
                {
                    m_pChatClient[i]->Restart();
                }
            }
        }

        Sleep(1);
    }

    g_Log.WriteLog(".........................................................................");

    while(true)
    {
        time_t curTime = time(0);

        //管理和chat的gate连接
        for (size_t i = 0; i < CLIENT_COUNT; ++i)
        {
            if( m_pChatClient[i] )
            {
                /*if( m_pChatClient[i]->GetState() == ISocket::SSF_DEAD )
                {
                    m_pChatClient[i]->Restart();
                }*/

                ChatClientPacketProcess* packet = (ChatClientPacketProcess*)m_pChatClient[i]->GetPacket();

                if (!packet->initSend && packet->getLoginStatus() == MSG_AUTHED)
                {
                    //if (rand() % 100 > 90)
                    {
                        packet->SendMax();   
                    }
                }
            }
        }

        Sleep(1);
    }
}

#include "MonitorClient.h"

int _tmain(int argc, _TCHAR* argv[])
{
    srand(time(0));

	//int size = sizeof(stPlayerMini);

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);

	//RemoteCount = "50";
	//MaxRemoteCount = "200";
	//CFG_BUIILDER->Get("World_LANAddress", ':', RemoteNetIP, RemoteNetPort);
	//CFG_BUIILDER->Get("Log_LANAddress", ':', LogServerIP, LogServerPort);
	//CFG_BUIILDER->Get("RemoteAccount_LANAddress", ':', RemoteAccountIP, RemoteAccountPort);
	//
	//RemoteNetIP= "192.168.0.65";
	//RemoteNetPort = "41000";

	////TestDataAgent();	
	//
	//_TestWorldDB1();
    //TestIdCode();

    //TestChatServer();
    //TestNetIO();
    
    unsigned int runStatus = 0;

    Monitor_Init(runStatus);

    while(runStatus == 0)
    {
        Sleep(1);
    }
    
    Monitor_Close();


    std::cout << "......................" << std::endl;
    system("pause");
  
	return 0;
}