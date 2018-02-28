#ifndef __TBLEXECUTION_H__
#define __TBLEXECUTION_H__

#include "DBLayer/Common/Define.h"
#include "TBLBase.h"
#include "BASE/functionLink.h"
#include <functional>

//--------Í¬²½sql-----------//
class CDBConn;
class TBLExecution:public TBLBase
{
public:
	TBLExecution(DataBase* db):TBLBase(db), m_id(0), m_pFunction(NULL)
	{

	}
	virtual ~TBLExecution() 
	{
	}

	template<class T>
	bool RegisterFunction(T func)
	{
		m_pFunction = func;
		return true;
	}

	void		SetId(int id) {	m_id = id;	}
	void		SetSql(std::string str) {m_sql = str;}
	int			GetId() { return m_id; }

	DBError		Commint();
private:
	int			m_id;
	std::string m_sql;
	std::function<void(int, int, void*)> m_pFunction;
};

#endif//__TBLEXECUTION_H__
