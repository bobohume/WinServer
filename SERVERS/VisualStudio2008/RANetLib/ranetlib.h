#ifndef _RANETLIB_H_
#define _RANETLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef RANETLIB_EXPORTS
#define RANETLIBDECL __declspec( dllexport )
#else
#define RANETLIBDECL
#endif    

#define RALIB_VERSION "1.0.0"

//网络连接建立和断开
//ip port为remote服务器上开放给当前服务的地址
extern RANETLIBDECL bool RA_Connect(const char* ip,int port);
extern RANETLIBDECL void RA_Disconnect(void);

//对帐号进行踢线操作,并返回当前操作的句柄
//@svrId 帐号所在的区,如果为0,则对所有区进行操作,但结果集只能反馈1个

extern RANETLIBDECL int RA_KickAccount(int svrId,const char* name);

//获取帐号的踢线结果
enum RAKickRespond
{
    E_KICK_NORESP = 0, //操作不存在，无法查询
    E_KICK_WAITTING,   //正在等待处理结果返回

    E_KICK_TIMEOUT,    //踢线操作超时
    E_KICK_KICKED,     //已踢线
    E_KICK_NOTFOUND,   //帐号不在游戏中
    E_KICK_UNKNOWN,
};

extern RANETLIBDECL int  RA_GetKickRespond(int seq,bool isRemoved = true);

//清除所有踢线结果
extern RANETLIBDECL void RA_ClearKickResponds(void);




#ifdef __cplusplus
}
#endif

#endif /*_RANETLIB_H_*/