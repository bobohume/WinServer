/** 
@file  
@brief	http下载类
@version	2010-11-19	

*/

#pragma once
#include <map>
#include "curl/curl.h"

enum HTTP_RESULTCODE
{
	HTTPRESULT_OK   = 0,		///<  成功	
	HTTPRESULT_TIMEOUT,         ///<  请求超时
	HTTPRESULT_CANTRESOLVE,     ///<  不能解析地址
	HTTPRESULT_CANTCONNECT,     ///<  不能连接
	HTTPRESULT_ERROR,           ///<  HTTP请求的数据错误 
	HTTPRESULT_TOOLAGRE,        ///<  返回的数据量太大
	HTTPRESULT_FAIL,		    ///<  失败
	HTTPRESULT_OPITON_ERR		///类型错误，无法生成有效url
};

class  CHttp
{
public:
	CHttp();
	~CHttp();	

public:
	/// 同步下载
	/** 
	@param strUrl		in:   下载的url	
	@param strResult	out:  结果数据

	@retval  HTTPRESULT_OK:成功   HTTPRESULT_FAIL:失败 
	*/	
	HTTP_RESULTCODE	SyncDownload(const std::string& strUrl, std::string& strResult);

	/// 异步下载
	/** 
	@param strUrl		in:  下载的url
	@param pCallback	in:  获取回调的指针
	*/	
	bool	AsyncDownload(const std::string& strUrl);

	/// 同步上传
	/** 
	@param strUrl		in:   上传的url
	@param strData		in:   上传的数据
	@param strResult	out:  结果数据

	@retval  HTTPRESULT_OK:成功   HTTPRESULT_FAIL:失败 
	*/	
	HTTP_RESULTCODE	SyncUpload(const std::string& strUrl, const std::string& strData, std::string& strResult);

	/// 异步上传
	/** 
	@param strUrl		in:	上传的url
	@param strData		in:	上传的数据
	@param pCallback	in:	获取回调的指针
	*/	
	bool	AsyncUpload(const std::string& strUrl, const std::string& strData);

	/// 同步上传，采用enctype="multipart/form-data"
	/** 
	@param strUrl		in:	上传的url
	@param mapData		in:	上传的字段内容
	@param mapFile		in:	上传的文件名
	@param strResult	out: 结果数据

	@retval  HTTPRESULT_OK: 成功   HTTPRESULT_FAIL: 失败 
	*/	
	HTTP_RESULTCODE	SyncUploadFormData(const std::string &strUrl, const std::map<std::string, std::string>& mapData,
										const std::map<std::string, std::string>& mapFile, std::string& strResult);

	/// 异步上传，采用enctype="multipart/form-data"
	/** 
	@param strUrl		in:	上传的url
	@param strData		in:	上传的数据
	@param pCallback	in:	获取回调的指针
	*/	
	bool	  AsyncUploadFormData(const std::string& strUrl,const std::map<std::string, std::string>& mapData, 
									const std::map<std::string, std::string>& mapFile);


public:
	static    size_t	ProcessResult(void* data, size_t size, size_t nmemb, void* pHttp);


public:
	void			Reset();

	CURLcode		DoHttpGet();
	CURLcode		DoHttpPost();
	CURLcode		DoHttpsPost();
	CURLcode		DoHttpPostFormData();	
	

	bool			WriteResultData(char* pchData, unsigned long uLength);
	bool			GetResultData(std::string& strData);

	//用一个列表回收资源
	void	  recoveryCurlRecourse(CURL* );

public:
	bool			m_bRunning;
	
private:
	char			errorInfo[CURL_ERROR_SIZE];
	std::string		m_strUrl;   
	std::string		m_strData;	 ///<  Post包体数据

	typedef	std::map<std::string, std::string>    CMapKeyValue;	
	CMapKeyValue	m_mapPostFile;   ///<  以form-data格式上传的文件或图片
	CMapKeyValue	m_mapPostData;   ///<  以form-data格式上传的post数据

	
	char*			m_pchResultData; ///<  上传或下载的结果存放
	unsigned long	m_uLength;		 ///<  上传或下载的结果存放的BUF的长度
	unsigned long	m_uWritePos;	 ///<  上传或下载的结果数据buf的当前书写位置

};