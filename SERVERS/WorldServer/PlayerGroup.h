#ifndef PLAYER_GROUP_H
#define PLAYER_GROUP_H

#include <list>

struct IPlayerGroup
{
	void GetPlayers( std::list<int>& playerIdList );
};


#endif