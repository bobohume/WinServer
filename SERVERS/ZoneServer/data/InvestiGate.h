//
//  InvestiGate.h
//  HelloLua
//
//  Created by th on 14-2-12.
//
//

#ifndef __HelloLua__InvestiGate__
#define __HelloLua__InvestiGate__


#include "Common/DataFile.h"
#include <string.h>
#include <hash_map>

class CInvestiGateData;
class CInvestiGateDataRes {

public:
	enum{
		InvestiGate_BEGIN_ID=5000001,
		InvestiGate_END_ID=  5000010,
	};

	typedef stdext::hash_map<S32, S32> LAYERGATEMAP;//层数对映关卡map
	typedef LAYERGATEMAP::iterator LAYERGATEITR;

	CInvestiGateDataRes();
	~CInvestiGateDataRes();

	typedef stdext::hash_map<S32,CInvestiGateData*> INVESTIGATEDATAMAP;
	typedef INVESTIGATEDATAMAP::iterator INVESTIGATEDATA_ITR;

	bool read();
	void close();
	void clear();
	CInvestiGateData* getInvestiGateData(S32 InvestiGateId);
	CInvestiGateData* getInvestiGateDataByLayer(S32 layer);
private:
	INVESTIGATEDATAMAP m_DataMap;
	LAYERGATEMAP m_LayerGateMap;
};

class CInvestiGateData
{
public:
	enum
	{
		MAX_INVESTIGATE_GOODS_NUM = 3,
	};

	CInvestiGateData(): m_InvestiGateId(0),m_Layer(0),m_LayerName(""),m_length(0),m_width(0),\
		m_RoomNum(0),m_MonsterGroup(0),m_MonsterNum(0),m_MonsterLevel(0), \
		m_NpcGroup(0),m_NpcNum(0),m_TrackGroup(0),m_TrackNum(0),m_EarthGroup(0),\
		m_Wall(0),m_Water(0),m_Null(0),m_Way(0),m_Door(0), m_RoomName("")
	{
		memset(m_GoodsGroup, 0, sizeof(m_GoodsGroup));
		memset(m_GoodsNum, 0, sizeof(m_GoodsNum));
	};
	~CInvestiGateData(){};

	S32 m_InvestiGateId;//关卡ID
	S32 m_Layer;//层数
	std::string m_LayerName;//关卡名字
	S32 m_length;//关卡长
	S32 m_width;//关卡宽
	S32 m_GateType;//关卡类型
	S32 m_RoomNum;//房间数量
	S32 m_MonsterGroup;//怪物组
	S32 m_MonsterNum;//怪物数量
	S32 m_MonsterLevel;//怪物等阶
	S32 m_NpcGroup;//npc组
	S32 m_NpcNum;//npc数量
	S32 m_GoodsGroup[MAX_INVESTIGATE_GOODS_NUM];//物品组
	S32 m_GoodsNum[MAX_INVESTIGATE_GOODS_NUM];//物品数量
	S32 m_TrackGroup;//陷阱组
	S32 m_TrackNum;//陷阱组数量
	S32 m_EarthGroup;//地表组
	S32 m_Wall;//墙
	S32 m_Water;//水
	S32 m_Null;//虚空
	S32 m_Way;//通道
	S32 m_Door;//门
	std::string m_RoomName;//固定房间名称
};

extern CInvestiGateDataRes* g_InvestiGateDataMgr;



#endif /* defined(__HelloLua__InvestiGate__) */
