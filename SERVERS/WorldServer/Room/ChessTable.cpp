#include "ChessTable.h"
#include "FvMask.h"
#include <set>
#include "CommLib/format.h"
#include "../WorldServer.h"
#include "../Common/MemGuard.h"
#include "../WorldServer.h"
#include "WinTCP/IPacket.h"
#include "WinTCP/dtServerSocket.h"
#include "../PlayerMgr.h"
#include "Common/mRandom.h"
#include "RoomMgr.h"
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

#define IDI_TIMER_XIAO_HU			1 //小胡

#define TIME_XIAO_HU  8

//构造函数
CChessTable::CChessTable()
{
	//游戏变量
	m_wBankerUser=INVALID_CHAIR;
	ZeroMemory(m_cbCardIndex,sizeof(m_cbCardIndex));
	ZeroMemory(m_bReady,sizeof(m_bReady));
	ZeroMemory(m_bDisband,sizeof(m_bDisband));
	ZeroMemory(m_bTrustee,sizeof(m_bTrustee));
	ZeroMemory( m_GangScore,sizeof(m_GangScore) );
	ZeroMemory( m_lGameScore,sizeof(m_lGameScore) );
	ZeroMemory( m_wLostFanShu,sizeof(m_wLostFanShu) );

	//出牌信息
	m_cbOutCardData=0;
	m_cbOutCardCount=0;
	m_wOutCardUser=INVALID_CHAIR;
	ZeroMemory(m_cbDiscardCard,sizeof(m_cbDiscardCard));
	ZeroMemory(m_cbDiscardCount,sizeof(m_cbDiscardCount));

	//发牌信息
	m_cbSendCardData=0;
	m_cbSendCardCount=0;
	m_cbLeftCardCount=0;
	
	//运行变量
	m_cbProvideCard=0;
	m_wResumeUser=INVALID_CHAIR;
	m_wCurrentUser=INVALID_CHAIR;
	m_wProvideUser=INVALID_CHAIR;

	//状态变量
	m_bSendStatus=false;
	m_bGangStatus = false;
	m_bGangOutStatus = false;

	//用户状态
	ZeroMemory(m_bResponse,sizeof(m_bResponse));
	ZeroMemory(m_cbUserAction,sizeof(m_cbUserAction));
	ZeroMemory(m_cbOperateCard,sizeof(m_cbOperateCard));
	ZeroMemory(m_cbPerformAction,sizeof(m_cbPerformAction));

	//组合扑克
	ZeroMemory(m_WeaveItemArray,sizeof(m_WeaveItemArray));
	ZeroMemory(m_cbWeaveItemCount,sizeof(m_cbWeaveItemCount));

	//结束信息
	m_cbChiHuCard=0;
	ZeroMemory( m_dwChiHuKind,sizeof(m_dwChiHuKind));
	memset( m_wProvider,INVALID_CHAIR,sizeof(m_wProvider));
	memset( m_cbChiHuNum,0,sizeof(m_cbChiHuNum));
	m_cbGameTypeIdex = 0;
	m_dwGameRuleIdex = 0;
	memset( m_TableScore, 0, sizeof(m_TableScore));
	m_cbTableNum = 0;
	SetGameStatus(GS_MJ_FREE);
	m_CreateTime = time(NULL);
	return;
}

//析构函数
CChessTable::~CChessTable(void)
{
	SERVER->GetWorkQueue()->GetTimerMgr().remove(this, &CChessTable::TimeProcess);
}

//复位桌子
void CChessTable::ResetTable()
{
	//游戏变量
	ZeroMemory(m_cbCardIndex,sizeof(m_cbCardIndex));
	ZeroMemory(m_bReady,sizeof(m_bReady));
	ZeroMemory(m_bDisband,sizeof(m_bDisband));
	ZeroMemory(m_bTrustee,sizeof(m_bTrustee));
	ZeroMemory( m_GangScore,sizeof(m_GangScore) );
	ZeroMemory( m_lGameScore,sizeof(m_lGameScore) );
	ZeroMemory( m_wLostFanShu,sizeof(m_wLostFanShu) );

	//出牌信息
	m_cbOutCardData=0;
	m_cbOutCardCount=0;
	m_wOutCardUser=INVALID_CHAIR;
	ZeroMemory(m_cbDiscardCard,sizeof(m_cbDiscardCard));
	ZeroMemory(m_cbDiscardCount,sizeof(m_cbDiscardCount));

	//发牌信息
	m_cbSendCardData=0;
	m_cbSendCardCount=0;
	m_cbLeftCardCount=0;


	//运行变量
	m_cbProvideCard=0;
	m_wResumeUser=INVALID_CHAIR;
	m_wCurrentUser=INVALID_CHAIR;
	m_wProvideUser=INVALID_CHAIR;
	//m_wBankerUser = INVALID_CHAIR;

	//状态变量
	m_bSendStatus=false;
	m_bGangStatus = false;
	m_bGangOutStatus = false;

	//用户状态
	ZeroMemory(m_bResponse,sizeof(m_bResponse));
	ZeroMemory(m_cbUserAction,sizeof(m_cbUserAction));
	ZeroMemory(m_cbOperateCard,sizeof(m_cbOperateCard));
	ZeroMemory(m_cbPerformAction,sizeof(m_cbPerformAction));

	//组合扑克
	ZeroMemory(m_WeaveItemArray,sizeof(m_WeaveItemArray));
	ZeroMemory(m_cbWeaveItemCount,sizeof(m_cbWeaveItemCount));

	//结束信息
	m_cbChiHuCard=0;
	ZeroMemory(m_dwChiHuKind,sizeof(m_dwChiHuKind));
	memset( m_wProvider,INVALID_CHAIR,sizeof(m_wProvider) );
	
	for( U16 i = 0; i < GAME_PLAYER; i++ )
	{
		m_ChiHuRight[i].SetEmpty();
	}

	//m_cbGameTypeIdex = 0;
	//m_dwGameRuleIdex = 0;

	return;
}


//游戏开始
bool CChessTable::OnEventGameStart()
{
	GameStart();
	AddGameRule(BIT(GAME_TYPE_ZZ_START));
	SERVER->GetWorkQueue()->GetTimerMgr().notify(this, &CChessTable::TimeProcess, 2000);
	return true;
}

void CChessTable::GameStart()
{
	//混乱扑克
	U32 lSiceCount = MAKEU32(MAKEU16(gRandGen.randI(1,6),gRandGen.randI(1,6)),MAKEU16(gRandGen.randI(1,6),gRandGen.randI(1,6)));
	if(!hasRule(GAME_TYPE_ZZ_START))
		m_wBankerUser = ((U8)(lSiceCount>>24)+(U8)(lSiceCount>>16)-1)%GAME_PLAYER;
	//else
	//	m_wBankerUser=(m_wBankerUser+GAME_PLAYER-1)%GAME_PLAYER;

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

	ZeroMemory(GameStart.cbCardData,sizeof(GameStart.cbCardData));
	//发送数据
	for (U16 i=0;i<GAME_PLAYER;i++)
	{
		//设置变量
		GameStart.cbUserAction = WIK_NULL;//m_cbUserAction[i];

		//ZeroMemory(GameStart.cbCardData,sizeof(GameStart.cbCardData));
		m_GameLogic.SwitchToCardData(m_cbCardIndex[i],&GameStart.cbCardData[MAX_COUNT  * i]);
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

		//m_pITableFrame->SendLookonData(i,SUB_S_GAME_START,&GameStart,sizeof(GameStart));
	}
			SendTableData("CLIENT_SERVER_SendCards", GameStart);

	m_bSendStatus = true;
    DispatchCardData(m_wCurrentUser);
}

void CChessTable::Shuffle(U8* pRepertoryCard,int nCardCount)
{
	m_cbLeftCardCount = nCardCount;
	m_GameLogic.RandCardData(pRepertoryCard,nCardCount);

	//分发扑克
	for (U16 i=0;i<GAME_PLAYER;i++)
	{
		//if( m_pITableFrame->GetTableUserItem(i) != NULL )
		{
			m_cbLeftCardCount-=(MAX_COUNT-1);
			m_GameLogic.SwitchToCardIndex(&pRepertoryCard[m_cbLeftCardCount],MAX_COUNT-1,m_cbCardIndex[i]);
		}
	}
}

//游戏结束
bool CChessTable::OnEventGameConclude(U16 wChairID, U8 cbReason)
{
	switch (cbReason)
	{
	case GER_NORMAL:		//常规结束
		{
			//变量定义
			CMD_S_GameEnd GameEnd;
			ZeroMemory(&GameEnd,sizeof(GameEnd));
			GameEnd.wLeftUser = INVALID_CHAIR;

			//设置中鸟数据
			ZeroMemory(GameEnd.cbCardDataNiao,sizeof(GameEnd.cbCardDataNiao));
			//GameEnd.cbNiaoPick = m_cbNiaoPick;
			//GameEnd.cbNiaoCount = m_cbNiaoCount;
			//for (int i = 0;i<MAX_NIAO_CARD && i< m_cbNiaoCount;i++)
			//{
			//	GameEnd.cbCardDataNiao[i] = m_cbCardDataNiao[i];
			//}
			
			//结束信息
			U16 wWinner = INVALID_CHAIR;
			U8 cbLeftUserCount = 0;			//判断是否流局
			bool bUserStatus[GAME_PLAYER];		//
			for (U16 i=0;i<GAME_PLAYER;i++)
			{
				GameEnd.cbCardCount[i]=m_GameLogic.SwitchToCardData(m_cbCardIndex[i],GameEnd.cbCardData[i]);
				m_ChiHuRight[i].GetRightData( &GameEnd.dwChiHuRight[i],MAX_RIGHT_COUNT );
				//m_StartHuRight[i].GetRightData( &GameEnd.dwStartHuRight[i],MAX_RIGHT_COUNT );
				
				//流局玩家数
				if( m_ChiHuRight[i].IsEmpty() ) cbLeftUserCount++;
				//当前玩家状态//*********
				//if( NULL != m_pITableFrame->GetTableUserItem(i) ) bUserStatus[i] = true;
				else bUserStatus[i] = false;
			}

			S64 lGangScore[GAME_PLAYER];
			ZeroMemory(&lGangScore,sizeof(lGangScore));
			for( U16 i = 0; i < GAME_PLAYER; i++ )
			{
				for( U8 j = 0; j < m_GangScore[i].cbGangCount; j++ )
				{
					for( U16 k = 0; k < GAME_PLAYER; k++ )
						lGangScore[k] += m_GangScore[i].lScore[j][k];
				}
			}


			if(cbLeftUserCount == GAME_PLAYER)
			{
				//流局下家玩家坐庄
				m_wBankerUser=(m_wBankerUser+GAME_PLAYER-1)%GAME_PLAYER;
			}
	
			for( U16 i = 0; i < GAME_PLAYER; i++ )
			{
				m_lGameScore[i] += lGangScore[i];				
			}

			//*********
			//扣分
			tagScoreInfo ScoreInfoArray[GAME_PLAYER];
			ZeroMemory(&ScoreInfoArray,sizeof(ScoreInfoArray));

			int	lGameTaxs[GAME_PLAYER];				//
			ZeroMemory(&lGameTaxs,sizeof(lGameTaxs));
			//统计积分
			for (U16 i=0;i<GAME_PLAYER;i++)
			{
				//*********
				//if( NULL == m_pITableFrame->GetTableUserItem(i) ) continue;

				//设置积分
				if (m_lGameScore[i])
				{
					//*********
					//lGameTaxs[i] = m_pITableFrame->CalculateRevenue(i,m_lGameScore[i]);
					m_lGameScore[i] -= lGameTaxs[i];
					if(hasRule(GAME_TYPE_ZZ_SUOHU))
					{
						m_TableScore[i] += m_lGameScore[i];
					}
					//SERVER->GetPlayerManager()->AddScore(GetPlayer(i), m_lGameScore[i]);
				}
				
				U8 ScoreKind;
				if( m_lGameScore[i] > 0L ) ScoreKind = SCORE_TYPE_WIN;
				else if( m_lGameScore[i] < 0L ) ScoreKind = SCORE_TYPE_LOSE;
				else ScoreKind = SCORE_TYPE_DRAW;

				ScoreInfoArray[i].lScore   = m_lGameScore[i];
				ScoreInfoArray[i].lRevenue = lGameTaxs[i];
				ScoreInfoArray[i].cbType   = ScoreKind;

			}

			if(hasRule(GAME_TYPE_ZZ_CHANGHU))
			{
				--m_cbTableNum;
			}

			//写入积分
			//*********
			//m_pITableFrame->WriteTableScore(ScoreInfoArray,CountArray(ScoreInfoArray));

			CopyMemory( GameEnd.lGameScore,m_lGameScore,sizeof(GameEnd.lGameScore) );
			CopyMemory( GameEnd.lGangScore,lGangScore,sizeof(GameEnd.lGangScore) );
			CopyMemory( GameEnd.wProvideUser,m_wProvider,sizeof(GameEnd.wProvideUser) );
			CopyMemory( GameEnd.lGameTax,lGameTaxs,sizeof(GameEnd.lGameTax));
			for (U16 i=0;i<GAME_PLAYER;i++)
			{
				CopyMemory( GameEnd.wLostFanShu[i],m_wLostFanShu[i],sizeof(m_wLostFanShu[i]) );
				CopyMemory(GameEnd.WeaveItemArray[i],m_WeaveItemArray[i],sizeof(m_WeaveItemArray[i]));
			}
			//CopyMemory( GameEnd.lStartHuScore,m_lStartHuScore,sizeof(GameEnd.lStartHuScore) );

			

			//组合扑克
			CopyMemory(GameEnd.cbWeaveCount,m_cbWeaveItemCount,sizeof(GameEnd.cbWeaveCount));

			//发送结束信息
			SendTableData("WC_CHESS_GAME_END", GameEnd);
			//m_pITableFrame->SendLookonData( INVALID_CHAIR,SUB_S_GAME_END,&GameEnd,sizeof(GameEnd) );

			//结束游戏
			SetGameStatus(GS_MJ_FREE);

			return true;
		}
	case GER_DISMISS:		//游戏解散
		{
			//变量定义
			CMD_S_GameEnd GameEnd;
			ZeroMemory(&GameEnd,sizeof(GameEnd));
			GameEnd.wLeftUser = INVALID_CHAIR;

			//设置变量
			memset( GameEnd.wProvideUser,INVALID_CHAIR,sizeof(GameEnd.wProvideUser) );

			//拷贝扑克
			for (U16 i=0;i<GAME_PLAYER;i++)
			{
				GameEnd.cbCardCount[i]=m_GameLogic.SwitchToCardData(m_cbCardIndex[i],GameEnd.cbCardData[i]);
			}

			//发送信息	
			SendTableData("WC_CHESS_GAME_END", GameEnd);
			//m_pITableFrame->SendLookonData(INVALID_CHAIR,SUB_S_GAME_END,&GameEnd,sizeof(GameEnd));

			//结束游戏
			SetGameStatus(GS_MJ_FREE);

			return true;
		}
	case GER_NETWORK_ERROR:		//网络错误
	case GER_USER_LEAVE:		//用户强退
		{
			m_bTrustee[wChairID] = true;

			return true;
		}
	}

	//错误断言
	ASSERT(false);
	return false;
}

//发送场景
bool CChessTable::OnEventSendGameScene(U16 wChiarID, bool bSendSecret, U64 nTableId, U32 playerId)
{
	switch (GetGameStatus())
	{
	case GS_MJ_FREE:	//空闲状态
		{
			//变量定义
			CMD_S_StatusFree StatusFree;
			memset(&StatusFree,0,sizeof(StatusFree));
			CChessTable* nTable = ROOMMGR->FindChessTable(nTableId);
			if(nTable)
			{
				ROOMMGR->RestartChessTable(playerId, nTableId);
					//if(nTable->ReadyGame(wChiarID))
					//{
					//	nTable->ResetTable();
					//	nTable->OnEventGameStart();
					//}
			}
			for(int i = 0; i < GAME_PLAYER; ++i)
			{
				StatusFree.lGameScore[i] = getTableScore(i);
			}
			//构造数据
			StatusFree.wBankerUser=m_wBankerUser;
			StatusFree.wCurrentUser = wChiarID;
			StatusFree.nTableID = nTableId;
			//StatusFree.lCellScore=m_pGameServiceOption->lCellScore;
			CopyMemory(StatusFree.bTrustee,m_bTrustee,sizeof(m_bTrustee));

			//发送场景
			SendTableData(wChiarID, "WC_CHESS_FREE_GAMESCENE", StatusFree);
			return true;
		}
	case GS_MJ_PLAY:	//游戏状态
		{
			//变量定义
			CMD_S_StatusPlay StatusPlay;
			memset(&StatusPlay,0,sizeof(StatusPlay));

			//游戏变量
			StatusPlay.nChairId = wChiarID;
			StatusPlay.nTableID = nTableId;
			StatusPlay.wBankerUser=m_wBankerUser;
			StatusPlay.wCurrentUser=m_wCurrentUser;
			StatusPlay.lCellScore=0;//m_pGameServiceOption->lCellScore;
			CopyMemory(StatusPlay.bTrustee,m_bTrustee,sizeof(m_bTrustee));

			//状态变量
			StatusPlay.cbActionCard=m_cbProvideCard;
			StatusPlay.cbLeftCardCount=m_cbLeftCardCount;
			StatusPlay.cbActionMask=(m_bResponse[wChiarID]==false)?m_cbUserAction[wChiarID]:WIK_NULL;

			//历史记录
			StatusPlay.wOutCardUser=m_wOutCardUser;
			StatusPlay.cbOutCardData=m_cbOutCardData;
			for (int i = 0;i<GAME_PLAYER;i++)
			{
				CopyMemory(StatusPlay.cbDiscardCard[i],m_cbDiscardCard[i],sizeof(U8)*60);
			}
			CopyMemory(StatusPlay.cbDiscardCount,m_cbDiscardCount,sizeof(StatusPlay.cbDiscardCount));

			//组合扑克
			CopyMemory(StatusPlay.WeaveItemArray,m_WeaveItemArray,sizeof(m_WeaveItemArray));
			CopyMemory(StatusPlay.cbWeaveCount,m_cbWeaveItemCount,sizeof(m_cbWeaveItemCount));


			//扑克数据
			StatusPlay.cbCardCount=m_GameLogic.SwitchToCardData(m_cbCardIndex[wChiarID],StatusPlay.cbCardData);
			StatusPlay.cbSendCardData=((m_cbSendCardData!=0)&&(m_wProvideUser==wChiarID))?m_cbSendCardData:0x00;

			for(int i = 0; i < GAME_PLAYER; ++i)
			{
				StatusPlay.wWinOrder[i] = m_GameLogic.GetCardCount(m_cbCardIndex[i]);
				StatusPlay.lGameScore[i] = getTableScore(i);
			}
			//StatusPlay.nTableNum = GetTableNum();
			//发送场景
			SendTableData(wChiarID, "WC_CHESS_PLAY_GAMESCENE", StatusPlay);
			return true;
		}
	}

	return false;
}

////定时器事件
struct stChessData
{
	U8 index;
	U8 value;

	bool operator < (const stChessData& Other) const
	{
		if(Other.index == value)
			return index < Other.index;

		return value < Other.value;
	}
};

void CChessTable::TimeProcess(U32 value)
{
	{
		U32 nCurTime = time(NULL);
		if(HasDisband())
		{
			return;
		}
		else if(nCurTime - m_CreateTime >= 24 * 60 * 60)
		{
			AddGameRule(BIT(GAME_TYPE_ZZ_DISBAND));
			this->DisbandGameSendtoPlayer(this);
		}
		//********自动
		else if(GetGameStatus() == GS_MJ_PLAY )
		{
			//ZeroMemory(m_cbUserAction,sizeof(m_cbUserAction));
			//SetGameStatus(GS_MJ_PLAY);

			//m_bSendStatus = true;
			if(m_wCurrentUser == 0 || m_wCurrentUser == INVALID_CHAIR)
				return;
			U8 cIndex = 0;
			std::set<stChessData> ChessSet;
			for(int i = 0; i < MAX_INDEX; ++i)
			{
				if(m_cbCardIndex[m_wCurrentUser][i] != 0)
				{
					stChessData data;
					data.index = i;
					data.value = m_cbCardIndex[m_wCurrentUser][i];
					ChessSet.insert(data);
				}
			}

			if(ChessSet.empty())
				return;

			if(m_wCurrentUser == 0 || m_wCurrentUser == 1)
				cIndex = (*ChessSet.begin()).index;
			else 
				cIndex = (*ChessSet.rbegin()).index;

			if(OnUserOutCard(m_wCurrentUser, m_GameLogic.SwitchToCardData(cIndex)))
			{
				
			}
			return;
		}
		else if(GetGameStatus() == GS_MJ_FREE)
		{
			if(hasRule(GAME_TYPE_ZZ_SUOHU))
			{
				bool bDisband = false;
				for(int i = 0; i < GAME_PLAYER; ++i)
				{
					if(m_TableScore[i] < 0)
					{
						bDisband = true;
						break;
					}
				}

				if(bDisband)
				{
					AddGameRule(BIT(GAME_TYPE_ZZ_DISBAND));
					this->DisbandGameSendtoPlayer(this);
				}
			}
			else if(hasRule(GAME_TYPE_ZZ_CHANGHU))
			{
				if(m_cbTableNum  <= 0)
				{
					AddGameRule(BIT(GAME_TYPE_ZZ_DISBAND));
					this->DisbandGameSendtoPlayer(this);
				}
			}
		}
	}
	return;
}
//
////游戏消息处理
//bool CChessTable::OnGameMessage(U16 wSubCmdID, VOID* pDataBuffer, U16 wDataSize, IServerUserItem * pIServerUserItem)
//{
//	switch (wSubCmdID)
//	{
//	case SUB_C_OUT_CARD:		//出牌消息
//		{
//			//效验消息
//			ASSERT(wDataSize==sizeof(CMD_C_OutCard));
//			if (wDataSize!=sizeof(CMD_C_OutCard)) return false;
//
//			//用户效验
//			if (pIServerUserItem->GetUserStatus()!=US_PLAYING) return true;
//
//			//消息处理
//			CMD_C_OutCard * pOutCard=(CMD_C_OutCard *)pDataBuffer;
//			return OnUserOutCard(pIServerUserItem->GetChairID(),pOutCard->cbCardData);
//		}
//	case SUB_C_OPERATE_CARD:	//操作消息
//		{
//			//效验消息
//			ASSERT(wDataSize==sizeof(CMD_C_OperateCard));
//			if (wDataSize!=sizeof(CMD_C_OperateCard)) return false;
//
//			//用户效验
//			if (pIServerUserItem->GetUserStatus()!=US_PLAYING) return true;
//
//			//消息处理
//			CMD_C_OperateCard * pOperateCard=(CMD_C_OperateCard *)pDataBuffer;
//			return OnUserOperateCard(pIServerUserItem->GetChairID(),pOperateCard->cbOperateCode,pOperateCard->cbOperateCard);
//		}
//	case SUB_C_TRUSTEE:
//		{
//			CMD_C_Trustee *pTrustee =(CMD_C_Trustee *)pDataBuffer;
//			if(wDataSize != sizeof(CMD_C_Trustee)) return false;
//
//
//			m_bTrustee[pIServerUserItem->GetChairID()]=pTrustee->bTrustee;
//			CMD_S_Trustee Trustee;
//			Trustee.bTrustee=pTrustee->bTrustee;
//			Trustee.wChairID = pIServerUserItem->GetChairID();
//			m_pITableFrame->SendTableData(INVALID_CHAIR,SUB_S_TRUSTEE,&Trustee,sizeof(Trustee));
//			m_pITableFrame->SendLookonData(INVALID_CHAIR,SUB_S_TRUSTEE,&Trustee,sizeof(Trustee));
//
//			return true;
//		}
//	case SUB_C_XIAOHU:
//		{
//			if (m_pITableFrame->GetGameStatus() != GS_MJ_XIAOHU)
//			{
//				return true;
//			}
//			U16 wChairID = pIServerUserItem->GetChairID();
//			m_dwChiHuKind[wChairID] = AnalyseChiHuCardCS_XIAOHU(m_cbCardIndex[wChairID],m_StartHuRight[wChairID]);
//
//			LONGLONG lStartHuScore = 0;
//			int wFanShu = m_GameLogic.GetChiHuActionRank_CS(m_StartHuRight[wChairID]);
//			lStartHuScore  = wFanShu*m_pGameServiceOption->lCellScore;
//			m_lStartHuScore[wChairID] += lStartHuScore*3;
//
//			for (int i=0;i<GAME_PLAYER;i++)
//			{
//				if (i == wChairID)continue;
//				m_lStartHuScore[i] -= lStartHuScore;
//			}
//
//			m_cbUserAction[wChairID] = WIK_NULL;
//
//			bool bEnd = true;
//			for (int i = 0;i < GAME_PLAYER;i++)
//			{
//				if (m_cbUserAction[i] != WIK_NULL)
//				{
//					bEnd = false;
//				}
//			}
//			if (bEnd)
//			{
//				ZeroMemory(m_cbUserAction,sizeof(m_cbUserAction));
//				m_pITableFrame->SetGameStatus(GS_MJ_PLAY);
//				m_bSendStatus = true;
//				DispatchCardData(m_wCurrentUser);
//				m_pITableFrame->KillGameTimer(IDI_TIMER_XIAO_HU);
//
//			}
//			return true;
//		}
//	}
//
//	return false;
//}
//
////框架消息处理
//bool CChessTable::OnFrameMessage(U16 wSubCmdID, VOID * pDataBuffer, U16 wDataSize, IServerUserItem * pIServerUserItem)
//{
//	return false;
//}
//
////用户坐下
//bool CChessTable::OnActionUserSitDown(U16 wChairID, IServerUserItem * pIServerUserItem, bool bLookonUser)
//{
//	return true;
//}
//
////用户起来
//bool CChessTable::OnActionUserStandUp(U16 wChairID, IServerUserItem * pIServerUserItem, bool bLookonUser)
//{
//	//庄家设置
//	if (bLookonUser==false)
//	{
//		m_bTrustee[wChairID]=false;
//		CMD_S_Trustee Trustee;
//		Trustee.bTrustee=false;
//		Trustee.wChairID = wChairID;
//		m_pITableFrame->SendTableData(INVALID_CHAIR,SUB_S_TRUSTEE,&Trustee,sizeof(Trustee));
//		m_pITableFrame->SendLookonData(INVALID_CHAIR,SUB_S_TRUSTEE,&Trustee,sizeof(Trustee));
//	}
//
//	return true;
//}

void CChessTable::SetPrivateInfo(U8 bGameTypeIdex,U32	bGameRuleIdex)
{
	m_cbGameTypeIdex = bGameTypeIdex;
	m_dwGameRuleIdex = bGameRuleIdex;
}

void CChessTable::AddGameRule(U32 nRule)
{
	m_dwGameRuleIdex |= nRule;
}

void CChessTable::ClearGameRule(U32 nRule)
{
	m_dwGameRuleIdex &= (~nRule);
}

bool CChessTable::ReadyGame(U32 nSlot)
{
	m_bReady[nSlot] = true;
	bool bAllReady = true;
	for(int i = 0; i < GAME_PLAYER; ++i)
	{
		if(m_bReady[i] == false)
		{
			bAllReady = false;
			break;
		}
	}
	return bAllReady;
}

bool CChessTable::ClearDisbandGame()
{
	ZeroMemory(m_bDisband,sizeof(m_bDisband));
	return true;
}

U8 CChessTable::DisbandGame(U32 nSlot, U8 nAgreeFlag)
{
	m_bDisband[nSlot] = nAgreeFlag;

	for(int i = 0; i < GAME_PLAYER; ++i)
	{
		if(m_bDisband[i] != 1)
		{
			return m_bDisband[i];
			break;
		}
	}
	return 1;
}

bool CChessTable::IsUserVaildCard(U16 wChairID, U8 cbCardData)
{
	if (m_GameLogic.IsValidCard(cbCardData)==false) 
		return false;

	if (wChairID >= GetMaxPlayers())
		return false;

	U8 cbIndex = m_GameLogic.SwitchToCardIndex(cbCardData);
	return m_cbCardIndex[wChairID][cbIndex];
}

bool CChessTable::IsCurrentUser(U16 wChairID)
{
	return m_wCurrentUser == wChairID;
}

template<class T>
bool CChessTable::SendTableData(U16 wChairID, const char* msg, T& obj)
{
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(GetPlayer(wChairID));
	if (NULL == pAccount)
		return false;

	CMemGuard Buffer(512 MEM_GUARD_PARAM);
	Base::BitStream sendPacket(Buffer.get(), 512);
	stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, msg, pAccount->GetAccountId(), SERVICE_CLIENT);
	sendPacket.writeBits(sizeof(obj)<<3,&obj);

	pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
	SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(),sendPacket);
	return true;
}

template<class T>
bool CChessTable::SendTableData(const char* msg, T& obj)
{
	for(int i = 0; i < GAME_PLAYER; ++i)
	{
		AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(GetPlayer(i));
		if (NULL == pAccount)
			continue;;

		CMemGuard Buffer(512 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 512);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, msg, pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeBits(sizeof(obj)<<3,&obj);

		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(),sendPacket);
	}
	return true;
}

bool CChessTable::DisbandGameSendtoPlayer(CChessTable* pTable)
{
	for(int i = 0; i < pTable->GetMaxPlayers(); ++i)
	{

		U32 nJPlayerId = pTable->GetJoinPlayer(i);
		if(nJPlayerId)
		{
			AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nJPlayerId);
			if(pAccount)
			{
				CMemGuard Buffer(256 MEM_GUARD_PARAM);
				Base::BitStream sendPacket(Buffer.get(), 256);
				stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, "WC_RefuseDisbandChessTable", pAccount->GetAccountId(), SERVICE_CLIENT,1);
				for(int j = 0; j < pTable->GetMaxPlayers(); ++j)
				{
					for(int k = 0; k < 3; ++k)
					{
						sendPacket.writeInt(pTable->m_cbChiHuNum[j][k],  Base::Bit8);
					}
					S64 score = pTable->getTableScore(j);
					sendPacket.writeBits(Base::Bit64, &score);
				}
				pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
				SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
			}
		}

	}
	return true;
}


//用户出牌
bool CChessTable::OnUserOutCard(U16 wChairID, U8 cbCardData)
{
	//std::printf("玩家[%d],打出一张[%d]")
	//效验状态
	ASSERT(GetGameStatus()==GS_MJ_PLAY);
	if (GetGameStatus()!=GS_MJ_PLAY) return true;

	//错误断言
	ASSERT(wChairID==m_wCurrentUser);
	ASSERT(m_GameLogic.IsValidCard(cbCardData)==true);

	//效验参数
	if (wChairID!=m_wCurrentUser) return true;
	if (m_GameLogic.IsValidCard(cbCardData)==false) return true;

	//删除扑克
	if (m_GameLogic.RemoveCard(m_cbCardIndex[wChairID],cbCardData)==false)
	{
		ASSERT(false);
		return true;
	}

	//设置变量
	m_bSendStatus=true;
	if( m_bGangStatus )
	{
		m_bGangStatus = false;
		m_bGangOutStatus = true;
	}
	m_cbUserAction[wChairID]=WIK_NULL;
	m_cbPerformAction[wChairID]=WIK_NULL;

	//出牌记录
	m_cbOutCardCount++;
	m_wOutCardUser=wChairID;
	m_cbOutCardData=cbCardData;

	//构造数据
	CMD_S_OutCard OutCard;
	OutCard.wOutCardUser=wChairID;
	OutCard.cbOutCardData=cbCardData;

	//发送消息
	//m_pITableFrame->SendLookonData(INVALID_CHAIR,SUB_S_OUT_CARD,&OutCard,sizeof(OutCard));
	SendTableData("WC_CHESS_OUT_CARD", OutCard);

	m_wProvideUser=wChairID;
	m_cbProvideCard=cbCardData;

	//用户切换
	m_wCurrentUser=(wChairID+GAME_PLAYER+1)%GAME_PLAYER;

	//响应判断
	bool bAroseAction=EstimateUserRespond(wChairID,cbCardData,EstimatKind_OutCard);

	//派发扑克
	if (bAroseAction==false) DispatchCardData(m_wCurrentUser);

	return true;
}

//用户操作
bool CChessTable::OnUserOperateCard(U16 wChairID, U8 cbOperateCode, U8 cbOperateCard)
{
	//效验状态
	ASSERT(GetGameStatus()!=GS_MJ_FREE);
	if (GetGameStatus()==GS_MJ_FREE)
		return true;

	//效验用户	注意：机器人有可能发生此断言
	//ASSERT((wChairID==m_wCurrentUser)||(m_wCurrentUser==INVALID_CHAIR));
	if ((wChairID!=m_wCurrentUser)&&(m_wCurrentUser!=INVALID_CHAIR)) 
		return true;

	//被动动作
	if (m_wCurrentUser==INVALID_CHAIR)
	{
		//效验状态
		if (m_bResponse[wChairID]==true) 
			return true;
		if ((cbOperateCode!=WIK_NULL)&&((m_cbUserAction[wChairID]&cbOperateCode)==0))
			return true;

		//变量定义
		U16 wTargetUser=wChairID;
		U8 cbTargetAction=cbOperateCode;

		//设置变量
		m_bResponse[wChairID]=true;
		m_cbPerformAction[wChairID]=cbOperateCode;
		m_cbOperateCard[wChairID]=m_cbProvideCard;

		//执行判断
		for (U16 i=0;i<GAME_PLAYER;i++)
		{
			//获取动作
			U8 cbUserAction=(m_bResponse[i]==false)?m_cbUserAction[i]:m_cbPerformAction[i];

			//优先级别
			U8 cbUserActionRank=m_GameLogic.GetUserActionRank(cbUserAction);
			U8 cbTargetActionRank=m_GameLogic.GetUserActionRank(cbTargetAction);

			//动作判断
			if (cbUserActionRank>cbTargetActionRank)
			{
				wTargetUser=i;
				cbTargetAction=cbUserAction;
			}
		}
		if (m_bResponse[wTargetUser]==false) 
			return true;

		//吃胡等待
		if (cbTargetAction==WIK_CHI_HU)
		{
			for (U16 i=0;i<GAME_PLAYER;i++)
			{
				if ((m_bResponse[i]==false)&&(m_cbUserAction[i]&WIK_CHI_HU))
				{
					int jjjj = 0;
				}
				//	return true;
			}
		}

		//放弃操作
		if (cbTargetAction==WIK_NULL)
		{
			//用户状态
			ZeroMemory(m_bResponse,sizeof(m_bResponse));
			ZeroMemory(m_cbUserAction,sizeof(m_cbUserAction));
			ZeroMemory(m_cbOperateCard,sizeof(m_cbOperateCard));
			ZeroMemory(m_cbPerformAction,sizeof(m_cbPerformAction));

			//发送扑克
			DispatchCardData(m_wResumeUser);

			return true;
		}

		//变量定义
		U8 cbTargetCard=m_cbOperateCard[wTargetUser];

		//出牌变量
		m_cbOutCardData=0;
		m_bSendStatus=true;
		m_wOutCardUser=INVALID_CHAIR;

		//胡牌操作
		if (cbTargetAction==WIK_CHI_HU)
		{
			setSpecialType();//结束后设置鸟牌
			//结束信息
			m_cbChiHuCard=cbTargetCard;

			for (U16 i=(m_wProvideUser+GAME_PLAYER-1)%GAME_PLAYER;i!=m_wProvideUser;i = (i+GAME_PLAYER-1)%GAME_PLAYER)
			{
				//过虑判断
				if ((m_cbPerformAction[i]&WIK_CHI_HU)==0)
					continue;

				//胡牌判断
				U8 cbWeaveItemCount=m_cbWeaveItemCount[i];
				tagWeaveItem * pWeaveItem=m_WeaveItemArray[i];
				m_dwChiHuKind[i] = AnalyseChiHuCard(m_cbCardIndex[i],pWeaveItem,cbWeaveItemCount,m_cbChiHuCard,m_ChiHuRight[i]);

				//插入扑克
				if (m_dwChiHuKind[i]!=WIK_NULL) 
				{
					m_cbCardIndex[i][m_GameLogic.SwitchToCardIndex(m_cbChiHuCard)]++;
					ProcessChiHuUser( i,false);
				}
			}

			OnEventGameConclude( INVALID_CHAIR,GER_NORMAL );

			return true;
		}

		//用户状态
		ZeroMemory(m_bResponse,sizeof(m_bResponse));
		ZeroMemory(m_cbUserAction,sizeof(m_cbUserAction));
		ZeroMemory(m_cbOperateCard,sizeof(m_cbOperateCard));
		ZeroMemory(m_cbPerformAction,sizeof(m_cbPerformAction));

		//组合扑克
		ASSERT(m_cbWeaveItemCount[wTargetUser]<4);
		U16 wIndex=m_cbWeaveItemCount[wTargetUser]++;
		m_WeaveItemArray[wTargetUser][wIndex].cbPublicCard=true;
		m_WeaveItemArray[wTargetUser][wIndex].cbCenterCard=cbTargetCard;
		m_WeaveItemArray[wTargetUser][wIndex].cbWeaveKind=cbTargetAction;
		m_WeaveItemArray[wTargetUser][wIndex].wProvideUser=(m_wProvideUser==INVALID_CHAIR)?wTargetUser:m_wProvideUser;

		//删除扑克
		switch (cbTargetAction)
		{
		case WIK_LEFT:		//上牌操作
			{
				//删除扑克
				U8 cbRemoveCard[3];
				m_GameLogic.GetWeaveCard(WIK_LEFT,cbTargetCard,cbRemoveCard);
				VERIFY( m_GameLogic.RemoveCard(cbRemoveCard,3,&cbTargetCard,1) );
				VERIFY( m_GameLogic.RemoveCard(m_cbCardIndex[wTargetUser],cbRemoveCard,2) );
			}
			break;
		case WIK_RIGHT:		//上牌操作
			{
				//删除扑克
				U8 cbRemoveCard[3];
				m_GameLogic.GetWeaveCard(WIK_RIGHT,cbTargetCard,cbRemoveCard);
				VERIFY( m_GameLogic.RemoveCard(cbRemoveCard,3,&cbTargetCard,1) );
				VERIFY( m_GameLogic.RemoveCard(m_cbCardIndex[wTargetUser],cbRemoveCard,2) );
			}
			break;
		case WIK_CENTER:	//上牌操作
			{
				//删除扑克
				U8 cbRemoveCard[3];
				m_GameLogic.GetWeaveCard(WIK_CENTER,cbTargetCard,cbRemoveCard);
				VERIFY( m_GameLogic.RemoveCard(cbRemoveCard,3,&cbTargetCard,1) );
				VERIFY( m_GameLogic.RemoveCard(m_cbCardIndex[wTargetUser],cbRemoveCard,2) );
			}
			break;
		case WIK_PENG:		//碰牌操作
			{
				//删除扑克
				U8 cbRemoveCard[]={cbTargetCard,cbTargetCard};
				VERIFY( m_GameLogic.RemoveCard(m_cbCardIndex[wTargetUser],cbRemoveCard,2) );
			}
			break;
		case WIK_GANG:		//杠牌操作
			{
				//删除扑克,被动动作只存在放杠
				U8 cbRemoveCard[]={cbTargetCard,cbTargetCard,cbTargetCard};
				VERIFY( m_GameLogic.RemoveCard(m_cbCardIndex[wTargetUser],cbRemoveCard,CountArray(cbRemoveCard)) );
			}
			break;
		default:
			ASSERT( false );
			return false;
		}

		//构造结果
		CMD_S_OperateResult OperateResult;
		OperateResult.wOperateUser=wTargetUser;
		OperateResult.cbOperateCard=cbTargetCard;
		OperateResult.cbOperateCode=cbTargetAction;
		OperateResult.wProvideUser=(m_wProvideUser==INVALID_CHAIR)?wTargetUser:m_wProvideUser;

		//发送消息
		SendTableData("WC_CHESS_OPERATE_RESULT", OperateResult);
		//m_pITableFrame->SendLookonData(INVALID_CHAIR,SUB_S_OPERATE_RESULT,&OperateResult,sizeof(OperateResult));
		//设置用户
		m_wCurrentUser=wTargetUser;

		//杠牌处理
		if (cbTargetAction==WIK_GANG)
		{
			//发送信息
			CMD_S_GangScore gs;
			ZeroMemory( &gs,sizeof(gs) );
			gs.wChairId = wTargetUser;
			gs.cbXiaYu = false;
			gs.lGangScore[wTargetUser] += 1L;//m_pGameServiceOption->lCellScore*2L;
			gs.lGangScore[m_wProvideUser] -= 1L;//-m_pGameServiceOption->lCellScore*2L;
			SendTableData("WC_CHESS_GANG_SCORE", gs);
			//m_pITableFrame->SendLookonData( INVALID_CHAIR,SUB_S_GANG_SCORE,&gs,sizeof(gs) );

			//杠得分
			U8 cbIndex = m_GangScore[wTargetUser].cbGangCount++;
			m_GangScore[wTargetUser].lScore[cbIndex][wTargetUser] += 1L;//m_pGameServiceOption->lCellScore*2;
			m_GangScore[wTargetUser].lScore[cbIndex][m_wProvideUser] -= 1L;//-m_pGameServiceOption->lCellScore*2;
			m_bGangStatus = true;
			DispatchCardData(wTargetUser,true);
		}

		return true;
	}

	//主动动作
	if (m_wCurrentUser==wChairID)
	{
		//效验操作
		if ((cbOperateCode==WIK_NULL)||((m_cbUserAction[wChairID]&cbOperateCode)==0))
			return true;

		//扑克效验
		ASSERT((cbOperateCode==WIK_NULL)||(cbOperateCode==WIK_CHI_HU)||
			(m_GameLogic.IsValidCard(cbOperateCard)==true));
		if ((cbOperateCode!=WIK_NULL)&&(cbOperateCode!=WIK_CHI_HU)
			&&(m_GameLogic.IsValidCard(cbOperateCard)==false)) 
			return true;

		//设置变量
		m_bSendStatus=true;
		m_cbUserAction[m_wCurrentUser]=WIK_NULL;
		m_cbPerformAction[m_wCurrentUser]=WIK_NULL;

		bool bPublic=false;
		U16  wGangProvideUser = INVALID_CHAIR;

		//执行动作
		switch (cbOperateCode)
		{
		case WIK_GANG:			//杠牌操作
			{
				//变量定义
				U8 cbWeaveIndex=0xFF;
				U8 cbCardIndex=m_GameLogic.SwitchToCardIndex(cbOperateCard);

				//杠牌处理
				if (m_cbCardIndex[wChairID][cbCardIndex]==1)
				{
					//寻找组合
					for (U8 i=0;i<m_cbWeaveItemCount[wChairID];i++)
					{
						U8 cbWeaveKind=m_WeaveItemArray[wChairID][i].cbWeaveKind;
						U8 cbCenterCard=m_WeaveItemArray[wChairID][i].cbCenterCard;
						if ((cbCenterCard==cbOperateCard)&&(cbWeaveKind==WIK_PENG))
						{
							bPublic=true;
							cbWeaveIndex=i;
							break;
						}
					}

					//效验动作
					ASSERT(cbWeaveIndex!=0xFF);
					if (cbWeaveIndex==0xFF) return false;

					//组合扑克
					m_WeaveItemArray[wChairID][cbWeaveIndex].cbPublicCard=true;
					//m_WeaveItemArray[wChairID][cbWeaveIndex].wProvideUser=wChairID;
					wGangProvideUser = m_WeaveItemArray[wChairID][cbWeaveIndex].wProvideUser;
					m_WeaveItemArray[wChairID][cbWeaveIndex].cbWeaveKind=cbOperateCode;
					m_WeaveItemArray[wChairID][cbWeaveIndex].cbCenterCard=cbOperateCard;
				}
				else
				{
					//扑克效验
					ASSERT(m_cbCardIndex[wChairID][cbCardIndex]==4);
					if (m_cbCardIndex[wChairID][cbCardIndex]!=4) 
						return false;

					//设置变量
					bPublic=false;
					cbWeaveIndex=m_cbWeaveItemCount[wChairID]++;
					m_WeaveItemArray[wChairID][cbWeaveIndex].cbPublicCard=false;
					m_WeaveItemArray[wChairID][cbWeaveIndex].wProvideUser=wChairID;
					m_WeaveItemArray[wChairID][cbWeaveIndex].cbWeaveKind=cbOperateCode;
					m_WeaveItemArray[wChairID][cbWeaveIndex].cbCenterCard=cbOperateCard;
				}

				//删除扑克
				m_cbCardIndex[wChairID][cbCardIndex]=0;

				m_bGangStatus = true;

				//杠牌得分
				S64 lScore = 1L;
				U8 cbGangIndex = m_GangScore[wChairID].cbGangCount++;
				if(bPublic)
				{
					m_GangScore[wChairID].lScore[cbGangIndex][wChairID] += lScore;//m_pGameServiceOption->lCellScore*2;
					if(wGangProvideUser != INVALID_CHAIR)
					{
						m_GangScore[wChairID].lScore[cbGangIndex][wGangProvideUser] -= lScore;//-m_pGameServiceOption->lCellScore*2;
					}
				}
				else
				{
					for( U16 i = 0; i < GAME_PLAYER; i++ )
					{
						if(  i == wChairID ) continue;

						m_GangScore[wChairID].lScore[cbGangIndex][i] = -lScore;
						m_GangScore[wChairID].lScore[cbGangIndex][wChairID] += lScore;
					}
				}

				//构造结果
				CMD_S_OperateResult OperateResult;
				OperateResult.wOperateUser=wChairID;
				OperateResult.wProvideUser=wChairID;
				OperateResult.cbOperateCode=cbOperateCode;
				OperateResult.cbOperateCard=cbOperateCard;

				//发送消息
				SendTableData("WC_CHESS_OPERATE_RESULT", OperateResult);
				//m_pITableFrame->SendLookonData(INVALID_CHAIR,SUB_S_OPERATE_RESULT,&OperateResult,sizeof(OperateResult));


				//发送信息
				CMD_S_GangScore gs;
				gs.wChairId = wChairID;
				ZeroMemory( &gs,sizeof(gs) );
				gs.cbXiaYu = bPublic?false:true;
				for( U16 i = 0; i < GAME_PLAYER; i++ )
				{
					if( i == wChairID ) continue;

					gs.lGangScore[i] = -lScore;
					gs.lGangScore[wChairID] += lScore;
				}

				SendTableData("WC_CHESS_GANG_SCORE", gs);
				//m_pITableFrame->SendLookonData( INVALID_CHAIR,SUB_S_GANG_SCORE,&gs,sizeof(gs) );

				//效验动作
				bool bAroseAction=false;
				if (bPublic==true) bAroseAction=EstimateUserRespond(wChairID,cbOperateCard,EstimatKind_GangCard);

				//发送扑克
				if (bAroseAction==false)
				{
					DispatchCardData(wChairID,true);
				}
				return true;
			}
		case WIK_CHI_HU:		//吃胡操作
			{
				setSpecialType();//结束后设置鸟牌
				//吃牌权位
				if (m_cbOutCardCount==0)
				{
					m_wProvideUser = m_wCurrentUser;
					m_cbProvideCard = m_cbSendCardData;
				}

				//普通胡牌
				U8 cbWeaveItemCount=m_cbWeaveItemCount[wChairID];
				tagWeaveItem * pWeaveItem=m_WeaveItemArray[wChairID];
				m_GameLogic.RemoveCard(m_cbCardIndex[wChairID],&m_cbProvideCard,1);
				m_dwChiHuKind[wChairID] = AnalyseChiHuCard(m_cbCardIndex[wChairID],pWeaveItem,cbWeaveItemCount,m_cbProvideCard,
					m_ChiHuRight[wChairID]);
				m_cbCardIndex[wChairID][m_GameLogic.SwitchToCardIndex(m_cbProvideCard)]++;

				//结束信息
				m_cbChiHuCard=m_cbProvideCard;

				ProcessChiHuUser( wChairID,false);

				
				OnEventGameConclude( INVALID_CHAIR,GER_NORMAL );

				return true;
			}
		}

		return true;
	}

	return false;
}

//发送操作
bool CChessTable::SendOperateNotify()
{
	//发送提示
	for (U16 i=0;i<GAME_PLAYER;i++)
	{
		if (m_cbUserAction[i]!=WIK_NULL)
		{
			//构造数据
			CMD_S_OperateNotify OperateNotify;
			OperateNotify.wResumeUser=m_wResumeUser;
			OperateNotify.cbActionCard=m_cbProvideCard;
			OperateNotify.cbActionMask=m_cbUserAction[i];

			//发送数据
			SendTableData(i, "WC_CHESS_OPERATE_NOTIFY", OperateNotify);
			//m_pITableFrame->SendLookonData(i,SUB_S_OPERATE_NOTIFY,&OperateNotify,sizeof(OperateNotify));
		}
	}

	return true;
}

//派发扑克
bool CChessTable::DispatchCardData(U16 wCurrentUser,bool bTail)
{
	//状态效验
	ASSERT(wCurrentUser!=INVALID_CHAIR);
	if (wCurrentUser==INVALID_CHAIR)
		return false;

	//丢弃扑克
	if ((m_wOutCardUser!=INVALID_CHAIR)&&(m_cbOutCardData!=0))
	{
		m_cbDiscardCount[m_wOutCardUser]++;
		m_cbDiscardCard[m_wOutCardUser][m_cbDiscardCount[m_wOutCardUser]-1]=m_cbOutCardData;
	}

	//荒庄结束
	if (m_cbLeftCardCount==0)
	{
		m_cbChiHuCard=0;
		m_wProvideUser=INVALID_CHAIR;
		OnEventGameConclude(m_wProvideUser,GER_NORMAL);

		return true;
	}

	//设置变量
	m_cbOutCardData=0;
	m_wCurrentUser=wCurrentUser;
	m_wOutCardUser=INVALID_CHAIR;
	tagGangCardResult GangCardResult;

	//杠后炮
	if( m_bGangOutStatus )
	{
		m_bGangOutStatus = false;
	}

	//发牌处理
	if (m_bSendStatus==true)
	{
		//发送扑克
		m_cbSendCardCount++;
		m_cbSendCardData= DoDispatchCardData();

		//胡牌判断
		CChiHuRight chr;
		m_wProvideUser = wCurrentUser;
		m_cbUserAction[wCurrentUser]|= AnalyseChiHuCard(m_cbCardIndex[wCurrentUser],m_WeaveItemArray[wCurrentUser],m_cbWeaveItemCount[wCurrentUser],m_cbSendCardData,chr);

		//加牌
		m_cbCardIndex[wCurrentUser][m_GameLogic.SwitchToCardIndex(m_cbSendCardData)]++;

		//设置变量
		m_cbProvideCard = m_cbSendCardData;

		//杠牌判断
		if (m_cbLeftCardCount>0)
		{
			m_cbUserAction[wCurrentUser]|=m_GameLogic.AnalyseGangCard(m_cbCardIndex[wCurrentUser],
				m_WeaveItemArray[wCurrentUser],m_cbWeaveItemCount[wCurrentUser],GangCardResult);
		}
	}


	//构造数据
	CMD_S_SendCard SendCard;
	SendCard.wCurrentUser=wCurrentUser;
	SendCard.bTail = bTail;
	SendCard.cbActionMask=m_cbUserAction[wCurrentUser];
	SendCard.cbCardData=(m_bSendStatus==true)?m_cbSendCardData:0x00;
	//AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(GetPlayer(wCurrentUser));
	//if (NULL == pAccount)
	//	return false;

	//CMemGuard Buffer(256 MEM_GUARD_PARAM);
	//Base::BitStream sendPacket(Buffer.get(), 256);
	//stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "CLIENT_SERVER_SendOneCards", GetPlayer(wCurrentUser), SERVICE_CLIENT);
	//sendPacket.writeBits(sizeof(SendCard)<<3,&SendCard);

	//pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
	//SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(),sendPacket);
	//发送数据
	//m_pITableFrame->SendLookonData(INVALID_CHAIR,SUB_S_SEND_CARD,&SendCard,sizeof(SendCard));
	SendTableData("WC_CHESS_SEND_CARD", SendCard);

	return true;
}

//响应判断
bool CChessTable::EstimateUserRespond(U16 wCenterUser, U8 cbCenterCard, enEstimatKind EstimatKind)
{
	//变量定义
	bool bAroseAction=false;

	//用户状态
	ZeroMemory(m_bResponse,sizeof(m_bResponse));
	ZeroMemory(m_cbUserAction,sizeof(m_cbUserAction));
	ZeroMemory(m_cbPerformAction,sizeof(m_cbPerformAction));

	//动作判断
	for (U16 i=0;i<GAME_PLAYER;i++)
	{
		//用户过滤
		if (wCenterUser==i) continue;

		//出牌类型
		if (EstimatKind==EstimatKind_OutCard)
		{
			////碰牌判断
			m_cbUserAction[i]|=m_GameLogic.EstimatePengCard(m_cbCardIndex[i],cbCenterCard);

			//杠牌判断
			if (m_cbLeftCardCount>0) 
			{
				m_cbUserAction[i]|=m_GameLogic.EstimateGangCard(m_cbCardIndex[i],cbCenterCard);
			}
		}

		//吃胡判断
		CChiHuRight chr;
		U8 cbWeaveCount=m_cbWeaveItemCount[i];
		m_cbUserAction[i]|= AnalyseChiHuCard(m_cbCardIndex[i],m_WeaveItemArray[i],cbWeaveCount,cbCenterCard,chr);
		

		//结果判断
		if (m_cbUserAction[i]!=WIK_NULL) 
			bAroseAction=true;
	}

	//长沙麻将吃操作
	//if (m_cbGameTypeIdex == GAME_TYPE_108)
	if(!hasRule(GAME_TYPE_ZZ_CHI))
	{
		m_cbUserAction[m_wCurrentUser]|= m_GameLogic.EstimateEatCard(m_cbCardIndex[m_wCurrentUser],cbCenterCard);

		//结果判断
		if (m_cbUserAction[m_wCurrentUser]!=WIK_NULL) 
			bAroseAction=true;
	}

	//结果处理
	if (bAroseAction==true) 
	{
		//设置变量
		m_wProvideUser=wCenterUser;
		m_cbProvideCard=cbCenterCard;
		m_wResumeUser=m_wCurrentUser;
		m_wCurrentUser=INVALID_CHAIR;
		
		//********自动
		for (U16 i=1;i<GAME_PLAYER;i++)
		{
			if(m_cbUserAction[i])
			{
				if(m_cbUserAction[i] & WIK_CHI_HU)
					OnUserOperateCard(i,WIK_CHI_HU, m_cbProvideCard);
				else if(m_cbUserAction[i] & WIK_XIAO_HU)
					OnUserOperateCard(i,WIK_XIAO_HU, m_cbProvideCard);
				else if(m_cbUserAction[i] & WIK_GANG)
					OnUserOperateCard(i,WIK_GANG, m_cbProvideCard);
				else if(m_cbUserAction[i] & WIK_PENG)
					OnUserOperateCard(i,WIK_PENG, m_cbProvideCard);
				else if(m_cbUserAction[i] & WIK_RIGHT)
					OnUserOperateCard(i,WIK_RIGHT, m_cbProvideCard);
				else if(m_cbUserAction[i] & WIK_CENTER)
					OnUserOperateCard(i,WIK_CENTER, m_cbProvideCard);
				else if(m_cbUserAction[i] & WIK_LEFT)
					OnUserOperateCard(i,WIK_LEFT, m_cbProvideCard);
				else
					OnUserOperateCard(i,m_cbUserAction[i], m_cbProvideCard);

				if(GetGameStatus() == GS_MJ_FREE)
					break;
			}
		}

		//if(m_wCurrentUser == INVALID_CHAIR && GetGameStatus() != GS_MJ_FREE)
		//	ASSERT(false);
		//发送提示
		SendOperateNotify();
		return true;
	}

	return false;
}

U16 CChessTable::GetChiHuActionRank(const CChiHuRight & ChiHuRight)
{	
	U16 wFanShu = 0;
	//if( !(ChiHuRight&CHR_ZI_MO).IsEmpty() )
	//	wFanShu *= 2;
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

//
void CChessTable::ProcessChiHuUser( U16 wChairId, bool bGiveUp)
{
	if( !bGiveUp )
	{
		//引用权位
		CChiHuRight &chr = m_ChiHuRight[wChairId];

		FiltrateRight( wChairId,chr );
		U16 wFanShu = 1;
		wFanShu = GetChiHuActionRank(chr);
		S64 lChiHuScore = wFanShu*1;//m_pGameServiceOption->lCellScore;
		
		//杠上炮,呼叫转移
		//if( !(chr&CHR_GANG_SHANG_PAO).IsEmpty() )
		//{
		//	U8 cbGangIndex = (m_GangScore[m_wProvideUser].cbGangCount>1)?(m_GangScore[m_wProvideUser].cbGangCount-1):0;
		//	//一炮多响的情况下,胡牌者平分杠得分
		//	U8 cbChiHuCount = 0;
		//	for( U16 i = 0; i < GAME_PLAYER; i++ )
		//		if( m_cbPerformAction[i]==WIK_CHI_HU ) cbChiHuCount++;
		//	if( cbChiHuCount == 1 )
		//	{
		//		S64 lScore = m_GangScore[m_wProvideUser].lScore[cbGangIndex][wChairId];
		//		m_GangScore[m_wProvideUser].lScore[cbGangIndex][wChairId] = m_GangScore[m_wProvideUser].lScore[cbGangIndex][m_wProvideUser];
		//		m_GangScore[m_wProvideUser].lScore[cbGangIndex][m_wProvideUser] = lScore;
		//	}
		//	else
		//	{
		//		S64 lGangScore = m_GangScore[m_wProvideUser].lScore[cbGangIndex][m_wProvideUser]/cbChiHuCount;
		//		lGangScore = getMax(lGangScore,(S64)2);//m_pGameServiceOption->lCellScore);
		//		for( U16 i = 0; i < GAME_PLAYER; i++ )
		//		{
		//			if( m_cbPerformAction[i]==WIK_CHI_HU )
		//				m_GangScore[m_wProvideUser].lScore[cbGangIndex][i] = lGangScore;
		//		}
		//		m_GangScore[m_wProvideUser].lScore[cbGangIndex][m_wProvideUser] = 0;
		//		for( U16 i = 0; i < GAME_PLAYER; i++ )
		//		{
		//			if( i != m_wProvideUser)
		//				m_GangScore[m_wProvideUser].lScore[cbGangIndex][m_wProvideUser] += m_GangScore[m_wProvideUser].lScore[cbGangIndex][i];
		//		}
		//	}
		//}
		//抢杠杠分不算
		//else if( !(chr&CHR_QIANG_GANG_HU).IsEmpty() )
		if( !(chr&CHR_QIANG_GANG_HU).IsEmpty() )
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

		//平湖不能点炮
		if( m_wProvideUser == wChairId || lChiHuScore == 1)
		{
			for( U16 i = 0; i < GAME_PLAYER; i++ )
			{
				if( i == wChairId ) continue;

				//胡牌分
				m_lGameScore[i] -= lChiHuScore;
				m_lGameScore[wChairId] += lChiHuScore;
				
				//中鸟分
				//m_lGameScore[i] -= m_cbNiaoPick* 1;//;m_pGameServiceOption->lCellScore;
				//m_lGameScore[wChairId] += m_cbNiaoPick* 1;//m_pGameServiceOption->lCellScore;
			}
			++m_cbChiHuNum[wChairId][0];
		}
		//点炮
		else
		{

			m_lGameScore[m_wProvideUser] -= lChiHuScore;
			m_lGameScore[wChairId] += lChiHuScore;
			++m_cbChiHuNum[wChairId][1];
			++m_cbChiHuNum[m_wProvideUser][2];
			//m_lGameScore[m_wProvideUser] -= m_cbNiaoPick*1;// m_pGameServiceOption->lCellScore;
			//m_lGameScore[wChairId] += m_cbNiaoPick* 1;//m_pGameServiceOption->lCellScore;
		}

		//设置变量
		m_wProvider[wChairId] = m_wProvideUser;
		m_bGangStatus = false;
		m_bGangOutStatus = false;
		m_wBankerUser = wChairId;

		//发送消息
		CMD_S_ChiHu ChiHu;
		ChiHu.wChiHuUser = wChairId;
		ChiHu.wProviderUser = m_wProvideUser;
		ChiHu.lGameScore = m_lGameScore[wChairId];
		ChiHu.cbCardCount = m_GameLogic.GetCardCount( m_cbCardIndex[wChairId] );
		ChiHu.cbChiHuCard = m_cbProvideCard;
		ChiHu.cbBaoPai = 0;
		SendTableData("WC_CHESS_CHI_HU", ChiHu);
		//m_pITableFrame->SendLookonData( INVALID_CHAIR,SUB_S_CHI_HU,&ChiHu,sizeof(ChiHu) );
	}

	return;
}

bool CChessTable::hasRule(U8 cbRule)
{
	return FvMask::HasAny(m_dwGameRuleIdex,_MASK_(cbRule));
}

U8 CChessTable::AnalyseChiHuCard(const U8 cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], U8 cbWeaveCount, U8 cbCurrentCard, CChiHuRight &ChiHuRight)
{
	bool bSelfSendCard = (m_wProvideUser == m_wCurrentUser);
	//变量定义
	U8 cbChiHuKind=WIK_NULL;
	bool bQiXiaoDui = false;
	CAnalyseItemArray AnalyseItemArray;

	if (hasRule(GAME_TYPE_ZZ_ZIMOHU) && !bSelfSendCard)
	{
		return WIK_NULL;
	}

	//设置变量
	AnalyseItemArray.clear();
	ChiHuRight.SetEmpty();
	//ChiHuRight |= CHR_SHU_FAN;
	int nGenCount = 0;

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
	//小七对特殊处理
	if( m_GameLogic.IsQiXiaoDui(cbCardIndex,WeaveItem,cbWeaveCount,cbCurrentCard,nGenCount) )
	{
		if(nGenCount == 1)
		{
			ChiHuRight |= CHR_HAOHUA_QI_XIAO_DUI;
		}
		else if(nGenCount == 2)
		{
			ChiHuRight |= CHR_DHAOHUA_QI_XIAO_DUI;
		}
		else if (nGenCount == 3)
		{
			ChiHuRight |= CHR_THAOHUA_QI_XIAO_DUI;
		}
		else 
		{
			ChiHuRight |= CHR_QI_XIAO_DUI;
		}

		cbChiHuKind = WIK_CHI_HU;
		bQiXiaoDui = true;
	}	

	//if( !ChiHuRight.IsEmpty() )
	//	cbChiHuKind = WIK_CHI_HU;
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
	if (!bValue && !bQiXiaoDui)
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

		//碰碰和
		if( m_GameLogic.IsPengPeng(&pAnalyseItem) ) 
			ChiHuRight |= CHR_PENGPENG_HU;
		cbChiHuKind = WIK_CHI_HU;
		break;
	}

	if( cbChiHuKind == WIK_CHI_HU )
	{
		if(cbWeaveCount == 4 &&  m_GameLogic.IsDanDiao(cbCardIndex,cbCurrentCard))
		{
			ChiHuRight |= CHR_QUAN_QIU_REN;
		}
		//清一色牌
		if( m_GameLogic.IsQingYiSe(cbCardIndex,WeaveItem,cbWeaveCount,cbCurrentCard) )
		{
			ChiHuRight |= CHR_QING_YI_SE;
		}
		if( m_GameLogic.IsZiYiSe(cbCardIndex,WeaveItem,cbWeaveCount,cbCurrentCard))
			ChiHuRight |= CHR_ZI_YI_SE;
	}

	//if (bSelfSendCard)
	//{
	//	cbChiHuKind = WIK_CHI_HU;
	//	ChiHuRight = CHR_SHU_FAN;
	//	ChiHuRight |= CHR_ZI_MO;
	//}

	return cbChiHuKind;
}

//
void CChessTable::FiltrateRight( U16 wChairId,CChiHuRight &chr )
{
	//权位增加
	//抢杠
	if( m_wCurrentUser == INVALID_CHAIR && m_bGangStatus )
	{
		chr |= CHR_QIANG_GANG_HU;
	}
	if (m_cbLeftCardCount==0)
	{
		chr |= CHR_HAI_DI_LAO;
	}
	//附加权位
	//杠上花
	if( m_wCurrentUser==wChairId && m_bGangStatus )
	{
		chr |= CHR_GANG_KAI;
	}
	//杠上炮
	if( m_bGangOutStatus && !m_bGangStatus )
	{
		chr |= CHR_GANG_SHANG_PAO;
	}
}

void CChessTable::SetTableScore(S64 lScore)
{
	for(int i = 0; i < GAME_PLAYER; ++i)
	{
		m_TableScore[i] = lScore;
	}
}

S64 CChessTable::getTableScore(U32 nSlot)
{
	return m_TableScore[nSlot];
}

//////////////////////////////////////////////////////////////////////////
