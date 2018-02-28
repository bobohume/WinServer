#ifndef BASE_MANAGER_H
#define BASE_MANAGER_H

#include <unordered_map>
#include "CommLib/CommLib.h"
#include "WINTCP/IPacket.h"
class DB_Execution;
enum enOpError
{
	OPERATOR_NONE_ERROR = 0,
	OPERATOR_PARAMETER_ERROR,//参数不合法
	OPERATOR_HASNO_CARD,//卡牌不存在
	OPERATOR_HASNO_MONEY,//身上金钱不足
	OPERATOR_HASNO_CARDID,//卡牌ID不存在
	OPERATOR_HASNO_SKILLID,//技能ID不存在
	OPERATOR_HASNO_ITEM,//物品不足
	OPERATOR_HASNO_ACHIEVEMENT,//功勋不足
	OPERATOR_CARD_MAXLEVEL,//卡牌达到最大等级
	OPERATOR_NOLEARN_SKILLID,//技能ID不能学习
	OPERATOR_SKILL_MAX,//技能已学满
	OPERATOR_SKILL_MAXLEVEL,//最高等级
	OPERATOR_CARD_NOFITCOLOR,//卡牌品质不满足
	OPERATOR_ABILITY_LEVEL_LIMIT,//提升品质，受等级限制
	OPERATOR_GROW_MAX,//成长上线
	OPERATOR_ALREADY_OWN_CARD,//已经拥有改卡牌
	OPERATOR_HIRE_FAILED,//库用失败
	OPERATOR_DISPATCHCARD_FAILED,//派遣失败
	OPERATOR_SKILL_LEARN_FAILED,//技能学习失败


	OPERATOR_HASNO_BATTLEINFO,//战斗信息不存在
	OPERATOR_BATTLE_CARD_EXIST,//选将重复
	OPERATOR_HASNO_GATEID,//据点ID不存在
	OPERATOR_HASNO_TROOPS,//兵力不足
	OPERATOR_GATE_PASSED,//据点不能被重复攻打

	OPERATOR_HASNO_PLAYER,//玩家不存在
	OPERATOR_IMPOSE_ERROR,//征收失败

	OPERATOR_OFFICER_CARD_EXIST,//大臣已任命，不能重复任命
	OPERATOR_OFFICER_EXIST,//官位已被任命
	OPERATOR_HASNO_OFFICER,//官位ID不存在
	OPERATOR_OFFICER_CARD_NOFIT,//大臣条件不满足
	OPERATOR_OFFICER_DESIGNATE_FAIL,//任命失败
	OPERATOR_OFFICER_RDESIGNATE_FAIL,//撤销官位失败

	OPERATOR_CARD_LVLIMIT_FAIL,//提升爵位失败
	OPERATOR_CARD_LEVELNOFIT,//等级未满足升级爵位条件
	OPERATOR_CARD_LEVELMAX,//升级爵位上限

	OPERATOR_PRINCESS_EMPTY,//妃子不存在
	OPERATOR_PRINCESS_TURN_CARD_FAILED,//翻牌错误
	OPERATOR_PRINCESS_HASNO_VIM,//精力不够
	OPERATOR_HASNO_GOLD,//黄金不够
	OPERATOR_HASHNO_PRINCESS,//妃子不存在
	OPERATOR_PRINCESSPOS_LIMIT,//妃位人数上限
	OPERATOR_HASNO_RITE,//礼仪不满足条件
	OPERATOR_DISPATCHPRINCESS_FAILED,//处理后宫事务失败
	OPERATOR_HASNO_CHARM,//魅力不足
	OPERATOR_PRINCESS_ADVANCE_TIME_FAILED,//加速完成失败
	OPERATOR_PRINCESS_LEARN_RITE_FAILED,//妃子学习礼仪失败
	OPERATOR_PRINCESS_SKILL_LVUP_FAILED,//妃子升级天赋失败
	OPERATOR_PRINCESS_FINISH_FAILED,//完成失败
	OPERATOR_PRINCESS_ADDMAXSLOT_FAILED,//增加开发队列失败
	OPERATOR_PRINCESS_PLANT_FAILED,//种花失败

	OPERATOR_PVE_NO_PLAYER,//玩家数据不存在
	OPERATOR_PVE_ERROR,//玩家错误

	OPERATOR_HASHNO_PRINCE,//皇子不存在
	OPERATOR_HASHNO_ACTIVITY,//皇子活力不足
	OPERATOR_PRINCE_EXIST_CARD,//太傅已被任命
	OPERATOR_PRINCE_ABILITY_NOREACH,//皇子赐爵能力不够
	OPERATOR_PRINCE_LIMIT_LEVEL,//皇子等级上限
	OPERATOR_EXILE_PRINCE_FAILED,//放逐皇子条件不满足
	OPERATOR_PRINCE_OPEN_HOURSE_FAILED,//皇子开府条件不满足

	OPERATOR_HASHNO_TECHNOLOGY,//科技不存在
	OPERATOR_HASHNO_TECHNOLOGYID,//科技ID不存在
	OPERATOR_HASHNO_INT,//学识不够
	OPERATOR_HASHNO_ORE,//矿石不够
	OPERATOR_TECHNOLOGY_COOLDOWN,//矿石不够

	OPERATOR_PVE_BUFF_ERROR,//BUFF不能重复购买
	OPERATOR_PVE_ADDBUFF_ERROR,//蓝宝石不足

	OPERATOR_SKILL_LEVELUP_ERROR,//请升级对应科技
	OPERATOR_ITEM_EQUIP_ERROR,//该物品不是装备
	OPERATOR_PRINCE_MAX_ERROR,//栏位不足产出皇子失败

	OPERATOR_MAXLIMIT_SHOPTIMES,//超过购买上限
	OPERATOR_HASHNO_STRENGTH,//体力不够
	OPERATOR_CITY_DEVELOPED,//停留只能对城市开发一次
	OPERATOR_CITY_MAX_LEVEL,//开发登记上限
	OPERATOR_MAX_PRINCE,//皇子已满，请购买上限

	OPERATOR_PVE_MAXTIMES,//挑战次数上限
	OPERATOR_PVE_NO_CARD,//没有可参战斗的大臣
	OPERATOR_PVE_BATTLE,//处于战斗中不能再次挑战
	OPERATOR_PVE_UNKNOW_ERROR,//PVE未知错误
	OPERATOR_PRINCE_ACTIVITY_MAX,//皇子活力上限
	OPERATOR_BAN_WORD,//含有非法字符
	OPERATOR_SET_FACE_COOLDOWN,//设置头像CD中

	OPERATOR_DINNER_ALREADY,//玩家已经参加宴会
	OPERATOR_DINNER_NO_PLAYER,//玩家不在宴会中
	OPERATOR_DINNER_NO,//宴会准备重，不能重复加入
	OPERATOR_DINNER_BEGIN,//宴会开始不能进入
	OPERATOR_DINNER_MAX_PLAYER,//宴会人已满
	OPERATOR_PRINCESS_NO,//妃子不存在

	OPERATOR_HASNO_COIN,//货币不足不能兑换
	OPERATOR_ALREADY_BUY,//不能重复购买

	OPERATOR_HASNO_CONQUEST,//功勋点不足

	OPERATOR_DINNER_NOROOM,//暂无举办的宴会
	OPERATOR_HASNO_AVENGE_ITEM,//挑战令不足
	OPERATOR_HIRE_CARD_MAX,//大臣席位不足，请升级对应科技
	OPERATOR_HASNO_IMPOSE_ITEM,	//征收令不足

	OPERATOR_ALREADY_ORG,//已经拥有联盟
	OPERATOR_CREATE_ORG_FAILED,//创建联盟失败
	OPERATOR_ORG_NO,//联盟不存在
	OPERATOR_ORG_APPLY_ALREADY,//不能重复申请
	OPERATOR_ORG_MASTER_NOT,//不是联盟管理员
	OPERATOR_ORG_MEMBER_NOT,//不是联盟成员
	OPERATOR_ORG_MAX_PLAYER,//联盟成员上限
	OPERATOR_ORG_MASTER,//盟主不能离开联盟
	OPERATOR_ORG_MASTER2,//盟主和副盟主不能北开除
	OPERATOR_ORG_DONATE_MAX,//捐献次数已满
	OPERATOR_ORG_LEAVE,//联盟等级不够
	OPERATOR_ORG_SKILLPOINT,//联盟技能点不够
	OPERATOR_ORG_CONTRIBUTE,//联盟贡献点不够
	OPERATOR_ORG_GIFTEXP,//联盟宝箱进度不够
	OPERATOR_ORG_NOFOUNT,//未找到改联盟

	OPERATOR_PVP_COOLDOWN,//PVP cd中
	OPERATOR_BOSS_NO,//bossID不存在
	OPERATOR_BOSS_PLAYER_NO,//玩家不在boss副本中
	OPERATOR_BOSS_PLAYER_EXIST,//玩家已在boss副本中
	OPERATOR_BOSS_BATTLE_NUM,//必须上阵三个大臣
	OPERATOR_ORG_BOSS_MAX,//联盟boss挑战上限
};

/************************************************************************/
/* 管理对象
/************************************************************************/
template<class T>
class CBaseManager
{
protected:
	typedef std::unordered_map<U32, T>					 PLAYER_MAP;
	typedef typename PLAYER_MAP::iterator				 PLAYER_ITR;
public:
	CBaseManager(){}
	virtual ~CBaseManager()
	{
		for (PLAYER_ITR itr = m_PlayerMap.begin(); itr != m_PlayerMap.end(); ++itr)
		{
		}
		m_PlayerMap.clear();
	}

	T GetData(U32 nPlayerID)
	{
		PLAYER_ITR itr = m_PlayerMap.find(nPlayerID);
		if (itr != m_PlayerMap.end())
		{
			return itr->second;
		}

		return T();
	}

	T CreateData(U32 nPayerID)
	{
		T pData = new T();
		pData->PlayerID = nPayerID;
		return pData;
	}

	void AddData(U32 nPlayerID, T pData)
	{
		if (pData)
		{
			m_PlayerMap.insert(PLAYER_MAP::value_type(nPlayerID, pData));
		}
	}

	void DeleteData(U32 nPlayerID)
	{
		PLAYER_ITR itr = m_PlayerMap.find(nPlayerID);
		if (itr != m_PlayerMap.end())
		{
			SAFE_DELETE(itr->second);
			m_PlayerMap.erase(itr);
		}
	}

	void RemovePlayer(U32 nPlayerID)
	{
		PLAYER_ITR  itr = m_PlayerMap.find(nPlayerID);
		if (itr != m_PlayerMap.end())
		{
			m_PlayerMap.erase(itr);
		}
	}

	//---------DB Operator---------//
	DB_Execution* LoadDB(U32 nPlayerID){}
	virtual bool UpdateDB(U32 nPlayerID, T pData){ return true;}
	virtual	bool InsertDB(U32 nPlayerID, T pData) { return true; }
	virtual bool DeleteDB(U32 nPlayerID) { return true; }

protected:
	PLAYER_MAP m_PlayerMap;//玩家管理类
};


template<class T, class U = U32>
class CBaseManagerEx
{
protected:
	typedef std::unordered_map<U, T>					 DATA_MAP;
	typedef typename DATA_MAP::iterator					 DATA_ITR;

	typedef std::unordered_map<U32, DATA_MAP>			 PLAYER_MAP;
	typedef typename PLAYER_MAP::iterator				 PLAYER_ITR;
public:
	CBaseManagerEx() {}
	virtual ~CBaseManagerEx()
	{
		for (PLAYER_ITR itr = m_PlayerMap.begin(); itr != m_PlayerMap.end(); ++itr)
		{
			for (DATA_ITR iter = itr->second.begin(); iter != itr->second.end(); ++iter)
			{
			}
			itr->second.clear();
		}
		m_PlayerMap.clear();
	}

	T GetData(U32 nPlayerID, U ID)
	{
		PLAYER_ITR itr = m_PlayerMap.find(nPlayerID);
		if (itr != m_PlayerMap.end())
		{
			DATA_MAP& DataMap = itr->second;
			DATA_ITR iter = DataMap.find(ID);
			if (iter != DataMap.end())
			{
				return iter->second;
			}
		}

		return T();
	}

	T CreateData(U32 nPayerID, U ID)
	{
		T* pData = new T();
		pData->ID = ID;
		pData->PlayerID = nPayerID;
		return pData;
	}

	bool AddData(U32 nPlayerID, U ID, T pData)
	{
		if (pData)
		{
			return m_PlayerMap[nPlayerID].insert(DATA_MAP::value_type(ID, pData)).second;
		}

		return false;
	}

	void DeleteData(U32 nPlayerID, U nID)
	{
		PLAYER_ITR itr = m_PlayerMap.find(nPlayerID);
		if (itr != m_PlayerMap.end())
		{
			DATA_MAP& DataMap = itr->second;
			DATA_ITR iter = DataMap.find(nID);
			if (iter != DataMap.end())
			{
				DataMap.erase(iter);
			}
		}
	}

	void RemovePlayer(U32 nPlayerID)
	{
		PLAYER_ITR  itr = m_PlayerMap.find(nPlayerID);
		if (itr != m_PlayerMap.end())
		{
			DATA_MAP& PlayerMap = itr->second;
			PlayerMap.clear();
			m_PlayerMap.erase(itr);
		}
	}

	//---------DB Operator---------//
	DB_Execution* LoadDB(U32 nPlayerID) {}
	virtual bool UpdateDB(U32 nPlayerID, T pData) { return true;  }
	virtual	bool InsertDB(U32 nPlayerID, T pData) { return true; }
	virtual bool DeleteDB(U32 nPlayerID, U nID) { return true; }

protected:
	PLAYER_MAP m_PlayerMap;//玩家管理类
};

struct stGo
{
	stGo() {};
	template<class T>
	void operator -(T func)
	{
		func();
	}
};

#define lamber stGo()-
#endif//BASE_MANAGER_H
