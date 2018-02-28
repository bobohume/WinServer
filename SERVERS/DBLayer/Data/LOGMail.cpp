#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "LOGMail.h"
#include "DBUtility.h"
#include "Common/PlayerStructEx.h"
#include "common/MailBase.h"
#include "Common/dbStruct.h"

//// ----------------------------------------------------------------------------
//// 保存邮件的数据日志信息
DBError LOGMail::Save(void* Info)
{
	stMailLog* pInfo = (stMailLog*)Info;
	DBError err = DBERR_UNKNOWERR;
	try
	{
		if(!pInfo)
			throw ExceptionResult(DBERR_DB_SQLPARAMETER);

		char szLogTime[32] = {0};
		GetDBTimeString(pInfo->logtime, szLogTime);
		char szSendTime[32] = {0};
		GetDBTimeString(pInfo->info.nTime, szSendTime);
		char szMsg[MAX_MAIL_ITEM_LENGTH * 2+1], szItem[MAX_MAIL_ITEM_ILENGTH * 2 + 1];
		ConvertHex(szMsg, MAX_MAIL_ITEM_LENGTH * 2+1, (unsigned char*)pInfo->info.content, MAX_MAIL_ITEM_LENGTH);
		ConvertHex(szItem, MAX_MAIL_ITEM_ILENGTH * 2+1, (unsigned char*)pInfo->info.item, MAX_MAIL_ITEM_ILENGTH);
		unsigned long titleSize = COMMON_STRING_LENGTH*2;
		TempAlloc<char> pszTitle(titleSize);
		SafeDBString(pInfo->info.title, COMMON_STRING_LENGTH, pszTitle, titleSize);

		unsigned long OutSize = 64;
		TempAlloc<char> pOut(OutSize);
		SafeDBString(pInfo->info.szSenderName, 32, pOut, OutSize);

		M_SQL(GetConn(),"EXECUTE Sp_CreateMailLog  %I64d,%d,%d,%d,%d,%d,%.2f,%.2f,%.2f,'%s',"\
			"%d,'%s',%d,%d,%d,'%s',%d,%d,%d,'%s',0x%s,0x%s",
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
			pInfo->info.nSender,
			(char*)pOut,
			pInfo->info.nMoney,
			pInfo->info.nItemId,
			pInfo->info.nItemCount,
			szSendTime,
			pInfo->info.nRecver,
			pInfo->info.isReaded,
			pInfo->info.isSystem,
			(char*)pszTitle,
			szMsg,
			szItem);
		GetConn()->Exec();
		err = DBERR_NONE;		
	}
	DBECATCH()
	return err;
}