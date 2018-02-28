//#ifndef _RANDTEAMPROCESS_H_
//#define _RANDTEAMPROCESS_H_
//
//class RandTeam;
//
//class RandTeamProcess
//{
//public:
//	virtual ~RandTeamProcess(void) {}
//
//    virtual bool PreProcess(RandTeam* pRandTeam) {return true;}
//	virtual bool Process(RandTeam* pRandTeam) = 0;
//};
//
//class RandTeamFailProcess : public RandTeamProcess
//{
//public:
//	//method from RandTeamProcess
//	virtual bool Process(RandTeam* pRandTeam);
//};
//
////组队完毕,进入副本确认
//class RandTeamEnterProcess : public RandTeamProcess
//{
//public:
//	//method from RandTeamProcess
//	virtual bool Process(RandTeam* pRandTeam);
//};
//
////组队等待列表
////开始组队或后续补充人员时设置成这个对象
//class RandTeamWaitProcess : public RandTeamProcess
//{
//public:
//	RandTeamWaitProcess(void);
//
//	//method from RandTeamProcess
//	virtual bool Process(RandTeam* pRandTeam);
//protected:
//	bool __SelectPlayer(RandTeam* pRandTeam,U32 charType,U32 count);
//};
//
//#endif /*_RANDTEAMPROCESS_H_*/