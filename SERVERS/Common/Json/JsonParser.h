/** 
@file  
@brief		json解析实现类
@version	2010-11-18		
*/

#pragma once
#include "json.h"
#include "string.h"

class JsonParser
{
public:
	JsonParser();
	JsonParser(Json::Value* pValue);
	~JsonParser();

	/// 判断当前parser是否为readonly。为readonly时，不能调用Parse接口；否则失败
	bool	IsReadyOnly();

	/// 解析json文件
	/*
	@param	pJsonContent	in，json字符串，必须为utf8
	*/
	bool	Parse(const char* pJsonContent);

	/// 根据名字获取子节点
	/*
	@param	pName	in，子节点名字
	*/
	JsonParser	GetChild(const char* pName); 

	/// 根据序号获取子节点
	/*
	@param	nIndex	in，子节点序号
	*/
	JsonParser	GetChild(const int nIndex);

	/// 根据名字获取子节字符串
	/*
	@param	pName	in，子节点名字
	*/
	/*返回值需调用ReleaseData释放*/
	std::string	GetString(const char* pName);

	/// 根据序号获取子节点字符串
	/*
	@param	nIndex	in，子节点序号
	*/
	/*返回值需调用ReleaseData释放*/
	std::string	GetString(const int nIndex);

	/// 根据名字获取子节点布尔值
	/*
	@param	pName	in，子节点名字
	*/
	bool	GetBool(const char* pName);

	/// 根据序号获取子节点布尔值
	/*
	@param	nIndex	in，子节点序号
	*/
	bool	GetBool(const int nIndex);

	/// 根据名字获取子节点整数值
	/*
	@param	pName	in，子节点名字
	*/
	int		GetInt(const char* pName);

	/// 根据序号获取子节点整数值
	/*
	@param	nIndex	in，子节点序号
	*/
	int		GetInt(const int nIndex);

	/// 根据名字获取子节点非符号整数值
	/*
	@param	pName	in，子节点名字
	*/
	unsigned int	GetUint(const char* pName);

	/// 根据序号获取子节点非符号整数值
	/*
	@param	nIndex	in，子节点序号
	*/
	unsigned int	GetUint(const int nIndex);

	/// 根据名字获取子节点64位非符号整数值
	/*
	@param	pName	in，子节点名字
	*/
	long long	GetUint64(const char* pName);

	/// 根据序号获取子节点64位非符号整数值
	/*
	@param	nIndex	in，子节点序号
	*/
	long long	GetUint64(const int nIndex);

	/****************************************************************/
	// add by natural
	// 添加节点
	void AddNode(std::string key, std::string value);

	// 获取Jason字符串
	std::string GetJasonString();

	template<typename T>
	Json::Value GetValue(T b)
	{
		Json::Value oValue(Json::Value::null);
		if(m_pRoot && Json::Value::null != (*m_pRoot)[b])
			oValue = (*m_pRoot)[b];
		return oValue;
	}

private:
	static JsonParser	GetReadOnlyReader(Json::Value* pRoot);
	Json::Value*		GetChildByName(std::string& strName);
	Json::Value*		GetChildByIndex(const int nIndex);


private:	
	Json::Value*		m_pRoot;		///<  json文件根节点
	bool				m_bReadOnly;	///<  是否只读
	bool				m_bDeleteSelf;	///<  是否需要自己管理内存释放
};