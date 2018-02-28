#ifndef USER_PACKET_PROCESS_H
#define USER_PACKET_PROCESS_H

#include "Common/CommonPacket.h"
#include "WorldServer.h"
#include "SocialManager.h"

using namespace Base;

class UserPacketProcess : public CommonPacket
{
	typedef CommonPacket Parent;

protected:
	void HandleClientLogin();
	void HandleClientLogout();
	bool HandleGamePacket(stPacketHead *pHead,int DataSize);
};

_inline void UserPacketProcess::HandleClientLogin()
{
	char IP[COMMON_STRING_LENGTH]="";
	char *pAddress = dynamic_cast<dtServerSocketClient *>(m_pSocket)->GetConnectedIP();
	if(pAddress)
		sStrcpy(IP,COMMON_STRING_LENGTH,pAddress,COMMON_STRING_LENGTH);

	SERVER->GetWorkQueue()->PostEvent(m_pSocket->GetClientId(),IP,sStrlen(IP,COMMON_STRING_LENGTH)+1,true,WQ_CONNECT);

	Parent::HandleClientLogin();
}

_inline void UserPacketProcess::HandleClientLogout()
{
	SERVER->GetWorkQueue()->PostEvent(m_pSocket->GetClientId(),NULL,0,false,WQ_DISCONNECT);
	Parent::HandleClientLogout();
}

_inline bool UserPacketProcess::HandleGamePacket(stPacketHead *pHead,int iSize)
{
	//if (pHead->Message>MSG_ORG_BEGIN && pHead->Message<MSG_ORG_END)
	//	SERVER->GetOrgWorkQueue()->PostEvent(m_pSocket->GetClientId(), pHead, iSize, true);
	if (SERVER->GetSocialManager()->IsValid(pHead->Message))
		SERVER->GetSocialWorkQueue()->PostEvent(m_pSocket->GetClientId(), pHead, iSize, true);
	//else if (pHead->Message == GM_MESSAGE)
    //    SERVER->GetGMWorkQueue()->PostEvent(m_pSocket->GetClientId(), pHead, iSize, true);
    else
		SERVER->GetWorkQueue()->PostEvent(m_pSocket->GetClientId(), pHead, iSize, true);

	return true;
}

#endif
