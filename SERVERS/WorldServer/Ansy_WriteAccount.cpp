#include "stdafx.h"
#include "Ansy_WriteAccount.h"
#include "DBContext.h"
#include "PlayerMgr.h"
#include "WorldServer.h"
#include "Common/LogHelper.h"

Ansy_WriteAccount::Ansy_WriteAccount(void)
{
    m_totalCount = 0;
    m_pAccounts  = 0;
}

int Ansy_WriteAccount::Execute(int ctxId,void* param)
{
    if(DB_CONTEXT_NOMRAL == ctxId)
    {
        FILE* fp = 0;
        fopen_s(&fp,"account.info","w+");

        if(0 != fp)
        {
			std::ostringstream OutStr;
			OutStr << m_totalCount;
			SERVER->GetLog()->writeLog(0, 0, "在线人数", OutStr.str().c_str());
            fprintf( fp, "\nPlayerCount: %d\n",m_totalCount);
            fprintf( fp, "|AccountName\t|PlayerName\t|playerId\t|Status\n" );

            if (m_totalCount > MAX_ACCOUNT_WRITE)
            {
                m_totalCount = MAX_ACCOUNT_WRITE;
            }

            for (U32 i = 0; i < m_totalCount; ++i)
            {
                AccoutWriteInfo& info = m_pAccounts[i];
                fprintf(fp, "|%s\t\t|%s\t\t|%d\t\t|%d\n",info.accountName,info.playerName,info.playerId,info.state);
            }
            
            fclose(fp);
        }

        PostLogicThread(this);
        return PACKET_NOTREMOVE;
    }
    else
    {
        //通知服务器数据已经写完了
        SERVER->GetPlayerManager()->m_canWriteAccout = true;
    }

    return PACKET_OK;
}