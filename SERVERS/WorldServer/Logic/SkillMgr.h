#ifndef __SKILL_MGR_H
#define __SKILL_MGR_H
#include "SkillBase.h"
#include "BaseMgr.h"
#include "Base/tVector.h"

class CSkillMgr : public CBaseManagerEx<SkillRef, S32>
{
	typedef CBaseManagerEx<SkillRef, S32>   Parent;
public:
	CSkillMgr();
	virtual ~CSkillMgr();

	static CSkillMgr* Instance();

	SkillRef CreateData(U32 nPlayerID, S32 nID);
	bool AddData(U32 nPlayerID, S32 nID);

	void CaculateStats(U32 nPlayerID, S32 nID);
	enOpError LevelUp(U32 nPlayerID, std::string UID, S32 nID);
	void DoLevelUp(U32 nPlayerID, S32 nID);
	//---------Notify Operator---------//
	void UpdateToClient(U32 nPlayerID, SkillRef pData);
	void SendInitToClient(U32 nPlayerID);

	//---------DB Operator---------//
	DB_Execution* LoadDB(U32 nPlayerID);

	bool UpdateDB(U32 nPlayerID, SkillRef pData);
	bool InsertDB(U32 nPlayerID, SkillRef pData);
	bool DeleteDB(U32 nPlayerID, S32 ID);

	bool HandleTechnologyLevelUpRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);

	static void Export(struct lua_State* L);
};

#define SKILLMGR CSkillMgr::Instance()
#endif//__SKILL_MGR_H