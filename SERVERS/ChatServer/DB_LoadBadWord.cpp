#include "DB_LoadBadWord.h"
#include "DBLayer/common/DBUtility.h"
#include "ChatServer.h"
#include "../ChatServer/DBContext.h"
#include <list>

DB_LoadBadWord::DB_LoadBadWord(void)
{

}

int DB_LoadBadWord::Execute(int ctxId,void* param)
{
	if(DB_CONTEXT_NOMRAL == ctxId)
	{

		CDBConn* pDBConn = (CDBConn*)param;
		assert(0 != pDBConn);

		TBLBadWord  badWordDb(0);
		badWordDb.AttachConn(pDBConn);

		DBError err = badWordDb.QueryWords(m_badWordList);

		if (DBERR_NONE != err)
		{
			 g_Log.WriteError("²éÑ¯Ãô¸Ð´Ê»ãÊ§°Ü(err=%s)",getDBErr(err));
		}


		PostLogicThread(this);
		return PACKET_NOTREMOVE;

	}
	else
	{
		SERVER->GetBadWordMgr()->clearBadWordList();

		for(std::vector<std::string>::iterator it = m_badWordList.begin(); it != m_badWordList.end(); ++it)
			SERVER->GetBadWordMgr()->addBadWordList(*it);
	}
    return PACKET_OK;
}