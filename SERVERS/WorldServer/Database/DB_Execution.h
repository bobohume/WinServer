#ifndef __DBEXECUTION_H__
#define __DBEXECUTION_H__

#include "CommLib/ThreadPool.h"
#include "CommLib/format.h"
#include <vector>
#include <map>
#include <hash_map>
#include <hash_set>
#include <functional>

//----------------------һ�첿sql------------------------------------------------------
class DB_Execution : public ThreadBase
{
public:
	enum DBType
	{
		DB_TYPE_INT									= 0,
		DB_TYPE_CHAR								= 1,
		DB_TYPE_FLOAT								= 2,
		DB_TYPE_TIME								= 3,
		DB_TYPE_BIGINT								= 4,
	};

	struct Field
	{
		char		type;		//0 ->int, 1->double,2->char
		int			vi;
		U64			vi64;
		double      vDouble;
		std::string vChar;
	};

	DB_Execution();
	~DB_Execution(void);

	virtual int Execute(int ctxId,void* param);

	bool		More();
	void		Reset();
	U64			GetBigInt();
	F64			GetNumber();
	int			GetInt();
	int			GetTime();
	const char* GetString();
	double		GetDouble();

	int			ColCount();
	const char*	ColName(int iCol);
	int			ColType(int iCol);
	int			GetRecordSetIndex();
	void		SetId(int id) {	m_id = id;	}
	void		SetSql(std::string str) {m_sql = str;}
	void		AddChild(DB_Execution* pChild);

	template<class U>
	void RegisterFunction( U func)
	{
		m_pFunction = func;
	}

	template<class T>
	void RegisterDoneFunction(T func)
	{
		m_pDoneFunction = func;
	}

	typedef std::vector<Field> TROW;

	struct CRow
	{
		int  mRecordIdx;
		TROW mRows;
	};

	struct CRowType
	{
		int m_cols;
		std::vector<std::string> m_colNames;
		std::vector<U8>			 m_colTypes;
	};

private:
	std::vector<CRow> m_rslts;
	int			m_curRow;
	int			m_curCol;
	int			m_cols;
	int			m_error;
	int			m_id;
	bool		m_isChecked;
	std::string m_sql;
	std::function<void(int, int, void*)> m_pFunction;
	std::function<void(int, int, void*)> m_pDoneFunction;
	std::vector<DB_Execution*> m_childs;
	stdext::hash_map<int,CRowType>	   m_rowTypes;
};

#endif //__DBEXECUTION_H__