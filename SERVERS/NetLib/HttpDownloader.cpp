#include "Stdafx.h"

#include "HttpDownloader.h"

#ifdef _DEBUG
#pragma comment( lib, "atlsd.lib" )
#else
#pragma comment( lib, "atls.lib" )
#endif

namespace CHttpDownloaderHelper
{
	Helper::Helper()
	{
		CoInitialize( NULL );
	}

	Helper::~Helper()
	{
		CoUninitialize();
	}
}

CHttpDownloaderHelper::Helper CHttpDownloader::_httpDownloaderHelper;

STDMETHODIMP CHttpDownloader::XMLHttpEventSink::QueryInterface(const IID &riid, void **ppvObject)
{
	if( ppvObject == NULL )
		return E_INVALIDARG;

	*ppvObject = NULL;

	if( riid == IID_IUnknown )
		*ppvObject = static_cast<IUnknown*>(this);
	else if( riid == IID_IDispatch )
		*ppvObject = static_cast<IDispatch*>(this);

	if( *ppvObject == NULL )
		return E_NOINTERFACE;

	AddRef();

	return S_OK;
}

STDMETHODIMP_(ULONG) CHttpDownloader::XMLHttpEventSink::AddRef()
{
	return InterlockedIncrement(&_refCount);
}


STDMETHODIMP_(ULONG) CHttpDownloader::XMLHttpEventSink::Release()
{
	LONG refCount = InterlockedDecrement(&_refCount);
	if( refCount == 0 )
	{
		delete this;
		return 0;
	}
	else
		return refCount;
}

STDMETHODIMP CHttpDownloader::XMLHttpEventSink::GetTypeInfoCount(UINT *pctinfo)
{
	return E_NOTIMPL;
}

STDMETHODIMP CHttpDownloader::XMLHttpEventSink::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo)
{
	return E_NOTIMPL;
}

STDMETHODIMP CHttpDownloader::XMLHttpEventSink::GetIDsOfNames(const IID &riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId)
{
	return E_NOTIMPL;
}

STDMETHODIMP CHttpDownloader::XMLHttpEventSink::Invoke(DISPID dispIdMember, const IID &riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
	// Since this class isn't used for anything else, Invoke will get called only for onreadystatechange, and
	// dispIdMember will always be 0.

	long state;
	// Retrieve the state
	_request->get_readyState(&state);

	if (state == 4)
	{
		// The request has completed.
		// Get the request status.
		long status;
		_request->get_status(&status);

		if( status == 200 )
		{
			// Get the response body if we were successful.
			VARIANT varResponse;
			VariantInit( &varResponse );

			IStream* pStream = NULL;
			_request->get_responseStream( &varResponse );

			HRESULT hr = V_UNKNOWN(&varResponse)->QueryInterface(IID_IStream, reinterpret_cast<void**>(&pStream));

			if( hr == S_OK )
			{	
				// handle the stream
				if( mHttpCallback( pStream, mUserItem ) )
					SetEvent(_completedEvent);
			}

			VariantClear( &varResponse );

		}
		else
		{
			// Signal the main thread we're done.
			SetEvent(_completedEvent);
		}


	}
	return S_OK;
}

CHttpDownloader::CHttpDownloader()
: mState( StateNone )
{
	//IXMLHttpRequest* request;
	//CoCreateInstance(CLSID_XMLHTTPRequest, NULL, CLSCTX_INPROC, IID_IXMLHttpRequest, reinterpret_cast<void**>(&request));
	//mXmlHttpRequest = request;

	mXmlHttpRequest.CoCreateInstance( CLSID_XMLHTTPRequest, NULL, CLSCTX_INPROC );
}


CHttpDownloader::~CHttpDownloader()
{

}

bool CHttpDownloader::ProcessReceive( IStream* pStream, void* userItem )
{
	bool result = false;

	CHttpDownloader* pThis = (CHttpDownloader*)userItem;

	CComBSTR strLength = "Content-Length";
	CComBSTR strRange = "Range";
	CComBSTR strValue;

	pThis->mXmlHttpRequest->getResponseHeader( strLength, &( strValue.m_str ) );
	int length = atoi(CW2A(strValue));
	printf("Response Content Length: %d", length );

	pThis->mXmlHttpRequest->getResponseHeader( strRange, &( strValue.m_str ) );
	int offset = atoi(CW2A(strValue));
	printf("Response Range: %d", offset );

	if(pThis-> mState == StateGetHeader )
	{
		pThis->mTotalReceiveSize = 0;
		pThis->mFileSize = length;

		if( length <= MAX_BUFFSER_SIZE )
		{
			pThis->mState = StateGetData;
		
			// start another download
			pThis->startReceive( pThis->mUrl, "GET", pThis->mCompletHandle );
		}
		else
		{
			result = true;
		}
	}	
	else
	{
		if( pThis->mState == StateGetData )
		{
			char* buf = new char[MAX_BUFFSER_SIZE];

			pThis->mTotalReceiveSize += length;

			fseek( pThis->mFile, offset, SEEK_SET );
			ULONG bytesRead = 0;
			pStream->Read( buf, MAX_BUFFSER_SIZE, &bytesRead );

			if( fwrite( buf, 1, bytesRead, pThis->mFile ) != bytesRead )
				result = true;

			if( pThis->mFileSize == pThis->mTotalReceiveSize )
			{
				fclose( pThis->mFile );
				pThis->mState = StateNone;
				result = true;
			}

			delete[] buf;
		}
	}

	return result;
}


HANDLE CHttpDownloader::startDownload( std::string url, std::string fileName )
{
	if( mState != StateNone )
		return NULL;

	HANDLE completedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	mCompletHandle = completedEvent;
	mUrl = url;
	mState = StateGetHeader;

	mFileName = fileName;

	fopen_s( &mFile, fileName.c_str(), "wb+" );
	mTotalReceiveSize = 0;

	startReceive(url, "HEAD", completedEvent );

	return completedEvent;
}

void CHttpDownloader::startReceive( std::string &url, std::string method, HANDLE completedEvent )
{
	VARIANT vNULL;
	VariantInit(&vNULL);

	VARIANT bAsync;
	VariantInit(&bAsync);
	bAsync.vt = VT_BOOL;
	bAsync.boolVal = TRUE;

	CComBSTR strMethod = method.c_str();
	CComBSTR strUrl = url.c_str();

	mXmlHttpRequest->open( strMethod, strUrl, bAsync, vNULL, vNULL );
	XMLHttpEventSink* pEvent = new XMLHttpEventSink( mXmlHttpRequest, completedEvent, (HttpDownloaderCallback)ProcessReceive, this );
	mXmlHttpRequest->put_onreadystatechange( (IDispatch*)pEvent );
	mXmlHttpRequest->send( vNULL );

	VariantClear(&vNULL);
	VariantClear(&bAsync);

	pEvent->Release();
}

bool CHttpDownloader::isFinished()
{
	return mTotalReceiveSize == mFileSize;
}

std::string CHttpDownloader::getFileName()
{
	return mFileName;
}