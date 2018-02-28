/////////////////////////////////////////////////////////////////
/// BadWordMgr.cpp
///
/// 聊天服务器敏感词汇管理 by Jermaine
/// 08/03/2012
/////////////////////////////////////////////////////////////////



#include "BadWordMgr.h"
#include "DB_LoadBadWord.h"
#include "ChatServer.h"
#include "DB_SaveBadWordPlayer.h"
#include "AtlConv.h"

#define UseGB2312 _acp = 936


CBadWordManager::CBadWordManager() 
{
}

CBadWordManager::~CBadWordManager()
{
}

bool CBadWordManager::TimeProcess(bool bExit)
{
	OLD_DO_LOCK( m_cs );
	DB_LoadBadWord* pHandle = new DB_LoadBadWord;
	SERVER->GetCommonDBManager()->SendPacket(pHandle);

	return true;
}

void CBadWordManager::addBadWordList(std::string badWord)
{
	OLD_DO_LOCK( m_cs );

	USES_CONVERSION;
	std::wstring str(A2W(badWord.c_str()));
	m_BadWordList.push_back(str);
}

void CBadWordManager::clearBadWordList()
{
	OLD_DO_LOCK( m_cs );
	m_BadWordList.clear();
}

size_t CBadWordManager::__isBadWord(const char* content)
{
	OLD_DO_LOCK( m_cs );

	USES_CONVERSION;
	const wchar_t * wWord = A2W(content);
	//const wchar_t * wStar = A2W("*");

	for (size_t i = 0; i < m_BadWordList.size(); ++i)
	{
		const wchar_t* wBadWord = m_BadWordList[i].c_str();
		if ( wcsstr(wWord,wBadWord) != NULL )
			return BADWORD;
// 		else if( wcsstr(wWord,wStar) != NULL )
// 			return STAR;
	}

	return NONE;
}

void CBadWordManager::__recordBadWordPlayerToDB(int playerId, const char* badWord)
{
	OLD_DO_LOCK( m_cs );
	DB_SaveBadWordPlayer* pHandle = new DB_SaveBadWordPlayer;

	pHandle->m_playerId = playerId;
	dStrcpy(pHandle->m_badWord,MAX_BADWORDS_LENGTH,badWord);

	SERVER->GetCommonDBManager()->SendPacket(pHandle);

}

bool CBadWordManager::__replaceBanWord(char* pNewWord,size_t uSize,const char* pOldWord,const wchar_t cReplace /* = '*'*/ )
{
	OLD_DO_LOCK( m_cs );

	USES_CONVERSION;
	std::wstring str(A2W(pOldWord));
	bool bBanWord = false;
	for(size_t i = 0; i < m_BadWordList.size(); ++i)
	{
		const wchar_t * wBadWord = m_BadWordList[i].c_str();
		size_t sIndex = str.find(wBadWord);

		while (sIndex != std::wstring::npos)
		{
			str.replace(sIndex,m_BadWordList[i].length(),m_BadWordList[i].length(),cReplace);
			sIndex = str.find(wBadWord,sIndex+1);
			bBanWord = true;
		}
	}

	if(bBanWord)
	{
		dStrcpy(pNewWord,uSize,W2A(str.c_str()));
	}
	else
		dStrcpy(pNewWord,uSize,pOldWord);

	return bBanWord;
}

int CBadWordManager::dealIncomingWord(stChatMessage* pChatMessage)
{
	OLD_DO_LOCK( m_cs );

	if (!pChatMessage)
		return -1;
	
	size_t msgLegalType = __isBadWord(pChatMessage->szMessage);

	if(BADWORD == msgLegalType)
	{

		__recordBadWordPlayerToDB(pChatMessage->nSender, pChatMessage->szMessage);
		return BADWORD;
	}

	return NONE;

// 	else if (STAR == msgLegalType)
// 	{
// 		__recordBadWordPlayerToDB(pChatMessage->nSender, pChatMessage->szMessage);
// 	}
}