#ifndef _TEAMSMDATA_H_
#define _TEAMSMDATA_H_

#include "CommLib/ShareMemoryKey.h"
#include "CommLib/ShareMemoryUserPool.h"
#include "common/TeamBase.h"

class TeamSMData
{
public:
	TeamSMData(void)
	{
		Clear();
	}

    void Clear(void)
    {
        m_nId        = 0;
        m_randTeamId = 0;
        m_szName[0]  = 0;
		m_bIsSquad   = false;
		m_cV         = 0;	// 物品分配方式
		m_cU         = 0;	// 经验分配方式
		m_nCopymapId = 0;   // 副本编号
        m_nChatChannelId = 0;
		memset(m_teammate, 0, sizeof(m_teammate));
		m_otherInfo.Clear();
    }

	stTeamMapMark    mapMark[MAX_TEAM_MAP_MARK_COUNT];
	stTeamTargetMark targetMark[MAX_TEAM_TARGET_COUNT];

	int  m_nId;
    int  m_randTeamId;
	char m_szName[MAX_TEAM_NAME_LENGTH];
	bool m_bIsSquad;
	char m_cU;
	char m_cV;
	int  m_nCopymapId;
	TeamMemberInfo  m_teammate[MAX_TEAMMATE_NUM];
    int  m_nChatChannelId;
	OtherTeamInfo m_otherInfo;
};

#define SM_MAXTEAMCOUNT 100000
typedef SMDataPool<TeamSMData,U32,SM_MAXTEAMCOUNT> SM_TEAM_POOL;
typedef SMDataPool<TeamSMData,U32,SM_MAXTEAMCOUNT>::POOL_DATA SM_TEAM_DATA;

#endif /*_TEAMSMDATA_H_*/