#ifndef _DB_LOGIN_H
#define _DB_LOGIN_H

#include "CommLib/ThreadPool.h"

class DB_Login :public  ThreadBase
{
public:
	enum
	{
		ERR_NONE				 = 1,
		ERR_AccountEXISTS		 = 2,
		ERR_AccountInsertDBERROR = 3,
	};

	DB_Login();
	virtual ~DB_Login() {}
	virtual int Execute(int ctxId, void* param);
	int socketId;
	int AccountSocketId;
	int AccountId;

	char AccountName[ACCOUNT_NAME_LENGTH];
	char Password[PASSWORD_LENGTH];
};


#endif