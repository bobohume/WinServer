/** 
@file  
@brief	QWBlogAPI的请求获取数据的接口
@version	2010-11-15	

*/

#pragma once
#include <string>
#include "Param.h"
#include "JSON/JsonParser.h"
#include "Http.h"
#include <map>

enum  EHttpMethod
{
	EHttpMethod_Get,
	EHttpMethod_Post,
};

/// 异步请求回调接口
class  VCallback
{
public:
	/// 异步请求回调函数
	/** 
		@param  eHttpRetCode    in :    Http错误码
		@param  pReqUrl			in :	请求URL
		@param  oParam			in :	该请求所带的参数类对象
		@param  pData			in :    返回的数据
		@param  nLen            in :    返回的数据长度
	*/	
	virtual void OnRequestComplete(HTTP_RESULTCODE eHttpRetCode, const char* pReqUrl, CParam& oParam, const char* pData, int nLen) = 0;
};

struct tagRequest
{
	std::string	strUrl;
	CParam		oParam;
	VCallback*	pCallback;
};


class  CRequest 
{
public:	
	~CRequest();
	static CRequest* GetInstance(); 

	
	/** 同步请求数据
	    @param  strUrl              in :  请求url
		@param	eHttpMethod			in :  请求的类型
		@param  oQWBlogParam		in :  该请求所带的参数类对象
		@param  strData				out ：同步返回的数据

		@retval		HTTPRESULT_OK:成功	其它:失败
	*/
	HTTP_RESULTCODE SyncRequest(std::string strUrl, EHttpMethod eHttpMethod,  
								CParam& oQWBlogParam, std::string& strData);
	

	/** 同步请求数据
	    @param  strUrl              in :  请求url
		@param	eHttpMethod			in :  请求的类型
		@param  oJsonParser			in :  附带的jason对象
		@param  strData				out ：同步返回的数据

		@retval		HTTPRESULT_OK:成功	其它:失败
	*/
	HTTP_RESULTCODE SyncHttpsRequest(std::string strUrl, EHttpMethod eHttpMethod,  
								JsonParser& oJsonParser, std::string& strData);

	/** 同步请求数据
	    @param strUrl           in : 请求url
		@param eRequest			in : 请求的类型
		@param oQWBlogParam		in : 该请求所带的参数类对象
		@param pCallback		in ：回调指针

		@retval		true:成功	false:失败
	*/
	bool AsyncRequest(std::string strUrl, EHttpMethod eHttpMethod,  
						CParam& oQWBlogParam, VCallback* pCallback);



	/// http完成回调函数,  继承类必须实现该函数来获得上传下载结果的通知
	/** 
		@param pHttp			in： CHttp对象
		@param eErrorCode		in： 状态代码，取值HTTP_RESULTCODE
		@param strData			out：结果数据
	*/	
	virtual void OnHttpComplete(CHttp* pHttp, HTTP_RESULTCODE eErrorCode);


private:
	CRequest();
	CRequest(const CRequest& other);
	CRequest& operator=(const CRequest& other);

	std::string GetBaseUrl(std::string& strUrl, CParam& oParam);

private:
	typedef std::map<CHttp*, tagRequest> MapHttp2Request;
	MapHttp2Request m_mapHttp2Request;
};