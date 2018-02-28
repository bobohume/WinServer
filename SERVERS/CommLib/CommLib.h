#ifndef _COMMLIB_H_
#define _COMMLIB_H_

#include "base/types.h"
#include <time.h>
#include <assert.h>
#include <string>
#include "ErrorCodeDefine.h"

#ifndef SAFE_DELETE
#define SAFE_DELETE(x)	if( (x)!=NULL ) { delete (x); (x)=NULL; }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(x)	if( (x)!=NULL ) { delete[] (x); (x)=NULL; }
#endif

#ifndef SAFE_FREE
#define SAFE_FREE(x)	if( (x)!=NULL ) { free(x); (x)=NULL; }
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x)	if( (x)!=NULL ) { (x)->Release(); (x)=NULL; }
#endif

#if defined(DEBUG)
	#define _MY_TRY   try
	#define _MY_CATCH catch(...)
#else
	#define _MY_TRY   try
	#define _MY_CATCH catch(...)
#endif

inline S32 _mClamp(S32 val, S32 low, S32 high)
{
   return getMax(getMin(val, high), low);
}

//获取每天零时零分零秒的时间值
U32 getZeroTime(U32 t);


//秒数转换
#define ONE_MINUTE (60)     
#define ONE_HOUR   (60 * ONE_MINUTE)
#define ONE_DAY    (60 * ONE_HOUR)

//检测运行的时间tick
class TickHelper
{
public:
    TickHelper(void);
    ~TickHelper(void);

    U64 m_startTick;
};

class EventGuard
{
public:
    EventGuard(const char* name);
    ~EventGuard(void);
    
    static bool CheckExist(const char* name);
protected:
    void* m_pEventHandle;
};

extern void SetConsole(std::string title, std::string version, std::string Ip = "", std::string Port = "");
//获取当前时刻进程的内存使用量
extern U64  GetUsePhysMemory(void);
extern void DumpUsedMemory(void);

#endif /*_COMMLIB_H_*/