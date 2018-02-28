#include "DataAgentHelper.h"
#include "zlib/zlib.h"
#include "QuickLz/quicklz.h"
#include "PlayerStructEx.h"

CDataAgentHelper* g_DataAgentHelper = NULL;

CDataAgentHelper::CDataAgentHelper()
{
	m_bIsReady		= false;
	m_pClient		= NULL;
}


CDataAgentHelper::~CDataAgentHelper()
{
	Close();
}

void CDataAgentHelper::Init( const char* szIpAddress, int nPort )
{
	m_pClient = new CommonClient<DataAgentProcess>;
	m_pClient->Initialize( szIpAddress, nPort );
	m_pClient->SetConnectType( ISocket::SERVER_CONNECT );

	m_pClient->Start();
	
	g_DataAgentHelper = this;
}

void CDataAgentHelper::Close()
{
	if( m_pClient )
	{
		delete m_pClient;
		m_pClient = NULL;
	}

	g_DataAgentHelper = NULL;
}

bool CDataAgentHelper::Send( int nId, int nType, Base::BitStream& stream, int playerId /*= 0*/, int sid /*= 0 */ )
{
	if( m_pClient->GetState() != ISocket::SSF_CONNECTED )
		return false;

	CMemGuard buf( MAX_PACKET_SIZE MEM_GUARD_PARAM);
	CMemGuard b( MAX_PLAYER_STRUCT_SIZE MEM_GUARD_PARAM);
	Base::BitStream SendPacket( buf.get(), MAX_PACKET_SIZE );
	stPacketHead* pHead = IPacket::BuildPacketHead( SendPacket, SERVER_DATAAGENT_Message, nId, SERVICE_DATAAGENT, playerId, sid );

	// copy stream to SendPacket
	int nLength = ( stream.getPosition() - sizeof( stPacketHead ) ) * 8;
	int nPos = stream.getPosition();

	if( nLength < 0 )
		nLength = 0;

	stream.setPosition( sizeof( stPacketHead ) );
	stream.readBits( nLength, b.get() );
	stream.setPosition( nPos );

	nLength /= 8;

	CMemGuard zipBuf( MAX_PACKET_SIZE MEM_GUARD_PARAM);
	int ZipBufSize  = MAX_PACKET_SIZE;
	int ret = SXZ_COMPRESS( zipBuf.get(), &ZipBufSize, b.get(), nLength);

	if( !ret )
	{
		SendPacket.writeFlag( true );
		SendPacket.writeInt( ZipBufSize, 16 );

		SendPacket.writeBits( ZipBufSize * 8, zipBuf.get() );

	}
	else
	{
		SendPacket.writeFlag( false );
	}

	pHead->PacketSize = SendPacket.getPosition() - sizeof( stPacketHead );
	m_pClient->Send( SendPacket );

	return true;
}

void CDataAgentHelper::Check()
{
	if( m_pClient )
	{
		if( m_pClient->GetState() == ISocket::SSF_DEAD )
			m_pClient->Restart();
	}
}

CAsyncSocket2* CDataAgentHelper::getSocket()
{
	return m_pClient;
}

void CDataAgentHelper::DataAgentProcess::OnConnect(int Error/* =0 */)
{
	if( !Error)
		SendClientConnect();
}

void CDataAgentHelper::DataAgentProcess::OnDisconnect()
{
	if( g_DataAgentHelper )
		g_DataAgentHelper->m_bIsReady = false;

	Parent::OnDisconnect();

	g_Log.WriteLog("与DataAgent断开连接");
}

void CDataAgentHelper::DataAgentProcess::HandleClientLogin()
{
	if( g_DataAgentHelper )
		g_DataAgentHelper->m_bIsReady = true;

	g_Log.WriteLog("与DataAgent建立连接");

	Parent::HandleClientLogin();
}

void CDataAgentHelper::DataAgentProcess::HandleClientLogout()
{
	Parent::HandleClientLogout();

}

bool CDataAgentHelper::DataAgentProcess::HandleGamePacket(stPacketHead *pHead,int iSize)
{
	return true;
}

