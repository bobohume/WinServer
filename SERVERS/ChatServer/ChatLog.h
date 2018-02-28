#ifndef __CHAT_LOGs_H__
#define __CHAT_LOGs_H__

#include "CommLib/ThreadPool.h"
#include <vector>
#include <map>

class DB_ExecutePool;

const int MAX_RECIEPT_LEN	= 10240;

//----------------------------------------------------------------------------
class ChatLog : public ThreadBase
{
public:

	ChatLog();

	char* LetSpacesGo(const char *pStr);
	virtual int Execute(int ctxId,void* param);

	int				m_Error;
	std::string		gkey;
	std::string		tkey;
	std::string		server_id;
	std::string		qid;
	std::string		name;
	int				type;
	std::string		content;
	int				time;
	std::string		ip;
	std::string		toqid;
	std::string		toname;
	std::string		sign;
};

#endif //__CHAT_LOGs_H__