#include "PlayerNameData.h"
#include "Commlib/CommLib.h"
#include "../WorldServer.h"
#include "Common/DataFile.h"
#include "Common/mRandom.h"

// 注意以下顺序不可随意修改
IMPLEMENT_GAMEDATA_CLASS( PlayerNameData ) 
{
	new( getClass(), "姓氏   ")GameDataProperty( GAMEDATA_OFFSET(SurName)	 , GameDataPropertyTypes::TypeString );
	new( getClass(), "中间名1")GameDataProperty( GAMEDATA_OFFSET(MiddleName1), GameDataPropertyTypes::TypeString );
	new( getClass(), "尾名1	")GameDataProperty( GAMEDATA_OFFSET(LastName1)	 , GameDataPropertyTypes::TypeString );
	new( getClass(), "中间名2")GameDataProperty( GAMEDATA_OFFSET(MiddleName2), GameDataPropertyTypes::TypeString );
	new( getClass(), "尾名2 ")GameDataProperty( GAMEDATA_OFFSET(LastName2), GameDataPropertyTypes::TypeString );
}


void PlayerNameMgr::LoadInfo()
{
	//这里暂且不删除内存数据.
	//m_PlayerNameMap.clear();
	m_SurNameVec.clear();
	m_Mid1NameVec.clear();
	m_Last1NameVec.clear();
	m_Mid2NameVec.clear();
	m_Last2NameVec.clear();

	CDataFile op;
	RData tempdata;

	if(!op.readDataFile(MONITOR_PLAYERNAME))
	{
		//MessageBoxA( 0, "载入副本DATA错误!","严重错误", MB_OK );
		return;
	}

	PlayerNameData* pNameData;

	for(int i=0; i<op.RecordNum; ++i)
	{
		pNameData = new PlayerNameData();

		for( int j=0; j<op.ColumNum; j++ )
		{
			op.GetData(tempdata);
			if( tempdata.m_Type == GameDataPropertyTypes::TypeString )
			{
				GameDataProperty* pProty = PlayerNameData::getClass()->getProperty( j );
				if( pProty )
					pProty->value<std::string>( pNameData ) = tempdata.m_string;
			}
			else if( tempdata.m_Type == GameDataPropertyTypes::TypeU8 )
			{
				GameDataProperty* pProty = PlayerNameData::getClass()->getProperty( j );
				if( pProty )
					pProty->value<int>( pNameData ) = tempdata.m_U8;
			}
			else if( tempdata.m_Type == GameDataPropertyTypes::TypeU16 )
			{
				GameDataProperty* pProty = PlayerNameData::getClass()->getProperty( j );
				if( pProty )
					pProty->value<int>( pNameData ) = tempdata.m_U16;
			}
			else if( tempdata.m_Type == GameDataPropertyTypes::TypeS32 )
			{
				GameDataProperty* pProty = PlayerNameData::getClass()->getProperty( j );
				if( pProty )
					pProty->value<int>( pNameData ) = tempdata.m_S32;
			}
			else
			{
				GameDataProperty* pProty = PlayerNameData::getClass()->getProperty( j );
				if( pProty )
					pProty->value<int>( pNameData ) = tempdata.m_U32;
			}
		}


		//m_PlayerNameMap[pNameData->SurName] = pNameData;
		m_SurNameVec.push_back(pNameData->SurName);
		m_Mid1NameVec.push_back(pNameData->MiddleName1);
		m_Mid2NameVec.push_back(pNameData->MiddleName2);
		m_Last1NameVec.push_back(pNameData->LastName1);
		m_Last2NameVec.push_back(pNameData->LastName2);
	}
}


/*PlayerNameData* PlayerNameMgr::GetData(std::string Name)
{
	NAME_MAPDATA::iterator iter = m_PlayerNameMap.find(Name);

	if (iter == m_PlayerNameMap.end())
		return 0;

	return iter->second;
}*/

const char* PlayerNameMgr::GetRandomName()
{
	static U32 nNameSize = 0, nRandomVal = 0;
	nNameSize = gRandGen.randI(MIN_PLAYERNAME_SIZE, MAX_PLAYERNAME_SIZE);
	static std::string s_strPlayerName;
	s_strPlayerName.clear();

	
	for(int i = 0; i < nNameSize; ++i)
	{
		if(i == 0)
		{
			nRandomVal = gRandGen.randI(0, PlayerNameMgr::getInstance()->m_SurNameVec.size() -1);
			s_strPlayerName += PlayerNameMgr::getInstance()->m_SurNameVec[nRandomVal];
		}
		else if(i == 1)
		{
			nRandomVal = gRandGen.randI(0, PlayerNameMgr::getInstance()->m_Mid1NameVec.size() -1);
			s_strPlayerName += PlayerNameMgr::getInstance()->m_Mid1NameVec[nRandomVal];	
		}
		else if(i == 2)
		{
			nRandomVal = gRandGen.randI(0, PlayerNameMgr::getInstance()->m_Last1NameVec.size() -1);
			s_strPlayerName += PlayerNameMgr::getInstance()->m_Last1NameVec[nRandomVal];	
		}
		else if(i == 3)
		{
			nRandomVal = gRandGen.randI(0, PlayerNameMgr::getInstance()->m_Mid2NameVec.size() -1);
			s_strPlayerName += PlayerNameMgr::getInstance()->m_Mid2NameVec[nRandomVal];	
		}
		else if(i == 4)
		{
			nRandomVal = gRandGen.randI(0, PlayerNameMgr::getInstance()->m_Last2NameVec.size() -1);
			s_strPlayerName += PlayerNameMgr::getInstance()->m_Last2NameVec[nRandomVal];	
		}
	}

	return s_strPlayerName.c_str();
}