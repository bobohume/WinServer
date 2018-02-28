#include "wintcp/dtServerSocketClient.h"
#include "wintcp/dtServerSocket.h"
#include "Common/CommonServer.h"
#include "Common/CommonClient.h"

#define MSG_HEARTBEAT 0x44f5

class UdpLog
{
    class UdpLogClient: public CommonPacket
    {
        typedef CommonPacket Parent;
    public:
        UdpLogClient(void)  
        {
            UdpLog::Instance()->m_pClient = this;
        }
        
        ~UdpLogClient(void) 
        {
            UdpLog::Instance()->m_pClient = 0;
        }

        void OnConnect(int Error=0) 
        {
            //m_isConnected = true;  
			UdpLog::Instance()->m_pClient->SendClientConnect();
        }
        
        void OnDisconnect()
        {
            Parent::OnDisconnect();
            m_isConnected = false;
        }

        bool HandleGamePacket(stPacketHead *pHead,int iSize)
        {
            return true;
        }
        
        void HandleClientLogin()
        {
            m_isConnected = true;
			Parent::HandleClientLogin();
        }
        
        void HandleClientLogout() 
		{
			Parent::HandleClientLogout();
		}

        bool m_isConnected;
    };

	UdpLogClient*  m_pClient;
	CAsyncSocket2* m_pClientSkt;

public:
    UdpLog(void) : m_pClient(0)
    {
        m_pClientSkt = new CommonClient<UdpLogClient>;
		m_pClientSkt->Initialize("127.0.0.1",10086);
		m_pClientSkt->SetConnectType(ISocket::SERVER_CONNECT);
    }
    
    static UdpLog* Instance(void)
    {
        static UdpLog local;
        return &local;
    }

    //·¢ËÍÐÄÌø
    void SendHeartBeat(bool first)
    {
        if (0 == m_pClientSkt)
            return;
        
        if (first || m_pClientSkt->GetState() == ISocket::SSF_SHUT_DOWN)
        {
            m_pClientSkt->Start();
        }

        static int count = 1;
        
        if (count % 20 == 0)
        {
            if (m_pClientSkt->GetState() == ISocket::SSF_DEAD)
            {
                m_pClientSkt->Restart();
            }
        }
        
        count++;
        
        if (0 == m_pClient || !m_pClient->m_isConnected)
        {
            return;
        }

        static int interval = 0;

        if (interval % 10 == 0)
        {
            unsigned __int64 curProcessId = GetCurrentProcessId();

            char buf[60];
            Base::BitStream sendPacket( buf, sizeof(buf) );
            stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket,"MSG_HEARTBEAT");

            sendPacket.writeBits(Base::Bit64,&curProcessId);

            pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
            m_pClientSkt->Send(sendPacket,PT_DROP);
        }

        interval++;
    }
};