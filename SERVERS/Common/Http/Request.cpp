#include "Request.h"
#include <assert.h>
#include "Common\UtilString.h"
#include "CommLib\CommLib.h"
#include "base\log.h"

CRequest::CRequest()
{
	curl_global_init(CURL_GLOBAL_ALL);
}

CRequest::~CRequest()
{
	curl_global_cleanup();
}

CRequest* CRequest::GetInstance()
{
	static CRequest s_request;
	return &s_request;
}

HTTP_RESULTCODE CRequest::SyncRequest(std::string strUrl, EHttpMethod eHttpMethod, CParam& oParam, std::string& strData)
{
	HTTP_RESULTCODE eResultCode = HTTPRESULT_FAIL;	
	if (eHttpMethod == EHttpMethod_Get)
	{
		eResultCode = CHttp().SyncDownload(GetBaseUrl(strUrl, oParam), strData);
	}
	else if (eHttpMethod == EHttpMethod_Post)
	{
		if (oParam.GetPicParamMap().empty())
		{
			char* pUrlParam = oParam.GetUrlParamString();
			eResultCode = CHttp().SyncUpload(strUrl, pUrlParam, strData);
			SAFE_DELETE_ARRAY(pUrlParam);
		}
		else
		{
			eResultCode = CHttp().SyncUploadFormData(strUrl, oParam.GetParamMap(), oParam.GetPicParamMap(), strData);
		}
	}
	
	char* result = Util::Utf8ToMbcs(strData.c_str());
	strData = result;
	SAFE_DELETE_ARRAY(result);

	return eResultCode;
}

HTTP_RESULTCODE CRequest::SyncHttpsRequest(std::string strUrl, EHttpMethod eHttpMethod, JsonParser& oJsonParser, std::string& strData)
{
	char* result = NULL;
	HTTP_RESULTCODE eResultCode = HTTPRESULT_FAIL;	

	do{
		if (eHttpMethod == EHttpMethod_Post)
		{
			std::string sReceipt = oJsonParser.GetJasonString();
			eResultCode = CHttp().SyncUpload(strUrl, sReceipt, strData);
			result = Util::Utf8ToMbcs(strData.c_str());
			strData = result;
		}
		else
			break;

	} while (false);
	
	SAFE_DELETE_ARRAY(result);

	return eResultCode;
}

bool CRequest::AsyncRequest(std::string strUrl, EHttpMethod eHttpMethod, CParam& oParam, VCallback* pCallback)
{
	bool bResult = false;
	CHttp* pHttp = new CHttp;
	if (eHttpMethod == EHttpMethod_Get)
	{
		bResult = pHttp->AsyncDownload(GetBaseUrl(strUrl, oParam));
	}
	else if (eHttpMethod == EHttpMethod_Post)
	{
		if (oParam.GetPicParamMap().empty())
		{
			char* pUrlParam = oParam.GetUrlParamString();
			bResult = pHttp->AsyncUpload(strUrl, pUrlParam);
			SAFE_DELETE_ARRAY(pUrlParam);
		}
		else
		{
			bResult = pHttp->AsyncUploadFormData(strUrl, oParam.GetParamMap(), oParam.GetPicParamMap());
		}
	}

	if (bResult)
	{
		tagRequest oRequest;
		oRequest.strUrl = strUrl;
		oRequest.oParam = oParam;
		oRequest.pCallback = pCallback;

		m_mapHttp2Request[pHttp] = oRequest;
	}
	else
	{
		SAFE_DELETE(pHttp);
	}

	return bResult;
}

void CRequest::OnHttpComplete(CHttp* pHttp, HTTP_RESULTCODE eErrorCode)
{
	if (pHttp == NULL)
	{
		assert(pHttp);
		return;
	}

	std::string strData;
	if (!pHttp->GetResultData(strData))
		eErrorCode = HTTPRESULT_TOOLAGRE;
	pHttp->m_bRunning = false;

	MapHttp2Request::iterator it = m_mapHttp2Request.find(pHttp);
	if (it == m_mapHttp2Request.end())
		return;

	tagRequest& oRequest = it->second;
	if (oRequest.pCallback != NULL)
	{
		char* result = Util::Utf8ToMbcs(strData.c_str());
		std::string strDecode = result;
		SAFE_DELETE_ARRAY(result);

		oRequest.pCallback->OnRequestComplete(eErrorCode, oRequest.strUrl.c_str(), oRequest.oParam, strDecode.c_str(), strDecode.length());
	}

	m_mapHttp2Request.erase(it);
	delete pHttp;
}

std::string CRequest::GetBaseUrl(std::string& strUrl, CParam& oParam)
{
	std::string strBaseUrl = strUrl;
	char* pUrlParam = oParam.GetUrlParamString();
	if (pUrlParam)
	{
		strBaseUrl += "?";
		strBaseUrl += pUrlParam;
		SAFE_DELETE_ARRAY(pUrlParam);
	}

	g_Log.WriteLog("Request url:%s.", strBaseUrl.c_str());
	return strBaseUrl;
}