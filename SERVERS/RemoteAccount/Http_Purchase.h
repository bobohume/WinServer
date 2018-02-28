#ifndef __HTTP_PURRCHASE_H__
#define __HTTP_PURRCHASE_H__

#include "CommLib/ThreadPool.h"
#include <vector>
#include <map>

class DB_ExecutePool;

const int MAX_RECIEPT_LEN	= 10240;

//----------------------------------------------------------------------------
class Http_Purchase : public ThreadBase
{
public:

	Http_Purchase();

	char* LetSpacesGo(const char *pStr);
	virtual int Execute(int ctxId,void* param);

	int				m_Error;
	U32				m_playerId;
	std::string		m_reciept;;
	std::string		m_URL;


	// return info
	std::string		m_Quantity;
	std::string		m_Product_id;
	std::string		m_itemId;
	std::string		m_Transaction_id;
	std::string		m_Purchase_date;
	
};

#endif //__HTTP_PURRCHASE_H__