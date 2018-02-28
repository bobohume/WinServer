#ifndef _PACKETPROCESS_H_
#define _PACKETPROCESS_H_

#include "wintcp/IPacket.h"
#include "PacketBase.h"
#include "CommLib.h"

//数据包容器,用来在指定环境下运行数据包的执行函数
class PacketProcess
{
    enum PACKET_FLAG
    {
        PF_NONE   =0,
        PF_REMOVE,
    };

    struct ASYNC_PACKET
    {
        PacketBase*	 m_pPacket;
        int	         m_sktHandle;
        stPacketHead m_head;
        
        //预留做为后续包的删除标志
        U32		     m_Flag;

        ASYNC_PACKET(void)
        {
            memset(this,0,sizeof(ASYNC_PACKET));
        };
        
        ~ASYNC_PACKET(void)
        {
            SAFE_DELETE(m_pPacket);
        };
    };
    
public:
	PacketProcess(void);
	virtual ~PacketProcess(void);
	
	void SetCtxId(int ctxId) { m_ctxId = ctxId;}
	
	//投递数据包,数据包被放存在缓冲中,并在run时处理
	virtual bool    SendPacket(PacketBase* pPacket,int SocketHandle,stPacketHead *pHead);

	virtual int	    run(void* param);
	
	virtual void	stop(void) { m_isActive = false;}
	
	bool			IsActive(void) const {return m_isActive;}

	bool			IsFinished(void);
private:
    bool __ResizeCache(void);
    bool __RecvPacket(PacketBase*& pPacket,int& SocketHandle,stPacketHead*& pHead,U32& Flag);
    
    int  m_ctxId;
	bool m_isActive;
	bool m_isFinished;

	//当前模块的消息缓存
	ASYNC_PACKET* m_PacketQue;
	U32			  m_QueSize;
	U32			  m_Head;
	U32			  m_Tail;
	
	CMyCriticalSection m_cs;
};

#endif /*_PACKETPROCESS_H_*/