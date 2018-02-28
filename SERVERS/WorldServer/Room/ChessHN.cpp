#include "ChessHN.h"
#include "FvMask.h"
#include <set>
#include "CommLib/format.h"
#include "../WorldServer.h"
#include "../Common/MemGuard.h"
#include "WinTCP/IPacket.h"
#include "WinTCP/dtServerSocket.h"
#include "../PlayerMgr.h"
#include "Common/mRandom.h"

CChessHN::CChessHN() : CChessTable()
{
	ZeroMemory(m_cbRepertoryCard_108,sizeof(m_cbRepertoryCard_108));
	ZeroMemory(m_cbRepertoryCard_112,sizeof(m_cbRepertoryCard_112));
	m_cbNiaoCount = 0;

	for( U16 i = 0; i < GAME_PLAYER; i++ )
	{
		m_ChiHuRight[i].SetEmpty();
		m_StartHuRight[i].SetEmpty();
	}

	ZeroMemory( m_lStartHuScore,sizeof(m_lStartHuScore) );
	ZeroMemory( m_cbCardDataNiao,sizeof(m_cbCardDataNiao) );
	m_dwGameRuleIdex = BIT(GAME_TYPE_ZZ_HONGZHONG);
}

CChessHN::~CChessHN()
{
}

//复位桌子
void CChessHN::ResetTable()
{
	Parent::ResetTable();
	//游戏变量
	m_cbNiaoCount = 0;
	ZeroMemory(m_cbRepertoryCard_108,sizeof(m_cbRepertoryCard_108));
	ZeroMemory(m_cbRepertoryCard_112,sizeof(m_cbRepertoryCard_112));

	for( U16 i = 0; i < GAME_PLAYER; i++ )
	{
		m_StartHuRight[i].SetEmpty();
	}
	ZeroMemory( m_lStartHuScore,sizeof(m_lStartHuScore) );
	ZeroMemory( m_cbCardDataNiao,sizeof(m_cbCardDataNiao) );

	m_dwGameRuleIdex = BIT(GAME_TYPE_ZZ_HONGZHONG);
	return;
}


void CChessHN::GameStart_ZZ()
{
	//混乱扑克
	U32 lSiceCount = MAKEU32(MAKEU16(gRandGen.randI(1,6),gRandGen.randI(1,6)),MAKEU16(gRandGen.randI(1,6),gRandGen.randI(1,6)));
	m_wBankerUser = ((U8)(lSiceCount>>24)+(U8)(lSiceCount>>16)-1)%GAME_PLAYER;

	//设置变量
	m_cbProvideCard=0;
	m_wProvideUser=INVALID_CHAIR;
	m_wCurrentUser=m_wBankerUser;

	//构造数据
	CMD_S_GameStart GameStart;
	GameStart.lSiceCount=lSiceCount;
	GameStart.wBankerUser=m_wBankerUser;
	GameStart.wCurrentUser=m_wCurrentUser;
	GameStart.cbLeftCardCount = m_cbLeftCardCount;

	SetGameStatus(GS_MJ_PLAY);
	GameStart.cbXiaoHuTag = 0;

	//发送数据
	for (U16 i=0;i<GAME_PLAYER;i++)
	{
		//设置变量
		GameStart.cbUserAction = WIK_NULL;//m_cbUserAction[i];

		ZeroMemory(GameStart.cbCardData,sizeof(GameStart.cbCardData));
		m_GameLogic.SwitchToCardData(m_cbCardIndex[i],GameStart.cbCardData);
	/*	if( m_pITableFrame->GetTableUserItem(i)->IsAndroidUser() )
		{
			U8 bIndex = 1;
			for( U16 j=0; j<GAME_PLAYER; j++ )
			{
				if( j == i ) continue;
				m_GameLogic.SwitchToCardData(m_cbCardIndex[j],&GameStart.cbCardData[MAX_COUNT*bIndex++]);
			}
		}*/

		//发送数据
		SendTableData(i, "CLIENT_SERVER_SendCards", GameStart);
		//m_pITableFrame->SendLookonData(i,SUB_S_GAME_START,&GameStart,sizeof(GameStart));
	}

	m_bSendStatus = true;
    DispatchCardData(m_wCurrentUser);
}

void CChessHN::GameStart_CS()
{
	//混乱扑克
	U32 lSiceCount = MAKEU32(MAKEU16(gRandGen.randI(1,6),gRandGen.randI(1,6)),MAKEU16(gRandGen.randI(1,6),gRandGen.randI(1,6)));
	m_wBankerUser = ((U8)(lSiceCount>>24)+(U8)(lSiceCount>>16)-1)%GAME_PLAYER;

	//设置变量
	m_cbProvideCard=0;
	m_wProvideUser=INVALID_CHAIR;
	m_wCurrentUser=m_wBankerUser;

	//构造数据
	CMD_S_GameStart GameStart;
	GameStart.lSiceCount=lSiceCount;
	GameStart.wBankerUser=m_wBankerUser;
	GameStart.wCurrentUser=m_wCurrentUser;
	GameStart.cbLeftCardCount = m_cbLeftCardCount;

	bool bInXaoHu = false;
	for (U16 i=0;i<GAME_PLAYER;i++)
	{
		//胡牌判断
		CChiHuRight chr;
		m_cbUserAction[i] |= AnalyseChiHuCardCS_XIAOHU(m_cbCardIndex[i],chr);
		if (m_cbUserAction[i] != WIK_NULL)
		{
			bInXaoHu = true;
		}
	}

	if (bInXaoHu)
	{
		SetGameStatus(GS_MJ_XIAOHU);
		//*********定时器
		//m_pITableFrame->SetGameTimer(IDI_TIMER_XIAO_HU,(TIME_XIAO_HU)*1000,1,0);
		GameStart.cbXiaoHuTag =1;
	}
	else
	{
		SetGameStatus(GS_MJ_PLAY);
		GameStart.cbXiaoHuTag = 0;
	}

	//发送数据
	for (U16 i=0;i<GAME_PLAYER;i++)
	{
		//设置变量
		GameStart.cbUserAction = WIK_NULL;//m_cbUserAction[i];

		ZeroMemory(GameStart.cbCardData,sizeof(GameStart.cbCardData));
		m_GameLogic.SwitchToCardData(m_cbCardIndex[i],GameStart.cbCardData);
	/*	if( m_pITableFrame->GetTableUserItem(i)->IsAndroidUser() )
		{
			U8 bIndex = 1;
			for( U16 j=0; j<GAME_PLAYER; j++ )
			{
				if( j == i ) continue;
				m_GameLogic.SwitchToCardData(m_cbCardIndex[j],&GameStart.cbCardData[MAX_COUNT*bIndex++]);
			}
		}*/

		//发送数据
		//*********
		SendTableData(i, "CLIENT_SERVER_SendCards", GameStart);
		//m_pITableFrame->SendLookonData(i,SUB_S_GAME_START,&GameStart,sizeof(GameStart));
	}

	if (GameStart.cbXiaoHuTag == 1)
	{
		SendOperateNotify();
	}
	else
	{
		m_bSendStatus = true;
		DispatchCardData(m_wCurrentUser);
	}
}

U8 CChessHN::AnalyseChiHuCardZZ(const U8 cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], U8 cbWeaveCount, U8 cbCurrentCard, CChiHuRight &ChiHuRight,bool bSelfSendCard)
{
		//变量定义
	U8 cbChiHuKind=WIK_NULL;
	CAnalyseItemArray AnalyseItemArray;

	if (hasRule(GAME_TYPE_ZZ_ZIMOHU) && !bSelfSendCard)
	{
		return WIK_NULL;
	}

	//设置变量
	AnalyseItemArray.clear();
	ChiHuRight.SetEmpty();
	ChiHuRight |= CHR_SHU_FAN;

	//抢杠胡
	if( m_wCurrentUser == INVALID_CHAIR && m_bGangStatus)
	{
		if (hasRule(GAME_TYPE_ZZ_QIANGGANGHU))
		{
			ChiHuRight |= CHR_QIANG_GANG_HU;
		}
		else
		{
			ChiHuRight.SetEmpty();
			return WIK_NULL;
		}
		
	}
	//构造扑克
	U8 cbCardIndexTemp[MAX_INDEX];
	CopyMemory(cbCardIndexTemp,cbCardIndex,sizeof(cbCardIndexTemp));

	//cbCurrentCard一定不为0			!!!!!!!!!
	ASSERT( cbCurrentCard != 0 );
	if( cbCurrentCard == 0 ) return WIK_NULL;

	//插入扑克
	if (cbCurrentCard!=0)
		cbCardIndexTemp[m_GameLogic.SwitchToCardIndex(cbCurrentCard)]++;

	//分析扑克
	bool bValue = m_GameLogic.AnalyseCard(cbCardIndexTemp,WeaveItem,cbWeaveCount,AnalyseItemArray);
	if (!bValue)
	{

		ChiHuRight.SetEmpty();
		return WIK_NULL;
	}

	//胡牌分析
	//牌型分析
	while(AnalyseItemArray.size())
	{
		//变量定义
		tagAnalyseItem pAnalyseItem=AnalyseItemArray.front();
		AnalyseItemArray.pop_front();
		if (hasRule(GAME_TYPE_ZZ_258))
		{
			U8 cbCardValue = pAnalyseItem.cbCardEye&MASK_VALUE;
			if( cbCardValue != 2 && cbCardValue != 5 && cbCardValue != 8 )
			{
				continue;
			}
		}
		cbChiHuKind = WIK_CHI_HU;
		break;
	}

	if (bSelfSendCard)
	{
		cbChiHuKind = WIK_CHI_HU;
		ChiHuRight = CHR_SHU_FAN;
		ChiHuRight |= CHR_ZI_MO;
	}

	return cbChiHuKind;
}
U8 CChessHN::AnalyseChiHuCardCS(const U8 cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], U8 cbWeaveCount, U8 cbCurrentCard, CChiHuRight &ChiHuRight)
{
	//变量定义
	U8 cbChiHuKind=WIK_NULL;
	CAnalyseItemArray AnalyseItemArray;

	//设置变量
	AnalyseItemArray.clear();
//	ChiHuRight.SetEmpty();

	//构造扑克
	U8 cbCardIndexTemp[MAX_INDEX];
	CopyMemory(cbCardIndexTemp,cbCardIndex,sizeof(cbCardIndexTemp));

	//cbCurrentCard一定不为0			!!!!!!!!!
	ASSERT( cbCurrentCard != 0 );
	if( cbCurrentCard == 0 ) return WIK_NULL;

	/*
	//	特殊番型
	*/
	//七小对牌
	int nGenCount = 0;
	if( m_GameLogic.IsQiXiaoDui(cbCardIndex,WeaveItem,cbWeaveCount,cbCurrentCard,nGenCount) )
	{
		if (nGenCount >0)
		{
			ChiHuRight |= CHR_HAOHUA_QI_XIAO_DUI;
		}
		else
		{
			ChiHuRight |= CHR_QI_XIAO_DUI;
		}
	}
	if( m_GameLogic.IsJiangJiangHu(cbCardIndex,WeaveItem,cbWeaveCount,cbCurrentCard) ) 
	{
		ChiHuRight |= CHR_JIANGJIANG_HU;
	}
	if(cbWeaveCount == 4 &&  m_GameLogic.IsDanDiao(cbCardIndex,cbCurrentCard))
	{
		ChiHuRight |= CHR_QUAN_QIU_REN;
	}
	
	if( !ChiHuRight.IsEmpty() )
		cbChiHuKind = WIK_CHI_HU;

	//插入扑克
	if (cbCurrentCard!=0)
		cbCardIndexTemp[m_GameLogic.SwitchToCardIndex(cbCurrentCard)]++;

	//分析扑克
	bool bValue = m_GameLogic.AnalyseCard(cbCardIndexTemp,WeaveItem,cbWeaveCount,AnalyseItemArray);

	//胡牌分析
	if (!bValue)
	{
		ChiHuRight.SetEmpty();
		return WIK_NULL;
	}
	else
	{
		cbChiHuKind = WIK_CHI_HU;
	}

	//牌型分析
	while(AnalyseItemArray.size())
	{
		//变量定义
		tagAnalyseItem  pAnalyseItem= AnalyseItemArray.front();
		AnalyseItemArray.pop_front();

		/*
		//	判断番型
		*/
		//碰碰和
		if( m_GameLogic.IsPengPeng(&pAnalyseItem) ) 
			ChiHuRight |= CHR_PENGPENG_HU;
	}

	//素番
	if( cbChiHuKind == WIK_CHI_HU && ChiHuRight.IsEmpty() )
		ChiHuRight |= CHR_SHU_FAN;

	if( cbChiHuKind == WIK_CHI_HU )
	{
		//清一色牌
		if( m_GameLogic.IsQingYiSe(cbCardIndex,WeaveItem,cbWeaveCount,cbCurrentCard) )
			ChiHuRight |= CHR_QING_YI_SE;
	}

	return cbChiHuKind;
}

U8 CChessHN::AnalyseChiHuCardCS_XIAOHU(const U8 cbCardIndex[MAX_INDEX], CChiHuRight &ChiHuRight)
{
	U8 cbReplaceCount = 0;
	U8 cbChiHuKind=WIK_NULL;

	//临时数据
	U8 cbCardIndexTemp[MAX_INDEX];
	CopyMemory(cbCardIndexTemp,cbCardIndex,sizeof(cbCardIndexTemp));

	bool bDaSiXi = false;//大四喜
	bool bBanBanHu = true;//板板胡
	U8 cbQueYiMenColor[3] = { 1,1,1 };//缺一色
	U8 cbLiuLiuShun = 0;//六六顺

	//计算单牌
	for (U8 i=0;i<MAX_INDEX;i++)
	{
		U8 cbCardCount=cbCardIndexTemp[i];

		if (cbCardCount == 0)
		{
			continue;
		}

		if( cbCardCount == 4)
		{
			bDaSiXi = true;
		}

		if(cbCardCount == 3 )
		{
			cbLiuLiuShun ++;
		}

		U8 cbValue = m_GameLogic.SwitchToCardData(i) & MASK_VALUE;
		if (cbValue == 2 || cbValue == 5 || cbValue == 8)
		{
			bBanBanHu = false;
		}

		U8 cbCardColor = m_GameLogic.SwitchToCardData(i)&MASK_COLOR;
		cbQueYiMenColor[cbCardColor>>4] = 0;
	}
	if (bDaSiXi)
	{
		ChiHuRight |= CHR_XIAO_DA_SI_XI;
		cbChiHuKind = WIK_XIAO_HU;
	}
	if (bBanBanHu)
	{
		ChiHuRight |= CHR_XIAO_BAN_BAN_HU;
		cbChiHuKind = WIK_XIAO_HU;
	}
	if (cbQueYiMenColor[0] || cbQueYiMenColor[1] || cbQueYiMenColor[2])
	{
		ChiHuRight |= CHR_XIAO_QUE_YI_SE;
		cbChiHuKind = WIK_XIAO_HU;
	}
	if (cbLiuLiuShun >= 2)
	{
		ChiHuRight |= CHR_XIAO_LIU_LIU_SHUN;
		cbChiHuKind = WIK_XIAO_HU;
	}
	return cbChiHuKind;
}

U16 CChessHN::GetChiHuActionRank_ZZ(const CChiHuRight & ChiHuRight)
{
	U16 wFanShu = 0;
	if( !(ChiHuRight&CHR_ZI_MO).IsEmpty() )
		wFanShu *= 2;
	if( !(ChiHuRight&CHR_GANG_KAI).IsEmpty() )
		wFanShu = 2;
	if( !(ChiHuRight&CHR_QIANG_GANG_HU).IsEmpty() )
		wFanShu = 1;

	if (wFanShu == 0)
	{
		if( !(ChiHuRight&CHR_SHU_FAN).IsEmpty() )
			wFanShu = 1;
	}

	ASSERT( wFanShu > 0 );
	return wFanShu;
}

U16 CChessHN::GetChiHuActionRank_CS(const CChiHuRight & ChiHuRight)
{
	//大胡
	U16 wFanShu = 0;

	if( !(ChiHuRight&CHR_PENGPENG_HU).IsEmpty() )
		wFanShu = 6;
	else if( !(ChiHuRight&CHR_JIANGJIANG_HU).IsEmpty() )
		wFanShu = 6;
	else if( !(ChiHuRight&CHR_QING_YI_SE).IsEmpty() )
		wFanShu = 6;
	else if( !(ChiHuRight&CHR_HAI_DI_LAO).IsEmpty() )
		wFanShu = 6;
	else if( !(ChiHuRight&CHR_HAI_DI_PAO).IsEmpty() )
		wFanShu = 6;
	else if( !(ChiHuRight&CHR_QI_XIAO_DUI).IsEmpty() )
		wFanShu = 6;
	else if( !(ChiHuRight&CHR_HAOHUA_QI_XIAO_DUI).IsEmpty() )
		wFanShu = 6;
	else if( !(ChiHuRight&CHR_GANG_KAI).IsEmpty() )
		wFanShu = 6;
	else if( !(ChiHuRight&CHR_QIANG_GANG_HU).IsEmpty() )
		wFanShu = 6;
	else if( !(ChiHuRight&CHR_GANG_SHANG_PAO).IsEmpty() )
		wFanShu = 6;
	else if( !(ChiHuRight&CHR_QUAN_QIU_REN).IsEmpty() )
		wFanShu = 6;

	else if( !(ChiHuRight&CHR_XIAO_DA_SI_XI).IsEmpty() )
		wFanShu = 1;
	else if( !(ChiHuRight&CHR_XIAO_BAN_BAN_HU).IsEmpty() )
		wFanShu = 1;
	else if( !(ChiHuRight&CHR_XIAO_LIU_LIU_SHUN).IsEmpty() )
		wFanShu = 1;
	else if( !(ChiHuRight&CHR_XIAO_QUE_YI_SE).IsEmpty() )
		wFanShu = 1;

	else if( !(ChiHuRight&CHR_SHU_FAN).IsEmpty() )
		wFanShu = 1;

	ASSERT( wFanShu > 0 );
	return wFanShu;
}

void CChessHN::GameStart()
{
	if (hasRule(GAME_TYPE_ZZ_HONGZHONG))
	{
		Shuffle(m_cbRepertoryCard_112,MAX_REPERTORY_112);
	}
	else
	{
		Shuffle(m_cbRepertoryCard_108,MAX_REPERTORY_108);
	}

	if (m_cbGameTypeIdex == GAME_TYPE_112)
	{
		GameStart_ZZ();
	}
	else if (m_cbGameTypeIdex == GAME_TYPE_108)
	{
		GameStart_CS();
	}
}

U8 CChessHN::DoDispatchCardData()
{
	if (hasRule(GAME_TYPE_ZZ_HONGZHONG))
	{
		return m_cbRepertoryCard_112[--m_cbLeftCardCount];
	}
	else
	{
		return m_cbRepertoryCard_108[--m_cbLeftCardCount];
	}
}

template<class T>
bool CChessHN::SendTableData(U16 wChairID, const char* msg, T& obj)
{
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(GetPlayer(wChairID));
	if (NULL == pAccount)
		return false;

	CMemGuard Buffer(256 MEM_GUARD_PARAM);
	Base::BitStream sendPacket(Buffer.get(), 256);
	stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, msg, pAccount->GetAccountId(), SERVICE_CLIENT);
	sendPacket.writeBits(sizeof(obj)<<3,&obj);

	pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
	SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(),sendPacket);
	return true;
}

template<class T>
bool CChessHN::SendTableData(const char* msg, T& obj)
{
	for(int i = 0; i < GAME_PLAYER; ++i)
	{
		AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(GetPlayer(i));
		if (NULL == pAccount)
			continue;;

		CMemGuard Buffer(256 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 256);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, msg, pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeBits(sizeof(obj)<<3,&obj);

		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(),sendPacket);
	}
	return true;
}

U8 CChessHN::AnalyseChiHuCard(const U8 cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], U8 cbWeaveCount, U8 cbCurrentCard, CChiHuRight &ChiHuRight,bool bSelfSendCard)
{
	if (GAME_TYPE_112 == m_cbGameTypeIdex)
	{
		return AnalyseChiHuCardZZ(cbCardIndex,WeaveItem,cbWeaveCount,cbCurrentCard,ChiHuRight,m_wProvideUser == m_wCurrentUser);
	}
	if (GAME_TYPE_108 == m_cbGameTypeIdex)
	{
		return AnalyseChiHuCardCS(cbCardIndex,WeaveItem,cbWeaveCount,cbCurrentCard,ChiHuRight);
	}
	ASSERT(false);
	return false;
}

U16 CChessHN::GetChiHuActionRank(const CChiHuRight & ChiHuRight)
{
	if ( m_cbGameTypeIdex == GAME_TYPE_112)
	{
		return GetChiHuActionRank_ZZ(ChiHuRight);
	}
	if ( m_cbGameTypeIdex == GAME_TYPE_108)
	{
		return GetChiHuActionRank_CS(ChiHuRight);
	}
	return 0;
}

void CChessHN::setNiaoCard()
{
	ZeroMemory(m_cbCardDataNiao,sizeof(m_cbCardDataNiao));
	m_cbNiaoCount = GetNiaoCardNum();

	if (m_cbNiaoCount>ZZ_ZHANIAO0)
	{
		U8 cbCardIndexTemp[MAX_INDEX];
		m_cbLeftCardCount-= m_cbNiaoCount;
		if (hasRule(GAME_TYPE_ZZ_HONGZHONG))
		{
			m_GameLogic.SwitchToCardIndex(&m_cbRepertoryCard_112[m_cbLeftCardCount],m_cbNiaoCount,cbCardIndexTemp);
		}
		else
		{
			m_GameLogic.SwitchToCardIndex(&m_cbRepertoryCard_108[m_cbLeftCardCount],m_cbNiaoCount,cbCardIndexTemp);
		}
		m_GameLogic.SwitchToCardData(cbCardIndexTemp,m_cbCardDataNiao);
	}
	m_cbNiaoPick = m_GameLogic.GetPickNiaoCount(m_cbCardDataNiao,m_cbNiaoCount);
}

U8 CChessHN::GetNiaoCardNum()
{
	U8 nNum = ZZ_ZHANIAO0;
	if(hasRule(GAME_TYPE_ZZ_ZHANIAO2))
	{
		nNum = ZZ_ZHANIAO2;
	}
	if(hasRule(GAME_TYPE_ZZ_ZHANIAO4))
	{
		nNum = ZZ_ZHANIAO4;
	}
	if(hasRule(GAME_TYPE_ZZ_ZHANIAO6))
	{
		nNum = ZZ_ZHANIAO6;
	}

	if (nNum>m_cbLeftCardCount)
	{
		nNum = m_cbLeftCardCount;
	}
	return nNum;
}

void CChessHN::ProcessChiHuUser( U16 wChairId, bool bGiveUp )
{
	if( !bGiveUp )
	{
		//引用权位
		CChiHuRight &chr = m_ChiHuRight[wChairId];

		FiltrateRight( wChairId,chr );
		U16 wFanShu = 0;
		wFanShu = GetChiHuActionRank(chr);
		U64 lChiHuScore = wFanShu*1;//m_pGameServiceOption->lCellScore;

		//杠上炮,呼叫转移
		if( !(chr&CHR_GANG_SHANG_PAO).IsEmpty() )
		{
			U8 cbGangIndex = (m_GangScore[m_wProvideUser].cbGangCount>1)?(m_GangScore[m_wProvideUser].cbGangCount-1):0;
			//一炮多响的情况下,胡牌者平分杠得分
			U8 cbChiHuCount = 0;
			for( U16 i = 0; i < GAME_PLAYER; i++ )
				if( m_cbPerformAction[i]==WIK_CHI_HU ) cbChiHuCount++;
			if( cbChiHuCount == 1 )
			{
				U64 lScore = m_GangScore[m_wProvideUser].lScore[cbGangIndex][wChairId];
				m_GangScore[m_wProvideUser].lScore[cbGangIndex][wChairId] = m_GangScore[m_wProvideUser].lScore[cbGangIndex][m_wProvideUser];
				m_GangScore[m_wProvideUser].lScore[cbGangIndex][m_wProvideUser] = lScore;
			}
			else
			{
				U64 lGangScore = m_GangScore[m_wProvideUser].lScore[cbGangIndex][m_wProvideUser]/cbChiHuCount;
				lGangScore = getMax(lGangScore,(U64)2);//m_pGameServiceOption->lCellScore);
				for( U16 i = 0; i < GAME_PLAYER; i++ )
				{
					if( m_cbPerformAction[i]==WIK_CHI_HU )
						m_GangScore[m_wProvideUser].lScore[cbGangIndex][i] = lGangScore;
				}
				m_GangScore[m_wProvideUser].lScore[cbGangIndex][m_wProvideUser] = 0;
				for( U16 i = 0; i < GAME_PLAYER; i++ )
				{
					if( i != m_wProvideUser)
						m_GangScore[m_wProvideUser].lScore[cbGangIndex][m_wProvideUser] += m_GangScore[m_wProvideUser].lScore[cbGangIndex][i];
				}
			}
		}
		//抢杠杠分不算
		else if( !(chr&CHR_QIANG_GANG_HU).IsEmpty() )
		{
			m_GangScore[m_wProvideUser].cbGangCount--;
		}

		if( m_wProvideUser != wChairId )
			m_wLostFanShu[m_wProvideUser][wChairId] = (U16)lChiHuScore;
		else
		{
			for( U16 i = 0; i < GAME_PLAYER; i++ )
			{
				if( i == wChairId ) continue;

				m_wLostFanShu[i][wChairId] = (U16)lChiHuScore;
			}
		}

		if( m_wProvideUser == wChairId )
		{
			for( U16 i = 0; i < GAME_PLAYER; i++ )
			{
				if( i == wChairId ) continue;

				//胡牌分
				m_lGameScore[i] -= lChiHuScore;
				m_lGameScore[wChairId] += lChiHuScore;

				//中鸟分
				m_lGameScore[i] -= m_cbNiaoPick* 1;//;m_pGameServiceOption->lCellScore;
				m_lGameScore[wChairId] += m_cbNiaoPick* 1;//m_pGameServiceOption->lCellScore;
			}
		}
		//点炮
		else
		{

			m_lGameScore[m_wProvideUser] -= lChiHuScore;
			m_lGameScore[wChairId] += lChiHuScore;

			m_lGameScore[m_wProvideUser] -= m_cbNiaoPick*1;// m_pGameServiceOption->lCellScore;
			m_lGameScore[wChairId] += m_cbNiaoPick* 1;//m_pGameServiceOption->lCellScore;
		}

		//设置变量
		m_wProvider[wChairId] = m_wProvideUser;
		m_bGangStatus = false;
		m_bGangOutStatus = false;

		//发送消息
		CMD_S_ChiHu ChiHu;
		ChiHu.wChiHuUser = wChairId;
		ChiHu.wProviderUser = m_wProvideUser;
		ChiHu.lGameScore = m_lGameScore[wChairId];
		ChiHu.cbCardCount = m_GameLogic.GetCardCount( m_cbCardIndex[wChairId] );
		ChiHu.cbChiHuCard = m_cbProvideCard;
		SendTableData("WC_CHESS_CHI_HU", ChiHu);
		//m_pITableFrame->SendLookonData( INVALID_CHAIR,SUB_S_CHI_HU,&ChiHu,sizeof(ChiHu) );
	}

	return;
}
