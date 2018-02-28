#include "DB_SaveBadWordPlayer.h"
#include "DBLayer/common/DBUtility.h"
#include "ChatServer.h"
#include "../ChatServer/DBContext.h"
#include "../DBLayer/Data/TBLBadWordPlayer.h"
#include <list>

DB_SaveBadWordPlayer::DB_SaveBadWordPlayer(void)
{
	m_playerId = 0;

	for (size_t i = 0; i < MAX_BADWORDS_LENGTH; ++i)
	{
		m_badWord[i] = 0;
	}

}

int DB_SaveBadWordPlayer::Execute(int ctxId,void* param)
{
	if(DB_CONTEXT_NOMRAL == ctxId)
	{

		CDBConn* pDBConn = (CDBConn*)param;
		assert(0 != pDBConn);

		TBLBadWordPlayer tblBadWordPlayer(0);
		tblBadWordPlayer.AttachConn(pDBConn);

		DBError err = tblBadWordPlayer.SaveBadWordPlayer(m_playerId,m_badWord);

		if (DBERR_NONE != err)
		{
			 g_Log.WriteError("±£´æÃô¸Ð´Ê»ãÍæ¼ÒÊ§°Ü(err=%s)",getDBErr(err));
		}


		

	}

	return PACKET_OK;
}