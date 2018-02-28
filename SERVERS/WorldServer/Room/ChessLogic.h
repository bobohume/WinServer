#ifndef CHESS_LOGIC_H
#define CHESS_LOGIC_H
#include "ChessBase.h"
#include "BASE/types.h"
#include <deque>
#pragma once
//#pragma pack(1)
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//逻辑掩码

#define	MASK_COLOR					0xF0								//花色掩码
#define	MASK_VALUE					0x0F								//数值掩码

//////////////////////////////////////////////////////////////////////////
//动作定义

//动作标志
#define WIK_NULL					0x00								//没有类型
#define WIK_LEFT					0x01								//左吃类型
#define WIK_CENTER					0x02								//中吃类型
#define WIK_RIGHT					0x04								//右吃类型
#define WIK_PENG					0x08								//碰牌类型
#define WIK_GANG					0x10								//杠牌类型
#define WIK_XIAO_HU					0x20//小胡							//吃牌类型
#define WIK_CHI_HU					0x40								//吃胡类型
#define WIK_ZI_MO					0x80								//自摸

//////////////////////////////////////////////////////////////////////////
//胡牌定义

//胡牌
#define CHK_NULL					0x00										//非胡类型
#define CHK_CHI_HU					0x01										//胡类型

// 大胡
#define CHR_PENGPENG_HU				0x00000001									//碰碰胡
#define CHR_JIANGJIANG_HU			0x00000002									//将将胡
#define CHR_QING_YI_SE				0x00000004									//清一色
#define CHR_HAI_DI_LAO				0x00000008									//海底捞
#define CHR_HAI_DI_PAO				0x00000010									//海底捞
#define CHR_QI_XIAO_DUI				0x00000020									//七小对
#define CHR_HAOHUA_QI_XIAO_DUI		0x00000040									//豪华七小对
#define CHR_GANG_KAI				0x00000080									//杠上开花
#define CHR_QIANG_GANG_HU			0x00000100									//抢杠胡
#define CHR_GANG_SHANG_PAO			0x00000200									//杠上跑(杠上开花)
#define CHR_QUAN_QIU_REN			0x00000400									//全求人
#define CHR_DHAOHUA_QI_XIAO_DUI		0x00000800									//双豪华七对
#define CHR_THAOHUA_QI_XIAO_DUI		0x00001000									//三豪华七对
#define CHR_KA_DAN					0x00002000									//卡单
#define CHR_ZI_YI_SE				0x00004000									//字一色

//小胡
#define CHR_XIAO_DA_SI_XI			0x00004000									//大四喜
#define CHR_XIAO_BAN_BAN_HU			0x00008000									//板板胡
#define CHR_XIAO_QUE_YI_SE			0x00010000									//缺一色
#define CHR_XIAO_LIU_LIU_SHUN		0x00020000									//六六顺


#define CHR_ZI_MO					0x01000000									//自摸
#define CHR_SHU_FAN					0x02000000									//素翻
//////////////////////////////////////////////////////////////////////////

#define ZI_PAI_COUNT	7

//类型子项
struct tagKindItem
{
	U8								cbWeaveKind;						//组合类型
	U8								cbCenterCard;						//中心扑克
	U8								cbCardIndex[3];						//扑克索引
	U8								cbValidIndex[3];					//实际扑克索引
};

//组合子项
struct tagWeaveItem
{
	U8								cbWeaveKind;						//组合类型
	U8								cbCenterCard;						//中心扑克
	U8								cbPublicCard;						//公开标志
	U16								wProvideUser;						//供应用户
};

//杠牌结果
struct tagGangCardResult
{
	U8								cbCardCount;						//扑克数目
	U8								cbCardData[4];						//扑克数据
};

//分析子项
struct tagAnalyseItem
{
	U8								cbCardEye;							//牌眼扑克
	bool                            bMagicEye;                          //牌眼是否是王霸
	U8								cbWeaveKind[4];						//组合类型
	U8								cbCenterCard[4];					//中心扑克
	U8								cbCardData[4][4];                   //实际扑克
};

//////////////////////////////////////////////////////////////////////////

#define MASK_CHI_HU_RIGHT			0x0fffffff

/*
//	权位类。
//  注意，在操作仅位时最好只操作单个权位.例如
//  CChiHuRight chr;
//  chr |= (chr_zi_mo|chr_peng_peng)，这样结果是无定义的。
//  只能单个操作:
//  chr |= chr_zi_mo;
//  chr |= chr_peng_peng;
*/
class CChiHuRight
{	
	//静态变量
private:
	static bool						m_bInit;
	static U32						m_dwRightMask[MAX_RIGHT_COUNT];

	//权位变量
private:
	U32								m_dwRight[MAX_RIGHT_COUNT];

public:
	//构造函数
	CChiHuRight();

	//运算符重载
public:
	//赋值符
	CChiHuRight & operator = ( U32 dwRight );

	//与等于
	CChiHuRight & operator &= ( U32 dwRight );
	//或等于
	CChiHuRight & operator |= ( U32 dwRight );

	//与
	CChiHuRight operator & ( U32 dwRight );
	CChiHuRight operator & ( U32 dwRight ) const;

	//或
	CChiHuRight operator | ( U32 dwRight );
	CChiHuRight operator | ( U32 dwRight ) const;

	//功能函数
public:
	//是否权位为空
	bool IsEmpty();

	//设置权位为空
	void SetEmpty();

	//获取权位数值
	U8 GetRightData( U32 dwRight[], U8 cbMaxCount );

	//设置权位数值
	bool SetRightData( const U32 dwRight[], U8 cbRightCount );

private:
	//检查权位是否正确
	bool IsValidRight( U32 dwRight );
};


//////////////////////////////////////////////////////////////////////////

//数组说明
typedef std::deque<tagAnalyseItem> CAnalyseItemArray;

//游戏逻辑类
class CChessLogic
{
	//变量定义
protected:
	static const U8				m_cbCardDataArray_108[MAX_REPERTORY_108];	    //扑克数据
	static const U8				m_cbCardDataArray_112[MAX_REPERTORY_112];	//扑克数据
	static const U8				m_cbCardDataArray_136[MAX_REPERTORY_136];	//扑克数据
	U8							m_cbMagicIndex;						//钻牌索引

	//函数定义
public:
	//构造函数
	CChessLogic();
	//析构函数
	virtual ~CChessLogic();

	//控制函数
public:
	//混乱扑克
	void RandCardData(U8 cbCardData[], U8 cbMaxCount);
	//删除扑克
	bool RemoveCard(U8 cbCardIndex[MAX_INDEX], U8 cbRemoveCard);
	//删除扑克
	bool RemoveCard(U8 cbCardIndex[MAX_INDEX], const U8 cbRemoveCard[], U8 cbRemoveCount);
	//删除扑克
	bool RemoveCard(U8 cbCardData[], U8 cbCardCount, const U8 cbRemoveCard[], U8 cbRemoveCount);
	//设置钻牌
	void SetMagicIndex( U8 cbMagicIndex ) { m_cbMagicIndex = cbMagicIndex; }
	//钻牌
	bool IsMagicCard( U8 cbCardData );

	//辅助函数
public:
	//有效判断
	bool IsValidCard(U8 cbCardData);
	//扑克数目
	U8 GetCardCount(const U8 cbCardIndex[MAX_INDEX]);
	//组合扑克
	U8 GetWeaveCard(U8 cbWeaveKind, U8 cbCenterCard, U8 cbCardBuffer[4]);

	//等级函数
public:
	//动作等级
	U8 GetUserActionRank(U8 cbUserAction);

	//动作判断
public:
	//吃牌判断
	U8 EstimateEatCard(const  U8 cbCardIndex[MAX_INDEX], U8 cbCurrentCard);
	//碰牌判断
	U8 EstimatePengCard(const U8 cbCardIndex[MAX_INDEX], U8 cbCurrentCard);
	//杠牌判断
	U8 EstimateGangCard(const U8 cbCardIndex[MAX_INDEX], U8 cbCurrentCard);

	//动作判断
public:
	//杠牌分析
	U8 AnalyseGangCard(const U8 cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], U8 cbWeaveCount, tagGangCardResult & GangCardResult);
	//吃胡分析
	U8 AnalyseChiHuCard(const U8 cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], U8 cbWeaveCount, U8 cbCurrentCard, CChiHuRight &ChiHuRight);
	//听牌分析
	U8 AnalyseTingCard( const U8 cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], U8 cbWeaveCount );
	//是否听牌
	bool IsTingCard( const U8 cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], U8 cbWeaveCount );
	//是否花猪
	bool IsHuaZhu( const U8 cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], U8 cbWeaveCount );

	//转换函数
public:
	//扑克转换
	U8 SwitchToCardData(U8 cbCardIndex);
	//扑克转换
	U8 SwitchToCardIndex(U8 cbCardData);
	//扑克转换
	U8 SwitchToCardData(const U8 cbCardIndex[MAX_INDEX], U8 cbCardData[MAX_COUNT]);
	//扑克转换
	U8 SwitchToCardIndex(const U8 cbCardData[], U8 cbCardCount, U8 cbCardIndex[MAX_INDEX]);

	//胡法分析
public:
	//大对子
	bool IsPengPeng( const tagAnalyseItem *pAnalyseItem );
	//清一色牌
	bool IsQingYiSe(const U8 cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], const U8 cbItemCount,const U8 cbCurrentCard);
	//字一色牌
	bool IsZiYiSe(const U8 cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], const U8 cbItemCount,const U8 cbCurrentCard);
	//七小对牌
	bool IsQiXiaoDui(const U8 cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[],const  U8 cbWeaveCount,const U8 cbCurrentCard,int& nGenCount);
	//卡单
	bool IsKaDan( const U8 cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], U8 cbWeaveCount );

	//带幺
	bool IsDaiYao( const tagAnalyseItem *pAnalyseItem );
	//将对
	bool IsJiangDui( const tagAnalyseItem *pAnalyseItem );
	//将将胡
	bool IsJiangJiangHu(const U8 cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[],const  U8 cbWeaveCount,const U8 cbCurrentCard);
	bool IsDanDiao(const U8 cbCardIndex[MAX_INDEX],const U8 cbCurrentCard);

	//分析扑克
	bool AnalyseCard(const U8 cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], U8 cbItemCount, CAnalyseItemArray & AnalyseItemArray);
	//排序,根据牌值排序
	bool SortCardList( U8 cbCardData[MAX_COUNT], U8 cbCardCount );

	U8 GetPickNiaoCount( U8 cbCardData[MAX_NIAO_CARD],U8 cbCardNum);
};

//////////////////////////////////////////////////////////////////////////
//#pragma pack()
#endif
