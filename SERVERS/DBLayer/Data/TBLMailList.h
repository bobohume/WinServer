#ifndef __TBLMAILLIST_H__
#define __TBLMAILLIST_H__

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif

#pragma once
#include <vector>

struct stMailItem;

class TBLMailList: public TBLBase
{
public:
	TBLMailList(DataBase* db):TBLBase(db) {}
	virtual ~TBLMailList() {}
	DBError Save(int &nRecver, stMailItem& mailItem);
	DBError UpdateReaded(int id);
	DBError setMailRecv(int id);
	DBError resetMailRecv(int id);
	DBError LoadForPage(int nRecver, std::vector<stMailItem>& Info, int& totalrow, int& noreads);
	DBError Load(int id, stMailItem& mailItem, bool bLoadMsg = false );
	DBError Delete(int id);
	DBError DeletePage(int id,std::vector<int>& idlist);
	DBError DeleteAll(int nRecver,std::vector<int> &);
	DBError LoadAll(int nRecver, std::vector<stMailItem*>& Info);
};

#endif//__TBLMAILLIST_H__