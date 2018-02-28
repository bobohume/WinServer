/** 
@file  
@brief		请求的参数类,负责参数的组装。参数按照URL请求的参数格式组装。
@version	2010-11-16		

**@exmaple:
format=json&pageflag=0&reqnum=20

CParam oParam;
oParam.AddParam(TXWeibo::Param::strFormat,TXWeibo::Param::strJson);
oParam.AddParam("pageflag","0");
oParam.AddParam("reqnum","20");
*
*/

#pragma once
#include <map>
#include <string>

class CParam
{
public:
	CParam();
	~CParam();

	///增加参数
	/**
	@param prKey   in : 参数名
	@param pValue  in : 参数值
	*/
	void	AddParam(const char* pKey, const char* pValue);

	///获取参数
	/**
	@param pKey  in : 参数名
	*/
	/*返回值需调用ReleaseData释放*/
	char*	GetParam(const char* pKey);

	///增加图片名参数
	/**
	@param pKey    in : 参数名。
	@param pValue  in : 参数值
	*/
	void	AddPicNameParam(const char* pKey, const char* pValue);
	///得到图片名
	/**
	@param pKey  in : 参数名
	*/
	/*返回值需调用ReleaseData释放*/
	char*	GetPicNameParam(const char* pKey);

	/// 得到组装好的参数的字符串
	/*返回值需调用ReleaseData释放*/
	char*	GetUrlParamString();

	/// 清除所有参数，这样CParam对象可以重复使用
	void	Clear();

public:
	typedef	std::map<std::string, std::string>	MapKey2Value;
	
	MapKey2Value& GetParamMap();
	MapKey2Value& GetPicParamMap();

protected:
	MapKey2Value m_mapParam;		///< 保存参数的map，把参数名映射到参数值
	MapKey2Value m_mapPicParam;		///< 保存图片名的map，把文件参数名映射到文件名
};