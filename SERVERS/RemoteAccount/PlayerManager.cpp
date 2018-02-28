#include "PlayerManager.h"
#include "Common\RegisterCommon.h"
#include "common\BillingBase.h"
#include "Common\BonusBase.h"
#include <map>
#include "QuickLz\quicklz.h"
#include "Common\PlayerStruct.h"
#include "../DBLayer/Data/TBLBase.h"
#include "CommLib/DBThreadManager.h"
#include "DB/DB_CreatePlayer.h"
#include "DB/DB_DeletePlayer.h"
#include "DB/DB_LoginPlayer.h"
#include "Common/OrgBase.h"
#include "DB/DB_CreateAccount.h"
#include "DB/DB_Login.h"
#include "DB/DB_CheckPlayerName.h"
#include "DB/DB_BindAccount.h"
#include "Http_Purchase.h"
#include "DB/DB_Reward.h"
#include "DB/DB_Code.h"
#include "DB/DB_CreateOrg.h"
#include "DB/DB_DeleteOrg.h"

WorldServerInfo g_worldServerInfo;
CMyCriticalSection g_worldLock;

CPlayerManager::CPlayerManager()
{	
}

CPlayerManager::~CPlayerManager()
{
	::WaitForSingleObject( NULL, -1 );
}

ServerEventFunction( CRemoteAccount, HandleWorldInfo, "WORLD_REMOTE_PlayerCountRequest" )
{
	CLocker lock( g_worldLock );

	int areaId         = phead->Id;

    g_Log.WriteLog("收到服[%d]的服务器信息",areaId);

    int createPlayerCount = pPack->readInt(Base::Bit32);
    int playerCont        = pPack->readInt(Base::Bit32);
    int maxPlayerCount    = pPack->readInt(Base::Bit32);

	int i = 0;
	for( i = 0; i < g_worldServerInfo.Count; i++ )
	{
		if( g_worldServerInfo.AreaId[i] == areaId )
		{
            g_worldServerInfo.maxCreatedPlayers[i] = createPlayerCount;
			g_worldServerInfo.PlayerCount[i]       = playerCont;
            g_worldServerInfo.maxPlayerCount[i]    = maxPlayerCount;
			return true;
		}
	}

	g_worldServerInfo.PlayerCount[i]       = playerCont;
	g_worldServerInfo.AreaId[i]            = areaId;
    g_worldServerInfo.maxPlayerCount[i]    = maxPlayerCount;
    g_worldServerInfo.maxCreatedPlayers[i] = createPlayerCount;

	g_worldServerInfo.Count++;
	return true;
}

// 目前这个消息用于ACCOUNT向REMOTE请求服务器玩家数据
ServerEventFunction( CRemoteAccount, HandleAccountWorldInfoRequest, "ACCOUNT_REMOTE_PlayerCountRequest" )
{
	CLocker lock( g_worldLock );

	MAKE_PACKET( sendPacket, "ACCOUNT_REMOTE_PlayerCountResponse", 0, SERVICE_ACCOUNTSERVER );
	g_worldServerInfo.WriteData( &sendPacket );
	SEND_PACKET_SOCK( sendPacket, socketId, CRemoteAccount::getInstance()->getServerSocket() );
	return true;
}


ServerEventFunction( CRemoteAccount, HandleRegister, "RA_RegisterRst")
{
	int AccountSocket = phead->Id;
	char AccountName[ACCOUNT_NAME_LENGTH] = "";
	char Password[PASSWORD_LENGTH] = "";
	bool bApple = pPack->readFlag();
	pPack->readString(AccountName, sizeof(AccountName));
	pPack->readString(Password, sizeof(Password));

	DB_CreateAccount* pDBHandle = new DB_CreateAccount();
	pDBHandle->socketId = socketId;
	pDBHandle->AccountSocketId = AccountSocket;
	pDBHandle->bApple = bApple;

	dMemcpy(pDBHandle->AccountName, AccountName, sizeof(AccountName));
	dMemcpy(pDBHandle->Password, Password, sizeof(Password));

	CRemoteAccount::getInstance()->GetDBManager()->SendPacket(pDBHandle);
	return true;
}

ServerEventFunction( CRemoteAccount, HandleCreatePlayer, "WORLD_REMOTE_CreatePlayer" )
{
	DB_CreatePlayer* pDBHandle = new DB_CreatePlayer;
	pDBHandle->areaId = phead->DestZoneId;
	pDBHandle->account = phead->Id;
	pDBHandle->socketId = socketId;
	pDBHandle->info.ReadData(pPack);
	CRemoteAccount::getInstance()->GetDBManager()->SendPacket(pDBHandle);

	return true;
}

ServerEventFunction( CRemoteAccount, HandleAutoPlayerName, "WORLD_REMOTE_AutoPlayerName" )
{
	char tempName[COMMON_STRING_LENGTH] = "";
	pPack->readString(tempName, COMMON_STRING_LENGTH);
	DB_CheckPlayerName* pDBHandle = new DB_CheckPlayerName;
	pDBHandle->areaId = phead->DestZoneId;
	pDBHandle->account = phead->Id;
	pDBHandle->socketId = socketId;
	dStrcpy(pDBHandle->m_CreateName, COMMON_STRING_LENGTH, tempName);
	CRemoteAccount::getInstance()->GetDBManager()->SendPacket(pDBHandle);

	return true;
}

ServerEventFunction( CRemoteAccount, HandleDeletePlayer, "WORLD_REMOTE_DeletePlayer" )
{
	DB_DeletePlayer* pDBHandle = new DB_DeletePlayer;
	pDBHandle->id = phead->Id;
	CRemoteAccount::getInstance()->GetDBManager()->SendPacket(pDBHandle);
	return true;
}

ServerEventFunction( CRemoteAccount, HandleLoginPlayer, "WORLD_REMOTE_PlayerLogin" )
{
	DB_LoginPlayer* pDBHandle = new DB_LoginPlayer;
	pDBHandle->account = phead->Id;
	pDBHandle->socketId = socketId;
	pDBHandle->areaId	= phead->DestZoneId;
	pDBHandle->loginAward = pPack->readFlag();
	if (pDBHandle->loginAward)
	{
		pPack->readString(pDBHandle->MachineUID, sizeof(pDBHandle->MachineUID));
		pPack->readString(pDBHandle->IP, sizeof(pDBHandle->IP));
	}
	pDBHandle->playerId = pPack->readInt(Base::Bit32);
	CRemoteAccount::getInstance()->GetDBManager()->SendPacket(pDBHandle);
	return true;
}

std::string getDate( const char *src )
{
	char dest[20];
	dest[0] = src[0];
	dest[1] = src[1];
	dest[2] = src[2];
	dest[3] = src[3];
	dest[4] = '-';
	dest[5] = src[4];
	dest[6] = src[5];
	dest[7] = '-';
	dest[8] = src[6];
	dest[9] = src[7];
	dest[10] = ' ';
	dest[11] = src[8];
	dest[12] = src[9];
	dest[13] = ':';
	dest[14] = src[10];
	dest[15] = src[11];
	dest[16] = ':';
	dest[17] = src[12];
	dest[18] = src[13];
	dest[19] = 0;

	return std::string( dest );
}

ServerEventFunction( CRemoteAccount, HandleServeRegister, "COMMON_RegisterRequest" )
{
    int type  = pPack->readInt(Base::Bit8);
    int line  = pPack->readInt(Base::Bit8);
    int id    = pPack->readInt(Base::Bit32);
    int times = pPack->readInt(Base::Bit32);
    
    if (SERVICE_WORLDSERVER == type)
    {
        CRemoteAccount::getInstance()->AddWorldServer(id,socketId);
    }

    return true;
}

ServerEventFunction(CRemoteAccount, HandleAlipayCharge, "Alipay_Charge")
{
	AlipayInfo info;
	info.playerID = phead->Id;
	info.accountID = pPack->readInt(Base::Bit32);
	pPack->readString(info.accountName, sizeof(info.accountName));
	info.areaId = pPack->readInt(Base::Bit32);
	pPack->readString(info.IP, sizeof(info.IP));
	info.price = pPack->readInt(Base::Bit32);
	info.socket = socketId;
	
	//DB_Alipay* pDBHandle = new DB_Alipay(info);
	//CRemoteAccount::getInstance()->GetDBManager()->SendPacket(pDBHandle);

	return true;
}
//收到注册账号信息
ServerEventFunction( CRemoteAccount, HandleCreateAccountRst, "RA_LoginRst")
{
	char AccountName[ACCOUNT_NAME_LENGTH] = "";
	char Password[PASSWORD_LENGTH] = "";
	pPack->readString(AccountName, sizeof(AccountName));
	//pPack->readString(Password, sizeof(Password));

	DB_Login* pDBHandle = new DB_Login();
	dMemcpy(pDBHandle->TokenID, AccountName, sizeof(AccountName));
	//dMemcpy(pDBHandle->Password, Password, sizeof(Password));
	pDBHandle->socketId = socketId;
	pDBHandle->AccountSocketId = phead->Id;

	CRemoteAccount::getInstance()->GetDBManager()->SendPacket(pDBHandle);
	return true;
}

//收到world发送的内购消息
ServerEventFunction( CRemoteAccount, HandleWorlPurchaseRequest, "CR_PURCHASE_REQUEST")
{
	char strReceipt[10240] = "";
	U32 nPlayerId = phead->Id;
	S32 iLengthen = phead->DestZoneId;
	if(iLengthen < 0)
		return false;

	pPack->readBits(iLengthen << 3, strReceipt);

	Http_Purchase* pDBHandle = new Http_Purchase();
	pDBHandle->m_playerId = nPlayerId;
	pDBHandle->m_reciept = strReceipt;
	CRemoteAccount::getInstance()->GetThreadPool()->SendPacket(pDBHandle);
	return true;
}

//收到绑定玩家账号请求
ServerEventFunction( CRemoteAccount, HandleClientBindAccountRequest, "CLIENT_Bind_AccountRequest")
{
	U32 nAccountId = phead->Id;
	U32 nAccountLen = 0, nPassWordLen = 0;
	char AccountName[50] = "";
	char PassWord[32] = "";
	pPack->readString(AccountName, 50);
	pPack->readString(PassWord, 32);

	nAccountLen  = strlen(AccountName);
	nPassWordLen = strlen(PassWord);
	if(nAccountLen < 1 && nAccountLen > 50)
		return false;

	if(nPassWordLen < 1 && nPassWordLen > 32)
		return false;

	DB_BindAccount* pDBHandle = new DB_BindAccount();
	pDBHandle->account    = nAccountId;
	dMemcpy(pDBHandle->m_AccountName, AccountName,  nAccountLen);
	dMemcpy(pDBHandle->m_Password,    PassWord,		nPassWordLen);
	CRemoteAccount::getInstance()->GetDBManager()->SendPacket(pDBHandle);
	return true;
}

ServerEventFunction(CRemoteAccount, HandleWorldRechargeRequest, "WR_RECHARGE_RESPONSE")
{
	std::string UID = pPack->readString(50);
	DB_Reward_Draw* pDBHandle = new DB_Reward_Draw();
	pDBHandle->UID = UID;
	CRemoteAccount::getInstance()->GetDBManager()->SendPacket(pDBHandle);
	return true;
}

ServerEventFunction(CRemoteAccount, HandleWorldCodeRequest, "WR_CODE_RESPONSE")
{
	std::string UID = pPack->readString(50);
	DB_Code_Draw* pDBHandle = new DB_Code_Draw();
	pDBHandle->UID = UID;
	CRemoteAccount::getInstance()->GetDBManager()->SendPacket(pDBHandle);
	return true;
}

ServerEventFunction(CRemoteAccount, HandleWorldCreateOrg, "WR_CreateOrg")
{
	DB_CreateOrg* pDBHandle = new DB_CreateOrg();
	pDBHandle->account = phead->Id;
	pDBHandle->socketId = socketId;
	pDBHandle->name = pPack->readString();
	pDBHandle->qq = pPack->readString();
	pDBHandle->memo = pPack->readString();
	pDBHandle->autoAcceptJoin = pPack->readFlag();
	CRemoteAccount::getInstance()->GetDBManager()->SendPacket(pDBHandle);
	return true;
}

ServerEventFunction(CRemoteAccount, HandleWorldDeleteOrg, "WR_DeleteOrg")
{
	DB_DeleteOrg* pDBHandle = new DB_DeleteOrg();
	pDBHandle->id = phead->Id;
	CRemoteAccount::getInstance()->GetDBManager()->SendPacket(pDBHandle);
	return true;
}