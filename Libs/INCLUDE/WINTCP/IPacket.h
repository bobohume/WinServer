#pragma once

#ifndef _IPACKET_H_
#define _IPACKET_H_

#include "Base/Locker.h"
#include "Base/Log.h"
#include "Base/bitStream.h"
#include "MessageCode.h"

#define STX                             0x27        //包文起始标识
#define CKX                             0x72        //包文校验标识

#pragma pack(push, IPACKET, 1)
struct stPacketHead
{
	char Stx;					//包标识
	char LineServerId;			//线路编号
	char DestServerType;		//目标服务器
	char Ckx;
    unsigned int Message;  		//网络消息
	unsigned int SrcZoneId;     //地图服务器编号
	unsigned int DestZoneId;	//目标
	int	 Id;					//玩家标识
	int  RSV;					//TCP包的CRC校验值，UDP转发会带IP，
	UINT PacketSize;			//包内容大小    
};
#pragma pack(pop, IPACKET)

class ISocket;
class IPacket
{
	CMyCriticalSection m_cs;    //多线程，临界区锁

	//加密处理
	virtual bool EncodePacket(stPacketHead *,char *&);
	virtual bool DecodePacket(stPacketHead *,char *&);

protected:
	//Socket
	ISocket *m_pSocket;
public:
	static int	GetHeadSize();
	static int	GetTotalSize(const char *pInBuffer);
	static bool	CheckHead(const char *pInBuffer);
	static stPacketHead* BuildCheckMark(const char *pInBuffer,int nSize);
	static const char *SeekToHead(const char *pInBuffer,int &nBufferSize);  //寻找包头（根据stx标示）
	//static stPacketHead *BuildPacketHead(Base::BitStream &Packeter,unsigned short Msg,int Id = 0,char DestServerType=0,unsigned long DestZoneId=0,unsigned long SrcZoneId=0,char LineServerId=0);
	static stPacketHead *BuildPacketHead(Base::BitStream &Packeter,const char* Msg,int Id = 0,char DestServerType=0,unsigned long DestZoneId=0,unsigned long SrcZoneId=0,char LineServerId=0);
	static char *BuildPacketHead(stPacketHead *pHead,const char* Msg,int Id = 0,char DestServerType=0,unsigned long DestZoneId=0,unsigned long SrcZoneId=0,char LineServerId=0);
	virtual bool HandlePacket(const char *pInData,int InSize) = 0;
	virtual void OnErrorPacket(const char *pInData,int InSize);
public:
	IPacket();
	virtual ~IPacket() {}
	virtual bool Send(char *,int,int ctrlType=0);

	virtual void Initialize();
	void BindSocket(ISocket *pSocket);
	ISocket *GetSocket(){return m_pSocket;};
};

#endif