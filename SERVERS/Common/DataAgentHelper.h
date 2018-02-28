#ifndef DATA_AGENT_HELPER_H
#define DATA_AGENT_HELPER_H

#include "base/memPool.h"
#include "base/bitStream.h"
#include "CommonClient.h"
#include "CommonPacket.h"
#include "wintcp/AsyncSocket.h"
#include "wintcp/dtServerSocket.h"
#include "wintcp/dtServerSocketClient.h"


// used for send important information to the DataAgent Server
class CDataAgentHelper
{
public:
	CDataAgentHelper();
	virtual ~CDataAgentHelper();

	void Init( const char* szIpAddress, int nPort );
	void Close();

	bool Send( int nId, int nType, Base::BitStream& stream, int playerId = 0, int sid = 0 );
	void Check();
	CAsyncSocket2* getSocket();
private:
	static int WorkQuene( LPVOID param );


	class DataAgentProcess
		: public CommonPacket
	{
		typedef CommonPacket Parent;
	public:
		void OnConnect(int Error=0);
		void OnDisconnect();
	protected:
		bool HandleGamePacket(stPacketHead *pHead,int iSize);
		void HandleClientLogin();
		void HandleClientLogout();
	};

	bool				m_bIsReady;
	CAsyncSocket2*		m_pClient;
};

#endif

