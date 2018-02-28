#ifndef __TBLPLAYER_H__
#define __TBLPLAYER_H__

#ifndef __DBERROR_H__
#include "DBError.h"
#endif//__DBERROR_H__

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif//__TBLBASE_H__

#include <vector>

struct SimplePlayerData;

class TBLPlayer : public TBLBase
{
public:
	TBLPlayer(DataBase* db):TBLBase(db) {}
	~TBLPlayer() {}

	//=================压缩二进制方式的角色数据表==============================
	DBError Load_Simple(int PlayerID,SimplePlayerData* pData);
	DBError Load_Simple(const char* name,SimplePlayerData* pData);
	DBError Save_Simple(unsigned int playerId,SimplePlayerData* pData);
	DBError Save_Simple_DEL(unsigned int playerId,SimplePlayerData* pData);
	DBError Create_Simple(SimplePlayerData* pData);

	DBError ClearFirstLogin(int PlayerId);
	DBError UpdateGold(int PlayerId, S32 Gold);
	DBError UpdateDrawGold(int PlayerId, S32 Gold);
	DBError UpdateVipLv(int PlayerId, U32 VipLv);
	DBError UpdateFirstClass(int PlayerId, S32 nVal);
	DBError UpdateFamily(int PlayerId, S32 nVal);
};

#endif//__TBLPLAYER_H__