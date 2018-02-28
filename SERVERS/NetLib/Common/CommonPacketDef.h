#pragma once

// 网络消息，需要请求与应答
#define DECLARE_NET_EVENT(p) p##Request, p##Response

// 通知类消息，不需要应答
#define DECLARE_NET_NOTIFY(p) p##Notify


struct NetEventTypes
{
	enum Type
	{
		None,

		CLIENT_SERVER_Login,				// 客户端登录
		CLIENT_SERVER_LoginAck,

		CLIENT_SERVER_Logout,				// 客户端登出
		CLIENT_SERVER_LogoutAck,

		MONITOR_SERVER_RegisterServer,		// 服务器注册

		MONITOR_SERVER_Register,			// 监视器注册
		MONITOR_SERVER_RegisterAck,

		CLIENT_SERVER_ServerList,			// 请求服务器列表
		CLIENT_SERVER_ServerListAck,

		SERVER_CLIENT_ServerOffline,		// 服务器上线
		SERVER_CLIENT_ServerOfflineAck,

		SERVER_CLIENT_ServerOnline,			// 服务器下线
		SERVER_CLIENT_ServerOnlineAck,


		CLIENT_SERVER_RefreshServer,		// 刷新服务器请求
	
		CLIENT_SERVER_StartServer,			// 启动服务
	
		CLIENT_SERVER_StopServer,			// 停止服务
        CLIENT_SERVER_TerminateServer,

		SERVER_MONITOR_StartServer,			// 启动服务
	
		SERVER_MONITOR_StopServer,			// 停止服务

		SERVER_MONITOR_RefreshServer,		// 刷新服务

		SERVER_MONITOR_UpdateServer,		// 更新服务
		
		CLIENT_SERVER_Update,				// 通知服务器更新

		MONITOR_SERVER_ServerStatusUpdate,	// 监视器通知服务器

		SERVER_CLIENT_ServerStatusUpdate,
        
        //监控服务器发送停机指令
		CLIENT_SERVER_STOPSERVERINTIME,
		SERVER_MONITOR_STOPSERVERINTIME,
        MONITOR_STOP_SERVER_REQUEST,
        MONITOR_STOP_SERVER_RESPOND,
        
        MSG_HEARTBEAT,

		Count,		
	};
} ;

template< typename _Ty >
struct StructBase
{
	StructBase()
	{
		memset( this, 0, sizeof(_Ty) );
	}

	StructBase( const _Ty& src )
	{
		memcpy( this, &src, sizeof( _Ty ) );
	}

	StructBase& operator = ( _Ty& src )
	{
		memcpy( this, &src, sizeof( _Ty ) );
		return *this;
	}
};

const int COMMON_STRING_LENGTH_T = 32;

struct ServerStruct : public StructBase<ServerStruct >
{
    ServerStruct(void): LastStatus(0),IsWaitForKill(0),ProcessHandle(0),lineId(0),isSelected(0) {}

    std::string     groupName;
	std::string		ServerName;			// 服务名
    std::string     ip;

	std::string		PathName;			// 启动路径
	std::string		Argument;			// 启动参数

	std::string		UpdateUrl;			// 更新路径
    std::string     updateFile;         // 更新文件下载后的保存路径

	int				SessionId;			// 会话ID
    char			LastStatus;		    // 状态
	char			Status;				// 状态
	char			CPUPayload;			// CPU负载

	bool			IsWaitForKill;		// 等待进程结束
    bool            IsCfgAutoRestart;
	HANDLE			ProcessHandle;		// 进程句柄
    bool            isProcessSelfCreate;// 进程是否自己创建
    HANDLE          processExitHandle;

    std::string     svrUpdateFile;      //下载的服务器更新配置文件路径

	bool			IsAutoRestart;
    int             lastHeartbeatTime;
    int             lineId;
    bool            isSelected;

    std::string GetId()
    {
        char buf[256] = {0};
        itoa(lineId,buf,10);
        return groupName + "-" + ServerName + "-" + ip + "-" + buf;
    }
};

struct ServerStatus
{
	enum Type
	{
		None,
		Start,
        ReStart,
		Stop,
        PassiveStop,
		Unknown,
		Offline,
		Update,
        Dead,
        UPDATE_STATUS_START,
        UPDATE_STATUS_DOWNLOAD,
        UPDATE_STATUS_PROGRASS,
        UPDATE_STATUS_FINISHED,
        UPDATE_STATUS_ERROR,
		Count
	};

    std::string groupName;
    std::string serverName;

	int type;
	int CPUPayload;

	ServerStatus()
	{
		type = None;
		CPUPayload = 0;
	}
};

static CPacketStream& operator >> ( CPacketStream& packet, ServerStatus& serverStatus )
{
	packet >> serverStatus.groupName >> serverStatus.serverName >> (int&)serverStatus.type >> serverStatus.CPUPayload;
	return packet;
}

static CPacketStream& operator << ( CPacketStream& packet, ServerStatus& serverStatus )
{
	packet << serverStatus.groupName << serverStatus.serverName << (int&)serverStatus.type << serverStatus.CPUPayload;
	return packet;
}