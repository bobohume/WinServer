#ifndef LOG_HELPER_H
#define LOG_HELPER_H

#include <cstring>
#include <string>
#include <hash_map>
#include <time.h>

class CLogHelperImpl;

struct dbStruct;

class CLogHelper
{
public:
	CLogHelper();
	virtual ~CLogHelper();

	void connect( const char* ipAddress, int port );
	void disconnect();
    void check();
	
	void writeLog( dbStruct* pStruct );
	void writeLog(int sourceid, int targetid, std::string optype, std::string val0="", std::string val1 = "", std::string val2  =  "", std::string val3 = "");
private:
    CLogHelperImpl* m_pImpl;
};

#endif