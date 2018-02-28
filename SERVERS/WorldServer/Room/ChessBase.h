#ifndef CHESS_BASE_H
#define CHESS_BASE_H
#include "BASE/types.h"
#include <assert.h>
//////////////////////////////////////////////////////////
//公共宏定义
//#pragma pack(1)

//组件属性
#define GAME_PLAYER					4									//游戏人数
enum GS_GAME_STATUS
{
	GS_MJ_FREE = 0,//空闲状态
	GS_MJ_PLAY,//游戏状态
	GS_MJ_XIAOHU,//小胡状态
};

//数组维数
#define CountArray(Array) (sizeof(Array)/sizeof(Array[0]))
//常量定义
#define MAX_WEAVE					4									//最大组合
#define MAX_INDEX					34									//最大索引
#define MAX_COUNT					14									//最大数目
#define MAX_REPERTORY_108			108									//最大库存(万索同)
#define MAX_REPERTORY_112			112									//红中麻将最大库存(万索同中)
#define MAX_REPERTORY_136			136									//最大库存(万索同东南西北中发白)
//参数定义
#define INVALID_CHAIR				0xFFFF								//无效椅子
#define INVALID_TABLE				0xFFFF								//无效桌子

#define MAX_NIAO_CARD				 6									//最大中鸟数


#define MAX_RIGHT_COUNT				1									//最大权位U32个数	

#define GAME_TYPE_108				0		//
#define GAME_TYPE_112				1		//
#define GAME_TYPE_136				2       //


#define MAKEU16(a, b)      ((U16)(((U8)(((U32)(a)) & 0xff)) | ((U16)((U8)(((U32)(b)) & 0xff))) << 8))
#define MAKEU32(a, b)      ((U32)(((U16)(((U32)(a)) & 0xffff)) | ((U32)((U16)(((U32)(b)) & 0xffff))) << 16))

#define ZeroMemory(dst,size) \
{\
	memset(dst, 0, size);\
	}\

#define ASSERT(x) assert((x))
#define VERIFY(x) if(!(x)) {assert((x)); }

#define CopyMemory(dst,src,size) \
	{\
		memcpy(dst,src,size);\
	}\
//////////////////////////////////////////////////////////////////////////

//组合子项
struct CMD_WeaveItem
{
	U8							cbWeaveKind;						//组合类型
	U8							cbCenterCard;						//中心扑克
	U8							cbPublicCard;						//公开标志
	U16							wProvideUser;						//供应用户
};

//////////////////////////////////////////////////////////////////////////
//服务器命令结构

#define SUB_S_GAME_START			100									//游戏开始
#define SUB_S_OUT_CARD				101									//出牌命令
#define SUB_S_SEND_CARD				102									//发送扑克
#define SUB_S_OPERATE_NOTIFY		104									//操作提示
#define SUB_S_OPERATE_RESULT		105									//操作命令
#define SUB_S_GAME_END				106									//游戏结束
#define SUB_S_TRUSTEE				107									//用户托管
#define SUB_S_CHI_HU				108									//
#define SUB_S_GANG_SCORE			110									//

//结束原因
#define GER_NORMAL					0x00								//常规结束
#define GER_DISMISS					0x01								//游戏解散
#define GER_USER_LEAVE				0x02								//用户离开
#define GER_NETWORK_ERROR			0x03								//网络错误

//积分类型
#define SCORE_TYPE_NULL				0x00								//无效积分
#define SCORE_TYPE_WIN				0x01								//胜局积分
#define SCORE_TYPE_LOSE				0x02								//输局积分
#define SCORE_TYPE_DRAW				0x03								//和局积分
#define SCORE_TYPE_FLEE				0x04								//逃局积分
#define SCORE_TYPE_PRESENT			0x10								//赠送积分
#define SCORE_TYPE_SERVICE			0x11								//服务积分

struct tagScoreInfo
{
	U8								cbType;								//积分类型
	S64								lScore;								//用户分数
	S64								lGrade;								//用户成绩
	S64								lRevenue;							//游戏税收
};

//游戏状态
struct CMD_S_StatusFree
{
	S64								lCellScore;							//基础金币
	U16								wBankerUser;						//庄家用户
	bool							bTrustee[GAME_PLAYER];				//是否托管
	U16								wCurrentUser;						//当前用户
	U64								nTableID;							//桌子ID
	S64								lGameScore[GAME_PLAYER];			//游戏积分
};

//游戏状态
struct CMD_S_StatusPlay
{
	//游戏变量
	S64								lCellScore;									//单元积分
	U16								wBankerUser;								//庄家用户
	U16								wCurrentUser;								//当前用户

	//状态变量
	U8								cbActionCard;								//动作扑克
	U8								cbActionMask;								//动作掩码
	U8								cbLeftCardCount;							//剩余数目
	bool							bTrustee[GAME_PLAYER];						//是否托管
	U16								wWinOrder[GAME_PLAYER];						//

	//出牌信息
	U16								wOutCardUser;								//出牌用户
	U8								cbOutCardData;								//出牌扑克
	U8								cbDiscardCount[GAME_PLAYER];				//丢弃数目
	U8								cbDiscardCard[GAME_PLAYER][60];				//丢弃记录

	//扑克数据
	U8								cbCardCount;								//扑克数目
	U8								cbCardData[MAX_COUNT];						//扑克列表
	U8								cbSendCardData;								//发送扑克

	//组合扑克
	U8								cbWeaveCount[GAME_PLAYER];					//组合数目
	CMD_WeaveItem					WeaveItemArray[GAME_PLAYER][MAX_WEAVE];		//组合扑克
	U64								nTableID;							        //桌子ID
	U16								nChairId;							        //slotID
	S64								lGameScore[GAME_PLAYER];			//游戏积分
	//U32								nTableNum;
	//std::string						m_strUserUrlImg[GAME_PLAYER];			//玩家微信头像
	//std::string						m_strUserNickName[GAME_PLAYER];			//玩家微信昵称
};

//游戏开始
struct CMD_S_GameStart
{
	U32								lSiceCount;									//骰子点数
	U32								wBankerUser;								//庄家用户
	U32								wCurrentUser;								//当前用户
	U8								cbUserAction;								//用户动作
	U8								cbCardData[MAX_COUNT*GAME_PLAYER];			//扑克列表
	U8								cbLeftCardCount;							//
	U8								cbXiaoHuTag;                           //小胡标记 0 没小胡 1 有小胡；

};

//出牌命令
struct CMD_S_OutCard
{
	U16								wOutCardUser;						//出牌用户
	U8								cbOutCardData;						//出牌扑克
};

//发送扑克
struct CMD_S_SendCard
{
	U8								cbCardData;							//扑克数据
	U8								cbActionMask;						//动作掩码
	U16								wCurrentUser;						//当前用户
	bool							bTail;								//末尾发牌
};


//操作提示
struct CMD_S_OperateNotify
{
	U16								wResumeUser;						//还原用户
	U8								cbActionMask;						//动作掩码
	U8								cbActionCard;						//动作扑克
};

//操作命令
struct CMD_S_OperateResult
{
	U16								wOperateUser;						//操作用户
	U16								wProvideUser;						//供应用户
	U8								cbOperateCode;						//操作代码
	U8								cbOperateCard;						//操作扑克
};

//游戏结束
struct CMD_S_GameEnd
{
	U8								cbCardCount[GAME_PLAYER];			//
	U8								cbCardData[GAME_PLAYER][MAX_COUNT];	//
	//结束信息
	U16								wProvideUser[GAME_PLAYER];			//供应用户
	U32								dwChiHuRight[GAME_PLAYER];			//胡牌类型
	U32								dwStartHuRight[GAME_PLAYER];		//起手胡牌类型
	S64								lStartHuScore[GAME_PLAYER];			//起手胡牌分数

	//积分信息
	S64								lGameScore[GAME_PLAYER];			//游戏积分
	int								lGameTax[GAME_PLAYER];				//

	U16								wWinOrder[GAME_PLAYER];				//胡牌排名

	S64								lGangScore[GAME_PLAYER];//详细得分
	U8								cbGenCount[GAME_PLAYER];			//
	U16								wLostFanShu[GAME_PLAYER][GAME_PLAYER];
	U16								wLeftUser;	//

	//组合扑克
	U8								cbWeaveCount[GAME_PLAYER];					//组合数目
	CMD_WeaveItem					WeaveItemArray[GAME_PLAYER][MAX_WEAVE];		//组合扑克


	U8								cbCardDataNiao[MAX_NIAO_CARD];	// 鸟牌
	U8								cbNiaoCount;	//鸟牌个数
	U8								cbNiaoPick;	//中鸟个数
};

//用户托管
struct CMD_S_Trustee
{
	bool							bTrustee;							//是否托管
	U32								wChairID;							//托管用户
};

//
struct CMD_S_ChiHu
{
	U16								wChiHuUser;							//
	U16								wProviderUser;						//
	U8								cbChiHuCard;						//
	U8								cbCardCount;						//
	S64								lGameScore;							//
	U8								cbWinOrder;							//
	U8						        cbBaoPai;
};

//
struct CMD_S_GangScore
{
	U16								wChairId;							//
	U8								cbXiaYu;							//
	S64								lGangScore[GAME_PLAYER];			//
};

//////////////////////////////////////////////////////////////////////////
//客户端命令结构

#define SUB_C_OUT_CARD				1									//出牌命令
#define SUB_C_OPERATE_CARD			3									//操作扑克
#define SUB_C_TRUSTEE				4									//用户托管
#define SUB_C_XIAOHU				5									//小胡

//出牌命令
struct CMD_C_OutCard
{
	U8								cbCardData;							//扑克数据
};

//操作命令
struct CMD_C_OperateCard
{
	U8								cbOperateCode;						//操作代码
	U8								cbOperateCard;						//操作扑克
};

//用户托管
struct CMD_C_Trustee
{
	bool							bTrustee;							//是否托管	
};

//起手小胡
struct CMD_C_XiaoHu
{
	U8								cbOperateCode;						//操作代码
	U8								cbOperateCard;						//操作扑克
};


//////////////////////////////////////////////////////////////////////////
//#pragma pack()
#endif
