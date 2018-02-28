/////////////////////////////////////////////////////////////////
/// @file UnitTest.cpp
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

/////////////////////////////////////////////////////////////////
/// Method:    UnitTestMain
/// FullName:  UnitTestMain
/// Access:    public 
/// Returns:   void
/// Brief:	   单元测试入口
/// Qualifier:
/////////////////////////////////////////////////////////////////
#include "UnitTest.h"

void UnitTestMain()
{
#ifdef DEBUG
	__security_init_cookie();
	_heap_init(1);
	_mtinit();
	_ioinit();
	_cinit( 1 );
	_cexit();
#endif
}


/////////////////////////////////////////////////////////////////
/// Method:    UnitTest
/// FullName:  UnitTest::UnitTest
/// Access:    public 
/// Returns:   
/// Brief:	   单元测试构造函数
/// Qualifier:
/// Parameter: const char * TestName
/// Parameter: UnitTestFunction function
/////////////////////////////////////////////////////////////////
UnitTest::UnitTest( const char* TestName, UnitTestFunction function )
{
	printf( "\n\rUNIT TEST: %s\n\r", TestName );

	try
	{
		function();

		printf( "\n\rTEST SUCCESSED !\n\r" );
	}
	catch( UnitTestException e )
	{
		printf( "\n\rTEST FAIL: %s\n\r", e.getError() );
	}
	catch( ... )
	{
		printf( "\n\rTEST FAIL: UNKNOWN\n\r" );
	}

}

/////////////////////////////////////////////////////////////////
/// Method:    ~UnitTest
/// FullName:  UnitTest::~UnitTest
/// Access:    public 
/// Returns:   
/// Brief:	   单元测试析构函数
/// Qualifier:
/////////////////////////////////////////////////////////////////
UnitTest::~UnitTest()
{

}

/////////////////////////////////////////////////////////////////
/// Method:    UnitTestException
/// FullName:  UnitTestException::UnitTestException
/// Access:    public 
/// Returns:   
/// Brief:	   单元测试异常构造函数
/// Qualifier:
/// Parameter: const char * error
/////////////////////////////////////////////////////////////////
UnitTestException::UnitTestException( const char* error )
{
	mError = error;
}

/////////////////////////////////////////////////////////////////
/// Method:    getError
/// FullName:  UnitTestException::getError
/// Access:    public 
/// Returns:   const char*
/// Brief:	   获取单元测试异常错误
/// Qualifier: const
/////////////////////////////////////////////////////////////////
const char* UnitTestException::getError() const
{
	return mError.c_str();
}

