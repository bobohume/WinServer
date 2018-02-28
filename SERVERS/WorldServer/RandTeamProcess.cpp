//#include "RandTeamProcess.h"
//
//bool RandTeamFailProcess::Process(RandTeam* pRandTeam)
//{
//    if (0 == pRandTeam)
//		return false;
//
//    U32 memberCount = pRandTeam->GetCount();
//
//    if (0 == memberCount)
//    {
//        pRandTeam->SetProcess(0);
//    }
//    //else
//    //{
//    //    //继续加入排队列表
//    //    pRandTeam->SetProcess();
//    //}
//}
//
//bool RandTeamAckProcess::PreProcess(RandTeam* pRandTeam)
//{
//    if (0 == pRandTeam)
//        return false;
//
//    //向队员发起随机组队确认
//    char buf[256];
//    Base::BitStream SendPacket(buf,sizeof(buf));
//
//    stPacketHead* pHead = IPacket::BuildPacketHead(SendPacket,WZ_RANDTEAM_TEAMINVITE,0,SERVICE_ZONESERVER);
//
//    SendPacket.writeInt(pRandTeam->GetMapId(),Base::Bit32);
//    SendPacket.writeInt(pRandTeam->GetMapLevel(),Base::Bit32);
//
//    pHead->PacketSize = SendPacket.getPosition() - sizeof(stPacketHead);
//    pRandTeam->Broadcast(pHead,SendPacket);
//
//    return true;
//}
//
//bool RandTeamAckProcess::Process(RandTeam* pRandTeam)
//{
//	if (0 == pRandTeam)
//		return false;
//
//	U32 readyCount = 0;
//	
//	for (U32 i = 0; i < TEAM_MEMBER_MAX; ++i)
//	{
//		RandTeamPlayer& member = pRandTeam->GetMember(i);
//		
//		if (!member.IsValid())
//			continue;
//
//		//如果有队员拒绝,则组队失败
//		if (RandTeamPlayer::PIS_REJECT == member.GetStatus())
//		{
//			pRandTeam->SetProcess(SERVER->GetRandTeamMgr()->GetFailProcess());
//			return true;
//		}
//
//		//向队员发送确认请求
//		if (RandTeamPlayer::PIS_NORMAL == member.GetStatus())
//		{
//			
//		}
//
//		if (RandTeamPlayer::PIS_ACCEPT == member.GetStatus())
//		{
//			readyCount++;
//		}
//	}
//
//	//所有玩家都已经确认
//	if (readyCount == pRandTeam->GetCount())
//	{
//		//判断角色是否分布正确
//		U32 nDefenders = pRandTeam->GetCharCount(COPYMAP_ACTOR_DEFENCE);
//		U32 nHealers   = pRandTeam->GetCharCount(COPYMAP_ACTOR_DOCTOR);
//		
//		if (nDefenders < pRandTeam->GetCharMaxCount(COPYMAP_ACTOR_DEFENCE) || nHealers < pRandTeam->GetCharMaxCount(COPYMAP_ACTOR_DOCTOR))
//		{
//			//缺少必要的防御者和治疗者
//			
//
//			pRandTeam->SetProcess(SERVER->GetRandTeamMgr()->GetFailProcess());
//		}
//		else
//		{
//			
//			//加入到排队列表
//			pRandTeam->SetProcess(SERVER->GetRandTeamMgr()->GetWaitProcess(),3600);
//		}
//
//		return true;
//	}
//
//	//超时,组队失败
//	if (pRandTeam->IsExpire())
//	{
//		pRandTeam->SetProcess(SERVER->GetRandTeamMgr()->GetFailProcess(),60);
//	}
//
//	return true;
//}
//
//bool RandTeamWaitProcess::Process(RandTeam* pRandTeam)
//{
//	if (0 == pRandTeam)
//		return false;
//
//	RandTeamPlayer& starter = pRandTeam->GetMember(0);
//
//	if (!starter.IsValid())
//		return false;
//	
//	//筛选角色
//	for (U32 i = 0; i < RandTeamPlayer::CT_COUNT; ++i)
//	{
//		U32 curCount = pRandTeam->GetCharCount(i);
//
//		if (pRandTeam->GetCharMaxCount(i) > curCount)
//		{
//			__SelectPlayer(pRandTeam,i,pRandTeam->GetCharMaxCount(i) - curCount);
//		}
//	}
//
//	//判断是否满足组队人数
//	if (pRandTeam->IsTeamFull())
//	{
//		//如果队伍未建立,向所有玩家发送进入确认
//		if (!pRandTeam->IsCreated())
//			pRandTeam->SetProcess(SERVER->GetRandTeamMgr()->GetEnterProcess(),60);
//		else
//			pRandTeam->SetProcess(SERVER->GetRandTeamMgr()->GetInviteProcess(),60);
//	}
//	
//	return true;
//}
