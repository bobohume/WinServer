#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include <time.h>
#include "TBLDiscount.h"
#include "Common/BillingBase.h"

TBLDiscount::TBLDiscount(DataBase* db):TBLBase(db)
{
}

TBLDiscount::~TBLDiscount()
{
}

// ----------------------------------------------------------------------------
// 查询商城所有折扣数据
DBError TBLDiscount::LoadAll(std::vector<stDiscount*>& Info)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		M_SQL(GetConn(),"SELECT * FROM Tbl_Discount");
		stDiscount* discount;
		while(GetConn()->More())
		{
			discount = new stDiscount;
			discount->DiscountID = GetConn()->GetInt();
			discount->GoodsID = GetConn()->GetInt();
			discount->DiscountRate = GetConn()->GetInt();
			discount->StartTime = GetConn()->GetInt();
			discount->EndTime = GetConn()->GetInt();
			Info.push_back(discount);
		}
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

// ----------------------------------------------------------------------------
// 查询商城折扣数据(因每周必定维护，所以查询数据量可以优化，仅一周数据即可)
DBError TBLDiscount::Load(std::vector<stDiscount*>& Info)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		struct tm today, after7day;
		time_t curtime, endtime;
		time(&curtime);
		localtime_s(&today, &curtime);
		endtime = curtime + (24 * 3600 * 7);
		localtime_s(&after7day, &endtime);

		M_SQL(GetConn(),"SELECT * FROM Tbl_Discount WHERE EndTime>='%d-%02d-%02d %02d:%02d' AND StartTime<='%d-%02d-%02d %02d:%02d'",
			today.tm_year + 1900, today.tm_mon + 1, today.tm_mday, today.tm_hour, today.tm_min,
			after7day.tm_year + 1900, after7day.tm_mon + 1, after7day.tm_mday, after7day.tm_hour, after7day.tm_min);
		stDiscount* discount;
		while(GetConn()->More())
		{
			discount = new stDiscount;
			discount->DiscountID = GetConn()->GetInt();
			discount->GoodsID = GetConn()->GetInt();
			discount->DiscountRate = GetConn()->GetInt();
			discount->StartTime = GetConn()->GetTime();
			discount->EndTime = GetConn()->GetTime();
			Info.push_back(discount);
		}
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

// ----------------------------------------------------------------------------
// 存储商城折扣数据
DBError TBLDiscount::Save(std::vector<stDiscount*>& info)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(),"TRUNCATE TABLE Tbl_Discount");
		GetConn()->Exec();

		char szRecord[8000] = {0};
		char szRow[128] = {0};
		bool isFirst = true;
		for(size_t i = 0; i < info.size(); i++)
		{
			if(i % 100 == 0)
			{
				if(!isFirst)
				{
					M_SQL(GetConn(),szRecord);
					GetConn()->Exec();
				}
				strcpy_s(szRecord, 8000, "INSERT Tbl_Discount (DiscountID,GoodsID,Rate,StartTime,"\
					"EndTime) SELECT ");
				isFirst = true;
			}

			if(!isFirst)
				strcat_s(szRecord, 8000, " UNION ALL SELECT ");

			sprintf_s(szRow, 128, "%d,%d,%d,%d,%d", 
				info[i]->DiscountID,
				info[i]->GoodsID,
				info[i]->DiscountRate,
				info[i]->StartTime,
				info[i]->EndTime);
			strcat_s(szRecord, 8000, szRow);
			isFirst = false;
		}

		if(!isFirst)
		{
			M_SQL(GetConn(),szRecord);
			GetConn()->Exec();
		}
	}
	DBECATCH()
	return err;
}