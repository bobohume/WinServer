#ifndef DINNER_BASE_H
#define DINNER_BASE_H
#include "BASE/types.h"
#include <string>
#include <memory>
#include <set>
#include "Common/UtilString.h"

#define MAX_DINNER_PLAYER 5
#define MAX_DINNER_NUM 20
#define MAX_STATE_NUM 3
#define MAX_SCOREDATA_NUM 120
#define MAX_INFODATA_NUM 32
enum enDinnerState
{
	DINNER_STATE_NONE = 0,
	DINNER_STATE_BEGIN,
	DINNER_STATE_END,
};

enum {
	DINNER_RITE = 0,
	DINNER_CULTURE,
	DINNER_BATTLE,
	DINNER_EXEC_TIME = 5 * 4,
	//DINNER_EXEC_TIME = 5
};

enum enDinnerSkillType {
	DINNER_SKILL_HUATI		= 1,
	DINNER_SKILL_QUANJIE,
	DINNER_SKILL_QISHI,
	DINNER_SKILL_SHOULI,
	DINNER_SKILL_NEILIAN,
	DINNER_SKILL_XINJI,
	DINNER_SKILL_CHENGZHUO,
	DINNER_SKILL_MEIHUO,
	DINNER_SKILL_MAX = DINNER_SKILL_MEIHUO,
};

struct stDinnerInfo 
{
	S32 Rite;
	S32 Culture;
	S32 Battle;
	S32 Charm;

	stDinnerInfo() : Rite(0), Culture(0), Battle(0), Charm(0)
	{
	}

	template<class T>
	stDinnerInfo& operator=(const T other) {
		Battle = other->Battle;
		Culture = other->Culture;
		Rite = other->Rite;
		Charm = other->Charm;
		return *this;
	}

	void WriteData(Base::BitStream* stream)
	{
		stream->writeInt(Rite,    Base::Bit32);
		stream->writeInt(Culture, Base::Bit32);
		stream->writeInt(Battle,  Base::Bit32);
		stream->writeInt(Charm,   Base::Bit32);
	}
};

//准备信息
struct stDinnerReadyInfo
{
	S32 PlayerId;
	S32 PrincessID;
	S32 JoinType;
	std::string PlayerName;
	stDinnerInfo Info;

	stDinnerReadyInfo() : PlayerId(0), PrincessID(0), PlayerName(""), JoinType(0)
	{
	}
};

struct stDinner
{
	S32 Id;
	S32 DinnerState;
	S32 DinnerNum;
	S8  DinnerType;
	S32 CreateTime;
	S32 StartTime;
	S32 PlayerId[MAX_DINNER_PLAYER];
	S32 PrincessID[MAX_DINNER_PLAYER];
	std::string PlayerName[MAX_DINNER_PLAYER];
	S32  JoinType[MAX_DINNER_PLAYER];
	char PrincessSkill[MAX_DINNER_PLAYER][MAX_INFODATA_NUM];//skill db
	char ScoreData[MAX_DINNER_PLAYER][MAX_SCOREDATA_NUM];//score db
	char InfoData[MAX_DINNER_PLAYER][MAX_INFODATA_NUM];//info db

	//NO DB
	S32 ExecTime;
	int Score[MAX_DINNER_PLAYER][MAX_DINNER_NUM+1];//ScoreData
	S8	Skill[MAX_DINNER_PLAYER][MAX_DINNER_NUM];//princessSkill
	stDinnerInfo Info[MAX_DINNER_PLAYER];//InfoData
	std::set<S32> PlayerSet;//旁观列表

	stDinner() : Id(0), DinnerState(0), DinnerNum(0), CreateTime(0), ExecTime(0), DinnerType(DINNER_RITE), StartTime(0)
	{
		memset(PlayerId, 0, sizeof(PlayerId));
		memset(PrincessID, 0, sizeof(PrincessID));
		memset(JoinType, 0, sizeof(JoinType));
		memset(Score, 0, sizeof(Score));
		memset(Skill, 0, sizeof(Skill));
		memset(PrincessSkill, 0, sizeof(PrincessSkill));
		memset(ScoreData, 0, sizeof(ScoreData));
		memset(InfoData, 0, sizeof(InfoData));
	}


	void WriteData(Base::BitStream* stream)
	{
		stream->writeInt(Id, Base::Bit32);
		stream->writeInt(DinnerState, Base::Bit8);
		stream->writeInt(DinnerNum,   Base::Bit8);
		stream->writeInt(DinnerType,  Base::Bit8);
		stream->writeInt(StartTime,  Base::Bit32);

		stream->writeInt(MAX_DINNER_PLAYER, Base::Bit16);
		for (auto i = 0; i < MAX_DINNER_PLAYER; ++i) {
			stream->writeFlag(true);
			stream->writeInt(PlayerId[i], Base::Bit32);
			stream->writeString(Util::MbcsToUtf8(PlayerName[i]), 64);
			stream->writeInt(PrincessID[i], Base::Bit32);
			stream->writeInt(JoinType[i], Base::Bit8);
			stream->writeInt(Score[i][MAX_DINNER_NUM], Base::Bit32);
			Info[i].WriteData(stream);
		}

		stream->writeInt(MAX_DINNER_PLAYER, Base::Bit16);
		for (auto i = 0; i < MAX_DINNER_PLAYER; ++i) {
			stream->writeFlag(true);
			stream->writeInt(MAX_DINNER_NUM, Base::Bit16);
			for (auto j = 0; j < MAX_DINNER_NUM; ++j) {
				stream->writeFlag(true);
				stream->writeInt(Skill[i][j], Base::Bit8);
				stream->writeInt(Score[i][j], Base::Bit32);
			}
		}
	}
};

typedef std::tr1::shared_ptr<stDinner> DinnerRef;
typedef std::tr1::shared_ptr<stDinnerReadyInfo> DinnerReadyRef;
#endif //DINNER_BASE_H