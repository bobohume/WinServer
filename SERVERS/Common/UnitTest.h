/////////////////////////////////////////////////////////////////
/// @file UnitTest.h
///
/// 用于单元测试.
///
/// 用于单元测试.
///
///	@作者 LivenHotch. 
///
/// @日期  06/03/2010
//  -------------------------------------------------------------
//  修改记录.
//  06/03/2010    文件创建
//
//  Copyright (C) 2010 - All Rights Reserved
/////////////////////////////////////////////////////////////////

#ifndef UNIT_TEST_H
#define UNIT_TEST_H

#include <string>
#include <stdio.h>

extern "C"
{
	int __cdecl _cinit ( int initFloatingPrecision );
	int __cdecl _heap_init( int mtflag );
	int  __cdecl _mtinit(void);
	void __cdecl __security_init_cookie(void);
	int  __cdecl _ioinit(void);
 	void __cdecl _cexit(void);
}

typedef void (*UnitTestFunction)();

class UnitTest
{
public:
	UnitTest( const char* TestName, UnitTestFunction function );
	virtual ~UnitTest();
};

class UnitTestException
{
public:
	UnitTestException( const char* error );

	const char* getError() const;
private:
	std::string mError;
};

#ifdef _UNIT_TEST

#define UNITTEST(p) \
	static void p##function(); \
	static UnitTest p##test( #p, p##function ); \
	static void p##function()

#else

#define UNITTEST(p) void UnitTest_##p()

#endif

#endif