#ifndef TCC_WAPPER_H
#define TCC_WAPPER_H

#include "libtcc.h"
#include <stdarg.h>

typedef void* (*TCCCall)( ... );

#include <string>

class CTCCWarper
{
public:
	CTCCWarper();
	virtual ~CTCCWarper();

	void registerSymbol( std::string strName, void* pSymbol );
	void getSymbol( std::string strName, void* pSymbol );
	int _cdecl run( char* szName, int argc = 0, ... );
	void loadScript( const char* szScript );
	void loadFile( const char* fileName );
	bool check();

	template<typename T>
	T get( std::string strName )
	{
		void* pValue = NULL;
		getSymbol( strName, &pValue );

		if( !pValue )
			return NULL;

		return *((T*)pValue);
	}

	void compileFile( const char* fileName );

private:
	TCCCall _getFunction( char* szName );
	static void _error( void* value, const char* msg );
private:
	TCCState* mTccState;
	std::string mLastErrorMessage;
	bool mIsReady;
};

#define C_API _cdecl

#endif