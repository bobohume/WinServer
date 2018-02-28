#include "GMMgr.h"
#include "../WorldServer.h"
#include "CommLib/format.h"

const char* s_Cmd = "Tbl_Cmd";

GMCmdProcedure::GMCmdProcedure()
{
	m_isEnded = false;
}

int GMCmdProcedure::run(void)
{
	while (!m_isEnded)
	{
		try
		{
			CRedisDB cmdDB(SERVER->GetRedisPool());
			std::vector<std::string> CmdVec;
			cmdDB.Command(fmt::sprintf("hkeys %s", s_Cmd), CmdVec);
			for (auto itr : CmdVec)
			{
				std::string cmd = cmdDB.Command(fmt::sprintf("hget %s %s", s_Cmd, itr.c_str()));
				SERVER->GetWorkQueue()->PostEvent(0, (void*)cmd.c_str(), cmd.length() + 1, true, WQ_SCRIPT);
				cmdDB.Command(fmt::sprintf("hdel %s %s", s_Cmd, itr.c_str()));
			}
		}
		catch (...)
		{
		}

		Sleep(2000);
	}

	m_isEnded = true;
	return 0;
}