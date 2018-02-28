#ifndef _ACCOUNTSMDATA_H_
#define _ACCOUNTSMDATA_H_

#include "CommLib/SimpleFlag.h"
#include "DBLayer\Data\TBLAccount.h"
#include <time.h>
#include "Common.h"

//没有和worldDB内存共享了
class AccountSMData
{
public:
    AccountSMData(void)
    {
        Clear();
    }

    void Clear(void)
    {
        status  = 0;
        m_state = 0;
        LastTime		= (int)time(NULL);
        isAdult			= true;
        UID				= 0;
        AccountId		= 0;
		loginType		= LOGIN_Normal;
        
        CurPlayerPos	= -1;
        PlayerNum		= 0;
        AccountName[0]	= 0;
        loginIP[0]      = 0;
        isGM			= 0;
        
        loginTime      = 0;
        logoutTime     = 0;
        m_playerId     = 0;

        memset(m_playerIdList,0,sizeof(m_playerIdList));;
		m_isFakePlayer = false;

        ClrBit();
    }
    
    void SetLastTime(int time)       {LastTime = time;}
    int  GetLastTime(void) const     {return LastTime;}
    void SetUID(T_UID id)            {UID = id;}
    U32  GetUID(void) const          {return UID;}
    void SetAccountId(U32 id)        {AccountId = id;}
    U32	 GetAccountId(void) const    {return AccountId;}
    void SetCurPlayerPos(int pos)    {CurPlayerPos = pos;}
    int  GetCurPlayerPos(void) const {return CurPlayerPos;}
    void SetStatus(int sts)          {status = sts;}
    int  GetStatus(void) const       {return status;}
    void SetLoginIP(const char* ip)  {strcpy_s(loginIP,sizeof(loginIP),ip);}
    const char* GetLoginIP(void) const {return loginIP;}
    void SetAccountName(const char* name) {strcpy_s(AccountName,sizeof(AccountName),name);}
    const char* GetAccountName(void) const {return AccountName;}
    void SetPlayerNum(int num)        {PlayerNum = num;}
    int  GetPlayerNum(void) const     {return PlayerNum;}
    void SetAdult(bool adult)         {isAdult = adult;}
    bool GetAdult(void) const         {return isAdult;}
    void SetGMFlag(char gm)           {isGM = gm;}
    char GetGMFlag(void) const        {return isGM;}

    void SetState(int state)          {m_state = state;}
    int  GetState(void) const         {return m_state;}

	void SetFakeFlag(bool flag)       {m_isFakePlayer = flag;}
    bool IsFakePlayer(void) const     {return m_isFakePlayer;}

    //设置事件标记
	DeclareFlags(U32);

//protected:
    int		LastTime;
    T_UID	UID;
    int     AccountId;
    int     m_playerId;
    int     CurPlayerPos;

    PlayerInfo m_playerIdList[MAX_ACTOR_COUNT];

    int		status;//账号标志，老玩家账号等
    int		loginTime;
    int		logoutTime;
	LOGIN_TYPE loginType;
    int     m_state;//Status账号状态
    char    loginIP[32];
    char	AccountName[ACCOUNT_NAME_LENGTH];
    char	PlayerNum;
    bool	isAdult;
    char	isGM;

	bool    m_isFakePlayer;
};

#endif /*_ACCOUNTSMDATA_H_*/