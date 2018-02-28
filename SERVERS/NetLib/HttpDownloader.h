#ifndef __HTTP_DOWNLAODER__
#define __HTTP_DOWNLAODER__


#ifndef interface
#define interface struct
#endif

#include <WinSock2.h>
#include <msxml.h>
#include <windows.h>
#include <atlcomcli.h>

#include <string>
#include <stdio.h>


namespace CHttpDownloaderHelper
{
	struct  Helper
	{
		Helper();
		virtual ~Helper();
	};
}

typedef bool (*HttpDownloaderCallback)( IStream* pStream, void* userItem );

class  CHttpDownloader
{
public:
	CHttpDownloader();
	virtual ~CHttpDownloader();

public:
	HANDLE startDownload( std::string url, std::string fileName );

	void startReceive( std::string &url, std::string method,HANDLE completedEvent );
	HANDLE startReceive( std::string url );

	std::string getFileName();
	bool isFinished();

private:
	class XMLHttpEventSink : public IDispatch
	{
	public:
		XMLHttpEventSink(IXMLHttpRequest* request, HANDLE completedEvent, HttpDownloaderCallback httpCallback, void* userItem ) : _refCount(1), _request(request), _completedEvent(completedEvent), mHttpCallback(httpCallback), mUserItem( userItem )
		{ 
			// Don't increase the reference count to the request object; doing so would create a circular reference
			// and thus a memory leak.
		}
		virtual ~XMLHttpEventSink() { }

		// IUnknown 
		STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject);
		STDMETHODIMP_(ULONG) AddRef();
		STDMETHODIMP_(ULONG) Release();

		// IDispatch
		STDMETHODIMP GetTypeInfoCount(UINT *pctinfo);
		STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo);
		STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId);
		STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);
	private:
		LONG _refCount;
		IXMLHttpRequest* _request;
		HANDLE _completedEvent;
		HttpDownloaderCallback mHttpCallback;
		void* mUserItem;
	};
private:
	enum 
	{
		StateNone,
		StateGetHeader,
		StateGetData,
	};

private:
	CComPtr<IXMLHttpRequest> mXmlHttpRequest; 

	int mState;
	FILE* mFile;
	std::string mFileName;
	int mTotalReceiveSize;
	int mFileSize;
	std::string mUrl;
	HANDLE mCompletHandle;

	static const int MAX_BUFFSER_SIZE = 1024 * 1024;

	static CHttpDownloaderHelper::Helper _httpDownloaderHelper;

	static bool ProcessReceive( IStream* pStream, void* userItem );
} ;

#endif