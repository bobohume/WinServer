#ifndef TOPRANK_MANAGER_H
#define TOPRANK_MANAGER_H

#include "TopBase.h"
#include <set>
#include <unordered_map>

// 排行榜管理器
class CTopManager 
{
public:
	CTopManager();
	virtual ~CTopManager();

	static CTopManager* Instance();

	// db
	void LoadDB();
	bool UpdateDB(int type, TopRankRef pData);
	bool InsertDB(std::string UID, int type, TopRankRef pData);
	bool DeleteDB(std::string UID, int type);
	void DeleteDB(int type);
	void DeletePlayerFromDB(int playerId, int type);

	// 更新排行榜,决定哪个需要被刷新或保存到数据库
	void Update(U32 );
	int getRankForPlayer(int type, int playerId);      //用于查询指定玩家指定榜单的排名值
	TopRankRef GetData(int type, std::string UID);
	TopRankRef GetData(int type, int playerId);
	
	void UpTop(int type, int playerId);

	void NewInData(int type, std::string UID, S32 Value0, S32 Value1 = 0, S32 Value2 = 0);
	void NewInData(int type, S32 nPlayerID, S32 Value0, S32 Value1 = 0, S32 Value2 = 0);
	void NewInDataEx(int type, S32 nPlayerID, S32 OldValue0, S32 Value0, S32 Value1 = 0, S32 Value2 = 0);
	std::vector<std::string> GetRank(int type, std::string UID, S32 nValue);
	std::vector<std::string> GetRank(int type, U32 start, U32 end);

	bool HandleOpenPveRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleOpenPveWinRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleOpenTopRankRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool HandleUpTopRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet);
	bool __isDesc(int type);//降序排列
	void Clear(U32 type);
	void DelTop(std::string UID, int type = -1);       //匹配玩家id删除相应的排行榜内存数据,type==-1为删除全部榜单
	static void Export(struct lua_State* L);
protected:
	void NewInData(int type, TopRankRef pData);//插入元素
	//清楚排行榜数据
	void InsertData(TopRankRef pData);
	bool isSpecial(int type);        //是否是特殊榜单(例如奇珍异兽)

	void DeleteOverdue(int iType, time_t curTime);;
	bool __isNeedOverDue(int type);//是否过期

	void UpdateTop(int iType);
private:
	//排行项,保存当前的排行榜数据
	typedef std::unordered_map<std::string, TopRankRef> TOPRANKITEMS;
	TOPRANKITEMS m_topRanks[eTopType_End];
};

#define TOPMGR CTopManager::Instance()
#endif//TOPRANK_MANAGER_H
