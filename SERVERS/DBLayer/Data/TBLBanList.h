#ifndef __TBLBANLIST_H__
#define __TBLBANLIST_H__

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif

#include <vector>

class CDBConn;

class TBLBanList: public TBLBase
{
public:
	struct stBan
	{
		char			target[50];
		unsigned char	type;
		int				loginBan;
		int				chatBan;
		char			loginBanReason[50];
		char			chatBanReason[50];
	};

	TBLBanList(DataBase* db):TBLBase(db) {}
	virtual ~TBLBanList() {}
	DBError	DeleteForOverTime(int curTime);
	DBError Delete(const char* target, int type);
	DBError Load(std::vector<stBan>& Info);
	DBError Update(stBan& Info);
};

#endif//__TBLBANLIST_H__