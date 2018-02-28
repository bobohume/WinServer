#include "DBUtility.h"
#include <memory.h>
#include <string>
#include <stdarg.h>
#include "Define.h"

void ConvertHex(char* pOut, int outLen, const unsigned char* pIn, int inLen)
{
	if(inLen * 2 >= outLen)
		return;
	memset(pOut,0, outLen);
	char szStr[4];
	for(int i = 0; i < inLen; i++)
	{
		memset(szStr, 0, 4);
		if(pIn[i] < 0x10)
			sprintf_s(szStr, 4, "0%X", pIn[i]);
		else
			sprintf_s(szStr, 4, "%X", pIn[i]);
		strcat_s(pOut, outLen, szStr);
	}
}

// ----------------------------------------------------------------------------
// 安全字符串(此方法通常用于过滤存储过程的参数项)
// 注意：此方法的输出参数，切记在调用完后delete,否则必定造成泄漏
bool SafeDBString(const char* pIn, unsigned long InLength, char* pOut, unsigned long OutLength)
{
	if (InLength == 0)
		return false;

	unsigned long j = 0;
	bool bFind = false;//是否找到--
	for (unsigned long i=0; i<InLength && j<OutLength; i++)
	{
		switch (pIn[i])
		{
		case '\0': 
			pOut[j++] = pIn[i];
			goto GoExit;
		case '\'':		//双写单引号
			pOut[j++] = pIn[i];
			pOut[j++] = pIn[i];
			break;
		case ';':		//过滤分号;
			break;
		case '-':		//过滤注释--
			{
				if(i+1 < InLength && pIn[i+1] == '-')
					i++;
				else
					pOut[j++] = pIn[i];
				bFind = true;
			}
			break;
		default:
			pOut[j++] = pIn[i];
			break;
		}
	}
	if (j < OutLength)
		pOut[j] = '\0';
	else
		pOut[OutLength-1] = '\0';

GoExit:
	if(bFind)
	{
		std::string s(pOut);
		std::string::size_type idx = s.find("--");
		while(idx != std::string::npos) 
		{
			s.replace(idx,2,"-");
			idx = s.find("--");
		}
		strcpy_s(pOut, OutLength, s.c_str());
	}

	return true;
}

// ----------------------------------------------------------------------------
// 字符串过滤，防止SQL注入攻击(此方法通常用于过滤SQL的条件项)
bool StrSafeCheck(const char* pIn, unsigned int maxlen)
{
	if(NULL == pIn)
		return false;		//空指针

	unsigned int actullen = strlen(pIn);
	if(actullen >= maxlen)	//字符串长度超界
		return false;

	for(unsigned int i = 0 ;i < actullen; i++)
	{
		switch(pIn[i])
		{
		case '\0':
			{
				return true;
			}
		case '\'':
		case '\"':
		case ')':
		case '(':
		case '=':
		case '%':
			{
				return false;
			}
		}
	}
	return true;
}

// ----------------------------------------------------------------------------
// 字符串过滤，防止SQL注入攻击(此方法通常用于过滤SQL的条件项)
bool StrSafeCheckEx(const char* pIn, unsigned int maxlen)
{
	if (NULL == pIn)
		return false;		//空指针

	unsigned int actullen = strlen(pIn);
	if (actullen >= maxlen)	//字符串长度超界
		return false;

	for (unsigned int i = 0; i < actullen; i++)
	{
		switch (pIn[i])
		{
		case '\0':
		{
			return true;
		}
		case '\'':
		case '\"':
		case ')':
		case '(':
		case ' ':
		case '=':
		case '%':
		{
			return false;
		}
		}
	}
	return true;
}

// ----------------------------------------------------------------------------
// 数据表导出文本数据
// ----------------------------------------------------------------------------

CWriteText::CWriteText(HANDLE file)
{
	mBuffer = NULL;
	mPos = 0;
	mFile = file;
	mTotalSize = 0;
}

CWriteText::~CWriteText()
{
}

void CWriteText::Save(char* data, ...)
{
	if(mFile == INVALID_HANDLE_VALUE)
	{
		printf("Txtfile handle invaild.\n");
		return;
	}
	char buf[TXTCHUNK];
	va_list argptr;
	va_start(argptr, data);
	vsprintf_s(buf, TXTCHUNK, data,argptr);
	va_end(argptr);

	int iLen = (int)strlen(buf) ;
	if( iLen <= 0 )
	{
		printf("Txtfile content invaild.\n");
		return;
	}

	if(mBuffer == NULL)
		mBuffer = (char*)VirtualAlloc(NULL, TXTBUFFER, MEM_COMMIT, PAGE_READWRITE);
	if(mPos + iLen >= TXTBUFFER)
	{
		memcpy(mBuffer + mPos, buf, TXTBUFFER - mPos);
		OVERLAPPED_WRITE* ov = new OVERLAPPED_WRITE;
		ov->AssignBuffer(mBuffer, TXTBUFFER);
		DWORD offset = (mTotalSize & 0xFFFFFFFF);
		DWORD offsethigh = ((mTotalSize >> 32) & 0xFFFFFFFF);
		ov->Write(mFile, offset, offsethigh);

		mBuffer = (char*)VirtualAlloc(NULL, TXTBUFFER, MEM_COMMIT, PAGE_READWRITE);
		memcpy(mBuffer, buf + TXTBUFFER-mPos, mPos + iLen - TXTBUFFER);
		mPos = mPos + iLen - TXTBUFFER;
		mTotalSize += TXTBUFFER;
	}
	else
	{
		memcpy(mBuffer + mPos, buf, iLen);
		mPos += iLen;
	}
}

void CWriteText::Flush()
{
	if(mFile == INVALID_HANDLE_VALUE)
	{
		printf("Txtfile handle invaild.\n");
		return;
	}
	if(mPos == 0 && mTotalSize == 0)
	{
		//printf("Pos = %d, TotalSize = %d\n", mPos, mTotalSize);
		return;
	}
	
	static DWORD SectorSize = 0;
	if(SectorSize == 0)
	{
		DWORD a,b,c;
		char szPath[MAX_PATH];
		::GetModuleFileName( NULL, szPath, MAX_PATH );
		char* cp = strchr(szPath, '\\');
		if (*cp)
			*(cp+1) = '\0';
		::GetDiskFreeSpace(szPath, &a, &SectorSize,&b, &c);
	}

	OVERLAPPED_WRITE* ov = new OVERLAPPED_WRITE;
	DWORD dwSize = RoundUp(mPos, SectorSize);
	ov->AssignBuffer(mBuffer, dwSize);
	DWORD offset = (mTotalSize & 0xFFFFFFFF);
	DWORD offsethigh = ((mTotalSize >> 32) & 0xFFFFFFFF);
	ov->Write(mFile, offset, offsethigh);

	mTotalSize += mPos;
	mPos = 0;
	mBuffer = NULL;
}


//=============================================================================
// 其它工具函数
// ============================================================================
#include "crc32.h"

unsigned int FCRC32(unsigned char* buf, int size)
{
	unsigned int crc;
	crc32_context crc32_ctx;
	CRC32_Init(&crc32_ctx);
	CRC32_Update(&crc32_ctx, buf, size);
	CRC32_Finish(&crc32_ctx, (unsigned long*)&crc);
	return crc;
}

//-----------------------------------------------------------------------------
// 递归创建目录
void EnsureDirectory(const char* path)
{
	static char dir[_MAX_PATH] = {0};
	static char div[_MAX_PATH] = {0};
	_splitpath_s(path,dir,_MAX_PATH, div, _MAX_PATH, NULL,0, NULL, 0);
	strcat_s(dir, _MAX_PATH, div);

	if (0xFFFFFFFF != GetFileAttributes(path))
		return;

	for (size_t i = 0; i < strlen(dir); i++)
	{
		if (dir[i] == '\\' || dir[i] == '/')
		{
			char tmp = dir[i];
			dir[i] = 0;
			if (0xFFFFFFFF == GetFileAttributes(dir))
			{
				::CreateDirectory(dir,0);
			}
			dir[i] = tmp;
		}
	}
}

// ----------------------------------------------------------------------------
// 删除非空的目录下所有文件
void DumpDirectory(const char* path)
{
	WIN32_FIND_DATA findData;
	char FULLPATH[MAX_PATH];
	sprintf_s(FULLPATH, MAX_PATH, "%s\\*", path);
	HANDLE handle = FindFirstFile(FULLPATH, &findData);
	if (handle == INVALID_HANDLE_VALUE)
		return;

	do
	{
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;

		// 确定是我们所需要的各种文件属性的文件
		if (findData.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY| FILE_ATTRIBUTE_OFFLINE|
			FILE_ATTRIBUTE_SYSTEM| FILE_ATTRIBUTE_TEMPORARY) )
			continue;

		sprintf_s(FULLPATH, MAX_PATH, "%s\\%s", path, findData.cFileName);
		::DeleteFile(FULLPATH);
	}while(FindNextFile(handle, &findData));
	FindClose(handle);
}