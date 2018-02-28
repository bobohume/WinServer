#ifdef NTJ_CLIENT
#include "platform/types.h"
#include "platform/platformNet.h"
#include "gameplay/gameevents/ClientGameNetEvents.h"
#include "gameplay/GameplayCommon.h"
#include "gameplay/data/MsgData.h"
#include "Util/LocalString.h"
#endif

#include "ClientErrorCode.h"
#include "Common/PacketType.h"
#include "wintcp/IPacket.h"



void ClientErrorCode::pack(void)
{
    IPacket::BuildPacketHead(m_sendPacket,WC_ERRORCODE,0,SERVICE_CLIENT);

    //ÆÁÏÔ×Ö·û´®
    m_sendPacket.writeInt(mMsgID,MSG_BIT16);
    m_sendPacket.writeInt(mParNum,Base::Bit8);

    for(U8 i=0; i<mParNum; ++i)
    {
        m_sendPacket.writeInt(mParType[i],Base::Bit8);
        mAny[i].pHold->pack(&m_sendPacket);
    }
}

bool ClientErrorCode::unpack(Base::BitStream* stream) 
{
	// ÆÁÏÔ×Ö·û´®
	mMsgID   = stream->readInt(MSG_BIT16);
	mParNum  = stream->readInt(Base::Bit8);

	for(U8 i = 0; i < mParNum; ++i)
	{
		mParType[i] = stream->readInt(Base::Bit8);
		mAny[i].setType(mParType[i]);

		if(0 == mAny[i].pHold)
        {
            return false;
        }

		mAny[i].pHold->unpack(stream);
	}

    return true;
}

void ClientErrorCode::process(Base::BitStream& SendPacket)
{
#ifdef NTJ_CLIENT

    if (!unpack(&SendPacket))
        return;

    if (MSG_NONE == mMsgID)
        return;

    MsgData* pData = g_MsgDataRepository.getMsgData(mMsgID);

    int mMsgType = pData ? pData->uShowType : SHOWTYPE_WARN;
    int mShowPos = pData ? pData->uShowPos  : SHOWPOS_SCRANDCHAT;

    if (mParNum > 0)
    {
        static char s_szBufFormat[1024] = "";
        dSprintf(s_szBufFormat,sizeof(s_szBufFormat),GetLocalStr(mMsgID),GETVAULE(mAny[0]),GETVAULE(mAny[1]),GETVAULE(mAny[2]),GETVAULE(mAny[3]));
        MessageEvent::show(mMsgType, s_szBufFormat, mShowPos);
    }
    else
    {
        MessageEvent::show(mMsgType, GetLocalStr(mMsgID), mShowPos);
    }

#endif	
}