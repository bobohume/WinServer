#ifndef CFG_BUILDER_H
#define CFG_BUILDER_H

#include <cstdio>
#include <conio.h>

#include <list>
#include <string>
#include <map>

//class CCfgBuilder
//{
//public:
//	CCfgBuilder();
//	virtual ~CCfgBuilder();
//
//	void addOption(std::string name, int& value );
//	void addOption(std::string name, std::string& value );
//
//	void doBuild();
//
//	void saveFile( const char* fileName );
//	void loadFile( const char* fileName );
//	
//private:
//	struct _option
//	{
//		std::string name;
//		void* value;
//
//		int type;
//
//		_option() : type(0), value( 0 ) {}
//
//		enum
//		{
//			TypeNone,
//			TypeInt,
//			TypeString,
//			TypeCount
//		};
//	};
//
//	typedef std::list< _option > OptionList;
//	typedef stdext::hash_map< std::string, _option* > OptionMap;
//
//	OptionList mOptionList;
//	OptionMap mOptionMap;
//};

class CMyLog;
class CCfgBuilder
{
public:
	~CCfgBuilder();
	std::string Get(const char* key);
	void Get(const char* key, char sep, std::string& first, std::string& second);
	std::string GetByHostIP(const char* key);
	void GetByHostIP(const char* key, char sep, std::string& first, std::string& second);
	std::string GetMatchHostIP();
	void SelectConfig(int config);
	void SelectConfig(bool* configArray, int maxConfig);
	void ReleaseConfig(bool* configArray, int maxConfig);
	friend void ConfigLog(const char* configName,const char *specMark=NULL);

    void Reload();
	static CCfgBuilder* GetInstance();
	static std::string s_CfgBuilderNo;
	static void ParseCfgBuilder(int argc, char* argv[]);
private:
	CCfgBuilder(const char* filename);
	char* Trim(char* str,bool toLower);
	std::string Token(const char* buffer, int& begin, int end,bool toLower = false);
	void Split(const std::string& buf, char sep, std::string& first, std::string& second);
	enum STATE
	{
		STATE_NONE,
		STATE_SECTION,
		STATE_VALUE
	};
	typedef std::map<std::string, std::string> SectionInfo;
	typedef SectionInfo::iterator SectionInfoIter;
	typedef std::pair<std::string, int> CfgKey;
	typedef std::map<CfgKey, SectionInfo> CfgInfo;
	typedef CfgInfo::iterator CfgInfoIter;

	CfgInfo m_cfgInfo;
	std::string m_matchHostIP;
	int m_config;
};

#define CFG_BUIILDER CCfgBuilder::GetInstance()

#endif /*CFG_BUILDER_H*/