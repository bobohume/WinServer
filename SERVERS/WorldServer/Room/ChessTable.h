#ifndef TABLE_FRAME_SINK_HEAD_FILE
#define TABLE_FRAME_SINK_HEAD_FILE

#pragma once
#include <hash_map>
#include "ChessLogic.h"
#include "RoomBase.h"

//////////////////////////////////////////////////////////////////////////
//枚举定义

//效验类型
enum enEstimatKind
{
	EstimatKind_OutCard,			//出牌效验
	EstimatKind_GangCard,			//杠牌效验
};

//杠牌得分
struct tagGangScore
{
	U8		cbGangCount;							//杠个数
	S64		lScore[MAX_WEAVE][GAME_PLAYER];			//每个杠得分
};

//////////////////////////////////////////////////////////////////////////

//游戏桌子类
class CChessTable : public ITable
{
public:
	static const U32 GAME_TYPE_ZZ_ZIMOHU			 = 0;	//只能自模胡
	static const U32 GAME_TYPE_ZZ_QIANGGANGHU		 = 1;	//可抢杠胡
	static const U32 GAME_TYPE_ZZ_SUOHU				 = 2;	//索胡
	static const U32 GAME_TYPE_ZZ_CHANGHU			 = 3;	//长胡
	static const U32 GAME_TYPE_ZZ_DISBAND			 = 4;	//解散标志
	static const U32 GAME_TYPE_ZZ_CHI				 = 5;	//吃的标志
	static const U32 GAME_TYPE_ZZ_START				 = 6;	//开始标志
	static const U32 NextMask						 = 7;

	//游戏变量
protected:
	U16								m_wBankerUser;							//庄家用户
	S64								m_lGameScore[GAME_PLAYER];				//游戏得分
	U8								m_cbCardIndex[GAME_PLAYER][MAX_INDEX];	//用户扑克
	//****************************************************************************//
	//m_cbCardIndex[i][j]数值代表个数，下标代表一万（1），两万（2）以此类推
	//****************************************************************************//
	U8							    m_bDisband[GAME_PLAYER];				//解散标志
	bool							m_bReady[GAME_PLAYER];					//准备标志
	bool							m_bTrustee[GAME_PLAYER];				//是否托管
	tagGangScore					m_GangScore[GAME_PLAYER];				//
	U16								m_wLostFanShu[GAME_PLAYER][GAME_PLAYER];//


	//出牌信息
protected:
	U16								m_wOutCardUser;							//出牌用户
	U8								m_cbOutCardData;						//出牌扑克
	U8								m_cbOutCardCount;						//出牌数目
	U8								m_cbDiscardCount[GAME_PLAYER];			//丢弃数目
	U8								m_cbDiscardCard[GAME_PLAYER][55];		//丢弃记录

	//发牌信息
protected:
	U8								m_cbSendCardData;						//发牌扑克
	U8								m_cbSendCardCount;						//发牌数目
	U8								m_cbLeftCardCount;						//剩余数目

	//运行变量
protected:
	U16								m_wResumeUser;							//还原用户
	U16								m_wCurrentUser;							//当前用户
	U16								m_wProvideUser;							//供应用户
	U8								m_cbProvideCard;						//供应扑克

	//状态变量
protected:
	bool							m_bSendStatus;							//发牌状态
	bool							m_bGangStatus;							//抢杆状态
	bool							m_bGangOutStatus;						//

	//用户状态
public:
	bool							m_bResponse[GAME_PLAYER];				//响应标志
	U8								m_cbUserAction[GAME_PLAYER];			//用户动作
	U8								m_cbOperateCard[GAME_PLAYER];			//操作扑克
	U8								m_cbPerformAction[GAME_PLAYER];			//执行动作
	std::string						m_strUserUrlImg[GAME_PLAYER];			//玩家微信头像
	std::string						m_strUserNickName[GAME_PLAYER];			//玩家微信昵称
	//游戏统计
	U8								m_cbChiHuNum[GAME_PLAYER][3];			 //玩家总结算
	//组合扑克
protected:
	U8								m_cbWeaveItemCount[GAME_PLAYER];		 //组合数目
	tagWeaveItem					m_WeaveItemArray[GAME_PLAYER][MAX_WEAVE];//组合扑克

	//结束信息
protected:
	U8								m_cbChiHuCard;							 //吃胡扑克
	U32								m_dwChiHuKind[GAME_PLAYER];				 //吃胡结果
	CChiHuRight						m_ChiHuRight[GAME_PLAYER];				 //
	U16								m_wProvider[GAME_PLAYER];				 //

	//组件变量
protected:
	U32								m_GameStatus;							 //游戏状态
	CChessLogic						m_GameLogic;							 //游戏逻辑
	U8								m_cbGameTypeIdex;						 //游戏类型
	U32								m_dwGameRuleIdex;						 //游戏规则
private:
	S64								m_TableScore[GAME_PLAYER];				 //房间分数
	S32								m_cbTableNum;							 //房卡数
	U32								m_CreateTime;							 //创建时间
	//函数定义
public:
	//构造函数
	CChessTable();
	//析构函数
	virtual ~CChessTable();

public:
	//初始化
	virtual bool InitTable() = 0;
	//复位桌子
	virtual void ResetTable();
	virtual U32 GetMaxPlayers() {return GAME_PLAYER; }
	virtual bool HasDisband()	{return hasRule(GAME_TYPE_ZZ_DISBAND);}

	//比赛接口
public:
	//设置基数
	virtual void SetGameBaseScore(U32 lBaseScore){};
	void SetGameStatus(U32 stauts){ m_GameStatus = stauts; }
	U32	 GetGameStatus()		{return m_GameStatus; }
	inline S32	GetTableNum()	{return m_cbTableNum; }
	inline void SetTableNum(S32 cbNum) {m_cbTableNum = cbNum; }
	void SetTableScore(S64 lScore);
	S64  getTableScore(U32 nSlot);
	void SetPrivateInfo(U8 bGameTypeIdex,U32 bGameRuleIdex);
	void AddGameRule(U32 nRule);
	void ClearGameRule(U32 nRule);
	bool ReadyGame(U32 nSlot);
	bool ClearDisbandGame();
	U8   DisbandGame(U32 nSlot, U8 nAgreeFlag);
	U8   AgreeDisband(U32 nSlot) {return m_bDisband[nSlot];}

	//游戏事int i = 0; i < 游戏开始
	virtual bool OnEventGameStart();
	//游戏结束
	virtual bool OnEventGameConclude(U16 wChairID,  U8 cbReason);
	//发送场景
	virtual bool OnEventSendGameScene(U16 wChiarID, bool bSendSecret, U64 nTableId, U32 playerId);

	void Shuffle(U8* RepertoryCard,int nCardCount); //洗牌
	//----一以下方法具体看各种地方不一样，具体实例
	virtual void GameStart();					//游戏开始
	virtual U8 DoDispatchCardData() = 0;
	virtual U8 AnalyseChiHuCard(const U8 cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], U8 cbWeaveCount, U8 cbCurrentCard, CChiHuRight &ChiHuRight);//分析牌
	virtual U16 GetChiHuActionRank(const CChiHuRight & ChiHuRight);

	//定时器事件
	virtual void TimeProcess(U32 value);
public:
	bool IsUserVaildCard(U16 wChairID, U8 cbCardData);
	bool IsCurrentUser(U16 wChairID);
	//用户出牌
	bool OnUserOutCard(U16 wChairID, U8 cbCardData);
	//用户操作
	bool OnUserOperateCard(U16 wChairID, U8 cbOperateCode, U8 cbOperateCard);
	//通知客户端表现
	template<class T>
	bool SendTableData(U16 wChairID, const char* msg, T& obj);
	template<class T>
	bool SendTableData(const char* msg, T& obj);
	bool hasRule(U8 cbRule);
	bool DisbandGameSendtoPlayer(CChessTable* pTable);

protected:
	//
	virtual void ProcessChiHuUser( U16 wChairId, bool bGiveUp);
	virtual void setSpecialType() {};
	//发送操作
	bool SendOperateNotify();
	//派发扑克
	bool DispatchCardData(U16 wCurrentUser,bool bTail=false);
	//响应判断
	bool EstimateUserRespond(U16 wCenterUser, U8 cbCenterCard, enEstimatKind EstimatKind);
	//
	void FiltrateRight( U16 wChairId,CChiHuRight &chr );
};

//////////////////////////////////////////////////////////////////////////

#endif
