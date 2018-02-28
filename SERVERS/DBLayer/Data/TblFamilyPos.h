#ifndef TBL_FAMILYPOS_H
#define TBL_FAMILYPOS_H

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif


class TBLFamilyPos : public TBLBase
{
public:
	TBLFamilyPos(DataBase* db);
	DBError CreateOrg(Org& org, const OrgMaster& master);
	DBError DisbandOrg(int orgId);
	DBError LoadOrgs(OrgDB& orgDB);
	DBError AddMember(int orgId, const OrgMember& player);
	DBError UpdateMember(int orgId, const OrgMember& player, bool hasDelete=false, int deleteTime=0);
	DBError UpdateOrg(const Org& org);
	DBError AddApply(int orgId, int player);
	DBError DelApply(int orgId, const OrgDelApplys& players);
	DBError UpdateDepot(int orgId, const OrgDepot& depot);
	DBError UpdateRes(int orgId, const OrgRes& res);
	DBError InitBoss(int orgId, const OrgBosses& bosses);
	DBError DelBoss(int orgId, const OrgIDs& delBosses);
	DBError UpdateBoss(int orgId, const OrgBoss& boss);
	DBError Assault(const OrgAssault& assault);
	DBError StopAssault(int orgId);
	DBError UpdateAssaultStat(int orgId, const OrgAssaultStat& assaultStat);
	DBError UpdateDemesne(const OrgDemesne& demesne);
	DBError DelDemesne(const OrgIDSet& orgs, DEMESNE_STATE state);
	DBError UpdateMemberStat(const OrgMember& player);
	DBError UpdateDemesneIncome(const OrgDemesneIncome& income);
	DBError DelDepot(int orgId, const OrgIDs& delDepot);

	//DBError loadRes(int orgId, stOrgResInfo& orgResInfo);
	//DBError saveRes(int resId, stOrgResInfo& orgResInfo);
	//DBError loadMaster(int orgId, stOrgMasterInfo& masterInfo);
	//DBError saveMaster(int orgMasterId, stOrgMasterInfo& masterInfo);
#ifdef _NTJ_UNITTEST
	DBError UT_ClearMember(int orgId);
#endif
} ;

#endif
