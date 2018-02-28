 #pragma once

#include <sstream>
#include <memory>

#include "build_no.h"
#include "DBLayer/Data/DBError.h"
#include "zlib/zlib.h"

#ifndef USE_QUICKLZ
#define SXZ_COMPRESS(a,b,c,d)	compress((Bytef*)a,(u_long*)b, (const Bytef*)c, (u_long)d)
#define SXZ_UNCOMPRESS(a,b,c,d)	uncompress((Bytef*)a,(u_long*)b, (const Bytef*)c, (u_long)d)
#else
#define SXZ_COMPRESS(a,b,c,d)	qz_compress((char*)a,(unsigned long*)b, (const char*)c, (unsigned long)d)
#define SXZ_UNCOMPRESS(a,b,c,d)	qz_uncompress((char*)a,(unsigned long*)b, (const char*)c, (unsigned long)d)
#endif//USE_QUICKLZ

     
const unsigned int MIN_PACKET_SIZE			= 128 * 10;
const unsigned int MAX_PACKET_SIZE			= 256*1024;
const unsigned int COMMON_STRING_LENGTH		= 32;
const unsigned int NETWORK_PROTOCOL_VERSION	= 1;
const unsigned int MD5_STRING_LENGTH		= 16;
const unsigned int MAX_ACTOR_COUNT			= 1;
const unsigned int MAX_DROP_TIME			= 30000;
const unsigned int MAX_CREATE_ACTOR_COUNT	= 1;
const unsigned int URL_STRING_LENGTH		= 1024;
const unsigned int MAX_PASS9_ERRMSG_LEN		= 128;
const unsigned int MAX_YY_REQUEST_CODE_LEN	= 1024;
const unsigned int MAX_PASS9_USERNAME_LEN	= 50;
const unsigned int MAX_PASS9_TICKET_LEN		= 32;
const unsigned int MAX_OPENID_LEN			= 128;
const unsigned int ACCOUNT_NAME_LENGTH		= 100;
const unsigned int PASSWORD_LENGTH			= 50;
const unsigned int MAX_NETBAR_DRAW			= 10;
const unsigned int MAX_BADWORDS_LENGTH		= 250;

/*--------------------------------------------------------------------------------------------*/

#define ITOA(NUMBER)  #NUMBER
#define __TEXT_LINE__(LINE) ITOA(LINE)
#ifdef _DEBUG
#define HACK(MSG) __FILE__"("__TEXT_LINE__(__LINE__)") : ★HACK★ → "#MSG
#else
#define HACK(MSG) __FILE__"("__TEXT_LINE__(__LINE__)") : HACK!! Error: "#MSG
#endif

#define DB_CATCH_LOG(TAG) 		catch (CDBException &e){	\
	std::stringstream LogStream; \
	LogStream << "Error Message("<<#TAG << "):"<< e.w_msgtext <<  std::endl << e.m_dberrstr << std::ends; \
	g_Log.WriteError(LogStream); \
}catch (...){ \
	std::stringstream LogStream; \
	LogStream << "Error Message("<<#TAG << "):非自定义异常" << std::ends; \
	g_Log.WriteError(LogStream); \
}


#define T_UID	 unsigned int
#define UID_BITS 32

typedef unsigned int t_queueToken;

// 以下宏重定义 [10/11/2010 cwm]
//#define __ENTER_FUNCTION {try{	
//
//#define __LEAVE_FUNCTION }catch(...){}}

enum LOGIN_STATUS
{
	SF_FROM_ACCOUNT,
	SF_FROM_WORLD,
	SF_FROM_GATE,
	SF_FROM_ZONE
};

enum SERVER_TYPE
{
	SERVICE_NONE,
	SERVICE_CLIENT,
	SERVICE_REMOTESERVER,
	SERVICE_ZONESERVER,
	SERVICE_ACCOUNTSERVER,
	SERVICE_WORLDSERVER,
	SERVICE_GATESERVER,
	SERVICE_GAMESERVER,
	SERVICE_REGAMESERVER,			// 游戏中重连GATE
	SERVICE_DATAAGENT,
	SERVICE_LOGSERVER,
	SERVICE_REGISTER,
	SERVICE_QUEUE,					// 排队专用firera
	SERVICE_CHATSERVER,				// 聊天服务
	SERVICE_WORLDDB,
	SERVICE_GETSTATUS,				// 取服务器状态
	SERVICE_TWEIBO,					// 腾讯微博登陆
	SERVICE_YYLOGIN,				// YY登陆
};

enum GAME_LOGIC_MSG
{
	BEGIN_LOGIC_MSG,
	ON_ZONE_RESTART,
	END_LOGIC_MSG,
};

enum EM_MSG
{
	MSG_INVALID,
	MSG_CONNECT,
	MSG_CONNECTED,
	MSG_AUTH,
	MSG_AUTHED,
	MSG_INFO,//请求account获取账号角色信息
	MSG_INFO_RET,//account发送账号角色信息给客户端

	MSG_END,
};

//_inline bool IsValidMessage(int Msg) {return Msg>BEGIN_NET_MESSAGE && Msg<END_NET_MESSAGE;}

#define ERROR_CODE int

enum LogicErr
{
	NONE_ERROR = 0,

	VERSION_ERROR = DBERR_MAX,			//版本号错误
	LANG_VERSION_ERROR,					//语言版本错误

	GAME_SERVER_INVALID,				//选择的服务器不可用
	GAME_UID_ERROR,						//会话过期(UID不符合)
	GAME_VCODE_ERROR,					//验证码不正确
	GAME_LINE_FULL,						//线路已满
	GAME_ZONE_NOFOUND,					//地图未找到(地图为开或者服务器为配置此地图)
	GAME_WORLD_FULL,					//服务器达到最大承载人数

	ACCOUNT_ID_ERROR,					//登录帐号错误(游戏内帐号错误)
	ACCOUNT_IN_USEING,                  //帐号在其他地方已登陆,提醒玩家过一会登陆。
	ACCOUNT_FREEZE,                     //帐号被冻结

	PLAYER_CREATE_FULL,					//角色建满
	PLAYER_NOFOUND,						//帐号未登录或者角色不存在
	PLAYER_NAME_ERROR,					//名字错误
	PLAYER_OLDPLAYER_BONUS_ERROR,		//老玩家登录奖励错误
	PLAYER_FIRSTLOGIN_BONUS_ERROR,		//新机器首次登陆奖励错误
	PLAYER_PUNISH,						//登陆验证码输入错误3次，5分钟内不能登陆
	PLAYER_STRUCT_ERROR,				//玩家角色异常或者转换失败
	UNKNOW_ERROR,						//未知错误

	ILLEGAL_GM,							//数据库中未匹配到登陆的gm帐号的ip和mac地址

    SERVER_NOT_OPENED,                  //服务器未开放
    

	ERROR_COUNT							
};

enum ERROR_SOURCE
{
	LOGIN_ACCOUNT_SOURCE,				//登陆帐号服务器出错
	LOGIN_GATE_SOURCE,					//登陆网关出错
	LOGIN_GAME_SOURCE,					//登陆游戏出错
	SWITCH_MAP_SOURCE,					//切换地图出错
	CREATE_PLAYER_SOURCE,				//创建角色出错
	DELETE_PLAYER_SOURCE,				//删除角色出错
};

enum ePlayerDataTransferType
{
	PLAYER_TRANS_TYPE_BASE = 0,
	PLAYER_TRANS_TYPE_NORMAL,
	PLAYER_TRANS_TYPE_ADV,

	PLAYER_TRANS_TYPE_END,
};

#define MAKE_PACKET( packetName, ... ) \
	CMemGuard buf##packetName( MAX_PACKET_SIZE MEM_GUARD_PARAM ); \
	Base::BitStream packetName( buf##packetName.get(), MAX_PACKET_SIZE ); \
	stPacketHead* packetName##Head =  IPacket::BuildPacketHead( packetName, __VA_ARGS__ );

#define SEND_PACKET( packetName, sockId ) \
	packetName##Head->PacketSize = packetName.getPosition() - sizeof( stPacketHead ); \
	SERVER->GetServerSocket()->Send( sockId, packetName );

#define SEND_PACKET_SOCK( packetName, sockId, _sock ) \
	packetName##Head->PacketSize = packetName.getPosition() - sizeof( stPacketHead ); \
	(_sock)->Send( sockId, packetName );

#define SEND_PACKET_NSOCK( packetName, _sock ) \
	packetName##Head->PacketSize = packetName.getPosition() - sizeof( stPacketHead ); \
	(_sock)->Send( packetName );

#define PASS9 "pass9"

struct WorldServerInfo
{
	static const int MAX_WORLD_COUNT = 256;

	int AreaId[MAX_WORLD_COUNT];
	int PlayerCount[MAX_WORLD_COUNT];
	int maxPlayerCount[MAX_WORLD_COUNT];
	int maxCreatedPlayers[MAX_WORLD_COUNT];

	int Count;
	char operation[COMMON_STRING_LENGTH];

	WorldServerInfo()
	{
		memset(this, 0, sizeof(WorldServerInfo));
		strcpy_s(operation, sizeof(operation), PASS9);
	}

	void convertPlayer(float status1, float status2)
	{
		for( int i = 0; i < Count; i++ )
		{
			float percent = 0;

			if (0 != maxPlayerCount[i])
			{
				percent = (float)PlayerCount[i] / (float)maxPlayerCount[i];
			}
            
			if(percent < status1)
				PlayerCount[i] = 1;
			else if(percent < status2)
				PlayerCount[i] = 2;
			else 
				PlayerCount[i] = 3;
		}
	}

	template< typename _Ty >
	void ReadData( _Ty* packet )
	{
		Count = packet->readInt( 32 );
		packet->readString(operation, sizeof(operation));

		for( int i = 0; i < Count; i++ )
		{
			AreaId[i]        = packet->readInt( 8 );
			PlayerCount[i]    = packet->readInt( 32 );
			maxPlayerCount[i] = packet->readInt( 32 );
			maxCreatedPlayers[i] = packet->readInt( 32 );
		}
	}

	template< typename _Ty >
	void WriteData( _Ty* packet )
	{
		packet->writeInt( Count, 32 );
		packet->writeString(operation, sizeof(operation));

		for( int i = 0; i < Count; i++ )
		{
			packet->writeInt( AreaId[i], 8 );
			packet->writeInt( PlayerCount[i], 32 );
			packet->writeInt( maxPlayerCount[i], 32 );
			packet->writeInt( maxCreatedPlayers[i], 32 );
		}
	}
};

struct stSwitchInfo
{
	int IP;
	int Port;
	int Id;
};

enum
{
  QUERYACCOUNTFORPLAYER_3RDFLAG  = 1<<0,		//查询第三方平台会员帐号注册标志
  QUERYACCOUNTFORPLAYER_PROMTION = 1<<1,		//查询角色的推广员帐号ID
};
