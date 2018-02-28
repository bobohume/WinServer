#include "CfgBuilder.h"
#include "Base64.h"
#include <fstream>
#include <WinSock2.h>
#include "Base/Log.h"
#include "base/types.h"

const int MAX_LINE_LENGTH = 2048;
const int BUFFER_LENGTH = 512;
const int MAX_TOKEN_LENGTH = 128;
const int MAX_HOSTNAME_LEN = 128;
const int DEFAULT_CONFIG = 0;

std::string CCfgBuilder::s_CfgBuilderNo = "";

CCfgBuilder* CCfgBuilder::GetInstance()
{
	static CCfgBuilder s_cfgBuilder("");
	return &s_cfgBuilder;
}

void CCfgBuilder::ParseCfgBuilder(int argc, char* argv[]) {
	for (auto i = 0; i < argc; ++i)
	{
		std::string cmd = argv[i];
		if (cmd.find("cfg=") != std::string::npos) {
			s_CfgBuilderNo = cmd.substr(strlen("cfg="));
		}
	}
}

CCfgBuilder::CCfgBuilder(const char* filename)
:m_config(DEFAULT_CONFIG)
{
	Reload();
}

CCfgBuilder::~CCfgBuilder()
{

}

char* CCfgBuilder::Trim(char* str,bool toLower)
{
	int begin = 0;
	while (str[begin]==' ' || str[begin]=='\t')
		begin++;
	int i = begin;

	while (str[i]!=' ' && str[i]!='\t' && str[i]!='\0')
    {   
        if (0 == __isascii(str[i]))
        {
            i++;
            continue;
        }

        if (isupper(str[i]) && toLower)
        {
            str[i] = _tolower(str[i]);
        }

		i++;
    }

	str[i] = '\0';
	return &str[begin];
}

void local_UTF8_To_String(char* dest,size_t size,const char* source)
{
    int nwLen = MultiByteToWideChar(CP_UTF8, 0, source, -1, NULL, 0);
    wchar_t * pwBuf = new wchar_t[nwLen + 1];//
    memset(pwBuf, 0, nwLen * 2 + 2);

    MultiByteToWideChar(CP_UTF8, 0, source, (int)strlen(source), pwBuf, nwLen);

    int nLen = WideCharToMultiByte(CP_ACP, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

    char * pBuf = new char[nLen + 1];
    memset(pBuf, 0, nLen + 1);

    WideCharToMultiByte(CP_ACP, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

    memset(dest, 0, size);

    if( size >= strlen(pBuf)+1 )
    {
        strcpy_s(dest,strlen(pBuf)+1,pBuf);
    }
    else
    {
        strncpy_s(dest,size,pBuf,size-1);
    }

    delete []pBuf;
    delete []pwBuf;

    pBuf = NULL;
    pwBuf = NULL;
}

std::string CCfgBuilder::Token(const char* srcBuffer, int& begin, int end,bool toLower)
{
    char buffer[BUFFER_LENGTH] = {0};
    local_UTF8_To_String(buffer,sizeof(buffer),srcBuffer);

	char token[MAX_TOKEN_LENGTH] = {0};
	int len = end - begin;
	if(len >= MAX_TOKEN_LENGTH - 1)
		len = MAX_TOKEN_LENGTH - 1;
	memcpy(token, &buffer[begin], len);
	token[len] = '\0';
	begin = end+1;
	return Trim(token,toLower);
}

std::string CCfgBuilder::Get(const char* key)
{
    char strKey[256] = {0};
    dStrcpy(strKey,sizeof(strKey),key);

    for (size_t i = 0; i < strlen(strKey); ++i)
    {
        if (isupper(strKey[i]))
        {
            strKey[i] = _tolower(strKey[i]);
        }
    }

	for (CfgInfoIter cfgIter=m_cfgInfo.begin(); cfgIter!=m_cfgInfo.end(); ++cfgIter)
	{
		SectionInfo& secInfo = cfgIter->second;
		SectionInfoIter secIter = secInfo.find(strKey);

		if (secIter != secInfo.end())
			return secIter->second;
	}

	return "";
}

void CCfgBuilder::Get(const char* key, char sep, std::string& first, std::string& second)
{
	Split(Get(key), sep, first, second);
}

void CCfgBuilder::Split(const std::string& buf, char sep, std::string& first, std::string& second)
{
	size_t index = buf.find(sep);
	first = buf.substr(0, index);
	second = buf.substr(index+1, buf.length()-index);
}

std::string CCfgBuilder::GetByHostIP(const char* key)
{
    char strKey[256] = {0};
    dStrcpy(strKey,sizeof(strKey),key);

    for (size_t i = 0; i < strlen(strKey); ++i)
    {
        if (isupper(strKey[i]))
        {
            strKey[i] = _tolower(strKey[i]);
        }
    }

	CfgInfoIter cfgIter = m_cfgInfo.find(CfgKey(GetMatchHostIP(), m_config));
	if (cfgIter != m_cfgInfo.end())
	{
		SectionInfo& secInfo = cfgIter->second;
		SectionInfoIter secIter = secInfo.find(strKey);
		if (secIter != secInfo.end())
			return secIter->second;
	}
	return "";
}

void CCfgBuilder::GetByHostIP(const char* key, char sep, std::string& first, std::string& second)
{
	Split(GetByHostIP(key), sep, first, second);
}

std::string CCfgBuilder::GetMatchHostIP()
{
	return m_matchHostIP;
}

void CCfgBuilder::SelectConfig(bool* configArray, int maxConfig)
{
	for (; m_config<maxConfig; m_config++)
	{
		if (!configArray[m_config])
		{
			configArray[m_config] = true;
			break;
		}
	}
}

void CCfgBuilder::ReleaseConfig(bool* configArray, int maxConfig)
{
	if (m_config < maxConfig)
		configArray[m_config] = false;
}

void CCfgBuilder::SelectConfig(int config)
{
	m_config = config;
}

void ConfigLog(const char* configName,const char *specMark)
{
	std::string info = CFG_BUIILDER->Get(configName);
	if (info == "")
	{
		std::string first = "";
		std::string second = "";
		std::string defaultInfo = CFG_BUIILDER->Get("DefaultLog");
		if (defaultInfo == "")
		{
			first = "on";
			second = "on";
		}
		else
		{
			CFG_BUIILDER->Split(defaultInfo, ':', first, second);
		}
		std::string logName = "";
		bool bFile = true;
		if (first != "off")
		{
			char path[256] = {0};
			if(specMark)
			{
				logName += specMark;
				logName += "/";
			}

			GetModuleFileNameA(0, path, _countof(path));
			char buf[256] = {0};
			_splitpath_s(path, 0, 0, 0, 0, buf, _countof(buf), 0, 0);
			logName += buf;
			if(specMark)
			{
				logName += "_";
				logName += specMark;
			}
			logName += "_";
			_itoa_s(GetCurrentProcessId(), buf, _countof(buf), 10);
			logName += buf;
			logName += "_";
		}
		else
		{
			bFile = false;
		}
		bool onScreen = true;
		if (second == "off")
			onScreen = false;
		g_Log.SetLogName(logName, onScreen, bFile);
	}
	else
	{
		std::string first = "";
		std::string second = "";
		CFG_BUIILDER->Split(info, ':', first, second);
		bool bFile = true;
		if (first != "off")
		{
			first += "_";
			char buf[256] = {0};
			_itoa_s(GetCurrentProcessId(), buf, _countof(buf), 10);
			first += buf;
			first += "_";
		}
		else
		{
			bFile = false;
		}
		bool onScreen = true;
		if (second == "off")
			onScreen = false;
		g_Log.SetLogName(first, onScreen, bFile);
	}
}

void CCfgBuilder::Reload()
{
    m_cfgInfo.clear();
	std::string strFileName = "..//SXZ_SERVER";
	strFileName += s_CfgBuilderNo;
	strFileName += ".CFG";
    const char* filename = strFileName.c_str();

    std::ifstream fileIn(filename);

	if (!fileIn.is_open())
	{
		g_Log.WriteError("Can't open file %s.", filename);
		return;
	}

	char line[MAX_LINE_LENGTH] = {0};
	char buffer[BUFFER_LENGTH] = {0};
	std::string section = "";
	std::map<std::string, int> secCount;

	while (!fileIn.eof())
	{
		fileIn.getline(line, _countof(line));
		dBase64::decode(line, (unsigned char*)buffer, strlen(line));
		STATE state = STATE_NONE;
		bool comment = false;
		int tokenBegin = 0;
		std::string key = "";
		int i = 0;
		for (; buffer[i]!='\0' && !comment; ++i)
		{
			switch (buffer[i])
			{
			case '[':
				if (state == STATE_NONE)
				{
					tokenBegin = i+1;
					state = STATE_SECTION;
				}
				break;
			case ']':
				if (state == STATE_SECTION)
				{
					section = Token(buffer, tokenBegin, i);
					if (section != "")
						m_cfgInfo[CfgKey(section, secCount[section]++)] = SectionInfo();
					state = STATE_NONE;
				}
				break;
			case '=':
				if (state == STATE_NONE)
				{
					key = Token(buffer, tokenBegin, i,true);
					if (key != "")
						state = STATE_VALUE;
				}
				break;
			case ';':
				if (state == STATE_VALUE)
				{
					if (section != "")
						m_cfgInfo[CfgKey(section, secCount[section]-1)][key] = Token(buffer, tokenBegin, i);
					state = STATE_NONE;
				}
				break;
			case '//':
				if (i>1 && buffer[i-1]=='//' && state==STATE_VALUE)
				{
					if (section != "")
						m_cfgInfo[CfgKey(section, secCount[section]-1)][key] = Token(buffer, tokenBegin, i);
					comment = true;
					state = STATE_NONE;
				}
				break;
			default:
				break;
			}
		}
		if (state == STATE_VALUE)
		{
			if (section != "")
				m_cfgInfo[CfgKey(section, secCount[section]-1)][key] = Token(buffer, tokenBegin, i);
			state = STATE_NONE;
		}
	}
	fileIn.close();

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);
	char hostName[MAX_HOSTNAME_LEN];
	gethostname(hostName, _countof(hostName));
	struct hostent* hosts = gethostbyname(hostName);
	if (hosts)
	{
		for (int i=0; hosts->h_addr_list[i]!=0; ++i)
		{
			struct in_addr address;
			memcpy(&address, hosts->h_addr_list[i], sizeof(struct in_addr));
			std::string hostAddress = inet_ntoa(address);
			if (m_cfgInfo.find(CfgKey(hostAddress, DEFAULT_CONFIG)) != m_cfgInfo.end())
			{
				m_matchHostIP = hostAddress;
				break;
			}
		}
	}

	WSACleanup();
}