#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "TBLArena.h"
#include "DBUtility.h"
#include "Common/PlayerStruct.h"

DBError	TBLArena::LoadAllArenas(ARENADATA_MAP& map,int season,int& lastSeason,bool& isRewarded)
{
    DBError err = DBERR_UNKNOWERR;

    try
	{		
		M_SQL(GetConn(),"SELECT playerId,type,totalCombat,win,lose,points FROM Tbl_Arena WHERE season=%d",season);

		while(GetConn()->More())
		{
			int playerId    = GetConn()->GetInt();
			int type        = GetConn()->GetInt();
            int totalCombat = GetConn()->GetInt();
            int win         = GetConn()->GetInt();
            int lose        = GetConn()->GetInt();
            int points      = GetConn()->GetInt();

            if (type < 4)
            {
                ArenaData& data = map[playerId].m_datas[type];

                data.total  = totalCombat;
                data.win    = win;
                data.lose   = lose;
                data.points = points;
            }
		}

        M_SQL(GetConn(),"SELECT season,rewardFlag FROM Tbl_ArenaStatus");

        if(GetConn()->More())
		{
            lastSeason = GetConn()->GetInt();
			isRewarded = GetConn()->GetInt() == 0?false:true;
        }
        
        err = DBERR_NONE;
	}
	DBECATCH()

	return err;
}

DBError	TBLArena::LoadArenaInfo(U32 playerId,U32 type,ArenaData* pData)
{
    DBError err = DBERR_UNKNOWERR;

    if (0 == pData)
        return err;

	try
	{		
		M_SQL(GetConn(),"SELECT totalCombat,win,lose,points FROM Tbl_Arena WHERE playerId=%d AND [type]=%d", playerId,type);

		if(GetConn()->More())
		{
			pData->total  = GetConn()->GetInt();
			pData->win    = GetConn()->GetInt();
            pData->lose   = GetConn()->GetInt();
            pData->points = GetConn()->GetInt();
		}
        else
        {
            //默认开始的时候是0分
            pData->points = 0;
        }

        err = DBERR_NONE;
	}
	DBCATCH(playerId)
	return err;
}

DBError	TBLArena::UpdateArenaInfo(U32 playerId,U32 type,ArenaData& data,int season)
{
    //Sp_SaveArenaInfo
    DBError err = DBERR_UNKNOWERR;

	try
	{		
		M_SQL(GetConn(),"EXECUTE Sp_SaveArenaInfo %d,%d,%d,%d,%d,%d,%d",playerId,type,data.total,data.win,data.lose,data.points,season);

        GetConn()->Exec();
        err = DBERR_NONE;
	}
	DBCATCH(playerId)
	return err;
}

DBError	TBLArena::ClearArena(int season)
{
    DBError err = DBERR_UNKNOWERR;

	try
	{		
		M_SQL(GetConn(),"EXECUTE Sp_ClearArenaInfo %d",season);

        GetConn()->Exec();
        err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

DBError TBLArena::Reward(int season,int item,const char* title,int playerId,int rank)
{
    DBError err = DBERR_UNKNOWERR;

	try
	{		
		M_SQL(GetConn(),"EXECUTE Sp_ArenaReward %d,%d,%d,%d,'%s'",playerId,item,rank,season,title);

        GetConn()->Exec();
        err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

DBError TBLArena::SaveChallengeMember(ChallengeMember* pMember)
{
    DBError err = DBERR_UNKNOWERR;

    if (0 == pMember)
        return err;

	try
	{		
        char binaryBuf1[1024] = {0};
        char binaryBuf2[1024] = {0};
        BattleReport report[MAX_BATTLE_REPORT];
        int i = 0;

        for (std::list<BattleReport>::iterator iter = pMember->reports.begin(); iter != pMember->reports.end(); ++iter,++i)
        {
            if (i >= MAX_BATTLE_REPORT)
                continue;

            report[i] = *iter;
        }
        
        int nSize = MAX_BATTLE_REPORT * sizeof(BattleReport);
        U8* pData = (U8*)&report[0];

        if (nSize > 250)
        {
            ConvertHex(binaryBuf1,sizeof(binaryBuf1),pData,250);
            nSize -= 250;
		    pData += 250;
            
            ConvertHex(binaryBuf2,sizeof(binaryBuf2),pData,nSize);
        }
        else
        {
            ConvertHex(binaryBuf1,sizeof(binaryBuf1),pData,nSize);
        }

		M_SQL(GetConn(),"EXECUTE Sp_SaveChallengeMember %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,0x%s,0x%s",
            pMember->playerId,
            pMember->rank,
            pMember->wins,
            pMember->total,
            pMember->arenaPoints,
            pMember->onlineLimit,
            pMember->offlineLimit,
            pMember->extraLimit,
            pMember->nChallengeBy,
            pMember->nLosed,
            pMember->data.SaveUID,
            pMember->data.quality,
            pMember->data.nSkills,
            pMember->data.nTalents,
            pMember->data.nPetSkills,
            pMember->data.nPetLevel,
            binaryBuf1,
            binaryBuf2);

        GetConn()->Exec();
        err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

DBError TBLArena::LoadChallengeMembers(std::vector<ChallengeMember*>& members)
{
    DBError err = DBERR_UNKNOWERR;

    try
	{		
		M_SQL(GetConn(),"SELECT playerId,rank,wins,total,arenaPoints,onlineLimit,offlineLimit,extraLimit,nChallengeBy,nLosed,SaveUID,quality,nSkills,nTalents,nPetSkills,nPetLevel,report1,report2 FROM Tbl_Challenge WHERE valid=1");

		while(GetConn()->More())
		{
            ChallengeMember* pMember = new ChallengeMember;

			pMember->playerId        = GetConn()->GetInt();
			pMember->rank            = GetConn()->GetInt();
            pMember->wins            = GetConn()->GetInt();
            pMember->total           = GetConn()->GetInt();
            pMember->arenaPoints     = GetConn()->GetInt();
            pMember->onlineLimit     = GetConn()->GetInt();
            pMember->offlineLimit    = GetConn()->GetInt();
            pMember->extraLimit      = GetConn()->GetInt();
            pMember->nChallengeBy    = GetConn()->GetInt();
            pMember->nLosed          = GetConn()->GetInt();
            pMember->data.SaveUID    = GetConn()->GetInt();
            pMember->data.quality    = GetConn()->GetInt();
            pMember->data.nSkills    = GetConn()->GetInt();
            pMember->data.nTalents   = GetConn()->GetInt();
            pMember->data.nPetSkills = GetConn()->GetInt();
            pMember->data.nPetLevel  = GetConn()->GetInt();

            BattleReport report[MAX_BATTLE_REPORT];
            char* pOffset = (char*)&report[0];

            int iLen = 0;
            char* pBlock = (char*)GetConn()->GetBinary("report1",iLen);

            if (iLen > 0)
            {
                memcpy(pOffset,pBlock,iLen);
            }
            
            int iLen2 = 0;
            pBlock = (char*)GetConn()->GetBinary("report2",iLen2);

            if (iLen2 > 0)
            {
                memcpy(pOffset + iLen,pBlock,iLen2);
            }
            
            for (int i = 0; i < MAX_BATTLE_REPORT; ++i)
            {
                if (0 == report[i].time)
                    continue;

                pMember->reports.push_back(report[i]);
            }

            members.push_back(pMember);           
		}

        err = DBERR_NONE;
	}
	DBECATCH()

	return err;
}