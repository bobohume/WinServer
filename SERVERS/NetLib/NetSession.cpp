#include "stdafx.h"
#include "NetSession.h"

int CNetSession::ms_id = 0;

CNetSession::CNetSession()
{
	m_dwId = ++ms_id;
	LastTickTime = 0;
}

CNetSession::~CNetSession()
{
	//delete[] (void*)m_dwId;
}

CNetSession::ID_HANDLE CNetSession::getId()
{
	return m_dwId;
}

SOCKADDR_IN* CNetSession::getAddress()
{
	return &mSockaddr;
}