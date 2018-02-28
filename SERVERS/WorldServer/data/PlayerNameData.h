#ifndef _PLAYER_NAME_DATA_H_
#define _PLAYER_NAME_DATA_H_

#include "Common/GameData.h"
#include "base/types.h"

class PlayerNameData
{
	DECLARE_GAMEDATA_CLASS(PlayerNameData);
public:
	std::string SurName;
	std::string MiddleName1;
	std::string LastName1;
	std::string MiddleName2;
	std::string LastName2;
};

class PlayerNameMgr
{
	typedef stdext::hash_map<std::string,PlayerNameData*> NAME_MAPDATA;
	typedef std::vector<std::string> NAME_VECDATA;
	enum
	{
		MIN_PLAYERNAME_SIZE			=	5,
		MAX_PLAYERNAME_SIZE			=	5,
	};

public:
	 void Reload(void)
	 {
		LoadInfo();
	 }

	 static PlayerNameMgr* getInstance()
	 {
		static PlayerNameMgr s_PlayerNmaeMgr;
		return &s_PlayerNmaeMgr;
	 }

	 void LoadInfo();
	 //NAME_MAPDATA& GetDatas(void) {return m_PlayerNameMap;}

	 //PlayerNameData*	GetData(std::string Name);
	 static const char* GetRandomName();

	 NAME_VECDATA m_SurNameVec;
	 NAME_VECDATA m_Mid1NameVec;
	 NAME_VECDATA m_Last1NameVec;
	 NAME_VECDATA m_Mid2NameVec;
	 NAME_VECDATA m_Last2NameVec;
private:
	//NAME_MAPDATA m_PlayerNameMap;
};

#endif /*_COPYMAP_H_*/