#ifndef __TBLGMLOGINCHECK_H__
#define __TBLGMLOGINCHECK_H__

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif

#define MAC_LENGTH  64
#define IP_LENGTH   32

class TBLGmLoginCheck: public TBLBase
{
private:
	char macAddr[MAC_LENGTH];
	char ip[IP_LENGTH];

public:
	TBLGmLoginCheck(DataBase* db):TBLBase(db) {};
	virtual ~TBLGmLoginCheck() {};

	DBError	QueryIP(const char* LoginIP,char* mac);
	DBError QueryMAC(const char* LoginMac,char* ip);
	DBError QueryMACandIP(const char* LoginMac,const char* ip);

};

#endif