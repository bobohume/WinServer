#define DB_TYPES_IMPL
#include "dbStruct.h"
#include <Windows.h>
#include <time.h>
#include "CommLib/format.h"
#include "DBLayer/Common/DBUtility.h"

dbStruct::dbStruct():
		sourceid(0)
		,targetid(0)
		,optype("")
		,logtime(0)
{
}

// ----------------------------------------------------------------------------
// 根据结构类型名，创建数据结构实例
dbStruct* dbStruct::creator(int structType)
{
	dbStruct* pData;
	switch (structType)
	{
	case STPLAYERLOG:		pData = new stPlayerLog;		break;
	default:				pData = NULL;				break;
	}
	return pData;
}

// ----------------------------------------------------------------------------
// 记录日志“何时何地谁跟谁发生何事”
void dbStruct::setlog(U32 _playerid, U32 _targetid, std::string _optype)
{
	time_t t;
	time(&t);
	sourceid	= CC_INT(_playerid, 0, 0x7FFFFFFF);;
	targetid	= CC_INT(_targetid, 0, 0x7FFFFFFF);
	optype		= _optype;
	logtime		= (U32)t;
}

// ----------------------------------------------------------------------------
// 网络数据PACK
void dbStruct::pack(Base::BitStream* stream)
{
	stream->writeInt(sourceid,			Base::Bit32);
	stream->writeInt(targetid,			Base::Bit32);
	stream->writeString(optype, MAX_STRING_LEN);
	stream->writeInt(logtime,			Base::Bit32);
}

// ----------------------------------------------------------------------------
// 网络数据UnPack
void dbStruct::unpack(Base::BitStream* stream)
{
	sourceid	= stream->readInt(Base::Bit32);
	targetid	= stream->readInt(Base::Bit32);
	optype		= stream->readString(MAX_STRING_LEN);
	logtime		= stream->readInt(Base::Bit32);
}

void stPlayerLog::pack(Base::BitStream* stream)
{
	dbStruct::pack(stream);
	for (int i = 0; i < MAX_VAL_SIZE; ++i)
		stream->writeString(val[i], MAX_STRING_LEN);
}

void stPlayerLog::unpack(Base::BitStream* stream)
{
	dbStruct::unpack(stream);
	for (int i = 0; i < MAX_VAL_SIZE; ++i)
		val[i] = stream->readString(MAX_STRING_LEN);
}

DBError LOGPlayer::Save(void* Info)
{
	stPlayerLog* pInfo = (stPlayerLog*)Info;
	if (pInfo)
	{
		char szLoginTime[32] = "";
		GetDBTimeString(_time32(NULL), szLoginTime);
		SetSql(fmt::sprintf("INSERT INTO Tbl_LogPlayer \
			  (SourceId,		TargetId,		OptType,		LogTime,		Val0,		Val1,		Val2,			Val3,			Val4) \
			VALUES (%d,			%d,				'%s',			'%s',			'%s',		'%s',		'%s',			'%s',			'%s')",
			pInfo->sourceid, pInfo->targetid, pInfo->optype, szLoginTime, pInfo->val[0], pInfo->val[1], pInfo->val[2], pInfo->val[3], pInfo->val[4]));
	}

	return Commint();
}