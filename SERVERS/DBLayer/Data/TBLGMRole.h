#ifndef __TBLGMROLE_H__
#define __TBLGMROLE_H__

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif

#include <vector>

class CDBConn;

class TBLGMRole: public TBLBase
{
public:
	struct stGMRole
	{
		unsigned char GMFlag;
		char		  GMCmd[60];
	};
	TBLGMRole(DataBase* db):TBLBase(db) {}
	virtual ~TBLGMRole() {}
	DBError	Load(std::vector<stGMRole>& gmroleInfo);
	DBError Save(int gmflag, const char* gmcmd);
	DBError Delete(int gmflag, const char* gmcmd);
};

#endif//__TBLGMROLE_H__