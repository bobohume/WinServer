#ifndef __TBLBADWORD_H__
#define __TBLBADWORD_H__

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif

#include <vector>
#include "Common/ZoneSetting.h"

class CDBConn;

class TBLBadWord: public TBLBase
{
public:
	TBLBadWord(DataBase* db):TBLBase(db) {}
	virtual ~TBLBadWord() {}
	DBError QueryWords(std::vector<std::string>& BadWordList);
};

#endif//__TBLWORLD_H__