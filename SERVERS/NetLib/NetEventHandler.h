template< int _NetMsgCount, typename _Ty >
class CNetEventHandler
{
	typedef void ( _Ty::*Type)( CNetSession::ID_HANDLE sessionId, t_packetHeader* pHead, CPacketStream* packet );
public:
	CNetEventHandler()
	{
		memset( mEvents, 0, sizeof( mEvents ) );
	}

	template< typename _T >
	void registerEvent( int netMsgCode, _T method )
	{
		mEvents[netMsgCode] = (Type)method;
	}

	void onEvent( int netMsgCode, CNetSession::ID_HANDLE sessionId, t_packetHeader* pHead, CPacketStream* packet )
	{
		if( mEvents[netMsgCode] != NULL )
			(((_Ty*)this)->*mEvents[netMsgCode])( sessionId, pHead, packet );
	}

private:
	Type mEvents[_NetMsgCount];
};

#define NET_EVENT_PARAMS CNetSession::ID_HANDLE sessionId, t_packetHeader* pHead, CPacketStream* pPacket
