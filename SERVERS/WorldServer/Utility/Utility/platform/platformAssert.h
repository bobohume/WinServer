
#ifndef _PLATFORMASSERT_H_
#define _PLATFORMASSERT_H_

#include "base/types.h"

class PlatformAssert
{	
public:
	enum Type 
	{
		PA_WARNING	= 3,	//警告等级输出
		PA_ERROR	= 2,	//错误等级输出
		PA_FOCUS	= 1		//致命错误等级输出
	};

	PlatformAssert();
	~PlatformAssert();	
	void Output(Type logType, const char* filename, U32 line, const char* msg);
	void Assert(Type logType, const char* filename, U32 line, const char* msg, bool isChoice=true);
	void Crash(Type logType, const char* msg);
};

extern PlatformAssert* g_pAssert;

#define AssertWarn(x,y) \
	{  if (((bool)(x))==(bool)0) \
		{g_pAssert->Output(PlatformAssert::PA_WARNING, __FILE__, __LINE__,  y);}}

#define AssertFatal(x, y) \
	{  if (((bool)(x))==(bool)0) \
		{g_pAssert->Output(PlatformAssert::PA_ERROR, __FILE__, __LINE__,  y);\
		 g_pAssert->Crash(PlatformAssert::PA_WARNING,y);}}

#define AssertRelease(x,y) \
	{  if (((bool)(x))==(bool)0) \
		{g_pAssert->Output(PlatformAssert::PA_ERROR, __FILE__, __LINE__,  y);\
		 g_pAssert->Crash(PlatformAssert::PA_ERROR,y);}}

#define AssertError(x, y, action) \
	{  if (((bool)(x))==(bool)0) \
		{g_pAssert->Output(PlatformAssert::PA_ERROR, __FILE__, __LINE__,  y);action;}}

//注：目前Assert功能说明
//----------------------------------------------------------------------------------------------
//                       DEBUG                          RELEASE
// AssertWarn		  LOG						      LOG
// AssertFatal        LOG+中断执行					  LOG
// AssertRelease      LOG+中断执行　　　　　　　　　　LOG+中断执行
// AssertError        LOG+action					  LOG+action


//注：原Assert功能说明
//----------------------------------------------------------------------------------------------
//                       DEBUG                          RELEASE
// AssertWarn		  LOG						      LOG
// AssertFatal        LOG+中断执行					  LOG
// AssertRelease      LOG+中断执行　　　　　　　　　　LOG+中断执行
// AssertISV          LOG+中断执行                    LOG+中断执行
// Safe_AssertWarn    LOG+action					  LOG+action
// Safe_AssertFatal   LOG+中断执行+action			  LOG+action
// Safe_AssertRelease LOG+中断执行+action			  LOG+中断执行+action

extern const char* avar(const char *in_msg, ...);

#endif // _PLATFORM_ASSERT_H_

