#ifndef __DBUTILITY_H__
#define __DBUTILITY_H__

#include <time.h>
#include <stdio.h>
#pragma once
#include <winsock2.h>

const DWORD TXTCHUNK	=	8192;
const DWORD TXTBUFFER	=	1024 * 64;

template <typename V, typename M>
inline V RoundDown(V Value, M Multiple)
{
	return((Value / Multiple) * Multiple);
}

template <typename V, typename M>
inline V RoundUp(V Value, M Multiple)
{
	return(RoundDown(Value, Multiple) + (((Value % Multiple) > 0) ? Multiple : 0));
}

class OVERLAPPED_WRITE : public OVERLAPPED
{
public:
	OVERLAPPED_WRITE() 
	{
		Internal = InternalHigh = 0;   
		Offset = OffsetHigh = 0;   
		hEvent = NULL;
		m_nBuffSize = 0;
		m_pvData = NULL;
	}

	~OVERLAPPED_WRITE()
	{
		if (m_pvData != NULL)
			VirtualFree(m_pvData, 0, MEM_RELEASE);
	}

	BOOL AssignBuffer(char* buffer, DWORD size)
	{
		m_nBuffSize = size;
		m_pvData	= buffer;
		return (m_pvData != NULL) && (size > 0);
	}

	BOOL Write(HANDLE hDevice, DWORD lowpos, DWORD highpos) 
	{
		Offset     = lowpos;
		OffsetHigh = highpos;
		BOOL ret = ::WriteFile(hDevice, m_pvData, m_nBuffSize, NULL, this);
		if(ret == FALSE)
		{
			DWORD code = GetLastError();
			if(code != ERROR_IO_PENDING)
				printf("Asynchronous IO write failure.(code=%d)\n", code);
		}
		return ret;
	}

	inline BOOL IsOK(DWORD WirteBytes) { return (m_nBuffSize == WirteBytes);}
	inline DWORD GetDataSize() { return m_nBuffSize;}
private:
	SIZE_T m_nBuffSize;
	PVOID  m_pvData;
};


template<int size>
extern void GetDBTimeString(unsigned long t, char (&str) [size])
{
	struct tm lt;
	time_t long_time = t;
	localtime_s(&lt, &long_time);
	sprintf_s(str, sizeof(str), "%d-%d-%d %d:%d:%d",
		lt.tm_year + 1900,
		lt.tm_mon + 1,
		lt.tm_mday,
		lt.tm_hour,
		lt.tm_min,
		lt.tm_sec);
}

extern void ConvertHex(char* pOut, int outLen, const unsigned char* pIn, int inLen);
extern bool SafeDBString(const char* pIn, unsigned long InLength, char* pOut, unsigned long OutLength);
extern bool StrSafeCheck(const char* pIn, unsigned int maxlen);
extern bool StrSafeCheckEx(const char* pIn, unsigned int maxlen);

//历史遗留问题，检查数据结构无符号整数字段与数据表字段是否匹配
#define INT_CHECK(a) if(a>0x7FFFFFFF) throw ExceptionResult(DBERR_DB_SQLPARAMETER)
#define U_INT_CHECK(a) if(a<0 || a>0x7FFFFFFF) throw ExceptionResult(DBERR_DB_SQLPARAMETER)
#define TINYINT_CHECK(a) if(a>0x7F) throw ExceptionResult(DBERR_DB_SQLPARAMETER)
#define U_TINYINT_CHECK(a) if(a<0 || a>0x7F) throw ExceptionResult(DBERR_DB_SQLPARAMETER)
#define SMALLINT_CHECK(a) if(a>0x7FFF) throw ExceptionResult(DBERR_DB_SQLPARAMETER)
#define U_SMALLINT_CHECK(a) if(a<0 || a>0x7FFF) throw ExceptionResult(DBERR_DB_SQLPARAMETER)

#define SAFEDBSTRING(a,b) \
	unsigned long u##b = _countof(a)*2;\
	TempAlloc<char> p##b(u##b);\
	SafeDBString(a, _countof(a), p##b, u##b)

class CWriteText
{
public:
	CWriteText(HANDLE file);
	~CWriteText();
	void Save(char* data, ...);
	void Flush();
	__int64 getTotalSize() { return mTotalSize;}
private:
	char*	mBuffer;
	int		mPos;
	HANDLE	mFile;
	__int64 mTotalSize;
};


extern unsigned int FCRC32(unsigned char* buf, int size);
extern void EnsureDirectory(const char* path);
extern void DumpDirectory(const char* path);

#endif//__DBUTILITY_H__
