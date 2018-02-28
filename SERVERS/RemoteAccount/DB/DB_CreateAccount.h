#ifndef _DB_CREATEACCOUNT_H
#define _DB_CREATEACCOUNT_H

#include "CommLib/ThreadPool.h"

class DB_CreateAccount :public  ThreadBase
{
public:
	enum
	{
		ERR_NONE				 = 1,
		ERR_AccountEXISTS		 = 2,
		ERR_AccountInsertDBERROR = 3,
	};

	DB_CreateAccount();
	virtual ~DB_CreateAccount() {}
	virtual int Execute(int ctxId, void* param);
	int AccountId;
	int error;
	int socketId;
	int AccountSocketId;
	bool bApple;

	char AccountName[ACCOUNT_NAME_LENGTH];
	char Password[PASSWORD_LENGTH];
};


#endif