#ifndef __CHESS_TABLE_GUIXI_H
#define __CHESS_TABLE_GUIXI_H
#include "ChessTable.h"

class CChessGX : public CChessTable
{
public:
	typedef CChessTable Parent;
	static const U32 GAME_TYPE_ZZ_MOBAO		 = Parent::NextMask + 0;
	static const U32 NextMask				 = Parent::NextMask + 1;

	//构造函数
	CChessGX();
	//析构函数
	virtual ~CChessGX();

	virtual bool InitTable() {return true;}
	virtual void ResetTable();
	virtual void GameStart();
	virtual U8 DoDispatchCardData();
	virtual U16 GetChiHuActionRank(const CChiHuRight & ChiHuRight);
	virtual void setSpecialType();
	virtual void ProcessChiHuUser( U16 wChairId, bool bGiveUp);
	virtual U8 AnalyseChiHuCard(const U8 cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], U8 cbWeaveCount, U8 cbCurrentCard, CChiHuRight &ChiHuRight);
protected:
	U8		m_cbRepertoryCard_136[MAX_REPERTORY_136];	//库存扑克
};

#endif //__CHESS_TABLE_GUIX