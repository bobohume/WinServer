#ifndef PVP_BASE_H
#define PVP_BASE_H
#include "CardBase.h"
#include "Common/UtilString.h"
#include <deque>

#define GAME_PLAYER	2 //游戏人数
#define GAME_BUFF	4 //游戏人数
struct stPvpInfo
{
	std::string UID;
	S32		CardID;
	S16		Level;
	S32		Agi;
	S32		Int;
	S32		Str;
	S32		Hp;
	S32		MaxHp;
	S32		Peerless;
	S32		AgiAtk;
	S32		IntAtk;
	S32		StrAtk;
	S32		SpeAtk;
	S32		Atk;
	S32		BattleVal;

	//battle no import
	S32		BaseAgi;
	S32		BaseInt;
	S32		BaseStr;
	S32		BaseHp;
	S32		BaseBattleVal;
	S16		AddAgi;
	S16		AddInt;
	S16		AddStr;
	S16		AddHp;

	stPvpInfo() :Agi(0), Int(0), Str(0), Hp(0), Peerless(0), AgiAtk(0),\
		IntAtk(0), StrAtk(0), SpeAtk(0), Atk(0), CardID(0), Level(0), MaxHp(0),\
		BattleVal(0), BaseAgi(0), BaseInt(0), BaseStr(0), BaseHp(0), BaseBattleVal(0), \
		AddAgi(0), AddInt(0), AddStr(0), AddHp(0)
	{
	}

	stPvpInfo& operator =(const CardRef pData)
	{
		Agi = pData->Agi;
		Int = pData->Int;
		Str = pData->Str;
		UID = pData->UID;
		CardID = pData->CardID;
		Level = pData->Level;
		MaxHp = Hp = (Agi + Int + Str) * 5;
		AgiAtk = Agi;
		IntAtk = Int;
		StrAtk = Str;
		SpeAtk = (Agi + Int + Str);
		BattleVal = pData->BattleVal;

		BaseAgi = Agi;
		BaseInt = Int;
		BaseStr = Str;
		BaseHp  = Hp;
		BaseBattleVal = BattleVal;
		return *this;
	}

	void WriteData(Base::BitStream* stream)
	{
		stream->writeInt(CardID, Base::Bit32);
		stream->writeInt(Level, Base::Bit16);
		stream->writeInt(Agi, Base::Bit32);
		stream->writeInt(Int, Base::Bit32);
		stream->writeInt(Str, Base::Bit32);
		stream->writeInt(Hp, Base::Bit32);
		stream->writeInt(MaxHp, Base::Bit32);
		stream->writeInt(Peerless, Base::Bit32);
		stream->writeInt(Atk, Base::Bit32);
		stream->writeInt(BattleVal, Base::Bit32);
		stream->writeInt(BaseAgi, Base::Bit32);
		stream->writeInt(BaseInt, Base::Bit32);
		stream->writeInt(BaseStr, Base::Bit32);
		stream->writeInt(BaseHp, Base::Bit32);
		stream->writeInt(BaseBattleVal, Base::Bit32);
	}
};

enum enGameState
{
	PVP_GAME_FREE,
	PVP_GAME_READY,
	PVP_GAME_BEGIN,
	PVP_GAME_PLAY,
	PVP_GAME_END,
};

enum enPveTyep
{
	PVP_PVE,
	PVP_PVE_HATE,
	PVP_PVE_RANK,
	PVP_PVP,
};

struct stPvpTable
{
	S64 nPlayerID[GAME_PLAYER];
	S32  nAction[GAME_PLAYER];
	S8   nBuff[GAME_PLAYER][GAME_BUFF];
	S8   nBuffId[GAME_PLAYER][GAME_BUFF];
	bool bTrustee[GAME_PLAYER];
	S8	 nSize[GAME_PLAYER];
	S8	 nCSize[GAME_PLAYER];
	S8	 nWinNum[GAME_PLAYER];
	bool bJumpBattle[GAME_PLAYER];//是否跳过战斗直接显示结果

	S8  nGameStatus;//游戏状态0,unbegin, 1,ready, 2,begin,  3,disband
	S8  nPvpType;//pvp类型0消息,1仇人,2,pve,3pvp
	S32 nTime;
	S32 nScore;//记录一局战斗的分数变动
	S32 nPvpBattleTime;//记录一局战斗的开始时间

	stPvpTable() : nGameStatus(PVP_GAME_FREE), nPvpType(0), nTime(0), nScore(0), nPvpBattleTime(0)
	{
		memset(nPlayerID, 0, sizeof(nPlayerID));
		memset(nAction,  -1, sizeof(nAction));
		memset(nBuff, 0, sizeof(nBuff));
		memset(nBuffId, 0, sizeof(nBuffId));
		memset(bTrustee, 0, sizeof(bTrustee));
		memset(nSize, 0, sizeof(nSize));
		memset(nCSize, 0, sizeof(nCSize));
		memset(nWinNum, 0, sizeof(nWinNum));
		memset(bJumpBattle, 0, sizeof(nWinNum));
	}
};

struct stPveInfo
{
	S32 PlayerID;
	S32 TargetID;
	S8  Type;
	S32 Unqiue;
	S8	CardNum;
	S32	Score;
	S32 AddScore;
	S32 BattleVal;
	S32 Time;

	//no db
	std::string TargetName;

	stPveInfo() :PlayerID(0), TargetID(0), Type(0), CardNum(0), Score(0), Time(0),
		TargetName(""), AddScore(0), BattleVal(0), Unqiue(0)
	{
	}

	void WriteData(Base::BitStream* stream)
	{
		stream->writeInt(TargetID, Base::Bit32);
		stream->writeString(Util::MbcsToUtf8(TargetName), 50);
		stream->writeInt(Type, Base::Bit8);
		stream->writeInt(CardNum, Base::Bit8);
		stream->writeInt(Score, Base::Bit32);
		stream->writeInt(AddScore, Base::Bit32);
		stream->writeInt(BattleVal, Base::Bit32);
		stream->writeInt(Time, Base::Bit32);
	}
};

typedef std::tr1::shared_ptr<stPvpTable>	PvpTableRef;
typedef std::tr1::shared_ptr<stPvpInfo>		PvpInfoRef;
typedef std::deque<PvpInfoRef>				DATA_DEQ;
typedef std::vector<PvpInfoRef>				DATA_VEC;
typedef std::tr1::shared_ptr<stPveInfo>		PveInfoRef;
#endif //PVP_BASE_H