
#include "stdafx.h"
#include "ranetlib.h"
#include "base/WorkQueue.h"
#include "base/log.h"
#include "base/locker.h"
#include "WinTCP/AsyncSocket.h"
#include "Common/CommonPacket.h"
#include "Common/CommonClient.h"
#include "Common/RALibDef.h"
#include <hash_map>

class PacketProcess : public CommonPacket
{
	typedef CommonPacket Parent;

	int m_ConnectTimes;
public:
    PacketProcess()
    {
        m_ConnectTimes = 0;
    }

    void OnConnect(int Error=0)
    {
        if(!Error)
            SendClientConnect();
    }

    void OnDisconnect()
    {
        Parent::OnDisconnect();

        if(m_ConnectTimes>=1)
        {
            g_Log.WriteError("与服务器断开连接");
        }
    }

	bool HandleGamePacket(stPacketHead *pHead,int iSize);

    void HandleClientLogin()
    {
        m_ConnectTimes++;

        if(m_ConnectTimes>1)
        {
            g_Log.WriteWarn("重新连接上服务器");
        }
        else
        {
            g_Log.WriteLog("连接服务器成功!");
        }

        Parent::HandleClientLogin();
    }
};

const unsigned int MAX_NAME_LENGTH			= 32;

//----------------------------------------------------------------------------------------------------

//封装个remote account的交互
class RaNetLib
{
public:
    RaNetLib(void)
    {
        WSADATA wsaData;
	    WSAStartup(MAKEWORD(2,2), &wsaData);

        m_pClient = new CommonClient<PacketProcess>;
        m_seq = 1;
    }

    ~RaNetLib(void)
    {
        WSACleanup();
        Shutdown();
    }
    
    static RaNetLib* Instance(void)
    {
        static RaNetLib local;
        return &local;
    }

    void Shutdown()
    {
        if (0 != m_pClient)
        {
            delete m_pClient;
            m_pClient = 0;
        }
    }
    
    bool Connect(const char* ip,int port)
    {
        if (0 == m_pClient)
            return false;

        if (0 == ip)
            return false;
        
        stAsyncParam aParam;
	    strcpy_s(aParam.Name,sizeof(aParam.Name),"RemoteClient");
        m_pClient->Initialize(ip,port,&aParam );
	    m_pClient->SetConnectType(ISocket::SERVER_CONNECT);
        
        static bool isFirst = true;
        
        if (isFirst)
        {
            m_pClient->Start();
            isFirst = false;
        }
        else
        {
            m_pClient->Restart();
        }

        //判断客户端是否连接成功
        int count = 30;

        while(count-- >= 0)
        {
            if(m_pClient->GetState() == ISocket::SSF_CONNECTED && 
              ((CommonPacket*)m_pClient->GetPacket())->getLoginStatus() == MSG_AUTHED)
		    {
                return true;
            }

            if (count % 5 == 0)
            {
                m_pClient->Restart();
            }
            
            Sleep(1000);
            g_Log.WriteLog("正在尝试连接服务器%s:%d",ip,port);
        }

        g_Log.WriteLog("连接服务器超时,无法建立连接",ip,port);
        return false;
    }

    void Disconnect(void)
    {
        if (0 == m_pClient)
            return;

        m_pClient->Disconnect();
    }

    char* dStrupr(char *str)
    {
        if (str == NULL)
            return(NULL);

        char* saveStr = str;

        while (*str)
        {
            *str = toupper(*str);
            str++;
        }

        return saveStr;
    }

    int KickAccount(int svrId,const char* _name)
    {
        if (0 == m_pClient)
            return -1;

        if (strlen(_name) > MAX_NAME_LENGTH)
            return -1;

        int seq = InterlockedIncrement(&m_seq);
        
        if (m_seq <= 0)
        {
            m_seq = 1;
        }

        char name[256] = {0};
        strcpy_s(name,sizeof(name),_name);
        dStrupr(name);

        char buf[128];
        Base::BitStream sendPacket(buf,sizeof(buf));
        stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket,RA_MSG_KICK_ACCOUNT,seq);
        
        sendPacket.writeString(name);
        sendPacket.writeInt(svrId,Base::Bit32);

        pHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
        m_pClient->Send(sendPacket);
        
        OLD_DO_LOCK(m_cs);
        
        m_kickRslts[seq] = E_KICK_WAITTING;
        return seq;
    }

    void OnKickRslt(int id,int status)
    {
        OLD_DO_LOCK(m_cs);

        stdext::hash_map<int,int>::iterator iter = m_kickRslts.find(id);

        if (iter == m_kickRslts.end())
            return;
        
        switch(status)
        {
        case E_KICKRESP_KICKED:
            status = E_KICK_KICKED;
            break;
        case E_KICKRESP_NOTFOUND:
            status = E_KICK_NOTFOUND;
            break;
        default:
            status = E_KICK_UNKNOWN;
        };
        
        if (E_KICK_KICKED != iter->second)
        {
            iter->second = status;
        }
    }

    int GetKickRespond(int seq,bool isRemoved)
    {
        OLD_DO_LOCK(m_cs);

        stdext::hash_map<int,int>::iterator iter = m_kickRslts.find(seq);

        if (iter == m_kickRslts.end())
        {
            return E_KICK_NORESP;
        }

        int status = iter->second;

        if (isRemoved && status != E_KICK_WAITTING)
        {
            m_kickRslts.erase(iter);
        }

        return status;
    }

    void ClearKickResponds(void)
    {
        OLD_DO_LOCK(m_cs);
        m_kickRslts.clear();
    }

    CAsyncSocket2* m_pClient;
    long volatile  m_seq;

    //踢线的操作结果
    stdext::hash_map<int,int> m_kickRslts;

    CMyCriticalSection m_cs;
};






//----------------------------------------------------------------------------------------------------

bool PacketProcess::HandleGamePacket(stPacketHead *pHead,int iSize)
{
    char *pData = (char*)pHead + GetHeadSize();
    Base::BitStream recv(pData,iSize - GetHeadSize());

    if (pHead->Message == RA_MSG_KICK_ACCOUNT_RESP)
    {   
        int status = recv.readInt(Base::Bit32);

        RaNetLib::Instance()->OnKickRslt(pHead->Id,status);
    }

	return true;
}

//----------------------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

RANETLIBDECL bool RA_Connect(const char* ip,int port)
{
    return RaNetLib::Instance()->Connect(ip,port);
}

RANETLIBDECL void RA_Disconnect(void)
{
    return RaNetLib::Instance()->Disconnect();
}

RANETLIBDECL int RA_KickAccount(int svrId,const char* name)
{
    if (0 == name)
        return -1;
    
    return RaNetLib::Instance()->KickAccount(svrId,name);
}

RANETLIBDECL int RA_GetKickRespond(int seq,bool isRemoved)
{
    return RaNetLib::Instance()->GetKickRespond(seq,isRemoved);
}

RANETLIBDECL void RA_ClearKickResponds(void)
{
    return RaNetLib::Instance()->ClearKickResponds();
}

#ifdef __cplusplus
}
#endif
