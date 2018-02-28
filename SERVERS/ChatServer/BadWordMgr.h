/////////////////////////////////////////////////////////////////
/// BadWordMgr.h
///
/// 聊天服务器敏感词汇管理 by Jermaine
/// 08/03/2012
/////////////////////////////////////////////////////////////////

#ifndef BADWORD_MGR_H
#define BADWORD_MGR_H

#include <hash_map>
#include <list>
#include "Common/TimerMgr.h"
#include "Common/ChatBase.h"


class CDBConnPool;
class CDBConn;

typedef unsigned int UINT;

enum BadType
{
	NONE,
	STAR,
	BADWORD
};


class CBadWordManager : public CTimeCriticalObject
{
public:
	CBadWordManager();
	~CBadWordManager();

	bool TimeProcess(bool bExit);

	void addBadWordList(std::string badWord);
	void clearBadWordList();
	size_t __isBadWord(const char* content);
	void __recordBadWordPlayerToDB(int playerId, const char* badWord);
	bool __replaceBanWord(char* pNewWord,size_t uSize,const char* pOldWord,const wchar_t cReplace = '*' );
	int dealIncomingWord(stChatMessage* pChatMessage); 


	CMyCriticalSection m_cs;

private:
	typedef std::vector<std::wstring>  BadWordList;
	BadWordList		m_BadWordList;


};

#endif