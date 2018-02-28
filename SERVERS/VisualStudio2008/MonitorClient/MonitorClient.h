#ifndef _MONITORCLIENT_H_
#define _MONITORCLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MONITORCLIENT_EXPORTS
#define MONITORDECL __declspec( dllexport )
#else
#define MONITORDECL
#endif    

#define MONITOR_DLL_VERSION "1.0.0"

//runStatus值定义
//0 : 正常运行 
//1 : 请求关闭(包括正常关闭和定时关闭)
//2 : 收到定时关闭请求

extern MONITORDECL bool Monitor_Init(unsigned int& runStatus);
extern MONITORDECL bool Monitor_Close(void);

//在定时关闭模式下，获取离关闭的剩余时间
extern MONITORDECL int  Monitor_GetCloseSeconds(void);

#ifdef __cplusplus
}
#endif

#endif /*_MONITORCLIENT_H_*/