#ifndef __TBLSOCIAL_H__
#define __TBLSOCIAL_H__

#ifndef __DBERROR_H__
#include "DBError.h"
#endif//__DBERROR_H__

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif

#pragma once
#include "Common\SocialBase.h"

struct stSocialItem;

class TBLSocial : public TBLBase
{
public:
	TBLSocial(DataBase* db):TBLBase(db) {}
	~TBLSocial() {}
	DBError Load(int PlayerID, SocialItemMap& info);
	DBError Save(int PlayerID, SocialItemMap& info);
	DBError Save(int playerId, const stSocialItem& social);
	DBError Update(const LimitMap& friendValue);
	DBError Update(int PlayerID, const stSocialItem& social);
	DBError Update(int PlayerID, const stSocialItem& social,int destType);
	DBError Delete(int PlayerID);
	DBError DeleteAll(int PlayerId);
	DBError DeleteTarget(int PlayerID, int TargetID);
	DBError DeleteType(int PlayerID, int type);
	DBError UpdateForGm(int PlayerID, const stSocialItem& social,int destType);
};

#endif//__TBLSOCIAL_H__