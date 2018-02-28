#ifndef _UTILITY_CONFIGFILE_GAMECONFIG_H__
#define _UTILITY_CONFIGFILE_GAMECONFIG_H__
#include "base/types.h"
#include <set>
#include <vector>
#include <string>

namespace GAMECFG
{
	enum
	{
		PLAYER_LEVELUP_NEED_ID				=  4500001,
		MASTER_LEVELUP_NEED_ID				=  4510001,
	};
};

//ÈËÎï²Ù×÷
extern U64  getPlayerLevelUpExp(U32 nLevel);
#endif