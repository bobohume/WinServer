#ifndef __TBLACCOUNT_H__
#define __TBLACCOUNT_H__

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif

#include <vector>
#include "Common/PacketType.h"
#include "Common/Common.h"

class TBLAccount: public TBLBase
{
public:
	enum ALoginType
	{
		Login_None		= 0,
		Login_Normal,
		Login_Fast,
		Login_Max,
	};

	struct stAccount
	{
		int				AccountID;
		char			AccountName[ACCOUNT_NAME_LENGTH];
		bool			IsAdult;
		unsigned char	Mode;
		unsigned int	Status;
		unsigned char	GMFlag;
		unsigned char	LoginStatus;
		int				LoginTime;
		int				LogoutTime;
		char			LoginIP[20];
		int				LoginError;
		int				TotalOnlineTime;
		int				AccountFunction;
		int				dTotalOnlineTime;
		int				dTotalOfflineTime;
	};

	TBLAccount(DataBase* db):TBLBase(db) {};
	virtual ~TBLAccount() {};
	DBError MakeAccount( const char* AccountName, int& accountId, int flag, bool bApple = false);
	DBError	QueryLoginIP(const char* AccountName, int& AccountID, char* LoginIP);
	ALoginType QueryAccountById(int AccountId);
	DBError	QueryID(const char* AccountName, int& AccountID);
	DBError SaveLoginInfo(int AccountID, const char* LoginIP, int LoginError);
	DBError	Load(const char* AccountName, stAccount* pInfo);
	DBError	Load(int AccountID, stAccount* pInfo);
	DBError QueryGMAccount(int GMFlag, std::vector<std::pair<int, std::string> >& Info);
	DBError QueryForGM(int Mode, const char* Key, const char* Condition, std::vector<stAccount>& AccountInfo);
	DBError UpdateGMFlag(const char* AccountName, int GMFlag);
	DBError UpdateLoginTime(int AccountID);
    DBError UpdateAccountLogoutTime(int AccountID,const char* time);        
	DBError UpdateLogoutTime(int AccountID, int CostTime, int dTotalOfflineTime, int dTotalOnlineTime );
	DBError CheckAccount( const char* AccountName, bool& isExisted );
	DBError	SetAccountStatus( int accountId, unsigned int status );
	DBError	SetAccountStatus( int accountId );
	DBError CheckFirstLoginStatus( int accountId, int playerId, const char* playName, int bonusCount );
	DBError CheckOldPlayerLogin( int accountId, int playerId, const char* playName );
    DBError LoadPlayerList(int accountId, Players& playerList);
	DBError	UpdateNetBarDrawTime( int accountId,int index );
	DBError QueryNetBarDrawTime(int AccountId, int* timeList);
private:
	void GetAccountInfo(stAccount* Info);
	void GetAccountInfo_GM(stAccount* Info);

};

#endif//__TBLACCOUNT_H__
