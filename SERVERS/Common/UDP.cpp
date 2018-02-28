#include "UDP.h"
#include "common/DumpHelper.h"
#include "encrypt/AuthCrypt.h"
#include <process.h>
#include "commlib/ServerCloseMonitor.h"

using namespace std;

const static int MAX_TRY_COUNT = 1000000;

CUDPServer::CUDPServer(const char *ip,int nPort,int side)
{
	m_sdClient				= INVALID_SOCKET;
	m_MaxWorkBufferSize		= UDP_SERVER_WORK_BUFFER_SIZE;
	m_nPort					= nPort;
	strcpy_s(m_sIP, 64, ip);
	m_PendingNum			= 0;
	m_pOutServer			= NULL;
    m_side                  = (UDP_SIDE)side;

	m_SendCount				= 0;
	m_SendErr				= 0;
	m_RecvCount				= 0;
	m_RecvErr				= 0;
	m_seq					= 0;

	m_pMemPool				= CMemPool::GetInstance();
}

CUDPServer::~CUDPServer(void)
{
	Stop();
}

BOOL CUDPServer::ReleaseBuffer(UDP_OVERLAPPED_PLUS *pBuf)
{
	m_pMemPool->Free((U8 *)pBuf);
	return TRUE;
}

UDP_OVERLAPPED_PLUS* CUDPServer::GetBuffer(int nSize)
{
	UDP_OVERLAPPED_PLUS *lpOverlapped = (UDP_OVERLAPPED_PLUS*)m_pMemPool->Alloc(nSize+sizeof(UDP_OVERLAPPED_PLUS));

	if (0 == lpOverlapped)
		return 0;

	memset(lpOverlapped,0,nSize+sizeof(UDP_OVERLAPPED_PLUS));

	if(nSize)
		lpOverlapped->WsaBuf.buf = (char *)lpOverlapped+sizeof(UDP_OVERLAPPED_PLUS);
	else
		lpOverlapped->WsaBuf.buf = NULL;
	lpOverlapped->WsaBuf.len = nSize;

	lpOverlapped->WsaBuf.len	= nSize;
	lpOverlapped->bCtxWrite		= 0;

	lpOverlapped->addr_len		= sizeof(SOCKADDR_IN);

	return lpOverlapped;
}

void CUDPServer::PostEvent(int Msg,void *pData)
{
	if(m_sdClient == INVALID_SOCKET )
	{
		m_pMemPool->Free((MemPoolEntry)pData);
		return;
	}

	BOOL ret = PostQueuedCompletionStatus(m_CompletionPort,0,Msg,(LPOVERLAPPED)pData);
	if(!ret)
	{
		g_Log.WriteError("CUDPServer::PostEvent error=%d",GetLastError());
		m_pMemPool->Free((MemPoolEntry)pData);
	}
}

BOOL CUDPServer::Send(const char *pBuffer,int nSize,int ToIP,int ToPort)
{
	if (nSize == 0 || nSize > static_cast<int>(m_MaxWorkBufferSize)) {
		g_Log.WriteError("UDP SendError size=%d maxSize=%d",nSize,m_MaxWorkBufferSize);
		return FALSE;
	}

	if(m_sdClient==INVALID_SOCKET)
		return FALSE;

	UDP_OVERLAPPED_PLUS *pOverlapped = GetBuffer(nSize);
	pOverlapped->addr.sin_family	= AF_INET;
	pOverlapped->addr.sin_addr.s_addr	= ToIP;
	pOverlapped->addr.sin_port			= (USHORT)ToPort;
	memcpy(pOverlapped->WsaBuf.buf ,(char *)pBuffer,nSize);
	pOverlapped->bCtxWrite			= 1;

	//PostEvent(OP_SEND,pOverlapped);

	return BeginSend(pOverlapped);
}


BOOL CUDPServer::BeginSend(UDP_OVERLAPPED_PLUS *lpOverlapped)
{
#ifdef ENABLE_ENCRPY_PACKET
	if (UDP_SIDE_CLIENT == m_side)
		AuthCrypt::Instance()->EncryptSend((uint8*)const_cast<char*>(lpOverlapped->WsaBuf.buf),lpOverlapped->WsaBuf.len);
#endif


	DWORD dwBytesWritten = 0, dwFlags = 0;

	if(WSASendTo(m_sdClient, &lpOverlapped->WsaBuf, 1, &dwBytesWritten, dwFlags,
		(sockaddr*)&lpOverlapped->addr,sizeof(SOCKADDR_IN), lpOverlapped, NULL))
	{
		int nLastError = WSAGetLastError();
		if (WSA_IO_PENDING != nLastError)
		{
			std::stringstream LogStream;
			LogStream << "WSASendTo Socket:" << " ErrorCode=" << nLastError << std::ends;
			g_Log.WriteError(LogStream);

			ReleaseBuffer(lpOverlapped);

			m_SendErr++;
			return FALSE;
		}
	}

	m_SendCount++;

	return TRUE;
}

BOOL CUDPServer::Stop()
{
	if(m_sdClient == INVALID_SOCKET)
		return TRUE;

	for(int i=0;i<m_CompletionThreadNum;++i)
		PostQueuedCompletionStatus(m_CompletionPort,0,OP_QUIT,0);

	if(m_CompletionThreadNum)
		WaitForMultipleObjects(m_CompletionThreadNum,m_CompletionThread,TRUE,60000);

	if(closesocket(m_sdClient) == SOCKET_ERROR )
		SocketError("Disconnect: closesocket",WSAGetLastError());

	m_sdClient = INVALID_SOCKET;

	//关闭完成端口
	if(m_CompletionPort)
	{
		CloseHandle(m_CompletionPort);
		m_CompletionPort = NULL;
	}

	for(int i=0;i<m_CompletionThreadNum;i++)
	{
		if(m_CompletionThread[i])
		{
			CloseHandle(m_CompletionThread[i]);
			m_CompletionThread[i] = NULL;
		}
	}

	return 0;
}

void CUDPServer::SocketError(char *from,int error)
{
	stringstream LogStream;

	if(error!=0){
		LogStream << from << " --> " << error << ends;
		g_Log.WriteError(LogStream);
	}else{
		LogStream << from << ends;
		g_Log.WriteLog(LogStream);
	}
}

BOOL CUDPServer::HandleSend(UDP_OVERLAPPED_PLUS *lpOverlapped,int ByteReceived)
{
	return ReleaseBuffer(lpOverlapped);
}

BOOL CUDPServer::HandleReceive(UDP_OVERLAPPED_PLUS *lpOverlapped,int ByteReceived)
{
	if (m_PendingNum < MAX_PENDING_READ_NUM)
	{
		DWORD dwLastError = 0 , dwBytesRecvd = 0 ;
		DWORD dwFlags=0;
		DWORD dwTry = 0;
		
		do 
		{
			dwTry++;
			if ( WSARecvFrom(m_sdClient, &lpOverlapped->WsaBuf, 1, &dwBytesRecvd,	&dwFlags, (sockaddr*)&lpOverlapped->addr,&lpOverlapped->addr_len, lpOverlapped, NULL) )
			{
				dwLastError = WSAGetLastError();
				
				if (WSA_IO_PENDING == dwLastError || dwLastError==0)
					break;
				else
				{
					//std::stringstream LogStream;
					//LogStream << "WSARecvError Socket:" << " ErrorCode=" << dwLastError << std::ends;
					//g_Log.WriteError(LogStream);

					m_RecvErr++;
					//return TRUE;
				}
			}
			else
			{
				break;
			}
		}while(dwTry<MAX_TRY_COUNT);

		if(WSA_IO_PENDING != dwLastError && 0 != dwLastError && dwTry>=MAX_TRY_COUNT)
		{
			std::stringstream LogStream;
			LogStream << "WSARecvError Socket:" << " ErrorCode=" << dwLastError << std::ends;
			g_Log.WriteError(LogStream);
			ServerCloseMonitor::Instance()->Close();
		}

		m_PendingNum++;
		return FALSE;
	}

	m_RecvCount++;

	lpOverlapped->WsaBuf.buf[ByteReceived] = 0;
	OnReceive(lpOverlapped->WsaBuf.buf, ByteReceived,(SOCKADDR_IN *)&lpOverlapped->addr);
	m_PendingNum--;
	if(m_PendingNum<0)
	{
		m_PendingNum = 0;
		g_Log.WriteError("m_PendingNum 递减出错");
	}

	return TRUE;
}

BOOL CUDPServer::OnNetFail(int dwErrorCode,UDP_OVERLAPPED_PLUS *ov,int dwByteCount)
{
    if (0 == ov)
        return TRUE;

	if(dwErrorCode == WSA_OPERATION_ABORTED)
	{
		ReleaseBuffer(ov);
		return TRUE;
	}

    //如果是发送包直接丢弃
    if (ov->bCtxWrite)
    {
	    g_Log.WriteWarn("丢弃一个UDP数据包,size=%d", ov->WsaBuf.len );
	    ReleaseBuffer(ov);
    }
    else
    {
		m_PendingNum--;
		if(m_PendingNum<0)
		{
			m_PendingNum = 0;
			g_Log.WriteError("m_PendingNum 递减出错");
		}

		if (m_PendingNum < MAX_PENDING_READ_NUM)
		{
			 //重现发起接收请求
			 DWORD dwLastError = 0 , dwBytesRecvd = 0 ;
			 DWORD dwFlags=0;
			 DWORD dwTry = 0;
			 do 
			 {
				 dwTry++;
				 if ( WSARecvFrom(m_sdClient, &ov->WsaBuf, 1, &dwBytesRecvd,	&dwFlags, (sockaddr*)&ov->addr,&ov->addr_len, ov, NULL) )
				 {
					 dwLastError = WSAGetLastError();

					 if (WSA_IO_PENDING == dwLastError || dwLastError==0)
						 break;
					 else
					 {
						 //std::stringstream LogStream;
						 //LogStream << "WSARecvError Socket:" << " ErrorCode=" << dwLastError << std::ends;
						 //g_Log.WriteError(LogStream);

						 m_RecvErr++;
						 //return TRUE;
					 }
				 }
				 else
				 {
					 break;
				 }
			 }while(dwTry<MAX_TRY_COUNT);

			 if(WSA_IO_PENDING != dwLastError && 0 != dwLastError && dwTry>=MAX_TRY_COUNT)
			 {
				 std::stringstream LogStream;
				 LogStream << "WSARecvError Socket:" << " ErrorCode=" << dwLastError << std::ends;
				 g_Log.WriteError(LogStream);
				 ServerCloseMonitor::Instance()->Close();
			 }
			 m_PendingNum++;
		}

        return TRUE;
    }

	return FALSE;
}

BOOL CUDPServer::OnReceive(char *buf,int nByte,SOCKADDR_IN *addr)
{
	uRawAddress *pRawAddr = (uRawAddress *)(&buf[nByte-sizeof(uRawAddress)]);
	int switchPort = pRawAddr->port;
	int switchIp = pRawAddr->ip;
	pRawAddr->port = addr->sin_port;
	pRawAddr->ip = addr->sin_addr.s_addr;

//#ifdef DEBUG
//	int port = htons(addr->sin_port);
//	char *tAddr = inet_ntoa(addr->sin_addr);
//#endif

	return m_pOutServer->Send(buf,nByte,switchIp,switchPort);
}

BOOL CUDPServer::Start()
{
	if(!Create())
		return FALSE;

	m_CompletionPort = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, (ULONG_PTR)OP_NORMAL, 0);
	if(!m_CompletionPort)
	{
		closesocket(m_sdClient);
		m_sdClient = INVALID_SOCKET;
		return false;
	}

	SYSTEM_INFO		l_si;
	::GetSystemInfo( &l_si );
	m_CompletionThreadNum	= l_si.dwNumberOfProcessors * 2 + 1;
	if(m_CompletionThreadNum>MAX_COMPLETION_THREAD_NUM)
		m_CompletionThreadNum = MAX_COMPLETION_THREAD_NUM;	

	if(!CreateIoCompletionPort((HANDLE)m_sdClient,m_CompletionPort,(ULONG_PTR)OP_NORMAL, 0))
	{
		CloseHandle(m_CompletionPort);
		closesocket(m_sdClient);
		m_sdClient = INVALID_SOCKET;
		return false;
	}

	//创建完成线程
	unsigned int threadID;
	for (int i=0; i < m_CompletionThreadNum; i++ )
	{
		m_CompletionThread[i] = (HANDLE)_beginthreadex( NULL, 0, NetRoutine, this, 0, &threadID );
	}

	int nSize = GetMaxWorkBufferSize();

	for(int i=0;i<MAX_PENDING_READ_NUM;i++)
	{
		UDP_OVERLAPPED_PLUS* pBuf = CUDPServer::GetBuffer(nSize);
		pBuf->WsaBuf.buf = pBuf->WsaBuf.buf  + HEAD_RESERVE;
		pBuf->WsaBuf.len = nSize - HEAD_RESERVE;
		HandleReceive(pBuf,nSize);
	}

	return TRUE;
}

BOOL CUDPServer::Create()
{
	if (m_sdClient != INVALID_SOCKET)
	{
		SocketError("Create: m_sdClient 没有释放",0);
		closesocket(m_sdClient);
		m_sdClient = INVALID_SOCKET;
	}

	m_sdClient = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, NULL, WSA_FLAG_OVERLAPPED);
	if (m_sdClient == INVALID_SOCKET)
	{
		SocketError("Create: WSASocket = INVALID_SOCKET",WSAGetLastError());
		return FALSE;
	}

	DWORD in_buf=0;
	DWORD dwBytes;
	int TimeOut = 5000;
	if(WSAIoctl(m_sdClient, SIO_ENABLE_CIRCULAR_QUEUEING, &in_buf, sizeof(in_buf), NULL, 0, &dwBytes, NULL, NULL))
	{
		SocketError("Create: WSAIoctl",WSAGetLastError());
		return FALSE;
	}

	//in_buf = 0;
	//if(setsockopt(m_sdClient,SOL_SOCKET,SO_RCVBUF,(char *)&in_buf,sizeof(in_buf)))
	//{
	//	SocketError("Create: WSAIoctl",WSAGetLastError());
	//	return FALSE;
	//}
	//
	//if(setsockopt(m_sdClient,SOL_SOCKET,SO_SNDBUF,(char *)&in_buf,sizeof(in_buf)))
	//{
	//	SocketError("Create: WSAIoctl",WSAGetLastError());
	//	return FALSE;
	//}

	SOCKADDR_IN		addr;
	addr.sin_family = AF_INET;
	if(m_sIP[0])
		addr.sin_addr.s_addr = inet_addr(m_sIP);
	else
		addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(m_nPort);					//htons(Port+(short)m_SocketID);
	if (bind(m_sdClient, (SOCKADDR *)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		closesocket(m_sdClient);
		m_sdClient = INVALID_SOCKET;
		SocketError("Create: bind",WSAGetLastError());
		return FALSE;
	}

	return TRUE;
}

void CUDPServer::CheckTimeEclipse()
{
	UDP_OVERLAPPED_PLUS *pOverlapped = GetBuffer(sizeof(uRawAddress));
	uRawAddress *pParam = (uRawAddress *)pOverlapped->WsaBuf.buf;
	pParam->id = m_seq++;
	pParam->port = GetTickCount();

	PostEvent(OP_TIMETRACE,pOverlapped);
}

unsigned int CUDPServer::NetRoutine(LPVOID pParam)
{
	CUDPServer *pServer = (CUDPServer*)pParam;
	INT dwAddrSize=sizeof(SOCKADDR_IN);
	DWORD dwFlags=0,nByte=0;

	UDP_OVERLAPPED_PLUS *ov=NULL;
	DWORD dwByteCountBak=0,dwByteCount=0,dwErrorCodeBak=0,dwErrorCode=0;
	int opCode = OP_NORMAL;
	char WriteFlag = 0;
	DWORD rf = 0;

	CMemPool *pMemPool		= CMemPool::GetInstance();

	while(1)
	{
		dwErrorCodeBak = dwByteCountBak = opCode = 0;
		ov = NULL;
		if(!::GetQueuedCompletionStatus(pServer->m_CompletionPort, &dwByteCountBak, (ULONG_PTR *)&opCode,(LPOVERLAPPED*)&ov, INFINITE ))
		{
			dwErrorCodeBak = WSAGetLastError();
		}

		if(opCode == OP_QUIT)
			break;

		if(opCode != OP_TIMETRACE)
		{
			if (ov == NULL)
				continue;
		}

		switch(opCode)
		{
		case OP_TIMETRACE:
			{
				uRawAddress *pParam = (uRawAddress *)ov->WsaBuf.buf;
				int eclapse = GetTickCount() - pParam->port;
				if(eclapse)  //Ray: 0就不打了
					g_Log.WriteLog("Id=%d [UDP] 处理速度 %d tick",pParam->id,eclapse);
				break;
			}
			break;

		case OP_SEND:
			pServer->BeginSend(ov);
			break;

		case OP_NORMAL:
			{
				dwErrorCode = 0;
				if(!WSAGetOverlappedResult(pServer->m_sdClient,ov,&dwByteCount,TRUE,&rf))
					dwErrorCode = WSAGetLastError();

				WriteFlag = ov->bCtxWrite;

				if (!dwErrorCode)
				{
					if (dwByteCount)
					{
						if (WriteFlag)
						{
							pServer->HandleSend(ov,dwByteCount);
						}
						else
						{
							while (pServer->HandleReceive(ov,dwByteCount));
						}
					}
					else
					{
						pServer->OnNetFail(dwErrorCode,ov,dwByteCount);
					}
				}
				else
				{
					pServer->OnNetFail(dwErrorCode,ov,dwByteCount);
				}
			}
			break;

		default:
			g_Log.WriteError("无效 opCode=%d",opCode);
			break;
		}
	}

	g_Log.WriteLog( "UDP Service terminated");

	return 0;
}

