#ifndef _RALIBDEF_H_
#define _RALIBDEF_H_

enum RANetMessage
{
    RA_MSG_KICK_ACCOUNT = 100,
    RA_MSG_KICK_ACCOUNT_RESP,
};

enum KickRespond
{
    E_KICKRESP_KICKED = 3,  //已踢线
    E_KICKRESP_NOTFOUND,   //帐号不在游戏中
};

#endif /*_RALIBDEF_H_*/