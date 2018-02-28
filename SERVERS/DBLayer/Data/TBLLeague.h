#ifndef TBL_LEAGUE_H
#define TBL_LEAGUE_H

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif

#include "Common\OrgBase.h"

class TBLLeague : public TBLBase
{
public:
	TBLLeague(DataBase* db);
	DBError CreateLeague(League& league);
	DBError LoadLeagues(LeagueDB& leagueDB);
	DBError DelApply(int leagueID, const OrgDelApplys& orgs);
	DBError DisbandLeague(int leagueID);
	DBError UpdateLeague(const League& league);
	DBError AddApply(int leagueID, int orgID);
	DBError UpdateRes(int leagueID, const LeagueRes& res);
	DBError UpdateDemesne(const LeagueDemesne& demesne);
	DBError DelDemesne(const OrgIDSet& leauges, DEMESNE_STATE state);
	DBError UpdateDemesneIncome(const LeagueDemesneIncome& income);
} ;

#endif
