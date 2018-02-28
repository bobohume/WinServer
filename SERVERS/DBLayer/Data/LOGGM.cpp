#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "LOGGM.h"
#include "DBUtility.h"
#include "Common/PlayerStructEx.h"
#include "Common/dbStruct.h"

// 判断是否存在半个汉字
bool IsExistHalfHZ(char* lpString)
{
    // 获取ANSI字符串的长度 
    int nLenOfMultiCharStr = strlen(lpString);
    int nLenOfWideCharStr = nLenOfMultiCharStr * sizeof(WCHAR);

    PWSTR pWideCharStr = (PWSTR)malloc(nLenOfWideCharStr);
    LPSTR pMultiCharStr = (LPSTR)malloc(nLenOfMultiCharStr);

    MultiByteToWideChar(CP_ACP, 0, lpString, -1, pWideCharStr, nLenOfWideCharStr);
    WideCharToMultiByte(CP_ACP, 0, pWideCharStr,-1,pMultiCharStr,nLenOfMultiCharStr,NULL,FALSE);

    nLenOfWideCharStr = strlen(pMultiCharStr);
    free(pMultiCharStr);
    free(pWideCharStr);
    return (nLenOfWideCharStr != nLenOfMultiCharStr);
}

// ----------------------------------------------------------------------------
// 安全字符串(此方法通常用于过滤存储过程的参数项)
// 注意：此方法的输出参数，切记在调用完后delete,否则必定造成泄漏
bool SafeDBStringExt(const char* pIn, unsigned long InLength, char* pOut, unsigned long OutLength)
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
		case '\'':		//有单引号替换成＃
			pOut[j++] = '#';
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
// 保存GM操作数据日志
DBError LOGGM::Save(void* Info)
{
	const int MAXLEN = 250;
	stGMLog* pInfo = (stGMLog*)Info;
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if(!pInfo)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		char szLogTime[32] = {0};
		GetDBTimeString(pInfo->logtime, szLogTime);
		unsigned long OutSize = _countof(pInfo->content)*2;
		TempAlloc<char> pOut(OutSize);
		SafeDBStringExt(pInfo->content, _countof(pInfo->content), pOut, OutSize);

		char content[9][MAXLEN+1];
		memset(content, 0, sizeof(content));
		int pos = 0;
		int size = strlen(pOut);
		for(int i = 0; i < 9; i++)
		{
			strncpy_s(content[i], MAXLEN+1, pOut + pos, _TRUNCATE);
			pos += MAXLEN;
			if(size >= pos)
			{
				if(IsExistHalfHZ(content[i]))
				{
					content[i][MAXLEN] = 0;
					pos --;
				}
			}
			else
			{
				break;
			}
		}
		
		M_SQL(GetConn(),"EXECUTE Sp_CreateGMLog  %I64d,%d,%d,%d,%d,%d,%.2f,%.2f,%.2f,'%s',"\
				"%d,'%s','%s','%s','%s','%s','%s','%s','%s','%s'",
			pInfo->eid,
			pInfo->sourceid,
			pInfo->targettype,
			pInfo->targetid,
			pInfo->optype,
			pInfo->zoneid,
			pInfo->posx,
			pInfo->posy,
			pInfo->posz,
			szLogTime,
			pInfo->flag,
			content[0],
			content[1],
			content[2],
			content[3],
			content[4],
			content[5],
			content[6],
			content[7],
			content[8]);
		GetConn()->Exec();
		err = DBERR_NONE;		
	}
	DBECATCH()
	return err;
}