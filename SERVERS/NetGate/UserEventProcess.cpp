#include "DBLib/dbLib.h"
#include "WinTCP/IPacket.h"
#include "WinTCP/dtServerSocket.h"
#include "Common/MemGuard.h"
#include "base/WorkQueue.h"
#include "Common/PacketType.h"
#include "Common/CommonClient.h"

#include "NetGate.h"
#include "UserEventProcess.h"
#include "PlayerMgr.h"

#include "Common/UDP.h"
#include "encrypt/AuthCrypt.h"
#include "EventProcess.h"

using namespace std;

static hash_map<unsigned long, bool> s_clientMsgFilters;
static bool s_clientMsgFilterInit = false;

#define C_MSG(x) s_clientMsgFilters[x] = true;

bool IsValidClientMsg(unsigned long msg)
{
    if (!s_clientMsgFilterInit)
    {  
		C_MSG(GetMessageCodeSimple("GM_MESSAGE""))"));                      
		C_MSG(GetMessageCodeSimple("CLIENT_WORLD_SelectPlayerRequest"));    
		C_MSG(GetMessageCodeSimple("CLIENT_WORLD_DeletePlayerRequest"));    
		C_MSG(GetMessageCodeSimple("CLIENT_WORLD_CreatePlayerRequest"));    
		C_MSG(GetMessageCodeSimple("CLIENT_WORLD_AutoPlayerNameRequest"));  
		C_MSG(GetMessageCodeSimple("CLIENT_GAME_LoginRequest"));            
		C_MSG(GetMessageCodeSimple("CLIENT_GAME_LogoutRequest"));           
		C_MSG(GetMessageCodeSimple("CLIENT_Bind_AccountRequest"));          
		C_MSG(GetMessageCodeSimple("CW_GETLINEPLAYERCOUNT"));               
		C_MSG(GetMessageCodeSimple("Alipay_Charge"));                                                
		C_MSG(GetMessageCodeSimple("CW_MailListRequest"));                  
		C_MSG(GetMessageCodeSimple("CW_ReadMailRequest"));    
		C_MSG(GetMessageCodeSimple("CW_GetMailItemRequest"));  

		C_MSG(GetMessageCodeSimple("CW_MakeLinkRequest"));                  
		C_MSG(GetMessageCodeSimple("CW_SocialInfoRequest"));                
		C_MSG(GetMessageCodeSimple("CW_DestoryLinkRequest"));               
		C_MSG(GetMessageCodeSimple("CW_RequestRankSeriesInfoEx"));          
		C_MSG(GetMessageCodeSimple("CR_PURCHASE_REQUEST"));                
		C_MSG(GetMessageCodeSimple("CW_VERIFY_PLAYER_FAKE_REQUEST"));    

		C_MSG(GetMessageCodeSimple("CLIENT_WORLD_ReadyRequest"));
		C_MSG(GetMessageCodeSimple("CLIENT_SERVER_SendCards"));
		C_MSG(GetMessageCodeSimple("CW_CreateChessTable"));
		C_MSG(GetMessageCodeSimple("CW_JoinChessTable"));
		C_MSG(GetMessageCodeSimple("CW_LeaveChessTable"));
		C_MSG(GetMessageCodeSimple("CW_DisbandChessTable"));
		C_MSG(GetMessageCodeSimple("CW_UserOutCard"));
		C_MSG(GetMessageCodeSimple("CW_UserOperateCard"));
		C_MSG(GetMessageCodeSimple("CW_ReadyChessTable"));
		C_MSG(GetMessageCodeSimple("CW_RestartChessTable"));
		C_MSG(GetMessageCodeSimple("CW_ITEM_USE_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_ITEM_COMPOSE_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_ITEM_BUY_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_ITEM_USEACCCARD_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_ITEM_USEBATTLECARD_REQUEST"));
		//-----------------------CARD-----------------------//
		C_MSG(GetMessageCodeSimple("CW_CARD_ADDLEVEL_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_CARD_UPLVLIMIT_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_CARD_ADDSKILL_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_CARD_ADDEQUIP_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_CARD_REMOVEEQUIP_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_CARD_ADDABILITY_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_CARD_ADDGROW_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_CARD_HIRE_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_CARD_DISPATCH_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_READ_APPLY_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_ADD_CARD_OFFICALRANK_REQUEST"));
		
		C_MSG(GetMessageCodeSimple("CW_BATTLE_INFO_REQUEST"));

		C_MSG(GetMessageCodeSimple("CW_BATTLE_INFO_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_BATTLE_COMBAT_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_BATTLE_COMBAT_EX_REQUEST"));	

		C_MSG(GetMessageCodeSimple("CW_IMPOSE_REQUEST"));

		C_MSG(GetMessageCodeSimple("CW_DESIGNATE_OFFICER_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_REVOKE_OFFICER_REQUEST"));

		C_MSG(GetMessageCodeSimple("CW_SUPPORT_PRINCESS_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_AWARD_PRINCESS_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_TURNCARD_PRINCESS_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_CHOOSE_PRINCESS_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_BESTOWED_PRINCESS_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_DISPATCH_PRINCESS_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_LEARNRITE_PRINCESS_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_PLANTFLOWER_PRINCESS_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_FINISH_PRINCESS_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_ADDMAXSLOT_PRINCESS_REQUEST"));

		C_MSG(GetMessageCodeSimple("CW_ADVANCETIME_REQUEST"));

		C_MSG(GetMessageCodeSimple("CW_OPEN_PVE_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_PVE_READY_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_PVE_BEGIN_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_PVE_AVENGE_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_PVE_BATTLE_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_PVE_END_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_PVE_SETJUMPBATTLE_REQUEST"));
		
		C_MSG(GetMessageCodeSimple("CW_TECHNOLOGY_LEVELUP_REQUEST"));

		C_MSG(GetMessageCodeSimple("CW_PRICESS_ADDACTIVITY_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_PRICESS_TRAIN_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_PRICESS_RENAME_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_PRICESS_OPENHOURSE_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_PRICESS_AWARDITEM_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_PRICESS_DELETE_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_PRICESS_ADDMAXSLOT_REQUEST"));
		
		C_MSG(GetMessageCodeSimple("CW_OPENTOPPVE_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_OPENTOPPVEWIN_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_PVE_ADDBUFF_REQUEST"));

		C_MSG(GetMessageCodeSimple("CW_ROLLDICE_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_ROLLFIXEDDICE_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_DEVELOPCITY_REQUEST"));

		C_MSG(GetMessageCodeSimple("CW_FINISHTIME_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_SETFACE_REQUEST"));

		C_MSG(GetMessageCodeSimple("CW_OPEN_DINNER_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_JOIN_DINNER_REQUEST"));
		
		C_MSG(GetMessageCodeSimple("CW_BUY_RANDOMSHOP_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_OPEN_RANDOMSHOP_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_REFLASH_RANDOMSHOP_REQUEST"));

		C_MSG(GetMessageCodeSimple("CW_DRAWITEM_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_EquipStrengthen_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_DISPATCH_CARD_REQUEST"));

		C_MSG(GetMessageCodeSimple("CW_OPENTOP_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_UPTOP_REQUEST"));

		C_MSG(GetMessageCodeSimple("CW_SETPLAYERFLAG_REQUEST"));
		C_MSG(GetMessageCodeSimple("CW_ACTIVITY_Request"));
		C_MSG(GetMessageCodeSimple("CW_DATANGUAN_Request"));

		//--------------------org-------------------------//
		C_MSG(GetMessageCodeSimple("CW_CreateOrg_Request"));
		C_MSG(GetMessageCodeSimple("CW_ChangeOrg_Request"));
		C_MSG(GetMessageCodeSimple("CW_ApplyOrg_Request"));
		C_MSG(GetMessageCodeSimple("CW_JoinOrg_Request"));
		C_MSG(GetMessageCodeSimple("CW_LeaveOrg_Request"));
		C_MSG(GetMessageCodeSimple("CW_FireMemberOrg_Request"));
		C_MSG(GetMessageCodeSimple("CW_DonateOrg_Request"));
		C_MSG(GetMessageCodeSimple("CW_SetOrgMasterLevel_Request"));
		C_MSG(GetMessageCodeSimple("CW_TransferOrgMaster_Request"));
		C_MSG(GetMessageCodeSimple("CW_LeaveUpSkill_Request"));
		C_MSG(GetMessageCodeSimple("CW_DisbandOrgl_Request"));
		C_MSG(GetMessageCodeSimple("CW_SearchTopOrg_Request"));
		C_MSG(GetMessageCodeSimple("CW_SearchOrgSimple_Request"));
		C_MSG(GetMessageCodeSimple("CW_SearchOrg_Request"));
		C_MSG(GetMessageCodeSimple("CW_BuyOrgItem_Request"));
		C_MSG(GetMessageCodeSimple("CW_BuyOrgGift_Request"));
		C_MSG(GetMessageCodeSimple("CW_GetOrgGift_Request"));

		//----------------------BOSS-----------------------//
		C_MSG(GetMessageCodeSimple("CW_Boss_OpenOrg_Request"));
		C_MSG(GetMessageCodeSimple("CW_Boss_BuyBuff_Request"));
		C_MSG(GetMessageCodeSimple("CW_BossInfo_Request"));
		C_MSG(GetMessageCodeSimple("CW_Boss_Battle_Request"));

		//---------------------vip-------------------------//
		C_MSG(GetMessageCodeSimple("CW_VIP_GIFT_REQUEST"));
		s_clientMsgFilterInit = true;
    }


    return s_clientMsgFilters[msg];
}

//监控参数
stdext::hash_map<U32, U32> g_UserEventNetFNListTotalFreq;
static  MessageCode s_MessageCode;

int SERVER_CLASS_NAME::UserEventProcess(LPVOID Param)
{
	WorkQueueItemStruct *pItem = (WorkQueueItemStruct*)Param;

	int nId;

	switch(pItem->opCode)
	{
	case WQ_CONNECT:
		g_Log.WriteLog( "收到一个客户端连接 Socket:%d IP:%s ",pItem->Id , pItem->Buffer );
		break;
	case WQ_DISCONNECT:
        {
            // 此处通知World该客户端已断开
            T_UID uid = 0;
			nId = SERVER->GetPlayerManager()->GetSocketAccount(pItem->Id, uid);

            if(nId)
                UserEventFn::NotifyWorldClientLost(nId, uid);

			g_Log.WriteLog( "disconnect incoming Socket:%d Account:%d UID:%d ",pItem->Id, nId,uid );

            // 相关清理
            SERVER->GetPlayerManager()->ReleaseSocketMap(pItem->Id);
        }
		break;
	case WQ_PACKET:
		{
			bool restartFlag = false;

			stPacketHead* pHead = (stPacketHead*)pItem->Buffer;

			{
//                InterlockedIncrement((volatile LONG *)&g_UserEventNetFNListTotalFreq[pHead->Message]);
				g_UserEventNetFNListTotalFreq[pHead->Message]++;

				if(pHead->DestServerType==SERVICE_WORLDSERVER || pHead->DestServerType==SERVICE_REMOTESERVER)
				{
                    //检测消息是否合法
                    if (!IsValidClientMsg(pHead->Message))
                    {
                        g_Log.WriteError("非法的客户端消息[%s],socketid[%d]",SHOWMSG_EVENT(pHead->Message),pItem->Id);
                        //assert(0);
                        return false;
                    }

					Base::BitStream switchPacket(pItem->Buffer,pItem->size);
					switchPacket.setPosition(pItem->size);
					UserEventFn::SwitchSendToWorld(pItem->Id, pHead, switchPacket);
				}
				else if( pHead->DestServerType == SERVICE_CHATSERVER)
				{
					Base::BitStream switchPacket(pItem->Buffer,pItem->size);
					switchPacket.setPosition(pItem->size);
					UserEventFn::SwitchSendToChat(pItem->Id, pHead, switchPacket);
				}
				else if(s_MessageCode.IsValid(pHead->Message))
				{
					char *pData	= (char *)(pHead) + IPacket::GetHeadSize();
					Base::BitStream RecvPacket(pData,pItem->size-IPacket::GetHeadSize());
					restartFlag = !s_MessageCode.Trigger(pItem->Id,pHead,RecvPacket);
				}
			}
			/*else
			{
				restartFlag = true;
			}*/

			if(restartFlag)
			{
				g_Log.WriteWarn("收到一条未处理的消息，断开这个连接 MsgId:%s Socket:%d",SHOWMSG_EVENT(pHead->Message),pItem->Id);
				SERVER->GetUserSocket()->PostEvent(dtServerSocket::OP_RESTART,pItem->Id);
			}
		}
		break;
	}

	return false;
}

void DumpUserRecvPackets(void)
{
    for (stdext::hash_map<U32, U32>::iterator itr = g_UserEventNetFNListTotalFreq.begin(); itr != g_UserEventNetFNListTotalFreq.end(); ++itr)
    {
        if (0 == itr->second)
            continue;

        g_Log.WriteLog("收到客户端消息[%s] 次数[%d]",SHOWMSG_EVENT(itr->first),itr->second);

        itr->second = 0;
    }
}

#include <assert.h>

namespace UserEventFn
{
	void Initialize()
	{
		s_MessageCode.Register("CLIENT_GATE_LoginRequest", &UserEventFn::HandleClientLoginRequest);
		s_MessageCode.Register("CLIENT_GATE_LogoutRequest", &UserEventFn::HandleClientLogoutRequest);;	
	}

	bool CheckClient(int sockId, stPacketHead *pHead)
	{
		T_UID uid = 0;
		int account = SERVER->GetPlayerManager()->GetSocketAccount(sockId, uid);
		if (!account || account != pHead->Id)
		{
			g_Log.WriteError("Old socket communication or viciousness[%d].", sockId);
			return false;
		}
		return true;
	}

	void SwitchSendToWorld(int sockId, stPacketHead *pHead,Base::BitStream &switchPacket,int ctrlType)
	{
		if (CheckClient(sockId, pHead))
			SERVER->GetWorldSocket()->Send(switchPacket,ctrlType);
	}

	void SwitchSendToChat(int sockId, stPacketHead *pHead,Base::BitStream &switchPacket,int ctrlType)
	{
		//if (CheckClient(sockId, pHead))
		SERVER->GetChatSocket()->Send(switchPacket,ctrlType);
	}

	void NotifyWorldClientLost( int nPlayerId,T_UID uid)
	{
		CMemGuard buf( 40 MEM_GUARD_PARAM);
		Base::BitStream SendPacket( buf.get(), 40 );
		stPacketHead* pHead = IPacket::BuildPacketHead( SendPacket, "GATE_WORLD_ClientLost", nPlayerId, SERVICE_WORLDSERVER );
        SendPacket.writeInt(uid, Base::Bit32);
		pHead->PacketSize = SendPacket.getPosition() - sizeof( stPacketHead );
		SERVER->GetWorldSocket()->Send( SendPacket );
	}

	bool HandleClientLogoutRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
	{
		ERROR_CODE Error = NONE_ERROR;

		int AccountId	  = pHead->Id;
		T_UID UID		  = Packet->readInt(UID_BITS);

		g_Log.WriteLog( "logout Socket:%d Account:%d UID:%d ",SocketHandle, AccountId,UID );

		char buf[64];
		Base::BitStream SendPacket(buf,sizeof(buf));
		stPacketHead *pSendHead = IPacket::BuildPacketHead(SendPacket,"GATE_WORLD_ClientLogoutRequest",AccountId);
		SendPacket.writeInt(UID,UID_BITS);
		pSendHead->PacketSize = SendPacket.getPosition()-IPacket::GetHeadSize();
		SERVER->GetWorldSocket()->Send(SendPacket);


		//尽快让客户端退出，有慢的可能退出不容易哦。。。。
		Base::BitStream retPacket(buf,sizeof(buf));
		pSendHead = IPacket::BuildPacketHead(retPacket,"CLIENT_GATE_LogoutResponse");
		pSendHead->PacketSize = retPacket.getPosition()-IPacket::GetHeadSize();
		SERVER->GetUserSocket()->Send(SocketHandle,retPacket);

		return true;
	}

	bool HandleClientLoginRequest(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet)
	{
		ERROR_CODE Error = NONE_ERROR;

		int AccountId	  = pHead->Id;
		int NetVersion    = Packet->readInt(Base::Bit32);
		T_UID UID		  = Packet->readInt(UID_BITS);
        bool isLineSwitch = Packet->readFlag();

		g_Log.WriteLog( "login incoming  Socket:%d Account:%d UID:%d ",SocketHandle, AccountId,UID );

		if(NetVersion != NETWORK_PROTOCOL_VERSION)
			Error = VERSION_ERROR;

		if(Error == NONE_ERROR)
		{
			Error = SERVER->GetPlayerManager()->AddAccountMap(SocketHandle,UID,AccountId);
			if(Error == NONE_ERROR)
			{
				CMemGuard Buffer(64 MEM_GUARD_PARAM);
				Base::BitStream SendPacket(Buffer.get(),64);
				stPacketHead *pSendHead = IPacket::BuildPacketHead(SendPacket,"GATE_WORLD_ClientLoginRequest",AccountId);
				SendPacket.writeInt(UID,UID_BITS);
                SendPacket.writeFlag(isLineSwitch);
				pSendHead->PacketSize = SendPacket.getPosition()-IPacket::GetHeadSize();
				SERVER->GetWorldSocket()->Send(SendPacket);
				return true;
			}
			else
			{
				g_Log.WriteLog( "Account UID Error: %d-%d", AccountId, UID );
			}
		}
		
		//assert(0);
		
		CMemGuard Buffer(64 MEM_GUARD_PARAM);
		Base::BitStream SendPacket(Buffer.get(),64);
		stPacketHead *pSendHead = IPacket::BuildPacketHead(SendPacket,"CLIENT_GATE_LoginResponse");
		SendPacket.writeInt(Error,Base::Bit16);
		pSendHead->PacketSize = SendPacket.getPosition()-IPacket::GetHeadSize();
		SERVER->GetUserSocket()->Send(SocketHandle,SendPacket);
		return true;
	}
}

