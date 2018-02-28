#ifndef __CHESS_TABLE_HUNAN_H
#define __CHESS_TABLE_HUNAN_H
#include "ChessTable.h"

#define ZZ_ZHANIAO0		0		//扎鸟2个
#define ZZ_ZHANIAO2		2		//扎鸟2个
#define ZZ_ZHANIAO4		4		//扎鸟4个
#define ZZ_ZHANIAO6		6		//扎鸟6个

class CChessHN : public CChessTable
{
public:
	typedef CChessTable Parent;
	static const U32 GAME_TYPE_ZZ_258				 = Parent::NextMask + 0;
	static const U32 GAME_TYPE_ZZ_ZHANIAO2			 = Parent::NextMask + 1;	//扎鸟2个
	static const U32 GAME_TYPE_ZZ_ZHANIAO4			 = Parent::NextMask + 2;	//扎鸟4个
	static const U32 GAME_TYPE_ZZ_ZHANIAO6			 = Parent::NextMask + 3;	//扎鸟6个
	static const U32 GAME_TYPE_ZZ_HONGZHONG			 = Parent::NextMask + 4;	//红中玩法
	static const U32 NextMask						 = Parent::NextMask + 5;
public:
	//构造函数
	CChessHN();
	//析构函数
	virtual ~CChessHN();

	virtual bool InitTable() {return true;}
	virtual void ResetTable();
	virtual void GameStart();
	virtual U8 AnalyseChiHuCard(const U8 cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], U8 cbWeaveCount, U8 cbCurrentCard, CChiHuRight &ChiHuRight,bool bSelfSendCard);
	virtual U16 GetChiHuActionRank(const CChiHuRight & ChiHuRight);
	virtual U8 DoDispatchCardData();

	//通知客户端表现
	template<class T>
	bool SendTableData(U16 wChairID, const char* msg, T& obj);
	template<class T>
	bool SendTableData(const char* msg, T& obj);

	void GameStart_ZZ();							//游戏开始
	void GameStart_CS();
	U16 GetChiHuActionRank_ZZ(const CChiHuRight & ChiHuRight);
	U16 GetChiHuActionRank_CS(const CChiHuRight & ChiHuRight);
	U8 AnalyseChiHuCardZZ(const U8 cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], U8 cbWeaveCount, U8 cbCurrentCard, CChiHuRight &ChiHuRight,bool bSelfSendCard);
	U8 AnalyseChiHuCardCS(const U8 cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], U8 cbWeaveCount, U8 cbCurrentCard, CChiHuRight &ChiHuRight);
	U8 AnalyseChiHuCardCS_XIAOHU(const U8 cbCardIndex[MAX_INDEX], CChiHuRight &ChiHuRight);

protected:
	virtual void ProcessChiHuUser( U16 wChairId, bool bGiveUp );
	virtual void setSpecialType() {setNiaoCard();}
	void setNiaoCard();
	U8 GetNiaoCardNum();

protected:
	U8								m_cbRepertoryCard_108[MAX_REPERTORY_108];		//库存扑克
	U8								m_cbRepertoryCard_112[MAX_REPERTORY_112];	//库存扑克

public:
	U64								m_lStartHuScore[GAME_PLAYER];			//起手胡分数	
	CChiHuRight						m_StartHuRight[GAME_PLAYER];			//起手胡权位
protected:
	U8								m_cbCardDataNiao[MAX_NIAO_CARD];		// 鸟牌
	U8								m_cbNiaoCount;							//鸟牌个数
	U8								m_cbNiaoPick;	
};

#endif //__CHESS_TABLE_GUIX