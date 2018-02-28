#ifndef _DB_EXCEPTION_H
#define _DB_EXCEPTION_H

class CDBException
{
public:
	CDBException();
	CDBException(int severity, int dberr, int oserr, const char * dberrstr,const char * oserrstr);
	CDBException(int severity, int dberr, int oserr, const char * dberrstr,const char * oserrstr,
		long msgno, int msgstate, int _severity, const char * msgtext, const char * srvname
		,const char *procname, unsigned short line);
	CDBException(long msgno, int msgstate, int _severity, const char * msgtext, const char * srvname
		,const char *procname, unsigned short line);
	virtual ~CDBException();

public:
	int         m_severity;			//错误的严重性
	int         m_dberr;			//数据库错误
	int         m_oserr;			//操作系统错误
	char        m_dberrstr[256];	//数据库错误信息
	char        m_oserrstr[256];	//操作系统错误信息

public:	//[扩展]本次异常的警告信息
	long        w_msgno;			//消息ID
	int         w_msgstate;			//消息状态
	int         w_severity;			//消息严重性
	char        w_msgtext[8192];	//消息文本
	char        w_srvname[64];		//服务器名
	char        w_procname[64];		//过程名
	unsigned short w_line;			//消息行数
};

#endif//_DB_EXCEPTION_H