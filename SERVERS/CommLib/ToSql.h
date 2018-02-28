#ifndef _TO_SQL_H_
#define _TO_SQL_H_
#include "BASE/types.h"
#include "BASE/Log.h"
#include <memory>
#include <sstream>
#include <assert.h>
#include <unordered_map>
#include <time.h>

static std::string m_SqlSpeStr = "#";
struct stSqlNode
{
	std::string type;
	std::string name;
	std::string sqlName;
	bool datetimeFlag;
	int offsize;
	int arraysize;

	stSqlNode* pNext;
	stSqlNode* pPrev;
	stSqlNode() : type(""), name(""), offsize(0), arraysize(0), sqlName(""), datetimeFlag(false), pNext(NULL), pPrev(NULL)
	{
	}
};

inline std::string GetDBTimeString(unsigned long t)
{
	struct tm lt;
	time_t long_time = t;
	localtime_s(&lt, &long_time);
	char str[32] = "";
	memset(str, 0, sizeof(str));
	sprintf_s(str, sizeof(str), "%d-%d-%d %d:%d:%d",
		lt.tm_year + 1900,
		lt.tm_mon + 1,
		lt.tm_mday,
		lt.tm_hour,
		lt.tm_min,
		lt.tm_sec);
	return str;
}

class ISqlUnit;
class CSqlUnitMgr 
{
	typedef std::unordered_map<std::string, ISqlUnit*> SQLUNIT_MAP;
public:
	CSqlUnitMgr();
	~CSqlUnitMgr();
	static  CSqlUnitMgr* Instance();
	void RegisterUnit(std::string, ISqlUnit *);
	ISqlUnit* GetUnit(std::string);
private:
	SQLUNIT_MAP m_SqlUnitMap;
};

#define SQLUNITMGR CSqlUnitMgr::Instance()

class ISqlUnit
{
public:
	ISqlUnit();
	~ISqlUnit();

	virtual void  RegisterUnit(char* Name, const char* type, int offsize, int arraysize, bool bPrimiKey = false, bool bDateTime = false) = 0;
};

template <class T>
class CSqlUnit : public ISqlUnit
{
	typedef typename std::tr1::shared_ptr<T> TRef;
public:
	CSqlUnit() : pBaseNode(NULL), m_NodeSize(0), m_Class(""), pKeyNode(NULL), pEndNode(NULL), pEndKeyNode(NULL)
	{
		CSqlUnitMgr::Instance()->RegisterUnit(typeid(T).name(), this);
	}

	~CSqlUnit()
	{
		while (pBaseNode)
		{
			stSqlNode* pNode = pBaseNode;
			pBaseNode = pBaseNode->pNext;
			delete pNode;
		}

		while (pKeyNode)
		{
			stSqlNode* pNode = pBaseNode;
			pKeyNode = pKeyNode->pNext;
			delete pNode;
		}
	}

	void SetName(std::string Name)
	{
		m_Class = Name;
	}

public:
	void RegisterUnit(char* Name, const char* type, int offsize, int arraysize, bool bPrimiKey = false, bool bDateTime = false)
	{
		std::string sName = Name;
		S32 iSize = sName.find_last_of(".");
		if (iSize != std::string::npos){
			sName = sName.substr(iSize + 1);
		}

		iSize = sName.find_last_of("[");
		if (iSize != std::string::npos){
			std::string sTemp = sName.substr(0, iSize);
			sName = sName.substr(iSize + 1);
			sName = sTemp + sName;
		}

		iSize = sName.find_last_of("]");
		if (iSize != std::string::npos) {
			sName = sName.substr(0, iSize);
		}

		if (!bPrimiKey)
		{
			stSqlNode* pNode = new stSqlNode();
			pNode->name = Name;
			pNode->sqlName = sName;
			pNode->type = type;
			pNode->offsize = offsize;
			pNode->arraysize = arraysize;
			pNode->datetimeFlag = bDateTime;
			if (!pBaseNode) {
				pBaseNode = pEndNode = pNode;
			}
			else {
				pNode->pPrev = pEndNode;
				pEndNode->pNext = pNode;
				pEndNode = pNode;
			}
			++m_NodeSize;
			DoAssert(pNode);
		}
		else
		{
			stSqlNode* pNode = new stSqlNode();
			pNode->name = Name;
			pNode->sqlName = sName;
			pNode->type = type;
			pNode->offsize = offsize;
			pNode->arraysize = arraysize;
			if (!pKeyNode) {
				pKeyNode = pEndKeyNode = pNode;
			}
			else {
				pNode->pPrev = pEndKeyNode;
				pEndKeyNode->pNext = pNode;
				pEndKeyNode = pNode;
			}
		
			++m_NodeSize;
			DoAssert(pNode);
		}
	}

	/*void _SetUnit(T * obj, stSqlNode* pNode, std::ostringstream& Str)
	{
		DoAssert(pNode);
		if (pNode->type == "int" || pNode->type == "signed int" || pNode->type == "unsigned int") 
			Str << *(int *)((uint64_t)obj + pNode->offsize);
		else if (pNode->type == "short" || pNode->type == "signed short" || pNode->type == "unsigned short")
			Str << *(short *)((uint64_t)obj + pNode->offsize);
		else if (pNode->type == "long" || pNode->type == "signed long" || pNode->type == "unsigned long") 
			Str << *(long *)((uint64_t)obj + pNode->offsize);
		else if (pNode->type == "__int64" || pNode->type == "signed __int64" || pNode->type == "unsigned __int64")
			Str << *(__int64 *)((uint64_t)obj + pNode->offsize);
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
		else if (pNode->type.find("__int64 [") != std::string::npos || pNode->type.find("unsigned __int64 [") != std::string::npos || \
			pNode->type.find("signed __int64 [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(__int64);
			for (int i = 0; i < size; ++i)
			{
				Str << *(__int64 *)((uint64_t)obj + pNode->offsize + i * sizeof(__int64));
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

	void _GetUnit(T *obj, std::string& str, stSqlNode* pNode)
	{
		DoAssert(pNode);
		std::string str1, str2;
		size_t iSize = 0;

		std::string sName = m_SqlSpeStr;
		sName += pNode->name;
		sName += "=";
		iSize = str.find(sName);
		if (iSize == std::string::npos)
			goto NetTime;

		str1 = str.substr(iSize + 1).c_str();
		iSize = str1.find(m_SqlSpeStr);
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
		else if (pNode->type == "__int64" || pNode->type == "signed __int64" || pNode->type == "unsigned __int64")
			*(__int64 *)((uint64_t)obj + pNode->offsize) = _atoi64(pValue);
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
		else if (pNode->type.find("__int64 [") != std::string::npos || pNode->type.find("unsigned __int64 [") != std::string::npos || \
			pNode->type.find("signed __int64 [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(__int64);
			for (int i = 0; i < size; ++i)
			{
				iSize = str1.find(":");
				if (iSize == std::string::npos)
					goto NetTime;

				str2 = str1.substr(0, iSize).c_str();
				char* pValue = (char*)str2.c_str();
				str1 = str1.substr(iSize + 1);
				*(__int64 *)((uint64_t)obj + pNode->offsize + i * sizeof(__int64)) = _atoi64(pValue);
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
		g_Log.WriteError("sql get class[%s], optye[%s] error", m_Class.c_str(), pNode->name.c_str());
	}*/
private:
	void DoAssert(stSqlNode* pNode)
	{
		if (pNode->type != "int" && pNode->type != "signed int" && pNode->type != "unsigned int" && \
			pNode->type != "short" && pNode->type != "signed short" && pNode->type != "unsigned short" && \
			pNode->type != "long" && pNode->type != "signed long" && pNode->type != "unsigned long" && \
			pNode->type != "__int64" && pNode->type != "signed __int64" && pNode->type != "unsigned __int64" &&\
			pNode->type != "float" && pNode->type != "double" && \
			pNode->type != "char" && pNode->type != "signed char" && pNode->type != "unsigned char" && \
			pNode->type != "class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >" && \
			pNode->type.find("int [") == std::string::npos && pNode->type.find("unsigned int [") == std::string::npos && \
			pNode->type.find("signed int [") == std::string::npos &&\
			pNode->type.find("short [") == std::string::npos && pNode->type.find("unsigned short [") == std::string::npos && \
			pNode->type.find("signed short [") == std::string::npos && \
			pNode->type.find("long [") == std::string::npos && pNode->type.find("unsigned long [") == std::string::npos && \
			pNode->type.find("signed long [") == std::string::npos && \
			pNode->type.find("__int64 [") == std::string::npos && pNode->type.find("unsigned __int64 [") == std::string::npos && \
			pNode->type.find("signed __int64 [") == std::string::npos &&\
			pNode->type.find("float [") == std::string::npos && pNode->type.find("double [") == std::string::npos &&\
			pNode->type.find("char [") == std::string::npos && pNode->type.find("unsigned char [") == std::string::npos && \
			pNode->type.find("signed char [") == std::string::npos && \
			pNode->type.find("class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> > [") == std::string::npos)
		{
			assert("sql cannot support");
		}
	}

	void _GetSql(T * obj, stSqlNode* pNode, std::ostringstream& Str, std::string Spre = ", ")
	{
		if (pNode->type == "int" || pNode->type == "signed int" || pNode->type == "unsigned int")
		{
			Str << pNode->sqlName
				<< "="
				<< "'";

			if (pNode->datetimeFlag)
				Str << GetDBTimeString(*(int *)((uint64_t)obj + pNode->offsize));
			else
				Str << *(int *)((uint64_t)obj + pNode->offsize);

			Str << "'";
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
				<< "'";
			if (pNode->datetimeFlag)
				Str << GetDBTimeString(*(long *)((uint64_t)obj + pNode->offsize));
			else
				Str << *(long *)((uint64_t)obj + pNode->offsize);

			Str<< "'";
			if (pNode->pNext != NULL)
				Str << Spre;
		}
		else if (pNode->type == "__int64" || pNode->type == "signed __int64" || pNode->type == "unsigned __int64")
		{
			Str << pNode->sqlName
				<< "="
				<< "'"
				<< *(__int64 *)((uint64_t)obj + pNode->offsize)
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
					<< "'";
				if (pNode->datetimeFlag)
					Str << GetDBTimeString(*(int *)((uint64_t)obj + pNode->offsize + i * sizeof(int)));
				else
					Str <<*(int *)((uint64_t)obj + pNode->offsize + i * sizeof(int));

				Str << "'";
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
					<< "'";
					if (pNode->datetimeFlag)
						Str << GetDBTimeString(*(long *)((uint64_t)obj + pNode->offsize + i * sizeof(long)));
					else
						Str << *(long *)((uint64_t)obj + pNode->offsize + i * sizeof(long));
				Str << "'";
				if (i != size - 1 || pNode->pNext != NULL)
					Str << Spre;
			}
		}
		else if (pNode->type.find("__int64 [") != std::string::npos || pNode->type.find("unsigned __int64 [") != std::string::npos || \
			pNode->type.find("signed __int64 [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(__int64);
			for (int i = 0; i < size; ++i)
			{
				Str << pNode->sqlName
					<< i
					<< "="
					<< "'"
					<< *(__int64 *)((uint64_t)obj + pNode->offsize + i * sizeof(__int64))
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
		}
	}

	void _GetSqlVal(T * obj, stSqlNode* pNode, std::ostringstream& Str, std::string Spre = ", ")
	{
		if (pNode->type == "int" || pNode->type == "signed int" || pNode->type == "unsigned int")
		{

			Str << "'";
			if (pNode->datetimeFlag)
				Str << GetDBTimeString(*(int *)((uint64_t)obj + pNode->offsize));
			else 
				Str << *(int *)((uint64_t)obj + pNode->offsize);
			Str << "'";
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
			Str << "'";
			if (pNode->datetimeFlag)
				Str << GetDBTimeString(*(long *)((uint64_t)obj + pNode->offsize));
			else
				Str << *(long *)((uint64_t)obj + pNode->offsize);
			Str << "'";
			if (pNode->pNext != NULL)
				Str << Spre;
		}
		else if (pNode->type == "__int64" || pNode->type == "signed __int64" || pNode->type == "unsigned __int64")
		{
			Str << "'"
				<< *(__int64 *)((uint64_t)obj + pNode->offsize)
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
				Str << "'";
				if (pNode->datetimeFlag)
					Str << GetDBTimeString(*(int *)((uint64_t)obj + pNode->offsize + i * sizeof(int)));
				else
					Str << *(int *)((uint64_t)obj + pNode->offsize + i * sizeof(int));
				Str	<< "'";
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
				Str << "'";
				if (pNode->datetimeFlag)
					Str << GetDBTimeString(*(long *)((uint64_t)obj + pNode->offsize + i * sizeof(long)));
				else
					Str << *(long *)((uint64_t)obj + pNode->offsize + i * sizeof(long));
				Str << "'";
				if (i != size - 1 || pNode->pNext != NULL)
					Str << Spre;
			}
		}
		else if (pNode->type.find("__int64 [") != std::string::npos || pNode->type.find("unsigned __int64 [") != std::string::npos || \
			pNode->type.find("signed __int64 [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(__int64);
			for (int i = 0; i < size; ++i)
			{
				Str << "'"
					<< *(__int64 *)((uint64_t)obj + pNode->offsize + i * sizeof(__int64))
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
		}
	}

	void _GetSqlName(stSqlNode* pNode, std::ostringstream& Str)
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
		else if (pNode->type == "__int64" || pNode->type == "signed __int64" || pNode->type == "unsigned __int64")
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
		else if (pNode->type.find("__int64 [") != std::string::npos || pNode->type.find("unsigned __int64 [") != std::string::npos || \
			pNode->type.find("signed __int64 [") != std::string::npos)
		{
			int size = pNode->arraysize / sizeof(__int64);
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
			Str << pNode->sqlName;
			if (pNode->pNext != NULL)
				Str << ", ";
		}
	}

public:
	std::string  GetClass()
	{
		return m_Class;
	}

	/*std::string  GetKey(T *obj)
	{
		std::string key;
		std::ostringstream Str;
		stSqlNode* pKNode = pKeyNode;
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
		stSqlNode* pKNode = pKeyNode;
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
		stSqlNode* pNode = pBaseNode;
		stSqlNode* pKNode = pKeyNode;
		//Str << GetKey(obj);
		Str << m_SqlSpeStr;
		while (pNode)
		{
			Str << pNode->name;
			Str << "=";
			_SetUnit(obj, pNode, Str);
			Str << m_SqlSpeStr;

			pNode = pNode->pNext;
		}

		while (pKNode)
		{
			Str << pKNode->name;
			Str << "=";
			_SetUnit(obj, pKNode, Str);
			Str << m_SqlSpeStr;

			pKNode = pKNode->pNext;
		}
		cmd = Str.str();
		return cmd;
	}

	void GetUnit(T *obj, std::string cmd)
	{
		stSqlNode* pNode = pBaseNode;
		stSqlNode* pKey = pKeyNode;
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
	}*/

	int GetSize()
	{
		return m_NodeSize;
	}

	template<class U0, class U1, class U2, class U3, class U4, class U5, class U6, class U7>
	std::string UpdateSqlEx(T *obj, U0 u0, U1 u1, U2 u2, U3 u3, U4 u4, U5 u5, U6 u6, U7 u7) {
		std::set<std::string> ArgvSet{ u0, u1, u2, u3, u4, u5, u6, u7 };
		std::ostringstream Str;
		std::string cmd;
		stSqlNode* pNode = pBaseNode;
		stSqlNode* pKNode = pKeyNode;
		Str << "UPDATE "
			<< m_Class
			<< " SET ";

		while (pNode)
		{
			auto itr = ArgvSet.find(pNode->name);
			if (itr != ArgvSet.end()) {
				_GetSql(obj, pNode, Str, ", ");
			}

			pNode = pNode->pNext;
		}

		auto nIndex = Str.str().find_last_of(",");
		if (nIndex != std::string::npos) {
			std::string str = Str.str();
			str = str.replace(nIndex, nIndex+2, "");
			Str.str("");
			Str << str
				<< " ";
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
	template<class U0, class U1, class U2, class U3, class U4, class U5, class U6>
	std::string UpdateSqlEx(T *obj, U0 u0, U1 u1, U2 u2, U3 u3, U4 u4, U5 u5, U6 u6) {
		return UpdateSqlEx(obj, u0, u1, u2, u3, u4, u5, u6, "");
	}
	template<class U0, class U1, class U2, class U3, class U4, class U5>
	std::string UpdateSqlEx(T *obj, U0 u0, U1 u1, U2 u2, U3 u3, U4 u4, U5 u5) {
		return UpdateSqlEx(obj, u0, u1, u2, u3, u4, u5, "", "");
	}
	template<class U0, class U1, class U2, class U3, class U4>
	std::string UpdateSqlEx(T *obj, U0 u0, U1 u1, U2 u2, U3 u3, U4 u4) {
		return UpdateSqlEx(obj, u0, u1, u2, u3, u4, "", "", "");
	}
	template<class U0, class U1, class U2, class U3>
	std::string UpdateSqlEx(T *obj, U0 u0, U1 u1, U2 u2, U3 u3) {
		return UpdateSqlEx(obj, u0, u1, u2, u3, "", "", "", "");
	}
	template<class U0, class U1, class U2>
	std::string UpdateSqlEx(T *obj, U0 u0, U1 u1, U2 u2) {
		return UpdateSqlEx(obj, u0, u1, u2, "", "", "", "", "");
	}
	template<class U0, class U1>
	std::string UpdateSqlEx(T *obj, U0 u0, U1 u1) {
		return UpdateSqlEx(obj, u0, u1, "", "", "", "", "", "");
	}
	template<class U0>
	std::string UpdateSqlEx(T *obj, U0 u0) {
		return UpdateSqlEx(obj, u0, "", "", "", "", "", "", "");
	}
	template<class U0, class U1, class U2, class U3, class U4, class U5, class U6, class U7>
	std::string UpdateSqlEx(TRef obj, U0 u0, U1 u1, U2 u2, U3 u3, U4 u4, U5 u5, U6 u6, U7 u7) {
		return UpdateSqlEx(obj.get(), u0, u1, u2, u3, u4, u5, u6, u7);
	}
	template<class U0, class U1, class U2, class U3, class U4, class U5, class U6>
	std::string UpdateSqlEx(TRef obj, U0 u0, U1 u1, U2 u2, U3 u3, U4 u4, U5 u5, U6 u6) {
		return UpdateSqlEx(obj.get(), u0, u1, u2, u3, u4, u5, u6, "");
	}
	template<class U0, class U1, class U2, class U3, class U4, class U5>
	std::string UpdateSqlEx(TRef obj, U0 u0, U1 u1, U2 u2, U3 u3, U4 u4, U5 u5) {
		return UpdateSqlEx(obj.get(), u0, u1, u2, u3, u4, u5, "", "");
	}
	template<class U0, class U1, class U2, class U3, class U4>
	std::string UpdateSqlEx(TRef obj, U0 u0, U1 u1, U2 u2, U3 u3, U4 u4) {
		return UpdateSqlEx(obj.get(), u0, u1, u2, u3, u4, "", "", "");
	}
	template<class U0, class U1, class U2, class U3>
	std::string UpdateSqlEx(TRef obj, U0 u0, U1 u1, U2 u2, U3 u3) {
		return UpdateSqlEx(obj.get(), u0, u1, u2, u3, "", "", "", "");
	}
	template<class U0, class U1, class U2>
	std::string UpdateSqlEx(TRef obj, U0 u0, U1 u1, U2 u2) {
		return UpdateSqlEx(obj.get(), u0, u1, u2, "", "", "", "", "");
	}
	template<class U0, class U1>
	std::string UpdateSqlEx(TRef obj, U0 u0, U1 u1) {
		return UpdateSqlEx(obj.get(), u0, u1, "", "", "", "", "", "");
	}
	template<class U0>
	std::string UpdateSqlEx(TRef obj, U0 u0) {
		return UpdateSqlEx(obj.get(), u0, "", "", "", "", "", "", "");
	}

	std::string UpdateSql(TRef obj){
		return UpdateSql(obj.get());
	}

	std::string UpdateSql(T *obj)
	{
		std::ostringstream Str;
		std::string cmd;
		stSqlNode* pNode = pBaseNode;
		stSqlNode* pKNode = pKeyNode;
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

	std::string InsertSql(TRef obj) {
		return InsertSql(obj.get());
	}

	std::string InsertSql(T *obj)
	{
		auto nNum = 0;
		std::ostringstream Str;
		std::string cmd;
		stSqlNode* pNode = pBaseNode;
		stSqlNode* pKNode = pKeyNode;
		Str << "INSERT INTO "
			<< m_Class
			<< "(";

		while (pKNode)
		{
			_GetSqlName(pKNode, Str);
			nNum++;
			pKNode = pKNode->pNext;
		}

		if (nNum) {
			Str << ",";
		}

		while (pNode)
		{
			_GetSqlName(pNode, Str);
			pNode = pNode->pNext;
			if (!pNode)
				Str << ")";
		}
		
		nNum = 0;
		Str << "VALUES"
			<< "(";

		pNode = pBaseNode;
		pKNode = pKeyNode;

		while (pKNode)
		{
			_GetSqlVal(obj, pKNode, Str);
			nNum++;
			pKNode = pKNode->pNext;
		}

		if (nNum) {
			Str << ",";
		}

		while (pNode)
		{
			_GetSqlVal(obj, pNode, Str);
			pNode = pNode->pNext;
			if (!pNode)
				Str << ")";
		}

		cmd = Str.str();
		return cmd;
	}

	std::string LoadSql(TRef obj) {
		return LoadSql(obj.get());
	}

	std::string LoadSql(T *obj)
	{
		std::ostringstream Str;
		std::string cmd;
		stSqlNode* pNode = pBaseNode;
		stSqlNode* pKNode = pKeyNode;
		Str << "SELECT ";

		while (pKNode)
		{
			_GetSql(obj, pKNode, Str);
			pKNode = pKNode->pNext;
		}
		Str << ", ";
		while (pNode)
		{
			_GetSqlName(pNode, Str);
			pNode = pNode->pNext;
		}

		Str <<" FROM "
			<< m_Class
			<<" WHERE ";
		while (pKNode)
		{
			_GetSql(obj, pKNode, Str);
			pKNode = pKNode->pNext;
		}

		cmd = Str.str();
		return cmd;
	}

	template<class K, class V>
	std::string LoadSql(K key, V val)
	{
		std::ostringstream Str;
		std::string cmd;
		stSqlNode* pNode = pBaseNode;
		stSqlNode* pKNode = pKeyNode;
		Str << "SELECT ";

		while (pKNode)
		{
			_GetSqlName(pKNode, Str);
			pKNode = pKNode->pNext;
		}

		Str << ", ";

		while (pNode)
		{
			_GetSqlName(pNode, Str);
			pNode = pNode->pNext;
		}

		Str << " FROM "
			<< m_Class
			<< " WHERE "
			<< key
			<< "="
			<< val;

		cmd = Str.str();
		return cmd;
	}

	std::string LoadSql()
	{
		std::ostringstream Str;
		std::string cmd;
		stSqlNode* pNode = pBaseNode;
		stSqlNode* pKNode = pKeyNode;
		Str << "SELECT ";

		while (pKNode)
		{
			_GetSqlName(pKNode, Str);
			pKNode = pKNode->pNext;
		}

		Str << ",";

		while (pNode)
		{
			_GetSqlName(pNode, Str);
			pNode = pNode->pNext;
		}

		Str << " FROM "
			<< m_Class;

		cmd = Str.str();
		return cmd;
	}

	std::string DelSql(TRef obj) {
		return DelSql(obj.get());
	}

	std::string DelSql(T *obj)
	{
		std::ostringstream Str;
		std::string cmd;
		stSqlNode* pNode = pBaseNode;
		stSqlNode* pKNode = pKeyNode;
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
	stSqlNode* pBaseNode;
	stSqlNode* pEndNode;
	stSqlNode* pKeyNode;//主键
	stSqlNode* pEndKeyNode;
	int m_NodeSize;
};

template <class T>
class  ToSql
{
public:
	typedef typename std::tr1::shared_ptr<T> TRef;
public:
	ToSql()
	{
		m_pSqlUnit = dynamic_cast<CSqlUnit<T> *>(SQLUNITMGR->GetUnit(typeid(T).name()));
		assert(m_pSqlUnit != NULL);
	}

	virtual ~ToSql()
	{
	}
	inline std::string UpdateSql(T* obj){
		std::string cmd = m_pSqlUnit->UpdateSql(obj);
		return cmd;
	}
	inline std::string UpdateSql(TRef obj)	{	
		return UpdateSql(obj.get()); 
	}
	inline std::string InsertSql(T* obj){
		std::string cmd = m_pSqlUnit->InsertSql(obj);
		return cmd;
	}
	inline std::string InsertSql(TRef obj)	{	
		return InsertSql(obj.get());
	}
	inline std::string DeleteSql(T* obj){
		std::string cmd = m_pSqlUnit->DelSql(obj);
		return cmd;
	}
	inline std::string DeleteSql(TRef obj){	
		return DeleteSql(obj.get());
	}
	inline std::string LoadSql(T* obj) {
		std::string cmd = m_pSqlUnit->LoadSql(obj);
		return cmd;
	}
	inline std::string LoadSql(TRef obj) {
		return LoadSql(obj.get());
	}

	template<class K, class V>
	inline std::string LoadSql(K key, V val) {
		std::string cmd = m_pSqlUnit->LoadSql(key, val);
		return cmd;
	}

	inline std::string LoadSql() {
		std::string cmd = m_pSqlUnit->LoadSql();
		return cmd;
	}
public:
	CSqlUnit<T>* m_pSqlUnit;
};


#define DECLARE_SQL_UNIT(cls)	static CSqlUnit<cls> s_Sql___##cls;
#define GET_SQL_UNIT(cls)		s_Sql___##cls
#define REGISTER_SQL_UNIT(cls, x) s_Sql___##cls.RegisterUnit(#x, typeid(cls::x).name(), Offset(x,cls), sizeof(cls::x));
#define REGISTER_SQL_UNITKEY(cls, x) s_Sql___##cls.RegisterUnit(#x, typeid(cls::x).name(), Offset(x,cls), sizeof(cls::x), true);//主键
#define REGISTER_SQL_DATETIME(cls, x) s_Sql___##cls.RegisterUnit(#x, typeid(cls::x).name(), Offset(x,cls), sizeof(cls::x), false, true);//日期

#endif//_TO_SQL_H_