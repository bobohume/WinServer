#include "DB_Execution.h"
#include "../DBContext.h"
#include "../DBLayer/common/DBUtility.h"
#include "StringTable.h"
#include "../stdafx.h"

DB_Execution::DB_Execution():m_error(0), m_curRow(-1), m_curCol(0), m_cols(0), m_id(0), \
	m_isChecked(false), m_pFunction(NULL), m_pDoneFunction(NULL)
{
}

DB_Execution::~DB_Execution(void)
{
}

int DB_Execution::Execute(int ctxId,void* param)
{
	if(DB_CONTEXT_NOMRAL == ctxId || DB_CONTEXT_CHILD == ctxId)
	{
		if (m_sql != "")
		{
#ifdef _DEBUG
			g_Log.WriteLog("提交数据库操作[%s]",m_sql.c_str());
#endif

			CDBConn* pDBConn = (CDBConn*)param;
			assert(0 != pDBConn);

			try
			{
				if(m_isChecked && !StrSafeCheck(m_sql.c_str(),4096))
					m_error =DBERR_DB_SQLPARAMETER;
				else
				{
					pDBConn->SQLExt(m_sql.c_str());

					if (!m_pFunction)
					{
						//不需要返回的直接执行好了
						pDBConn->Exec();
					}
					else
					{
						while(pDBConn->Fetch())
						{
							m_rslts.push_back(CRow());
							CRow& tRow = m_rslts.back();
							TROW& row = tRow.mRows;
							tRow.mRecordIdx = pDBConn->GetRecordSetIndex();
							int cols = 0;

							if (m_rowTypes.find(tRow.mRecordIdx) == m_rowTypes.end())
							{	
								CRowType& tRowType = m_rowTypes[tRow.mRecordIdx];
								tRowType.m_cols = pDBConn->GetNumCols();
								cols = tRowType.m_cols;
									
								for (int iCol = 0; iCol < cols; ++iCol)
								{
									tRowType.m_colNames.push_back(pDBConn->GetColName(iCol));

									int colType = pDBConn->GetColType(iCol);

									switch(colType)
									{
									case SQLCHAR: 
									case SQLVARCHAR:
										tRowType.m_colTypes.push_back(DB_TYPE_CHAR);
										break;
									case SQLINT1:
									case SQLINT2:
									case SQLINT4:
										tRowType.m_colTypes.push_back(DB_TYPE_INT);
										break;
									case SQLFLT4:
									case SQLFLT8:
										tRowType.m_colTypes.push_back(DB_TYPE_FLOAT);
										break;
									case SQLNUMERIC:
										tRowType.m_colTypes.push_back(DB_TYPE_BIGINT);
										break;
									case SQLDATETIM4:
									case SQLDATETIME:
										tRowType.m_colTypes.push_back(DB_TYPE_TIME);
										break;
									default:
										g_Log.WriteError("数据库字段类型无法识别[%d]",colType);
										assert(0);
										throw;
										break;
									}
								}
							}
							else
							{
								cols = m_rowTypes[tRow.mRecordIdx].m_cols;
							}

							row.reserve(cols);

							for (int iCol = 0; iCol < cols; ++iCol)
							{
								Field value;

								int colType = pDBConn->GetColType(iCol);

								switch(colType)
								{
								case SQLCHAR: 
								case SQLVARCHAR:
									{
										const char* pstr = pDBConn->GetString();
										value.vChar = pstr ? pstr : "";
										value.vChar = (value.vChar == " " ? "" : value.vChar);
									}
									break;
								case SQLINT1:
								case SQLINT2:
								case SQLINT4:
									value.vi = pDBConn->GetInt();
									break;
								case SQLFLT4:
								case SQLFLT8:
									value.vDouble = pDBConn->GetDouble();
									break;
								case SQLNUMERIC:
									value.vi64 = pDBConn->GetBigInt();
									break;
								case SQLDATETIM4:
								case SQLDATETIME:
									value.vi = pDBConn->GetTime();
									break;
								default:
									g_Log.WriteError("数据库字段类型无法识别[%d]",colType);
									assert(0);
									throw;
									break;
								}

								row.push_back(value);
							}

							m_error = DBERR_NONE;
						}
					}
				}
			}
			catch(...)
			{
				m_error = DBERR_UNKNOWERR;

#ifdef _DEBUG
			g_Log.WriteWarn("数据库操作[%s]发生异常",m_sql.c_str());
#endif
			}
		}

		for (size_t i = 0; i < m_childs.size(); ++i)
		{
			m_childs[i]->Execute(DB_CONTEXT_CHILD,param);
		}

		if (DB_CONTEXT_CHILD != ctxId && m_pFunction)
		{
			PostLogicThread(this);
			return PACKET_NOTREMOVE;
		}
	}
	else
	{
#ifdef _DEBUG
		g_Log.WriteLog("数据库操作[%s]...",m_sql.c_str());
#endif
		if (m_pFunction)
		{
			m_pFunction(m_id, m_error, this);
		}

		for (size_t i = 0; i < m_childs.size(); ++i)
		{
			m_childs[i]->Execute(ctxId,param);
		}

		for (size_t i = 0; i < m_childs.size(); ++i)
		{
			delete m_childs[i];
		}

		m_childs.clear();

		if (m_pDoneFunction)
		{
			m_pDoneFunction(m_id, m_error, this);
		}
	}

	return PACKET_OK;
}

bool DB_Execution::More()
{
	m_curRow++;

	if (m_curRow >= m_rslts.size())
		return false;

	m_curCol = 0;
	m_cols   = m_rowTypes[m_rslts[m_curRow].mRecordIdx].m_cols;
	return true;
}

void DB_Execution::Reset()
{
	m_curRow = -1;
	m_curCol = 0;
}

int DB_Execution::GetInt()
{
	assert(m_curCol < m_cols);
	assert(m_curRow < m_rslts.size());

	return m_rslts[m_curRow].mRows[m_curCol++].vi;
}

U64	DB_Execution::GetBigInt()
{
	assert(m_curCol < m_cols);
	assert(m_curRow < m_rslts.size());

	return m_rslts[m_curRow].mRows[m_curCol++].vi64;
}

F64	DB_Execution::GetNumber()
{
	assert(m_curCol < m_cols);
	assert(m_curRow < m_rslts.size());

	return (F64)m_rslts[m_curRow].mRows[m_curCol++].vi64;
}

int DB_Execution::GetTime()
{
	assert(m_curCol < m_cols);
	assert(m_curRow < m_rslts.size());

	return m_rslts[m_curRow].mRows[m_curCol++].vi;
}

const char* DB_Execution::GetString()
{
	assert(m_curCol < m_cols);
	assert(m_curRow < m_rslts.size());

	return m_rslts[m_curRow].mRows[m_curCol++].vChar.c_str();
}

double DB_Execution::GetDouble()
{
	assert(m_curCol < m_cols);
	assert(m_curRow < m_rslts.size());

	return m_rslts[m_curRow].mRows[m_curCol++].vDouble;
}

void DB_Execution::AddChild(DB_Execution* pChild)
{
	if (0 == pChild)
		return;

	m_childs.push_back(pChild);
}

int DB_Execution::ColCount() 
{
	assert(m_curRow < m_rslts.size());
	return m_rowTypes[m_rslts[m_curRow].mRecordIdx].m_cols;
}

const char*	DB_Execution::ColName(int iCol)  
{
	assert(m_curRow < m_rslts.size());
	return m_rowTypes[m_rslts[m_curRow].mRecordIdx].m_colNames[iCol].c_str();
}

int	 DB_Execution::ColType(int iCol) 
{
	assert(m_curRow < m_rslts.size());
	return m_rowTypes[m_rslts[m_curRow].mRecordIdx].m_colTypes[iCol];
}

int	DB_Execution::GetRecordSetIndex()
{
	assert(m_curRow < m_rslts.size());
	return m_rslts[m_curRow].mRecordIdx;
}

