#ifndef _COMMON_REDIS_H
#define _COMMON_REDIS_H
#include "redis/hiredis/hiredis.h"
#include "BASE/types.h"
#include "BASE/Log.h"
#include <memory>
#include <sstream>
#include <assert.h>
#include <vector>
#include <list>
#include <unordered_map>
#include "SimpleTimer.h"

static std::string m_SpeStr = "#";
//redis 连接池
class CRedisConn;
class CRedisException
{
public:
	CRedisException();
	~CRedisException();
};

class CRedisConnPool
{
public:
	CRedisConnPool();
	~CRedisConnPool();

public:
	bool Open(int nMaxConn, const char* Ip, S32 Port, const char* Pass = "");
	void Close();

public:
	CRedisConn* Lock();
	bool Unlock(CRedisConn *);

protected:
	std::list<CRedisConn *>			m_pFreeList;                //空队列
	CMyCriticalSection            m_csFreeList;               //空队列唯一使用

	char m_Ip[32];
	S32  m_Port;
	int							m_nConnCount;				//连接数
};

class CRedisConn
{
	friend CRedisConnPool;
public:
	CRedisConn(CRedisConnPool *);
	~CRedisConn();
	void Detach();
	std::string Command(std::string cmd);
	void Command(std::string cmd, std::vector<std::string>& VecCmd);
	template<class T>
	int get1(const std::string &Key, T Val);
	template<class T>
	void set1(const std::string &Key, T Val, int Len);
	template<class T>
	void set2(const std::vector<std::string> &vtKey, const std::vector<T> vtVal, const std::vector<int> &vtLen);
private:
	void SetAddress(const char* Ip, U16 Port);
	void TimeProcess(U32 bExit);
	bool Connect(const char* Ip, U16 Port, U32 timeout);
	void DisConnect();
	std::string ParseRedis(redisReply *pReply);
	void		ParseRedis(redisReply *pReply, std::vector<std::string>& CmdVec);

private:
	SimpleTimer<30>				m_Timer;			//30秒检测断线
	CRedisConnPool*				m_pPool;            //绑定的连接池
	redisContext* m_pHandle;
	char m_Ip[32];
	U16  m_Port;
};

class CRedisDB
{
public:
	CRedisDB(CRedisConnPool* db);

	virtual ~CRedisDB();
	void	 AttachConn(CRedisConn* conn);
	CRedisConn* GetConn();
	std::string Command(std::string cmd);
	void Command(std::string cmd, std::vector<std::string>& VecCmd);
protected:
	bool	 m_isNeedDetach;
	CRedisConn* mRedisConn;	//注:一个数据库连接当次只能处理一个数据操作,不能在当次数据操作的
};

struct stNode
{
	std::string type;
	std::string name;
	std::string sqlName;
	int offsize;
	int arraysize;

	stNode* pNext;
	stNode() : type(""), name(""), offsize(0), arraysize(0), sqlName("")
	{
	}
};

class IRedisUnit;
class CRedisUnitMgr 
{
	typedef std::unordered_map<std::string, IRedisUnit*> REDISUNIT_MAP;
public:
	CRedisUnitMgr();
	~CRedisUnitMgr();
	static  CRedisUnitMgr* Instance();
	void RegisterUnit(std::string, IRedisUnit *);
	IRedisUnit* GetUnit(std::string);
private:
	REDISUNIT_MAP m_RedisUnitMap;
};

#define REDISUNITMGR CRedisUnitMgr::Instance()

class IRedisUnit
{
public:
	IRedisUnit();
	~IRedisUnit();

	virtual void  RegisterUnit(char* Name, const char* type, int offsize, int arraysize, bool bPrimiKey = false) = 0;
};

template <class T>
class CRedisUnit : public IRedisUnit
{
public:
	CRedisUnit() : pBaseNode(NULL), m_NodeSize(0), m_Class(""), pKeyNode(NULL)
	{
		CRedisUnitMgr::Instance()->RegisterUnit(typeid(T).name(), this);
	}

	~CRedisUnit()
	{
		while (pBaseNode)
		{
			stNode* pNode = pBaseNode;
			pBaseNode = pBaseNode->pNext;
			delete pNode;
		}

		while (pKeyNode)
		{
			stNode* pNode = pBaseNode;
			pKeyNode = pKeyNode->pNext;
			delete pNode;
		}
	}

	void SetName(std::string Name)
	{
		m_Class = Name;
	}

	void DoAssert(stNode* pNode)
	{
		if (pNode->type != "int" && pNode->type != "signed int" && pNode->type != "unsigned int" && \
			pNode->type != "short" && pNode->type != "signed short" && pNode->type != "unsigned short" && \
			pNode->type != "long" && pNode->type != "signed long" && pNode->type != "unsigned long" && \
			pNode->type != "_int64" && pNode->type != "signed _int64" && pNode->type != "unsigned _int64" &&\
			pNode->type != "float" && pNode->type != "double" && \
			pNode->type != "char" && pNode->type != "signed char" && pNode->type != "unsigned char" && \
			pNode->type != "class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >" && \
			pNode->type.find("int [") == std::string::npos && pNode->type.find("unsigned int [") == std::string::npos && \
			pNode->type.find("signed int [") == std::string::npos &&\
			pNode->type.find("short [") == std::string::npos && pNode->type.find("unsigned short [") == std::string::npos && \
			pNode->type.find("signed short [") == std::string::npos && \
			pNode->type.find("long [") == std::string::npos && pNode->type.find("unsigned long [") == std::string::npos && \
			pNode->type.find("signed long [") == std::string::npos && \
			pNode->type.find("_int64 [") == std::string::npos && pNode->type.find("unsigned _int64 [") == std::string::npos && \
			pNode->type.find("signed _int64 [") == std::string::npos &&\
			pNode->type.find("float [") == std::string::npos && pNode->type.find("double [") == std::string::npos &&\
			pNode->type.find("char [") == std::string::npos && pNode->type.find("unsigned char [") == std::string::npos && \
			pNode->type.find("signed char [") == std::string::npos && \
			pNode->type.find("class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> > [") == std::string::npos)
		{
			assert("redis cannot support");
		}
	}

	void RegisterUnit(char* Name, const char* type, int offsize, int arraysize, bool bPrimiKey = false)
	{
		std::string sName = Name;
		S32 iSize = sName.find_last_of(".");
		if (iSize != std::string::npos)
		{
			sName = sName.substr(iSize + 1);
		}

		if (!bPrimiKey)
		{
			stNode* pNode = new stNode();
			pNode->name = Name;
			pNode->sqlName = sName;
			pNode->type = type;
			pNode->offsize = offsize;
			pNode->arraysize = arraysize;
			pNode->pNext = pBaseNode;
			pBaseNode = pNode;
			++m_NodeSize;
			DoAssert(pNode);
		}
		else
		{
			stNode* pNode = new stNode();
			pNode->name = Name;
			pNode->sqlName = sName;
			pNode->type = type;
			pNode->offsize = offsize;
			pNode->arraysize = arraysize;
			pNode->pNext = pKeyNode;
			pKeyNode = pNode;
			++m_NodeSize;
			DoAssert(pNode);
		}
	}

	void _SetUnit(T * obj, stNode* pNode, std::ostringstream& Str)
	{
		DoAssert(pNode);
		if (pNode->type == "int" || pNode->type == "signed int" || pNode->type == "unsigned int")
			Str << *(int *)((uint64_t)obj + pNode->offsize);
		else if (pNode->type == "short" || pNode->type == "signed short" || pNode->type == "unsigned short")
			Str << *(short *)((uint64_t)obj + pNode->offsize);
		else if (pNode->type == "long" || pNode->type == "signed long" || pNode->type == "unsigned long")
			Str << *(long *)((uint64_t)obj + pNode->offsize);
		else if (pNode->type == "_int64" || pNode->type == "signed _int64" || pNode->type == "unsigned _int64")
			Str << *(_int64 *)((uint64_t)obj + pNode->offsize);
		else if (pNode->type == "float")
			Str << *(float *)((uint64_t)obj + pNode->offsize);
		else if (pNode->type == "double")
			Str << *(double *)((uint64_t)obj + pNode->offsize);
		else if (pNode->type == "char" || pNode->type == "signed char" || pNode->type == "unsigned char")
		{
			int val = *(char *)((uint64_t)obj + pNode->offsize);
			Str << val;
		}
		else if (pNode->type == "class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >")
			Str << *(std::string *)((uint64_t)obj + pNode->offsize);
		else if (pNode->type.find("int [") != std::string::npos || pNode->type.find("unsigned int [") != std::string::npos || \
			pNode->type.find("signed int [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(int);
			for (int i = 0; i < size; ++i)
			{
				Str << *(int *)((uint64_t)obj + pNode->offsize + i * sizeof(int));
				Str << ":";
			}
		}
		else if (pNode->type.find("short [") != std::string::npos || pNode->type.find("unsigned short [") != std::string::npos || \
			pNode->type.find("signed short [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(short);
			for (int i = 0; i < size; ++i)
			{
				Str << *(short *)((uint64_t)obj + pNode->offsize + i * sizeof(short));
				Str << ":";
			}
		}
		else if (pNode->type.find("long [") != std::string::npos || pNode->type.find("unsigned long [") != std::string::npos || \
			pNode->type.find("signed long [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(long);
			for (int i = 0; i < size; ++i)
			{
				Str << *(long *)((uint64_t)obj + pNode->offsize + i * sizeof(long));
				Str << ":";
			}
		}
		else if (pNode->type.find("_int64 [") != std::string::npos || pNode->type.find("unsigned _int64 [") != std::string::npos || \
			pNode->type.find("signed _int64 [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(_int64);
			for (int i = 0; i < size; ++i)
			{
				Str << *(_int64 *)((uint64_t)obj + pNode->offsize + i * sizeof(_int64));
				Str << ":";
			}
		}
		else if (pNode->type.find("float [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(float);
			for (int i = 0; i < size; ++i)
			{
				Str << *(float *)((uint64_t)obj + pNode->offsize + i * sizeof(float));
				Str << ":";
			}
		}
		else if (pNode->type.find("double [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(double);
			for (int i = 0; i < size; ++i)
			{
				Str << *(double *)((uint64_t)obj + pNode->offsize + i * sizeof(double));
				Str << ":";
			}
		}
		else if (pNode->type.find("class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> > [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(std::string);
			for (int i = 0; i < size; ++i)
			{
				Str << *(std::string *)((uint64_t)obj + pNode->offsize + i * sizeof(std::string));
				Str << ":";
			}
		}
		else
		{
			int size = pNode->arraysize / sizeof(char);
			for (int i = 0; i < size; ++i)
			{
				int val = *(char *)((uint64_t)obj + pNode->offsize + i * sizeof(char));
				Str << val;
				Str << ":";
			}
		}
	}

	void _GetUnit(T *obj, std::string& str, stNode* pNode)
	{
		DoAssert(pNode);
		std::string str1, str2;
		size_t iSize = 0;

		std::string sName = m_SpeStr;
		sName += pNode->name;
		sName += "=";
		iSize = str.find(sName);
		if (iSize == std::string::npos)
			goto NetTime;

		str1 = str.substr(iSize + 1).c_str();
		iSize = str1.find(m_SpeStr);
		if (iSize == std::string::npos)
			goto NetTime;

		str1 = str1.substr(strlen(pNode->name.c_str()) + 1, iSize - strlen(pNode->name.c_str()) - 1);
		char* pValue = (char*)str1.c_str();

		if (pNode->type == "int" || pNode->type == "signed int" || pNode->type == "unsigned int")
			*(int *)((uint64_t)obj + pNode->offsize) = atoi(pValue);
		else if (pNode->type == "short" || pNode->type == "signed short" || pNode->type == "unsigned short")
			*(short *)((uint64_t)obj + pNode->offsize) = atoi(pValue);
		else if (pNode->type == "long" || pNode->type == "signed long" || pNode->type == "unsigned long")
			*(long *)((uint64_t)obj + pNode->offsize) = atol(pValue);
		else if (pNode->type == "_int64" || pNode->type == "signed _int64" || pNode->type == "unsigned _int64")
			*(_int64 *)((uint64_t)obj + pNode->offsize) = _atoi64(pValue);
		else if (pNode->type == "float")
			*(float *)((uint64_t)obj + pNode->offsize) = atof(pValue);
		else if (pNode->type == "double")
			*(double *)((uint64_t)obj + pNode->offsize) = atof(pValue);
		else if (pNode->type == "char" || pNode->type == "signed char" || pNode->type == "unsigned char")
		{
			int val = atoi(pValue);
			*(char *)((uint64_t)obj + pNode->offsize) = val;
		}
		else if (pNode->type == "class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >")
			*((std::string  *)((uint64_t)obj + pNode->offsize)) = pValue;
		else if (pNode->type.find("int [") != std::string::npos || pNode->type.find("unsigned int [") != std::string::npos || \
			pNode->type.find("signed int [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(int);
			for (int i = 0; i < size; ++i)
			{
				iSize = str1.find(":");
				if (iSize == std::string::npos)
					goto NetTime;

				str2 = str1.substr(0, iSize).c_str();
				char* pValue = (char*)str2.c_str();
				str1 = str1.substr(iSize + 1);
				*(int *)((uint64_t)obj + pNode->offsize + i * sizeof(int)) = atoi(pValue);
			}
		}
		else if (pNode->type.find("short [") != std::string::npos || pNode->type.find("unsigned short [") != std::string::npos || \
			pNode->type.find("signed short [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(short);
			for (int i = 0; i < size; ++i)
			{
				iSize = str1.find(":");
				if (iSize == std::string::npos)
					goto NetTime;

				str2 = str1.substr(0, iSize).c_str();
				char* pValue = (char*)str2.c_str();
				str1 = str1.substr(iSize + 1);
				*(short *)((uint64_t)obj + pNode->offsize + i * sizeof(short)) = atoi(pValue);
			}
		}
		else if (pNode->type.find("long [") != std::string::npos || pNode->type.find("unsigned long [") != std::string::npos || \
			pNode->type.find("signed long [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(long);
			for (int i = 0; i < size; ++i)
			{
				iSize = str1.find(":");
				if (iSize == std::string::npos)
					goto NetTime;

				str2 = str1.substr(0, iSize).c_str();
				char* pValue = (char*)str2.c_str();
				str1 = str1.substr(iSize + 1);
				*(long *)((uint64_t)obj + pNode->offsize + i * sizeof(long)) = atol(pValue);
			}
		}
		else if (pNode->type.find("_int64 [") != std::string::npos || pNode->type.find("unsigned _int64 [") != std::string::npos || \
			pNode->type.find("signed _int64 [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(_int64);
			for (int i = 0; i < size; ++i)
			{
				iSize = str1.find(":");
				if (iSize == std::string::npos)
					goto NetTime;

				str2 = str1.substr(0, iSize).c_str();
				char* pValue = (char*)str2.c_str();
				str1 = str1.substr(iSize + 1);
				*(_int64 *)((uint64_t)obj + pNode->offsize + i * sizeof(_int64)) = _atoi64(pValue);
			}
		}
		else if (pNode->type.find("float [") != std::string::npos || pNode->type.find("unsigned float [") != std::string::npos || \
			pNode->type.find("signed float [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(float);
			for (int i = 0; i < size; ++i)
			{
				iSize = str1.find(":");
				if (iSize == std::string::npos)
					goto NetTime;

				str2 = str1.substr(0, iSize).c_str();
				char* pValue = (char*)str2.c_str();
				str1 = str1.substr(iSize + 1);
				*(float *)((uint64_t)obj + pNode->offsize + i * sizeof(float)) = _atoi64(pValue);
			}
		}
		else if (pNode->type.find("float [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(float);
			for (int i = 0; i < size; ++i)
			{
				iSize = str1.find(":");
				if (iSize == std::string::npos)
					goto NetTime;

				str2 = str1.substr(0, iSize).c_str();
				char* pValue = (char*)str2.c_str();
				str1 = str1.substr(iSize + 1);
				*(float *)((uint64_t)obj + pNode->offsize + i * sizeof(float)) = atof(pValue);
			}
		}
		else if (pNode->type.find("double [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(double);
			for (int i = 0; i < size; ++i)
			{
				iSize = str1.find(":");
				if (iSize == std::string::npos)
					goto NetTime;

				str2 = str1.substr(0, iSize).c_str();
				char* pValue = (char*)str2.c_str();
				str1 = str1.substr(iSize + 1);
				*(double *)((uint64_t)obj + pNode->offsize + i * sizeof(double)) = atof(pValue);
			}
		}
		else if (pNode->type.find("class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> > [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(std::string);
			for (int i = 0; i < size; ++i)
			{
				iSize = str1.find(":");
				if (iSize == std::string::npos)
					goto NetTime;

				str2 = str1.substr(0, iSize).c_str();
				char* pValue = (char*)str2.c_str();
				str1 = str1.substr(iSize + 1);
				*(std::string *)((uint64_t)obj + pNode->offsize + i * sizeof(std::string)) = (pValue);
			}
		}
		else
		{
			int size = pNode->arraysize / sizeof(char);
			for (int i = 0; i < size; ++i)
			{
				iSize = str1.find(":");
				if (iSize == std::string::npos)
					goto NetTime;

				str2 = str1.substr(0, iSize).c_str();
				char* pValue = (char*)str2.c_str();
				str1 = str1.substr(iSize + 1);
				int val = atoi(pValue);
				*(char *)((uint64_t)obj + pNode->offsize + i * sizeof(char)) = (char)val;
			}
		}
		//dStrcpy((char *)((uint64_t)obj + pNode->offsize), strlen(pValue), pValue);

		return;
	NetTime:
		g_Log.WriteError("redis get class[%s], optye[%s] error", m_Class.c_str(), pNode->name.c_str());
	}

	void _GetSql(T * obj, stNode* pNode, std::ostringstream& Str, std::string Spre = ", ")
	{
		if (pNode->type == "int" || pNode->type == "signed int" || pNode->type == "unsigned int")
		{
			Str << pNode->sqlName
				<< "="
				<< "'"
				<< *(int *)((uint64_t)obj + pNode->offsize)
				<< "'";
			if (pNode->pNext != NULL)
				Str << Spre;
		}
		else if (pNode->type == "short" || pNode->type == "signed short" || pNode->type == "unsigned short")
		{
			Str << pNode->sqlName
				<< "="
				<< "'"
				<< *(short *)((uint64_t)obj + pNode->offsize)
				<< "'";
			if (pNode->pNext != NULL)
				Str << Spre;
		}
		else if (pNode->type == "long" || pNode->type == "signed long" || pNode->type == "unsigned long")
		{
			Str << pNode->sqlName
				<< "="
				<< "'"
				<< *(long *)((uint64_t)obj + pNode->offsize)
				<< "'";
			if (pNode->pNext != NULL)
				Str << Spre;
		}
		else if (pNode->type == "_int64" || pNode->type == "signed _int64" || pNode->type == "unsigned _int64")
		{
			Str << pNode->sqlName
				<< "="
				<< "'"
				<< *(_int64 *)((uint64_t)obj + pNode->offsize)
				<< "'";
			if (pNode->pNext != NULL)
				Str << Spre;
		}
		else if (pNode->type == "float")
		{
			Str << pNode->sqlName
				<< "="
				<< "'"
				<< *(float *)((uint64_t)obj + pNode->offsize)
				<< "'";
			if (pNode->pNext != NULL)
				Str << Spre;
		}
		else if (pNode->type == "double")
		{
			Str << pNode->sqlName
				<< "="
				<< "'"
				<< *(double *)((uint64_t)obj + pNode->offsize)
				<< "'";
			if (pNode->pNext != NULL)
				Str << Spre;
		}
		else if (pNode->type == "char" || pNode->type == "signed char" || pNode->type == "unsigned char")
		{
			int val = *(char *)((uint64_t)obj + pNode->offsize);
			Str << pNode->sqlName
				<< "="
				<< "'"
				<< val
				<< "'";
			if (pNode->pNext != NULL)
				Str << Spre;
		}
		else if (pNode->type == "class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >")
		{
			Str << pNode->sqlName
				<< "="
				<< "'"
				<< *(std::string *)((uint64_t)obj + pNode->offsize)
				<< "'";
			if (pNode->pNext != NULL)
				Str << Spre;
		}
		else if (pNode->type.find("int [") != std::string::npos || pNode->type.find("unsigned int [") != std::string::npos || \
			pNode->type.find("signed int [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(int);
			for (int i = 0; i < size; ++i)
			{
				Str << pNode->sqlName
					<< i
					<< "="
					<< "'"
					<< *(int *)((uint64_t)obj + pNode->offsize + i * sizeof(int))
					<< "'";
				if (i != size - 1 || pNode->pNext != NULL)
					Str << Spre;
			}
		}
		else if (pNode->type.find("short [") != std::string::npos || pNode->type.find("unsigned short [") != std::string::npos || \
			pNode->type.find("signed short [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(short);
			for (int i = 0; i < size; ++i)
			{
				Str << pNode->sqlName
					<< i
					<< "="
					<< "'"
					<< *(short *)((uint64_t)obj + pNode->offsize + i * sizeof(short))
					<< "'";
				if (i != size - 1 || pNode->pNext != NULL)
					Str << Spre;
			}
		}
		else if (pNode->type.find("long [") != std::string::npos || pNode->type.find("unsigned long [") != std::string::npos || \
			pNode->type.find("signed long [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(long);
			for (int i = 0; i < size; ++i)
			{
				Str << pNode->sqlName
					<< i
					<< "="
					<< "'"
					<< *(long *)((uint64_t)obj + pNode->offsize + i * sizeof(long))
					<< "'";
				if (i != size - 1 || pNode->pNext != NULL)
					Str << Spre;
			}
		}
		else if (pNode->type.find("_int64 [") != std::string::npos || pNode->type.find("unsigned _int64 [") != std::string::npos || \
			pNode->type.find("signed _int64 [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(_int64);
			for (int i = 0; i < size; ++i)
			{
				Str << pNode->sqlName
					<< i
					<< "="
					<< "'"
					<< *(_int64 *)((uint64_t)obj + pNode->offsize + i * sizeof(_int64))
					<< "'";
				if (i != size - 1 || pNode->pNext != NULL)
					Str << Spre;
			}
		}
		else if (pNode->type.find("float [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(float);
			for (int i = 0; i < size; ++i)
			{
				Str << pNode->sqlName
					<< i
					<< "="
					<< "'"
					<< *(float *)((uint64_t)obj + pNode->offsize + i * sizeof(float))
					<< "'";
				if (i != size - 1 || pNode->pNext != NULL)
					Str << Spre;
			}
		}
		else if (pNode->type.find("double [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(double);
			for (int i = 0; i < size; ++i)
			{
				Str << pNode->sqlName
					<< i
					<< "="
					<< "'"
					<< *(double *)((uint64_t)obj + pNode->offsize + i * sizeof(double))
					<< "'";
				if (i != size - 1 || pNode->pNext != NULL)
					Str << Spre;
			}
		}
		else if (pNode->type.find("class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> > [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(std::string);
			for (int i = 0; i < size; ++i)
			{
				Str << pNode->sqlName
					<< i
					<< "="
					<< "'"
					<< *(std::string *)((uint64_t)obj + pNode->offsize + i * sizeof(std::string))
					<< "'";
				if (i != size - 1 || pNode->pNext != NULL)
					Str << Spre;
			}
		}
		else
		{
			Str << pNode->sqlName
				<< "="
				<< "'"
				<< (char *)((uint64_t)obj + pNode->offsize)
				<< "'";
			if (pNode->pNext != NULL)
				Str << Spre;
			/*int size = pNode->arraysize / sizeof(char);
			for (int i = 0; i < size; ++i)
			{
			int val = *(char *)((uint64_t)obj + pNode->offsize + i * sizeof(char));
			Str << pNode->sqlName
			<< i
			<< "="
			<< "'"
			<< val
			<< "'";
			if (i != size - 1 || pNode->pNext != NULL)
			Str << Spre;
			}*/
		}
	}

	void _GetSqlVal(T * obj, stNode* pNode, std::ostringstream& Str, std::string Spre = ", ")
	{
		if (pNode->type == "int" || pNode->type == "signed int" || pNode->type == "unsigned int")
		{

			Str << "'"
				<< *(int *)((uint64_t)obj + pNode->offsize)
				<< "'";
			if (pNode->pNext != NULL)
				Str << Spre;
		}
		else if (pNode->type == "short" || pNode->type == "signed short" || pNode->type == "unsigned short")
		{
			Str << "'"
				<< *(short *)((uint64_t)obj + pNode->offsize)
				<< "'";
			if (pNode->pNext != NULL)
				Str << Spre;
		}
		else if (pNode->type == "long" || pNode->type == "signed long" || pNode->type == "unsigned long")
		{
			Str << "'"
				<< *(long *)((uint64_t)obj + pNode->offsize)
				<< "'";
			if (pNode->pNext != NULL)
				Str << Spre;
		}
		else if (pNode->type == "_int64" || pNode->type == "signed _int64" || pNode->type == "unsigned _int64")
		{
			Str << "'"
				<< *(_int64 *)((uint64_t)obj + pNode->offsize)
				<< "'";
			if (pNode->pNext != NULL)
				Str << Spre;
		}
		else if (pNode->type == "float")
		{
			Str << "'"
				<< *(float *)((uint64_t)obj + pNode->offsize)
				<< "'";
			if (pNode->pNext != NULL)
				Str << Spre;
		}
		else if (pNode->type == "double")
		{
			Str << "'"
				<< *(double *)((uint64_t)obj + pNode->offsize)
				<< "'";
			if (pNode->pNext != NULL)
				Str << Spre;
		}
		else if (pNode->type == "char" || pNode->type == "signed char" || pNode->type == "unsigned char")
		{
			int val = *(char *)((uint64_t)obj + pNode->offsize);
			Str << "'"
				<< val
				<< "'";
			if (pNode->pNext != NULL)
				Str << Spre;
		}
		else if (pNode->type == "class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >")
		{
			Str << "'"
				<< *(std::string *)((uint64_t)obj + pNode->offsize)
				<< "'";
			if (pNode->pNext != NULL)
				Str << Spre;
		}
		else if (pNode->type.find("int [") != std::string::npos || pNode->type.find("unsigned int [") != std::string::npos || \
			pNode->type.find("signed int [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(int);
			for (int i = 0; i < size; ++i)
			{
				Str << "'"
					<< *(int *)((uint64_t)obj + pNode->offsize + i * sizeof(int))
					<< "'";
				if (i != size - 1 || pNode->pNext != NULL)
					Str << Spre;
			}
		}
		else if (pNode->type.find("short [") != std::string::npos || pNode->type.find("unsigned short [") != std::string::npos || \
			pNode->type.find("signed short [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(short);
			for (int i = 0; i < size; ++i)
			{
				Str << "'"
					<< *(short *)((uint64_t)obj + pNode->offsize + i * sizeof(short))
					<< "'";
				if (i != size - 1 || pNode->pNext != NULL)
					Str << Spre;
			}
		}
		else if (pNode->type.find("long [") != std::string::npos || pNode->type.find("unsigned long [") != std::string::npos || \
			pNode->type.find("signed long [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(long);
			for (int i = 0; i < size; ++i)
			{
				Str << "'"
					<< *(long *)((uint64_t)obj + pNode->offsize + i * sizeof(long))
					<< "'";
				if (i != size - 1 || pNode->pNext != NULL)
					Str << Spre;
			}
		}
		else if (pNode->type.find("_int64 [") != std::string::npos || pNode->type.find("unsigned _int64 [") != std::string::npos || \
			pNode->type.find("signed _int64 [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(_int64);
			for (int i = 0; i < size; ++i)
			{
				Str << "'"
					<< *(_int64 *)((uint64_t)obj + pNode->offsize + i * sizeof(_int64))
					<< "'";
				if (i != size - 1 || pNode->pNext != NULL)
					Str << Spre;
			}
		}
		else if (pNode->type.find("float [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(float);
			for (int i = 0; i < size; ++i)
			{
				Str << "'"
					<< *(float *)((uint64_t)obj + pNode->offsize + i * sizeof(float))
					<< "'";
				if (i != size - 1 || pNode->pNext != NULL)
					Str << Spre;
			}
		}
		else if (pNode->type.find("double [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(double);
			for (int i = 0; i < size; ++i)
			{
				Str << "'"
					<< *(double *)((uint64_t)obj + pNode->offsize + i * sizeof(double))
					<< "'";
				if (i != size - 1 || pNode->pNext != NULL)
					Str << Spre;
			}
		}
		else if (pNode->type.find("class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> > [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(std::string);
			for (int i = 0; i < size; ++i)
			{
				Str << "'"
					<< *(std::string *)((uint64_t)obj + pNode->offsize + i * sizeof(std::string))
					<< "'";
				if (i != size - 1 || pNode->pNext != NULL)
					Str << Spre;
			}
		}
		else
		{
			Str << "'"
				<< (char *)((uint64_t)obj + pNode->offsize)
				<< "'";
			if (pNode->pNext != NULL)
				Str << Spre;
			/*int size = pNode->arraysize / sizeof(char);
			for (int i = 0; i < size; ++i)
			{
			int val = *(char *)((uint64_t)obj + pNode->offsize + i * sizeof(char));
			Str << "'"
			<< val
			<< "'";
			if (i != size - 1 || pNode->pNext != NULL)
			Str << Spre;
			}*/
		}
	}

	void _GetSqlName(T * obj, stNode* pNode, std::ostringstream& Str)
	{
		if (pNode->type == "int" || pNode->type == "signed int" || pNode->type == "unsigned int")
		{
			Str << pNode->sqlName;
			if (pNode->pNext != NULL)
				Str << ", ";
		}
		else if (pNode->type == "short" || pNode->type == "signed short" || pNode->type == "unsigned short")
		{
			Str << pNode->sqlName;
			if (pNode->pNext != NULL)
				Str << ", ";
		}
		else if (pNode->type == "long" || pNode->type == "signed long" || pNode->type == "unsigned long")
		{
			Str << pNode->sqlName;
			if (pNode->pNext != NULL)
				Str << ", ";
		}
		else if (pNode->type == "_int64" || pNode->type == "signed _int64" || pNode->type == "unsigned _int64")
		{
			Str << pNode->sqlName;
			if (pNode->pNext != NULL)
				Str << ", ";
		}
		else if (pNode->type == "float")
		{
			Str << pNode->sqlName;
			if (pNode->pNext != NULL)
				Str << ", ";
		}
		else if (pNode->type == "double")
		{
			Str << pNode->sqlName;
			if (pNode->pNext != NULL)
				Str << ", ";
		}
		else if (pNode->type == "char" || pNode->type == "signed char" || pNode->type == "unsigned char")
		{
			Str << pNode->sqlName;
			if (pNode->pNext != NULL)
				Str << ", ";
		}
		else if (pNode->type == "class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >")
		{
			Str << pNode->sqlName;
			if (pNode->pNext != NULL)
				Str << ", ";
		}
		else if (pNode->type.find("int [") != std::string::npos || pNode->type.find("unsigned int [") != std::string::npos || \
			pNode->type.find("signed int [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(int);
			for (int i = 0; i < size; ++i)
			{
				Str << pNode->sqlName
					<< i;
				if (i != size - 1 || pNode->pNext != NULL)
					Str << ", ";
			}
		}
		else if (pNode->type.find("short [") != std::string::npos || pNode->type.find("unsigned short [") != std::string::npos || \
			pNode->type.find("signed short [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(short);
			for (int i = 0; i < size; ++i)
			{
				Str << pNode->sqlName
					<< i;
				if (i != size - 1 || pNode->pNext != NULL)
					Str << ", ";
			}
		}
		else if (pNode->type.find("long [") != std::string::npos || pNode->type.find("unsigned long [") != std::string::npos || \
			pNode->type.find("signed long [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(long);
			for (int i = 0; i < size; ++i)
			{
				Str << pNode->sqlName
					<< i;
				if (i != size - 1 || pNode->pNext != NULL)
					Str << ", ";
			}
		}
		else if (pNode->type.find("_int64 [") != std::string::npos || pNode->type.find("unsigned _int64 [") != std::string::npos || \
			pNode->type.find("signed _int64 [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(_int64);
			for (int i = 0; i < size; ++i)
			{
				Str << pNode->sqlName
					<< i;
				if (i != size - 1 || pNode->pNext != NULL)
					Str << ", ";
			}
		}
		else if (pNode->type.find("float [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(float);
			for (int i = 0; i < size; ++i)
			{
				Str << pNode->sqlName
					<< i;
				if (i != size - 1 || pNode->pNext != NULL)
					Str << ", ";
			}
		}
		else if (pNode->type.find("double [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(double);
			for (int i = 0; i < size; ++i)
			{
				Str << pNode->sqlName
					<< i;
				if (i != size - 1 || pNode->pNext != NULL)
					Str << ", ";
			}
		}
		else if (pNode->type.find("class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> > [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(std::string);
			for (int i = 0; i < size; ++i)
			{
				Str << pNode->sqlName
					<< i;
				if (i != size - 1 || pNode->pNext != NULL)
					Str << ", ";
			}
		}
		else
		{
			int size = pNode->arraysize / sizeof(char);
			for (int i = 0; i < size; ++i)
			{
				Str << pNode->sqlName
					<< i;
				if (i != size - 1 || pNode->pNext != NULL)
					Str << ", ";
			}
		}
	}

	std::string  GetClass()
	{
		return m_Class;
	}

	std::string  GetKey(T *obj)
	{
		std::string key;
		std::ostringstream Str;
		stNode* pKNode = pKeyNode;
		Str << m_Class
			<< "_";

		while (pKNode)
		{
			//Str << pKNode->name;
			//Str << "_";
			_SetUnit(obj, pKNode, Str);
			Str << "_";
			pKNode = pKNode->pNext;
		}
		Str << " ";
		key = Str.str();
		return key;
	}

	template<class U>
	std::string  GetHashKey(T *obj, U field)
	{
		std::string key;
		std::ostringstream Str;
		stNode* pKNode = pKeyNode;
		Str << m_Class
			<< "_"
			<< field;

		Str << " "
			<< GetKey(obj)
			<< " ";

		key = Str.str();
		return key;
	}

	std::string SetUnit(T *obj)
	{
		std::ostringstream Str;
		std::string cmd;
		stNode* pNode = pBaseNode;
		stNode* pKNode = pKeyNode;
		//Str << GetKey(obj);
		Str << m_SpeStr;
		while (pNode)
		{
			Str << pNode->name;
			Str << "=";
			_SetUnit(obj, pNode, Str);
			Str << m_SpeStr;

			pNode = pNode->pNext;
		}

		while (pKNode)
		{
			Str << pKNode->name;
			Str << "=";
			_SetUnit(obj, pKNode, Str);
			Str << m_SpeStr;

			pKNode = pKNode->pNext;
		}
		cmd = Str.str();
		return cmd;
	}

	void GetUnit(T *obj, std::string cmd)
	{
		stNode* pNode = pBaseNode;
		stNode* pKey = pKeyNode;
		while (pNode)
		{
			_GetUnit(obj, cmd, pNode);
			pNode = pNode->pNext;
		}

		while (pKey)
		{
			_GetUnit(obj, cmd, pKey);
			pKey = pKey->pNext;
		}
	}

	int GetSize()
	{
		return m_NodeSize;
	}

	std::string UpdateSql(T *obj)
	{
		std::ostringstream Str;
		std::string cmd;
		stNode* pNode = pBaseNode;
		stNode* pKNode = pKeyNode;
		Str << "UPDATE "
			<< m_Class
			<< " SET ";

		while (pNode)
		{
			_GetSql(obj, pNode, Str);
			Str << " ";

			pNode = pNode->pNext;
		}

		Str << " WHERE ";
		while (pKNode)
		{
			Str << pKNode->name;
			Str << "=";
			_GetSqlVal(obj, pKNode, Str, "");
			if (pKNode->pNext)
			{
				Str << " "
					<< "AND ";
			}

			pKNode = pKNode->pNext;
		}
		cmd = Str.str();
		return cmd;
	}

	std::string InsertSql(T *obj)
	{
		std::ostringstream Str;
		std::string cmd;
		stNode* pNode = pBaseNode;
		stNode* pKNode = pKeyNode;
		Str << "INSERT INTO "
			<< m_Class
			<< "(";

		while (pNode)
		{
			_GetSqlName(obj, pNode, Str);
			pNode = pNode->pNext;
		}

		Str << ",";

		while (pKNode)
		{
			_GetSqlName(obj, pKNode, Str);
			pKNode = pKNode->pNext;
			if (!pKNode)
				Str << ")";
		}

		Str << "VALUES"
			<< "(";

		pNode = pBaseNode;
		pKNode = pKeyNode;
		while (pNode)
		{
			_GetSqlVal(obj, pNode, Str);
			pNode = pNode->pNext;
		}

		Str << ",";

		while (pKNode)
		{
			_GetSqlVal(obj, pKNode, Str);
			pKNode = pKNode->pNext;
			if (!pKNode)
				Str << ")";
		}

		cmd = Str.str();
		return cmd;
	}

	std::string LoadSql(T *obj)
	{
		std::ostringstream Str;
		std::string cmd;
		stNode* pNode = pBaseNode;
		stNode* pKNode = pKeyNode;
		Str << "SELECT "
			<< m_Class
			<< " ";

		while (pNode)
		{
			_GetSqlName(obj, pNode, Str);
			pNode = pNode->pNext;
		}

		Str << " WHERE ";
		while (pKNode)
		{
			_GetSql(obj, pKNode, Str);
			pKNode = pKNode->pNext;
		}

		cmd = Str.str();
		return cmd;
	}

	std::string DelSql(T *obj)
	{
		std::ostringstream Str;
		std::string cmd;
		stNode* pNode = pBaseNode;
		stNode* pKNode = pKeyNode;
		Str << "DELETE "
			<< m_Class;
		Str << " WHERE ";
		while (pKNode)
		{
			_GetSql(obj, pKNode, Str, "");
			if (pKNode->pNext)
			{
				Str << " "
					<< "AND ";
			}
			pKNode = pKNode->pNext;
		}

		cmd = Str.str();
		return cmd;
	}

private:
	std::string m_Class;
	stNode* pBaseNode;
	stNode* pKeyNode;//主键
	int m_NodeSize;
};

template <class T>
class  IRedisCmd
{
public:
	typedef typename std::tr1::shared_ptr<T> TRef;
public:
	IRedisCmd(CRedisConn* pConn): m_pRedisConn(pConn)
	{
		m_pRedisUnit = dynamic_cast<CRedisUnit<T> *>(REDISUNITMGR->GetUnit(typeid(T).name()));
		assert(m_pRedisUnit != NULL);
		assert(m_pRedisConn != NULL);
	}

	IRedisCmd(CRedisDB* pDB) : m_pRedisConn(pDB->GetConn())
	{
		m_pRedisUnit = dynamic_cast<CRedisUnit<T> *>(REDISUNITMGR->GetUnit(typeid(T).name()));
		assert(m_pRedisUnit != NULL);
		assert(m_pRedisConn != NULL);
	}

	virtual ~IRedisCmd()
	{
	}
	inline std::string UpdateSql(T* obj){
		std::string cmd = m_pRedisUnit->UpdateSql(obj);
		return cmd;
	}
	inline std::string UpdateSql(TRef obj)	{	
		return UpdateSql(obj.get()); 
	}
	inline std::string InsertSql(T* obj){
		std::string cmd = m_pRedisUnit->InsertSql(obj);
		return cmd;
	}
	inline std::string InsertSql(TRef obj)	{	
		return InsertSql(obj.get());
	}
	inline std::string DeleteSql(T* obj){
		std::string cmd = m_pRedisUnit->DelSql(obj);
		return cmd;
	}
	inline std::string DeleteSql(TRef obj){	
		return DeleteSql(obj.get());
	}
	virtual void GetVal(T* obj, std::string str) = 0;
	virtual void GetVal(TRef obj, std::string str) = 0;
public:
	CRedisUnit<T>* m_pRedisUnit;
	CRedisConn* m_pRedisConn;
};

template <class T>
class  BRedisCmd : public IRedisCmd<T>
{
public:
	BRedisCmd(CRedisConn* pConn):IRedisCmd(pConn) {};
	BRedisCmd(CRedisDB* pDB) :IRedisCmd(pDB) {};
	~BRedisCmd() {};

	void GetVal(T* obj, std::string str){
		m_pRedisUnit->GetUnit(obj, str);
	}

	void GetVal(TRef obj, std::string str){
		GetVal(obj.get(), str);
	}
};

template <class  T>
class HashCmd : public BRedisCmd<T>
{
public:
	explicit HashCmd(CRedisConn* pConn) :BRedisCmd(pConn) {};
	explicit HashCmd(CRedisDB* pDB) :BRedisCmd(pDB) {};
	~HashCmd() {};

	template<class U>
	void hget(T* obj, U filed){
		std::string str = "hget ";
		str += m_pRedisUnit->GetHashKey(obj, filed);
		str = m_pRedisConn->Command(str);
		GetVal(obj, str);
	}
	template<class U>
	void hget(TRef obj, U filed) {
		hget(obj.get(), filed);
	}
	template<class U>
	void hset(T* obj, U filed){
		std::string cmd = m_pRedisUnit->SetUnit(obj);
		std::string str = "hset ";
		str += m_pRedisUnit->GetHashKey(obj, filed);
		str += cmd;
		m_pRedisConn->Command(str.c_str());
	}
	template<class U>
	void hset(TRef obj, U filed){
		hset(obj.get(), filed);
	}
	template<class U>
	void hdel(T* obj, U filed){
		std::string str = "hdel ";
		str += m_pRedisUnit->GetHashKey(obj, filed);
		str = m_pRedisConn->Command(str);
	}
	template<class U>
	void hdel(TRef obj, U filed){
		hdel(obj.get(), filed);
	}
	template<class U>
	std::string hlen(U field){
		std::ostringstream str;
		str << "hlen"
			<< " "
			<< m_pRedisUnit->GetClass()
			<< "_"
			<< field;
		return m_pRedisConn->Command(str.str());
	}
	template<class U>
	void hvals(U field, std::vector<std::string>& CmdVec) {
		std::ostringstream str;
		str << "hvals"
			<< " "
			<< m_pRedisUnit->GetClass()
			<< "_"
			<< field;
		m_pRedisConn->Command(str.str(), CmdVec);
	}
};

template <class  T>
class KeyCmd : public BRedisCmd<T>
{
public:
	explicit KeyCmd(CRedisConn* pConn) :BRedisCmd(pConn) {}
	explicit KeyCmd(CRedisDB* pDB) :BRedisCmd(pDB) {}
	~KeyCmd() {};

	void get(T* obj, std::string key) {
		std::string str = "get ";
		str += key;
		str = m_pRedisConn->Command(str);
		m_pRedisUnit->GetUnit(obj, str);
	}
	void get(TRef obj, std::string key){
		get(obj.get(), key);
	}
	void set(T* obj){
		std::string cmd = m_pRedisUnit->SetUnit(obj);
		std::string str = "set ";
		str += m_pRedisUnit->GetKey(obj);
		str += cmd;
		m_pRedisConn->Command(str.c_str());
	}
	void set(TRef obj){
		set(obj.get());
	}
	void del(T* obj){
		std::string str = "del ";
		str += m_pRedisUnit->GetKey(obj);
		str = m_pRedisConn->Command(str);
	}
	void del(TRef obj){
		del(obj.get());
	}
	void keys(U32 nPlayerID, std::vector<std::string>& VecCMD){
		std::ostringstream str;
		str << "keys "
			<< m_pRedisUnit->GetClass()
			<< "_*"
			<< nPlayerID
			<< "*";
		m_pRedisConn->Command(str.str(), VecCMD);
	}
};

#define DECLARE_REDIS_UNIT(cls)	static CRedisUnit<cls> s_Redis___##cls;
#define GET_REDIS_UNIT(cls)		s_Redis___##cls
#define REGISTER_REDIS_UNIT(cls, x) s_Redis___##cls.RegisterUnit(#x, typeid(cls::x).name(), Offset(x,cls), sizeof(cls::x));
#define REGISTER_REDIS_UNITKEY(cls, x) s_Redis___##cls.RegisterUnit(#x, typeid(cls::x).name(), Offset(x,cls), sizeof(cls::x), true);

#endif