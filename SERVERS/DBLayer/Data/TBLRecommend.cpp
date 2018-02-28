#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "TBLRecommend.h"
#include <time.h>
#include "DBLayer/Common/DBUtility.h"
#include "Common/BillingBase.h"

TBLRecommend::TBLRecommend(DataBase* db):TBLBase(db)
{
}

TBLRecommend::~TBLRecommend()
{
}

// ----------------------------------------------------------------------------
// 查询推荐配置数据
DBError TBLRecommend::Load(std::vector<stRecommend*>& Info)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		time_t curtime;
		time(&curtime);
		unsigned int today = (unsigned int)curtime;
		unsigned int after7day = (unsigned int)curtime + 3600 * 24 * 7;
		char szToday[32], szAfter7Day[32];
		GetDBTimeString(today, szToday);
		GetDBTimeString(after7day, szAfter7Day);

		typedef stdext::hash_map<int, stRecommend*> RECOMMENDMAP;
		RECOMMENDMAP recommendlist;
		stRecommend* recommend;

		M_SQL(GetConn(),"EXECUTE Sp_QueryRecommend '%s','%s'", szToday, szAfter7Day);
		while(GetConn()->More())
		{
			U32 RecommendID = GetConn()->GetInt();
			U64 PriceID = GetConn()->GetBigInt();
			RECOMMENDMAP::iterator it = recommendlist.find(RecommendID);
			if(it != recommendlist.end())
				recommend = it->second;
			else
			{
				recommend = new stRecommend;
				recommend->RecommendID = RecommendID;
				recommendlist.insert(RECOMMENDMAP::value_type(RecommendID, recommend));
			}

			for(int i = 0; i< stRecommend::MAX_GOODS; i++)
			{
				if(recommend->PriceID[i] == 0)
				{
					recommend->PriceID[i] = PriceID;
					break;
				}
			}
		}

		for each(const std::pair<int, stRecommend*>& it in recommendlist)
		{
			Info.push_back(it.second);
		}
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}