#ifndef __CHALLENGEBASE_H__
#define __CHALLENGEBASE_H__

#include "Base/BitStream.h"
#include "Common/PlayerStructEx.h"
#include <list>

#define MAX_BATTLE_REPORT 5
#define MAX_BATTLE_RANK   9999

//竞技场中用来进行离线计算的玩家数据
class ChallengeData
{
public:
    U32	SaveUID;    //角色版本标志  

    int quality;    //品质总值
    int nSkills;    //技能熟练度
    int nTalents;   //天赋投入点数
    int level;      //角色当前等级
    int nPetSkills; //灵兽综合值
    int nPetLevel;  //灵兽等级

    void Pack(Base::BitStream* stream)
    {
        stream->writeBits(sizeof(ChallengeData)<<3,this);
    }

    void UnPack(Base::BitStream* stream)
    {
        stream->readBits(sizeof(ChallengeData)<<3, this);
    }
};

class BattleReport
{
public:
    BattleReport(void)
    {
        memset(this,0,sizeof(BattleReport));
    }

    bool self;          //是否自己发起的挑战
    U32  time;          //挑战时间
    U32  targetId;      //被挑战或挑战的玩家对象
    char targetName[32];
    int  oldRank;       //前后2次排名结果
    int  newRank;
};

class ChallengeMember
{
public:
    ChallengeMember(void)
    {
        playerId    = 0;
        rank        = 9999;
        wins        = 0;
        total       = 0;
        arenaPoints = 10;
        onlineLimit = 0;
        offlineLimit= 0;
        extraLimit  = 0;
        nChallengeBy= 0;
        nLosed      = 0;
        targetId    = 0;
        offlineMode = false;
        starter     = false;
        active      = false;
        curPoints   = 0;
    }

    U32 playerId;
    int rank;               //排名
    U32 wins;               //连胜
    U32 total;              //总计
    int arenaPoints;        //竞技熟练
    int onlineLimit;        //在线挑战次数
    int offlineLimit;       //离线挑战次数
    int extraLimit;         //额外挑战次数
    U32 nChallengeBy;       //今天被挑战次数
    U32 nLosed;             //今天失败次数

    std::list<BattleReport> reports;

    U32  targetId;
    bool offlineMode;
    bool starter;
    bool active;
    int  curPoints;

    ChallengeData data;

    
};

#endif//__CHALLENGEBASE_H__