#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "TBLBadWord.h"


// ----------------------------------------------------------------------------
// ²éÑ¯¹ýÂËÎÄ×Ö
DBError TBLBadWord::QueryWords(std::vector<std::string>& BadWordList)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(),"SELECT badWord FROM Tbl_BadWord");
		while(GetConn()->More())
		{
			std::string str(GetConn()->GetString());
			BadWordList.push_back(str);
		}

		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

