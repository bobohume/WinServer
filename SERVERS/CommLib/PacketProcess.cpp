#include "stdafx.h"

#include "PacketProcess.h"
#include "PacketFactoryManager.h"

#define MAX_CACHE_SIZE 2048

PacketProcess::PacketProcess(void)
{   
    m_isActive  = true;
	m_isFinished= true;
    m_PacketQue = new ASYNC_PACKET[MAX_CACHE_SIZE] ;
    assert(m_PacketQue);
    m_QueSize   = MAX_CACHE_SIZE ;
    m_Head      = 0;
    m_Tail      = 0;
}

PacketProcess::~PacketProcess(void)
{
    SAFE_DELETE_ARRAY(m_PacketQue);
    m_isActive = false;
    m_QueSize  = 0;
    m_Head     = 0;
    m_Tail     = 0;
}

int PacketProcess::run(void* param)
{
    bool ret = false;
    PacketBase* pPacket = 0;
    stPacketHead *pHead = 0;
    int SocketHandle    = 0;
    U32 Flag = 0;

    for(U32 i = 0; i < m_QueSize; ++i)
    {
        ret = __RecvPacket(pPacket,SocketHandle,pHead,Flag);

        if( !ret )
            break;

        assert(pPacket);

        if( Flag == PF_REMOVE )
        {
            SAFE_REMOVE_PACKET(pPacket);
            continue;
        }

        bool bNeedRemove = true ;

        _MY_TRY
        {
            U32 uret = pPacket->Execute(m_ctxId,pHead,SocketHandle,param);
                
            if(uret == PACKET_NOTREMOVE)
            {
                bNeedRemove = false;
            }
        }
        _MY_CATCH
        {
			g_Log.WriteError("PacketProcess 执行异常");
        }

        //回收消息
        if(bNeedRemove)
        {
            SAFE_REMOVE_PACKET(pPacket);
        }
    }
    
    return 0;
}

bool PacketProcess::__RecvPacket(PacketBase*& pPacket,int& SocketHandle,stPacketHead*& pHead,U32& Flag)
{
    OLD_DO_LOCK(m_cs);

    if( m_PacketQue[m_Head].m_pPacket == NULL)
    {	
		//已经执行完所有的包
		m_isFinished = true;
        return false ;
    }

    pPacket      = m_PacketQue[m_Head].m_pPacket;
    SocketHandle = m_PacketQue[m_Head].m_sktHandle;
    pHead        = &m_PacketQue[m_Head].m_head;
    Flag         = m_PacketQue[m_Head].m_Flag;

    m_PacketQue[m_Head].m_pPacket   = NULL;
    m_PacketQue[m_Head].m_sktHandle = 0;
    m_PacketQue[m_Head].m_Flag      = PF_NONE;

    m_Head++;

    if( m_Head >= m_QueSize ) 
        m_Head = 0;

    return true;
}

bool PacketProcess::__ResizeCache(void)
{
    ASYNC_PACKET* pNew = new ASYNC_PACKET[m_QueSize + MAX_CACHE_SIZE];

    if(pNew == NULL)
        return false ;

    memcpy(pNew,&(m_PacketQue[m_Head]),sizeof(ASYNC_PACKET) * (m_QueSize-m_Head));

    if( m_Head != 0 )
    {
        memcpy(&(pNew[m_QueSize - m_Head]), &(m_PacketQue[0]), sizeof(ASYNC_PACKET) * (m_Head));
    }

    memset(m_PacketQue,0,sizeof(ASYNC_PACKET) * m_QueSize);
    SAFE_DELETE_ARRAY(m_PacketQue);
    m_PacketQue = pNew;

    m_Head    = 0 ;
    m_Tail    = m_QueSize ;
    m_QueSize = m_QueSize + MAX_CACHE_SIZE ;

    return true ;
}

bool PacketProcess::SendPacket(PacketBase* pPacket,int SocketHandle,stPacketHead *pHead)
{
    OLD_DO_LOCK(m_cs);

	m_isFinished = false;

    if(m_PacketQue[m_Tail].m_pPacket)
    {
        //缓冲区满
        bool ret = __ResizeCache();

        if (!ret)
        {
            assert(0);
            return false;
        }
    }

    m_PacketQue[m_Tail].m_pPacket   = pPacket;
    m_PacketQue[m_Tail].m_sktHandle = SocketHandle;
    m_PacketQue[m_Tail].m_head      = *pHead;

    m_Tail++;

    if( m_Tail >= m_QueSize ) 
        m_Tail = 0;

    return true ;
}

bool PacketProcess::IsFinished(void)
{
	OLD_DO_LOCK(m_cs);
	return m_isFinished;
}