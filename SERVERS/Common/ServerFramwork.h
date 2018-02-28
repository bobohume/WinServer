#ifndef SERVER_FRAMEWORK_H
#define SERVER_FRAMEWORK_H

#include "CommLib/CommLib.h"

#pragma warning( disable: 4356 )

#include "wintcp/dtServerSocket.h"
#include "WaitObject.h"
#include "Common/Script.h"

#include "base/WorkQueue.h"
#include "base/Locker.h"
#include "base/bitStream.h"
#include "common/CommonPacket.h"
#include "common/CommonServer.h"
#include "WINTCP\dtServerSocketClient.h"

#include "Common/CppTypeInfo.h"
#include "CommLib/ServerCloseMonitor.h"

#include <string>

static int ret_error( int error, std::string msg )
{
	g_Log.WriteError( msg );
	return error;
}

typedef CTI::Delegate<int, int, int, stPacketHead*, Base::BitStream* > ClassEventFunction;

#define ServerEventFunction(c,funct,message)  \
	bool __ef_##funct(int socketId, stPacketHead* phead, Base::BitStream* pPack ); \
	EventFunctionBuilder<c> _ef_##funct##builder( message, __ef_##funct ); \
	bool __ef_##funct(int socketId, stPacketHead* phead, Base::BitStream* pPack )

template< typename ServerClass >
struct EventFunctionBuilder
{
	template< typename _Ty >
	EventFunctionBuilder( const char* msg, _Ty function )
	{
		ServerClass::getInstance()->mMsgCode.Register(msg, function);
	}
};

struct IServerMoudle
{
	virtual ~IServerMoudle() {}
};

template<class T>
class CServerFramework : public CommonPacket
{
protected:

	friend class CommonServer<T>;

	struct t_server_param
	{
		int				workThreadCount;
		int				port;
		const char*		ipAddr;
		stServerParam	param;
	};


protected:
	CServerFramework()
	{
		mServer				= NULL;
		mWorkQueue			= NULL;
	}

	virtual ~CServerFramework()
	{
		SAFE_DELETE( mServer );
		SAFE_DELETE( mWorkQueue );
	}

public:
	void init( const char* szServerName)
	{
		mWorkQueue = new CWorkQueue();
		mServer = new CommonServer< T >;

		ISocket::InitNetLib();

		if(onInit() == false)
		{
			g_Log.WriteError(">>>>>>>>>>>>>>>>>>>>>>>服务启动失败<<<<<<<<<<<<<<<<<<<<<<<");
			exit(1);
		}
		else
		{
			g_Log.WriteLog("************************服务已启动*************************");
		}		
	}

	virtual bool onInit() { return true; }

	virtual void onShutdown() {}

	virtual bool onLogic(void* param) { return true; }

	virtual void onTimer(void* param) {}

	virtual void onConfigChange(void* param) {}

	void shutdown()
	{
		if( mServer )
			mServer->Stop();

		if( mWorkQueue )
			mWorkQueue->Stop();

		onShutdown();

		ISocket::UninitNetLib();
	}

	virtual void onMainLoop()
	{

	}

	inline dtServerSocket* getServerSocket()
	{
		return mServer;
	}

	void main(U32& runMode)
	{
		while(!IsClosed(runMode))
		{
			onMainLoop();
            Sleep(1000);
		}
	}

	virtual void onWork()
	{

	}
    
    virtual bool IsClosed(U32& runMode)
    {
        return runMode == SERVER_RUN_REQUESTCLOSE;
    }

    virtual void onDisconnected(int id)
	{

	}

	inline CWorkQueue* getWorkQueue()
	{
		return mWorkQueue;
	}

	static int _eventProcess( LPVOID param )
	{
		WorkQueueItemStruct *pItem = (WorkQueueItemStruct*)param;

		switch(pItem->opCode)
		{
		case WQ_CONNECT:
			//SERVER->AddClient(pItem->Id,pItem->Buffer);
			break;
		case WQ_DISCONNECT:
            getInstance()->onDisconnected(pItem->Id);
			break;
		case WQ_PACKET:
			{
				int msg = 0;
				try
				{	
					stPacketHead* pHead = (stPacketHead*)pItem->Buffer;
					msg = pHead->Message;
					if(getInstance()->mMsgCode.IsValid(pHead->Message))
					{
						char *pData	= (char *)(pHead) + IPacket::GetHeadSize();
						Base::BitStream RecvPacket(pData,pItem->size-IPacket::GetHeadSize());
						Base::BitStream* pPacket = &RecvPacket;
						return getInstance()->mMsgCode.Trigger(pItem->Id,pHead,RecvPacket);
					}
				}
				catch( ... )
				{
                    g_Log.WriteError( "严重错误：未处理异常[WQ_PACKET: MSGCODE=%s]", SHOWMSG_EVENT(msg));
				}
			}
			break;
		 case WQ_CONFIGMONITOR:
			{
				getInstance()->onConfigChange(pItem->Buffer);
			}
			break;
		case WQ_TIMER:
			{
				getInstance()->onTimer(pItem->Buffer);
			}
			break;
		case WQ_LOGIC:
			{
				getInstance()->onLogic(pItem->Buffer);
				pItem->Buffer = 0;
			}
			break;
		default:
			break;
		}

		return false;
	}

protected:
	typedef CommonPacket Parent;

	_inline void HandleClientLogin()
	{
		char IP[COMMON_STRING_LENGTH]="";

		char *pAddress = dynamic_cast<dtServerSocketClient *>(m_pSocket)->GetConnectedIP();
		if(pAddress)
			sStrcpy(IP,COMMON_STRING_LENGTH,pAddress,COMMON_STRING_LENGTH);
		
		T::getInstance()->getWorkQueue()->PostEvent(m_pSocket->GetClientId(),IP,sStrlen(IP,COMMON_STRING_LENGTH)+1,true,WQ_CONNECT);
		
		Parent::HandleClientLogin();
	}

	_inline void HandleClientLogout()
	{
		T::getInstance()->getWorkQueue()->PostEvent(m_pSocket->GetClientId(),NULL,0,false,WQ_DISCONNECT);
		Parent::HandleClientLogout();
	}

	virtual bool HandleGamePacket(stPacketHead *pHead,int iSize)
	{
		T::getInstance()->getWorkQueue()->PostEvent(m_pSocket->GetClientId(),pHead,iSize,true);
		return true;
	}

protected:	
	dtServerSocket*			mServer;
	CWorkQueue*				mWorkQueue;

public:
	MessageCode				mMsgCode;

	static T* getInstance()
	{
		if( mInstance == NULL )
			mInstance = new T();

		return mInstance;
	}

private:
	static T* mInstance;
};

template<class T> T* CServerFramework<T>::mInstance = NULL;

#endif