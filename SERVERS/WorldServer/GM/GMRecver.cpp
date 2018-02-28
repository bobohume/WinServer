#include "DBLib/dbLib.h"
#include "GMRecver.h"
#include "..\WorldServer.h"
#include "Common\CommonPacket.h"
#include "wintcp\dtServerSocket.h"
#include "..\WorldServer.h"
#include <iostream>
#include "..\PlayerMgr.h"
#include "WINTCP\MessageCode.h"
#include "DBLayer\Data\TBLPlayer.h"
#include "data/TBLBase.h"

CMyLog g_gmLog;

CGMRecver::CGMRecver(void)
{
    m_pDBConn = SERVER_CLASS_NAME::GetActorDB()->GetPool()->Lock();
	REGISTER_EVENT_METHOD("CW_GM_MESSAGE",  this, &CGMRecver::HandleCommand);
}

CGMRecver::~CGMRecver(void)
{
}

bool CGMRecver::Initialize()
{
	return true;
}

void CGMRecver::Shutdown(void)
{
    if (0 != m_pDBConn)
    {
        SERVER_CLASS_NAME::GetActorDB()->GetPool()->Unlock(m_pDBConn);
        m_pDBConn = 0;
    }
}

bool CGMRecver::HandleCommand(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet)
{
	U32 PlayerId = pHead->Id;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(PlayerId);
	if(pAccount && pAccount->IsGM())
	{
		U32 nPlayerId = Packet->readInt(Base::Bit32);
		U32 nCard	  = Packet->readInt(Base::Bit32);
		U32 fPrice    = Packet->readInt(Base::Bit32);

		int error = AddGold(nPlayerId, nCard, fPrice);
		CMemGuard Buffer(64 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(),64);
		stPacketHead *pSendHead = IPacket::BuildPacketHead(sendPacket,"CLIENT_GATE_LoginResponse");
		sendPacket.writeInt(error,Base::Bit16);
		pSendHead->PacketSize = sendPacket.getPosition()-IPacket::GetHeadSize();
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

const char* CGMRecver::Format(const char* fmt,...)
{
	static char buffer[4096];

	va_list args;
	va_start(args,fmt);
	_vsnprintf_s(buffer,sizeof(buffer),sizeof(buffer),fmt,(char*)args);

	return buffer;
}

U32 CGMRecver::AddGold(int PlayerId, S32 Gold, F32 Price)
{
	int retCode = 1;
	PlayerSimpleDataRef spd = SERVER->GetPlayerManager()->GetPlayerData(PlayerId);
	if(spd)
	{
		spd->Gold = mClampEx(spd->Gold + Gold, 0, U32_MAX);
		M_SQL(m_pDBConn, "EXEC Sp_PlayerPurchase  %d, %d, %f", PlayerId, Gold, Price);
		if(m_pDBConn->More())
		{
			retCode = m_pDBConn->GetInt();
		}
	}

	return retCode;
}