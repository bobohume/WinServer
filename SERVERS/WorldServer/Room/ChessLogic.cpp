#include "ChessLogic.h"
#include "math.h"
#include "Common/mRandom.h"

//////////////////////////////////////////////////////////////////////////

//静态变量
bool		CChiHuRight::m_bInit = false;
U32		CChiHuRight::m_dwRightMask[MAX_RIGHT_COUNT];

//构造函数
CChiHuRight::CChiHuRight()
{
	ZeroMemory( m_dwRight,sizeof(m_dwRight) );

	if( !m_bInit )
	{
		m_bInit = true;
		for( U8 i = 0; i < CountArray(m_dwRightMask); i++ )
		{
			if( 0 == i )
				m_dwRightMask[i] = 0;
			else
				m_dwRightMask[i] = (U32(pow((float)2,(float)(i-1))))<<28;
		}
	}
}

//赋值符重载
CChiHuRight & CChiHuRight::operator = ( U32 dwRight )
{
	U32 dwOtherRight = 0;
	//验证权位
	if( !IsValidRight( dwRight ) )
	{
		//验证取反权位
		ASSERT( IsValidRight( ~dwRight ) );
		if( !IsValidRight( ~dwRight ) ) return *this;
		dwRight = ~dwRight;
		dwOtherRight = MASK_CHI_HU_RIGHT;
	}

	for( U8 i = 0; i < CountArray(m_dwRightMask); i++ )
	{
		if( (dwRight&m_dwRightMask[i]) || (i==0&&dwRight<0x10000000) )
			m_dwRight[i] = dwRight&MASK_CHI_HU_RIGHT;
		else m_dwRight[i] = dwOtherRight;
	}

	return *this;
}

//与等于
CChiHuRight & CChiHuRight::operator &= ( U32 dwRight )
{
	bool bNavigate = false;
	//验证权位
	if( !IsValidRight( dwRight ) )
	{
		//验证取反权位
		ASSERT( IsValidRight( ~dwRight ) );
		if( !IsValidRight( ~dwRight ) ) return *this;
		//调整权位
		U32 dwHeadRight = (~dwRight)&0xF0000000;
		U32 dwTailRight = dwRight&MASK_CHI_HU_RIGHT;
		dwRight = dwHeadRight|dwTailRight;
		bNavigate = true;
	}

	for( U8 i = 0; i < CountArray(m_dwRightMask); i++ )
	{
		if( (dwRight&m_dwRightMask[i]) || (i==0&&dwRight<0x10000000) )
		{
			m_dwRight[i] &= (dwRight&MASK_CHI_HU_RIGHT);
		}
		else if( !bNavigate )
			m_dwRight[i] = 0;
	}

	return *this;
}

//或等于
CChiHuRight & CChiHuRight::operator |= ( U32 dwRight )
{
	//验证权位
	if( !IsValidRight( dwRight ) ) return *this;

	for( U8 i = 0; i < CountArray(m_dwRightMask); i++ )
	{
		if( (dwRight&m_dwRightMask[i]) || (i==0&&dwRight<0x10000000) )
			m_dwRight[i] |= (dwRight&MASK_CHI_HU_RIGHT);
	}

	return *this;
}

//与
CChiHuRight CChiHuRight::operator & ( U32 dwRight )
{
	CChiHuRight chr = *this;
	return (chr &= dwRight);
}

//与
CChiHuRight CChiHuRight::operator & ( U32 dwRight ) const
{
	CChiHuRight chr = *this;
	return (chr &= dwRight);
}

//或
CChiHuRight CChiHuRight::operator | ( U32 dwRight )
{
	CChiHuRight chr = *this;
	return chr |= dwRight;
}

//或
CChiHuRight CChiHuRight::operator | ( U32 dwRight ) const
{
	CChiHuRight chr = *this;
	return chr |= dwRight;
}

//是否权位为空
bool CChiHuRight::IsEmpty()
{
	for( U8 i = 0; i < CountArray(m_dwRight); i++ )
		if( m_dwRight[i] ) return false;
	return true;
}

//设置权位为空
void CChiHuRight::SetEmpty()
{
	ZeroMemory( m_dwRight,sizeof(m_dwRight) );
	return;
}

//获取权位数值
U8 CChiHuRight::GetRightData( U32 dwRight[], U8 cbMaxCount )
{
	ASSERT( cbMaxCount >= CountArray(m_dwRight) );
	if( cbMaxCount < CountArray(m_dwRight) ) return 0;

	CopyMemory( dwRight,m_dwRight,sizeof(U32)*CountArray(m_dwRight) );
	return CountArray(m_dwRight);
}

//设置权位数值
bool CChiHuRight::SetRightData( const U32 dwRight[], U8 cbRightCount )
{
	ASSERT( cbRightCount <= CountArray(m_dwRight) );
	if( cbRightCount > CountArray(m_dwRight) ) return false;

	ZeroMemory( m_dwRight,sizeof(m_dwRight) );
	CopyMemory( m_dwRight,dwRight,sizeof(U32)*cbRightCount );
	
	return true;
}

//检查仅位是否正确
bool CChiHuRight::IsValidRight( U32 dwRight )
{
	U32 dwRightHead = dwRight & 0xF0000000;
	for( U8 i = 0; i < CountArray(m_dwRightMask); i++ )
		if( m_dwRightMask[i] == dwRightHead ) return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////
//静态变量

//扑克数据
const U8 CChessLogic::m_cbCardDataArray_108[MAX_REPERTORY_108]=
{
	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,						//万子
	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,						//万子
	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,						//万子
	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,						//万子
	0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,						//索子
	0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,						//索子
	0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,						//索子
	0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,						//索子
	0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,						//同子
	0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,						//同子
	0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,						//同子
	0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,						//同子
};

//扑克数据
const U8 CChessLogic::m_cbCardDataArray_112[MAX_REPERTORY_112]=
{
	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,						//万子
	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,						//万子
	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,						//万子
	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,						//万子
	0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,						//索子
	0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,						//索子
	0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,						//索子
	0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,						//索子
	0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,						//同子
	0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,						//同子
	0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,						//同子
	0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,						//同子

	0x35,0x35,0x35,0x35,												//红中
};

//扑克数据
const U8 CChessLogic::m_cbCardDataArray_136[MAX_REPERTORY_136]=
{
	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,						//万子
	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,						//万子
	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,						//万子
	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,						//万子
	0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,						//索子
	0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,						//索子
	0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,						//索子
	0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,						//索子
	0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,						//同子
	0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,						//同子
	0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,						//同子
	0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,						//同子
	0x31,0x32,0x33,0x34,0x35,0x36,0x37,									//东、南、西、北、中、发、白
	0x31,0x32,0x33,0x34,0x35,0x36,0x37,									//东、南、西、北、中、发、白
	0x31,0x32,0x33,0x34,0x35,0x36,0x37,									//东、南、西、北、中、发、白
	0x31,0x32,0x33,0x34,0x35,0x36,0x37,									//东、南、西、北、中、发、白
};
//////////////////////////////////////////////////////////////////////////

//构造函数
CChessLogic::CChessLogic()
{
	m_cbMagicIndex = MAX_INDEX;
}

//析构函数
CChessLogic::~CChessLogic()
{
}

//混乱扑克
void CChessLogic::RandCardData(U8 cbCardData[], U8 cbMaxCount)
{
	//混乱准备
#define RAND_CARD(CardDataArry)         \
	U8 cbCardDataTemp[CountArray(CardDataArry)];\
	CopyMemory(cbCardDataTemp,CardDataArry,sizeof(CardDataArry));\
	U8 cbRandCount=0,cbPosition=0;\
	do\
	{\
		cbPosition= gRandGen.randI(0, cbMaxCount-cbRandCount-1);\
		cbCardData[cbRandCount++]=cbCardDataTemp[cbPosition];\
		cbCardDataTemp[cbPosition]=cbCardDataTemp[cbMaxCount-cbRandCount];\
	} while (cbRandCount<cbMaxCount);\


	if ( cbMaxCount == MAX_REPERTORY_108)
	{
		RAND_CARD(m_cbCardDataArray_108);
		SetMagicIndex(MAX_INDEX);
	}
	else if( cbMaxCount == MAX_REPERTORY_112)
	{
		RAND_CARD(m_cbCardDataArray_112);
	}
	else if( cbMaxCount == MAX_REPERTORY_136)
	{
		RAND_CARD(m_cbCardDataArray_136);
	}

	return;
}

//删除扑克
bool CChessLogic::RemoveCard(U8 cbCardIndex[MAX_INDEX], U8 cbRemoveCard)
{
	//效验扑克
	ASSERT(IsValidCard(cbRemoveCard));
	ASSERT(cbCardIndex[SwitchToCardIndex(cbRemoveCard)]>0);

	//删除扑克
	U8 cbRemoveIndex=SwitchToCardIndex(cbRemoveCard);
	if (cbCardIndex[cbRemoveIndex]>0)
	{
		cbCardIndex[cbRemoveIndex]--;
		return true;
	}

	//失败效验
	ASSERT(false);

	return false;
}

//删除扑克
bool CChessLogic::RemoveCard(U8 cbCardIndex[MAX_INDEX], const U8 cbRemoveCard[], U8 cbRemoveCount)
{
	//删除扑克
	for (U8 i=0;i<cbRemoveCount;i++)
	{
		//效验扑克
		ASSERT(IsValidCard(cbRemoveCard[i]));
		ASSERT(cbCardIndex[SwitchToCardIndex(cbRemoveCard[i])]>0);

		//删除扑克
		U8 cbRemoveIndex=SwitchToCardIndex(cbRemoveCard[i]);
		if (cbCardIndex[cbRemoveIndex]==0)
		{
			//错误断言
			ASSERT(false);

			//还原删除
			for (U8 j=0;j<i;j++) 
			{
				ASSERT(IsValidCard(cbRemoveCard[j]));
				cbCardIndex[SwitchToCardIndex(cbRemoveCard[j])]++;
			}

			return false;
		}
		else 
		{
			//删除扑克
			--cbCardIndex[cbRemoveIndex];
		}
	}

	return true;
}

//删除扑克
bool CChessLogic::RemoveCard(U8 cbCardData[], U8 cbCardCount, const U8 cbRemoveCard[], U8 cbRemoveCount)
{
	//检验数据
	ASSERT(cbCardCount<=14);
	ASSERT(cbRemoveCount<=cbCardCount);

	//定义变量
	U8 cbDeleteCount=0,cbTempCardData[14];
	if (cbCardCount>CountArray(cbTempCardData))
		return false;
	CopyMemory(cbTempCardData,cbCardData,cbCardCount*sizeof(cbCardData[0]));

	//置零扑克
	for (U8 i=0;i<cbRemoveCount;i++)
	{
		for (U8 j=0;j<cbCardCount;j++)
		{
			if (cbRemoveCard[i]==cbTempCardData[j])
			{
				cbDeleteCount++;
				cbTempCardData[j]=0;
				break;
			}
		}
	}

	//成功判断
	if (cbDeleteCount!=cbRemoveCount) 
	{
		ASSERT(false);
		return false;
	}

	//清理扑克
	U8 cbCardPos=0;
	for (U8 i=0;i<cbCardCount;i++)
	{
		if (cbTempCardData[i]!=0) 
			cbCardData[cbCardPos++]=cbTempCardData[i];
	}

	return true;
}

//有效判断
bool CChessLogic::IsValidCard(U8 cbCardData)
{
	U8 cbValue=(cbCardData&MASK_VALUE);
	U8 cbColor=(cbCardData&MASK_COLOR)>>4;
	return (((cbValue>=1)&&(cbValue<=9)&&(cbColor<=2))||((cbValue>=1)&&(cbValue<=7)&&(cbColor==3)));
}

//扑克数目
U8 CChessLogic::GetCardCount(const U8 cbCardIndex[MAX_INDEX])
{
	//数目统计
	U8 cbCardCount=0;
	for (U8 i=0;i<MAX_INDEX;i++) 
		cbCardCount+=cbCardIndex[i];

	return cbCardCount;
}

//获取组合
U8 CChessLogic::GetWeaveCard(U8 cbWeaveKind, U8 cbCenterCard, U8 cbCardBuffer[4])
{
	//组合扑克
	switch (cbWeaveKind)
	{
	case WIK_LEFT:		//上牌操作
		{
			//设置变量
			cbCardBuffer[0]=cbCenterCard;
			cbCardBuffer[1]=cbCenterCard+1;
			cbCardBuffer[2]=cbCenterCard+2;

			return 3;
		}
	case WIK_RIGHT:		//上牌操作
		{
			//设置变量
			cbCardBuffer[0]=cbCenterCard;
			cbCardBuffer[1]=cbCenterCard-1;
			cbCardBuffer[2]=cbCenterCard-2;

			return 3;
		}
	case WIK_CENTER:	//上牌操作
		{
			//设置变量
			cbCardBuffer[0]=cbCenterCard;
			cbCardBuffer[1]=cbCenterCard-1;
			cbCardBuffer[2]=cbCenterCard+1;

			return 3;
		}
	case WIK_PENG:		//碰牌操作
		{
			//设置变量
			cbCardBuffer[0]=cbCenterCard;
			cbCardBuffer[1]=cbCenterCard;
			cbCardBuffer[2]=cbCenterCard;

			return 3;
		}
	case WIK_GANG:		//杠牌操作
		{
			//设置变量
			cbCardBuffer[0]=cbCenterCard;
			cbCardBuffer[1]=cbCenterCard;
			cbCardBuffer[2]=cbCenterCard;
			cbCardBuffer[3]=cbCenterCard;

			return 4;
		}
	default:
		{
			ASSERT(false);
		}
	}

	return 0;
}

//动作等级
U8 CChessLogic::GetUserActionRank(U8 cbUserAction)
{
	//胡牌等级
	if (cbUserAction&WIK_CHI_HU) { return 4; }

	//杠牌等级
	if (cbUserAction&WIK_GANG) { return 3; }

	//碰牌等级
	if (cbUserAction&WIK_PENG) { return 2; }

	//上牌等级
	if (cbUserAction&(WIK_RIGHT|WIK_CENTER|WIK_LEFT)) { return 1; }

	return 0;
}

//吃牌判断
U8 CChessLogic::EstimateEatCard(const U8 cbCardIndex[MAX_INDEX], U8 cbCurrentCard)
{
	//参数效验
	ASSERT(IsValidCard(cbCurrentCard));

	//过滤判断
	if ( cbCurrentCard>=0x31 || IsMagicCard(cbCurrentCard) ) 
		return WIK_NULL;

	//变量定义
	U8 cbExcursion[3]={0,1,2};
	U8 cbItemKind[3]={WIK_LEFT,WIK_CENTER,WIK_RIGHT};

	//吃牌判断
	U8 cbEatKind=0,cbFirstIndex=0;
	U8 cbCurrentIndex=SwitchToCardIndex(cbCurrentCard);
	for (U8 i=0;i<CountArray(cbItemKind);i++)
	{
		U8 cbValueIndex=cbCurrentIndex%9;
		if ((cbValueIndex>=cbExcursion[i])&&((cbValueIndex-cbExcursion[i])<=6))
		{
			//吃牌判断
			cbFirstIndex=cbCurrentIndex-cbExcursion[i];

			//吃牌不能包含有王霸
			if( m_cbMagicIndex != MAX_INDEX &&
				m_cbMagicIndex >= cbFirstIndex && m_cbMagicIndex <= cbFirstIndex+2 ) continue;

			if ((cbCurrentIndex!=cbFirstIndex)&&(cbCardIndex[cbFirstIndex]==0))
				continue;
			if ((cbCurrentIndex!=(cbFirstIndex+1))&&(cbCardIndex[cbFirstIndex+1]==0))
				continue;
			if ((cbCurrentIndex!=(cbFirstIndex+2))&&(cbCardIndex[cbFirstIndex+2]==0))
				continue;

			//设置类型
			cbEatKind|=cbItemKind[i];
		}
	}

	return cbEatKind;
}

//碰牌判断
U8 CChessLogic::EstimatePengCard(const U8 cbCardIndex[MAX_INDEX], U8 cbCurrentCard)
{
	//参数效验
	ASSERT(IsValidCard(cbCurrentCard));

	//过滤判断
	if ( IsMagicCard(cbCurrentCard) ) 
		return WIK_NULL;

	//碰牌判断
	return (cbCardIndex[SwitchToCardIndex(cbCurrentCard)]>=2)?WIK_PENG:WIK_NULL;
}

//杠牌判断
U8 CChessLogic::EstimateGangCard(const U8 cbCardIndex[MAX_INDEX], U8 cbCurrentCard)
{
	//参数效验
	ASSERT(IsValidCard(cbCurrentCard));

	//过滤判断
	if ( IsMagicCard(cbCurrentCard) ) 
		return WIK_NULL;

	//杠牌判断
	return (cbCardIndex[SwitchToCardIndex(cbCurrentCard)]==3)?WIK_GANG:WIK_NULL;
}

//杠牌分析
U8 CChessLogic::AnalyseGangCard(const U8 cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], U8 cbWeaveCount, tagGangCardResult & GangCardResult)
{
	//设置变量
	U8 cbActionMask=WIK_NULL;
	ZeroMemory(&GangCardResult,sizeof(GangCardResult));

	//手上杠牌
	for (U8 i=0;i<MAX_INDEX;i++)
	{
		if( i == m_cbMagicIndex ) continue;
		if (cbCardIndex[i]==4)
		{
			cbActionMask|=WIK_GANG;
			GangCardResult.cbCardData[GangCardResult.cbCardCount++]=SwitchToCardData(i);
		}
	}

	//组合杠牌
	for (U8 i=0;i<cbWeaveCount;i++)
	{
		if (WeaveItem[i].cbWeaveKind==WIK_PENG)
		{
			if (cbCardIndex[SwitchToCardIndex(WeaveItem[i].cbCenterCard)]==1)
			{
				cbActionMask|=WIK_GANG;
				GangCardResult.cbCardData[GangCardResult.cbCardCount++]=WeaveItem[i].cbCenterCard;
			}
		}
	}

	return cbActionMask;
}

//吃胡分析
U8 CChessLogic::AnalyseChiHuCard(const U8 cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], U8 cbWeaveCount, U8 cbCurrentCard, CChiHuRight &ChiHuRight)
{
	//变量定义
	U8 cbChiHuKind=WIK_NULL;
	CAnalyseItemArray AnalyseItemArray;

	//设置变量
	AnalyseItemArray.clear();
	ChiHuRight.SetEmpty();

	//构造扑克
	U8 cbCardIndexTemp[MAX_INDEX];
	CopyMemory(cbCardIndexTemp,cbCardIndex,sizeof(cbCardIndexTemp));

	//cbCurrentCard一定不为0			!!!!!!!!!
	ASSERT( cbCurrentCard != 0 );
	if( cbCurrentCard == 0 ) return WIK_NULL;

	/*
	//	特殊番型
	*/
	//七小对牌
	int nGenCount = 0;
	if( IsQiXiaoDui(cbCardIndex,WeaveItem,cbWeaveCount,cbCurrentCard,nGenCount) ) 
		ChiHuRight |= CHR_QI_XIAO_DUI;

	if( !ChiHuRight.IsEmpty() )
		cbChiHuKind = WIK_CHI_HU;

	//插入扑克
	if (cbCurrentCard!=0)
		cbCardIndexTemp[SwitchToCardIndex(cbCurrentCard)]++;

	//分析扑克
	AnalyseCard(cbCardIndexTemp,WeaveItem,cbWeaveCount,AnalyseItemArray);

	//胡牌分析
	if (AnalyseItemArray.size()>0)
	{
		//
		cbChiHuKind = WIK_CHI_HU;

		//牌型分析
		while(AnalyseItemArray.size())
		{
			//变量定义
			tagAnalyseItem pAnalyseItem= (AnalyseItemArray.front());
			AnalyseItemArray.pop_front();
			/*
			//	判断番型
			*/
			//碰碰和
			if( IsPengPeng(&pAnalyseItem) ) 
				ChiHuRight |= CHR_PENGPENG_HU;
			//带幺
			//if( IsDaiYao(pAnalyseItem) )
			//	ChiHuRight |= CHR_DAI_YAO;
			//将将胡
			if( IsJiangJiangHu(cbCardIndex,WeaveItem,cbWeaveCount,cbCurrentCard) )
				ChiHuRight |= CHR_JIANGJIANG_HU;
		}
	}

	//素番
	if( cbChiHuKind == WIK_CHI_HU && ChiHuRight.IsEmpty() )
		ChiHuRight |= CHR_SHU_FAN;

	if( cbChiHuKind == WIK_CHI_HU )
	{
		//清一色牌
		if( IsQingYiSe(cbCardIndex,WeaveItem,cbWeaveCount,cbCurrentCard) )
			ChiHuRight |= CHR_QING_YI_SE;
	}

	return cbChiHuKind;
}

//听牌分析
U8 CChessLogic::AnalyseTingCard( const U8 cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], U8 cbWeaveCount )
{
	return WIK_NULL;
}

//是否听牌
bool CChessLogic::IsTingCard( const U8 cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], U8 cbWeaveCount )
{
	//复制数据
	U8 cbCardIndexTemp[MAX_INDEX];
	CopyMemory( cbCardIndexTemp,cbCardIndex,sizeof(cbCardIndexTemp) );

	CChiHuRight chr;
	for( U8 i = 0; i < MAX_INDEX-7; i++ )
	{
		U8 cbCurrentCard = SwitchToCardData( i );
		if( WIK_CHI_HU == AnalyseChiHuCard( cbCardIndexTemp,WeaveItem,cbWeaveCount,cbCurrentCard,chr ) )
			return true;
	}
	return false;
}

//扑克转换
U8 CChessLogic::SwitchToCardData(U8 cbCardIndex)
{
	ASSERT(cbCardIndex<34);
	return ((cbCardIndex/9)<<4)|(cbCardIndex%9+1);
}

//扑克转换
U8 CChessLogic::SwitchToCardIndex(U8 cbCardData)
{
	ASSERT(IsValidCard(cbCardData));
	return ((cbCardData&MASK_COLOR)>>4)*9+(cbCardData&MASK_VALUE)-1;
}

//扑克转换
U8 CChessLogic::SwitchToCardData(const U8 cbCardIndex[MAX_INDEX], U8 cbCardData[MAX_COUNT])
{
	//转换扑克
	U8 cbPosition=0;
	//钻牌
	if( m_cbMagicIndex != MAX_INDEX )
	{
		for( U8 i = 0; i < cbCardIndex[m_cbMagicIndex]; i++ )
			cbCardData[cbPosition++] = SwitchToCardData(m_cbMagicIndex);
	}
	for (U8 i=0;i<MAX_INDEX;i++)
	{
		if( i == m_cbMagicIndex ) continue;
		if (cbCardIndex[i]!=0)
		{
			for (U8 j=0;j<cbCardIndex[i];j++)
			{
				ASSERT(cbPosition<MAX_COUNT);
				cbCardData[cbPosition++]=SwitchToCardData(i);
			}
		}
	}

	return cbPosition;
}

//扑克转换
U8 CChessLogic::SwitchToCardIndex(const U8 cbCardData[], U8 cbCardCount, U8 cbCardIndex[MAX_INDEX])
{
	//设置变量
	ZeroMemory(cbCardIndex,sizeof(U8)*MAX_INDEX);

	//转换扑克
	for (U8 i=0;i<cbCardCount;i++)
	{
		ASSERT(IsValidCard(cbCardData[i]));
		cbCardIndex[SwitchToCardIndex(cbCardData[i])]++;
	}

	return cbCardCount;
}

//分析扑克
bool CChessLogic::AnalyseCard(const U8 cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], U8 cbWeaveCount, CAnalyseItemArray & AnalyseItemArray)
{
	//计算数目
	U8 cbCardCount=GetCardCount(cbCardIndex);

	//效验数目
	ASSERT((cbCardCount>=2)&&(cbCardCount<=MAX_COUNT)&&((cbCardCount-2)%3==0));
	if ((cbCardCount<2)||(cbCardCount>MAX_COUNT)||((cbCardCount-2)%3!=0))
		return false;

	//变量定义
	U8 cbKindItemCount=0;
	tagKindItem KindItem[27*2+7+14];
	ZeroMemory(KindItem,sizeof(KindItem));

	//需求判断
	U8 cbLessKindItem=(cbCardCount-2)/3;
	ASSERT((cbLessKindItem+cbWeaveCount)==4);

	//单吊判断
	if (cbLessKindItem==0)
	{
		//效验参数
		ASSERT((cbCardCount==2)&&(cbWeaveCount==4));

		//牌眼判断
		for (U8 i=0;i<MAX_INDEX;i++)
		{
			if (cbCardIndex[i]==2 || 
				( m_cbMagicIndex != MAX_INDEX && i != m_cbMagicIndex && cbCardIndex[m_cbMagicIndex]+cbCardIndex[i]==2 ) )
			{
				//变量定义
				tagAnalyseItem AnalyseItem;
				ZeroMemory(&AnalyseItem,sizeof(AnalyseItem));

				//设置结果
				for (U8 j=0;j<cbWeaveCount;j++)
				{
					AnalyseItem.cbWeaveKind[j]=WeaveItem[j].cbWeaveKind;
					AnalyseItem.cbCenterCard[j]=WeaveItem[j].cbCenterCard;
					GetWeaveCard( WeaveItem[j].cbWeaveKind,WeaveItem[j].cbCenterCard,AnalyseItem.cbCardData[j] );
				}
				AnalyseItem.cbCardEye=SwitchToCardData(i);
				if( cbCardIndex[i] < 2 || i == m_cbMagicIndex )
					AnalyseItem.bMagicEye = true;
				else AnalyseItem.bMagicEye = false;

				//插入结果
				AnalyseItemArray.push_back(AnalyseItem);

				return true;
			}
		}

		return false;
	}

	//拆分分析
	U8 cbMagicCardIndex[MAX_INDEX];
	CopyMemory(cbMagicCardIndex,cbCardIndex,sizeof(cbMagicCardIndex));
	U8 cbMagicCardCount = 0;
	if( m_cbMagicIndex != MAX_INDEX )
	{
		cbMagicCardCount = cbCardIndex[m_cbMagicIndex];
		if( cbMagicCardIndex[m_cbMagicIndex] ) cbMagicCardIndex[m_cbMagicIndex] = 1;		//减小多余组合
	}
	if (cbCardCount>=3)
	{
		for (U8 i=0;i<MAX_INDEX;i++)
		{
			//同牌判断
			if (cbMagicCardIndex[i]+cbMagicCardCount>=3)
			{
				ASSERT( cbKindItemCount < CountArray(KindItem) );
				KindItem[cbKindItemCount].cbCardIndex[0]=i;
				KindItem[cbKindItemCount].cbCardIndex[1]=i;
				KindItem[cbKindItemCount].cbCardIndex[2]=i;
				KindItem[cbKindItemCount].cbWeaveKind=WIK_PENG;
				KindItem[cbKindItemCount].cbCenterCard=SwitchToCardData(i);
				KindItem[cbKindItemCount].cbValidIndex[0] = cbMagicCardIndex[i]>0?i:m_cbMagicIndex;
				KindItem[cbKindItemCount].cbValidIndex[1] = cbMagicCardIndex[i]>1?i:m_cbMagicIndex;
				KindItem[cbKindItemCount].cbValidIndex[2] = cbMagicCardIndex[i]>2?i:m_cbMagicIndex;
				cbKindItemCount++;
				if(cbMagicCardIndex[i]+cbMagicCardCount>=6)
				{
					ASSERT( cbKindItemCount < CountArray(KindItem) );
					KindItem[cbKindItemCount].cbCardIndex[0]=i;
					KindItem[cbKindItemCount].cbCardIndex[1]=i;
					KindItem[cbKindItemCount].cbCardIndex[2]=i;
					KindItem[cbKindItemCount].cbWeaveKind=WIK_PENG;
					KindItem[cbKindItemCount].cbCenterCard=SwitchToCardData(i);
					KindItem[cbKindItemCount].cbValidIndex[0] = cbMagicCardIndex[i]>3?i:m_cbMagicIndex;
					KindItem[cbKindItemCount].cbValidIndex[1] = m_cbMagicIndex;
					KindItem[cbKindItemCount].cbValidIndex[2] = m_cbMagicIndex;
					cbKindItemCount++;
				}
			}

			//连牌判断
			if ((i<(MAX_INDEX-9))&&((i%9)<7))
			{
				//只要财神牌数加上3个顺序索引的牌数大于等于3,则进行组合
				if( cbMagicCardCount+cbMagicCardIndex[i]+cbMagicCardIndex[i+1]+cbMagicCardIndex[i+2] >= 3 )
				{
					U8 cbIndex[3] = { i==m_cbMagicIndex?0:cbMagicCardIndex[i],(i+1)==m_cbMagicIndex?0:cbMagicCardIndex[i+1],
						(i+2)==m_cbMagicIndex?0:cbMagicCardIndex[i+2] };
					int nMagicCountTemp = cbMagicCardCount;
					U8 cbValidIndex[3];
					while( nMagicCountTemp+cbIndex[0]+cbIndex[1]+cbIndex[2] >= 3 )
					{
						for( U8 j = 0; j < CountArray(cbIndex); j++ )
						{
							if( cbIndex[j] > 0 ) 
							{
								cbIndex[j]--;
								cbValidIndex[j] = i+j;
							}
							else 
							{
								nMagicCountTemp--;
								cbValidIndex[j] = m_cbMagicIndex;
							}
						}
						if( nMagicCountTemp >= 0 )
						{
							ASSERT( cbKindItemCount < CountArray(KindItem) );
							KindItem[cbKindItemCount].cbCardIndex[0]=i;
							KindItem[cbKindItemCount].cbCardIndex[1]=i+1;
							KindItem[cbKindItemCount].cbCardIndex[2]=i+2;
							KindItem[cbKindItemCount].cbWeaveKind=WIK_LEFT;
							KindItem[cbKindItemCount].cbCenterCard=SwitchToCardData(i);
							CopyMemory( KindItem[cbKindItemCount].cbValidIndex,cbValidIndex,sizeof(cbValidIndex) );
							cbKindItemCount++;
						}
						else break;
					}
				}
			}
		}
	}

	//组合分析
	if (cbKindItemCount>=cbLessKindItem)
	{
		//变量定义
		U8 cbCardIndexTemp[MAX_INDEX];
		ZeroMemory(cbCardIndexTemp,sizeof(cbCardIndexTemp));

		//变量定义
		U8 cbIndex[4]={0,1,2,3};
		tagKindItem * pKindItem[4];
		ZeroMemory(&pKindItem,sizeof(pKindItem));

		//开始组合
		do
		{
			//设置变量
			CopyMemory(cbCardIndexTemp,cbCardIndex,sizeof(cbCardIndexTemp));
			for (U8 i=0;i<cbLessKindItem;i++)
				pKindItem[i]=&KindItem[cbIndex[i]];

			//数量判断
			bool bEnoughCard=true;
			for (U8 i=0;i<cbLessKindItem*3;i++)
			{
				//存在判断
				U8 cbCardIndex=pKindItem[i/3]->cbValidIndex[i%3]; 
				if (cbCardIndexTemp[cbCardIndex]==0)
				{
					bEnoughCard=false;
					break;
				}
				else 
					cbCardIndexTemp[cbCardIndex]--;
			}

			//胡牌判断
			if (bEnoughCard==true)
			{
				//牌眼判断
				U8 cbCardEye=0;
				bool bMagicEye = false;
				for (U8 i=0;i<MAX_INDEX;i++)
				{
					if (cbCardIndexTemp[i]==2)
					{
						cbCardEye=SwitchToCardData(i);
						if( i == m_cbMagicIndex ) bMagicEye = true;
						break;
					}
					else if( i!=m_cbMagicIndex && 
						m_cbMagicIndex != MAX_INDEX && cbCardIndexTemp[i]+cbCardIndexTemp[m_cbMagicIndex]==2 )
					{
						cbCardEye = SwitchToCardData(i);
						bMagicEye = true;
					}
				}

				//组合类型
				if (cbCardEye!=0)
				{
					//变量定义
					tagAnalyseItem AnalyseItem;
					ZeroMemory(&AnalyseItem,sizeof(AnalyseItem));

					//设置组合
					for (U8 i=0;i<cbWeaveCount;i++)
					{
						AnalyseItem.cbWeaveKind[i]=WeaveItem[i].cbWeaveKind;
						AnalyseItem.cbCenterCard[i]=WeaveItem[i].cbCenterCard;
						GetWeaveCard( WeaveItem[i].cbWeaveKind,WeaveItem[i].cbCenterCard,
							AnalyseItem.cbCardData[i] );
					}

					//设置牌型
					for (U8 i=0;i<cbLessKindItem;i++) 
					{
						AnalyseItem.cbWeaveKind[i+cbWeaveCount]=pKindItem[i]->cbWeaveKind;
						AnalyseItem.cbCenterCard[i+cbWeaveCount]=pKindItem[i]->cbCenterCard;
						AnalyseItem.cbCardData[cbWeaveCount+i][0] = SwitchToCardData(pKindItem[i]->cbValidIndex[0]);
						AnalyseItem.cbCardData[cbWeaveCount+i][1] = SwitchToCardData(pKindItem[i]->cbValidIndex[1]);
						AnalyseItem.cbCardData[cbWeaveCount+i][2] = SwitchToCardData(pKindItem[i]->cbValidIndex[2]);
					}

					//设置牌眼
					AnalyseItem.cbCardEye=cbCardEye;
					AnalyseItem.bMagicEye = bMagicEye;

					//插入结果
					AnalyseItemArray.push_back(AnalyseItem);
				}
			}

			//设置索引
			if (cbIndex[cbLessKindItem-1]==(cbKindItemCount-1))
			{
				U8 i=cbLessKindItem-1;
				for (;i>0;i--)
				{
					if ((cbIndex[i-1]+1)!=cbIndex[i])
					{
						U8 cbNewIndex=cbIndex[i-1];
						for (U8 j=(i-1);j<cbLessKindItem;j++) 
							cbIndex[j]=cbNewIndex+j-i+2;
						break;
					}
				}
				if (i==0)
					break;
			}
			else
				cbIndex[cbLessKindItem-1]++;
		} while (true);

	}

	return (AnalyseItemArray.size()>0);
}

//钻牌
bool CChessLogic::IsMagicCard( U8 cbCardData )
{
	if( m_cbMagicIndex != MAX_INDEX )
		return SwitchToCardIndex(cbCardData) == m_cbMagicIndex;
	return false;
}

//排序,根据牌值排序
bool CChessLogic::SortCardList( U8 cbCardData[MAX_COUNT], U8 cbCardCount )
{
	//数目过虑
	if (cbCardCount==0||cbCardCount>MAX_COUNT) return false;

	//排序操作
	bool bSorted=true;
	U8 cbSwitchData=0,cbLast=cbCardCount-1;
	do
	{
		bSorted=true;
		for (U8 i=0;i<cbLast;i++)
		{
			if (cbCardData[i]>cbCardData[i+1])
			{
				//设置标志
				bSorted=false;

				//扑克数据
				cbSwitchData=cbCardData[i];
				cbCardData[i]=cbCardData[i+1];
				cbCardData[i+1]=cbSwitchData;
			}	
		}
		cbLast--;
	} while(bSorted==false);

	return true;
}

/*
// 胡法分析函数
*/

//大对子
bool CChessLogic::IsPengPeng( const tagAnalyseItem *pAnalyseItem )
{
	for( U8 i = 0; i < CountArray(pAnalyseItem->cbWeaveKind); i++ )
	{
		if( pAnalyseItem->cbWeaveKind[i]&(WIK_LEFT|WIK_CENTER|WIK_RIGHT) )
			return false;
	}
	return true;
}

//清一色牌
bool CChessLogic::IsQingYiSe(const U8 cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], const U8 cbItemCount,const U8 cbCurrentCard)
{
	//胡牌判断
	U8 cbCardColor=0xFF;

	for (U8 i=0;i<MAX_INDEX;i++)
	{
		if(i==m_cbMagicIndex) continue;
		if (cbCardIndex[i]!=0)
		{
			//花色判断
			if (cbCardColor!=0xFF)
				return false;

			//设置花色
			cbCardColor=(SwitchToCardData(i)&MASK_COLOR);

			//设置索引
			i=(i/9+1)*9-1;
		}
	}

	//不包过字牌
	if((cbCardColor == 0x30))
	{
		return false;
	}

	//如果手上只有王霸
	if( cbCardColor == 0xFF )
	{
		ASSERT( m_cbMagicIndex != MAX_INDEX && cbCardIndex[m_cbMagicIndex] > 0 );
		//检查组合
		ASSERT( cbItemCount > 0 );
		cbCardColor = WeaveItem[0].cbCenterCard&MASK_COLOR;
	}

	if((cbCurrentCard&MASK_COLOR)!=cbCardColor && !IsMagicCard(cbCurrentCard) ) return false;

	//组合判断
	for (U8 i=0;i<cbItemCount;i++)
	{
		U8 cbCenterCard=WeaveItem[i].cbCenterCard;
		if ((cbCenterCard&MASK_COLOR)!=cbCardColor)	return false;
	}

	//不包过字牌
	if((cbCardColor == 0x30))
	{
		return false;
	}

	return true;
}

//清一色牌
bool CChessLogic::IsZiYiSe(const U8 cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], const U8 cbItemCount,const U8 cbCurrentCard)
{
	//胡牌判断
	U8 cbCardColor=0xFF;

	for (U8 i=0;i<MAX_INDEX;i++)
	{
		if(i==m_cbMagicIndex) continue;
		if (cbCardIndex[i]!=0)
		{
			//花色判断
			if (cbCardColor!=0xFF)
				return false;

			//设置花色
			cbCardColor=(SwitchToCardData(i)&MASK_COLOR);

			//设置索引
			i=(i/9+1)*9-1;
		}
	}

	//字牌
	if(cbCardColor != 0x30)
	{
		return false;
	}

	//如果手上只有王霸
	if( cbCardColor == 0xFF )
	{
		ASSERT( m_cbMagicIndex != MAX_INDEX && cbCardIndex[m_cbMagicIndex] > 0 );
		//检查组合
		ASSERT( cbItemCount > 0 );
		cbCardColor = WeaveItem[0].cbCenterCard&MASK_COLOR;
	}

	if((cbCurrentCard&MASK_COLOR)!=cbCardColor && !IsMagicCard(cbCurrentCard) ) return false;

	//组合判断
	for (U8 i=0;i<cbItemCount;i++)
	{
		U8 cbCenterCard=WeaveItem[i].cbCenterCard;
		if ((cbCenterCard&MASK_COLOR)!=cbCardColor)	return false;
	}

	//字牌
	if(cbCardColor != 0x30)
	{
		return false;
	}

	return true;
}

//七小对牌
bool CChessLogic::IsQiXiaoDui(const U8 cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], const U8 cbWeaveCount,const U8 cbCurrentCard,int& nGenCount)
{
	//组合判断
	if (cbWeaveCount!=0) return false;

	//单牌数目
	U8 cbReplaceCount = 0;
	nGenCount = 0;

	//临时数据
	U8 cbCardIndexTemp[MAX_INDEX];
	CopyMemory(cbCardIndexTemp,cbCardIndex,sizeof(cbCardIndexTemp));

	//插入数据
	U8 cbCurrentIndex = SwitchToCardIndex(cbCurrentCard);
	cbCardIndexTemp[cbCurrentIndex]++;

	//计算单牌
	for (U8 i=0;i<MAX_INDEX;i++)
	{
		U8 cbCardCount=cbCardIndexTemp[i];

		//王牌过滤
		if( i == m_cbMagicIndex ) continue;

		//单牌统计
		if( cbCardCount == 1 || cbCardCount == 3 ) 	cbReplaceCount++;

		if (cbCardCount == 4 )
		{
			nGenCount++;
		}
	}
	
	//王牌不够
	if( m_cbMagicIndex != MAX_INDEX && cbReplaceCount > cbCardIndexTemp[m_cbMagicIndex] ||
		m_cbMagicIndex == MAX_INDEX && cbReplaceCount > 0 )
		return false;

	return true;

}

//卡单
bool CChessLogic::IsKaDan( const U8 cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], U8 cbWeaveCount )
{
	//复制数据
	U32 nCurNum = 0;
	U8 cbCardIndexTemp[MAX_INDEX];
	CopyMemory( cbCardIndexTemp,cbCardIndex,sizeof(cbCardIndexTemp) );

	CChiHuRight chr;
	for( U8 i = 0; i < MAX_INDEX; i++ )
	{
		U8 cbCurrentCard = SwitchToCardData( i );
		if( WIK_CHI_HU == AnalyseChiHuCard( cbCardIndexTemp,WeaveItem,cbWeaveCount,cbCurrentCard,chr ) )
			nCurNum++;

		if(nCurNum >= 2)
			return true;
	}
	return false;
}

//将将胡
bool CChessLogic::IsJiangJiangHu(const U8 cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[],const  U8 cbWeaveCount,const U8 cbCurrentCard)
{
	//单牌数目
	U8 cbReplaceCount = 0;

	//临时数据
	U8 cbCardIndexTemp[MAX_INDEX];
	CopyMemory(cbCardIndexTemp,cbCardIndex,sizeof(cbCardIndexTemp));

	//插入数据
	U8 cbCurrentIndex = SwitchToCardIndex(cbCurrentCard);
	cbCardIndexTemp[cbCurrentIndex]++;

	//计算单牌
	for (U8 i=0;i<MAX_INDEX;i++)
	{
		U8 cbValue =(SwitchToCardData(i) & MASK_VALUE);

		//单牌统计
		if( cbValue != 2 && cbValue != 5 && cbValue == 8 )
		{
			return false;
		}
	}
	return true;
}

bool CChessLogic::IsDanDiao(const U8 cbCardIndex[MAX_INDEX],const U8 cbCurrentCard)
{
	//单牌数目
	U8 cbReplaceCount = 0;

	//临时数据
	U8 cbCardIndexTemp[MAX_INDEX];
	CopyMemory(cbCardIndexTemp,cbCardIndex,sizeof(cbCardIndexTemp));

	//插入数据
	U8 cbCurrentIndex = SwitchToCardIndex(cbCurrentCard);
	cbCardIndexTemp[cbCurrentIndex]++;

	//计算单牌
	int nTaltal = 0;
	bool bDuizi = false;
	for (U8 i=0;i<MAX_INDEX;i++)
	{
		U8 cbCardCount=cbCardIndexTemp[i];

		//王牌过滤
		if( i == m_cbMagicIndex ) continue;

		//单牌统计
		if( cbCardCount == 2) 	
		{
			bDuizi = true;
		}
		nTaltal += cbCardCount;
	}

	if (bDuizi && nTaltal == 2)
	{
		return true;
	}
	return false;
}

//带幺
bool CChessLogic::IsDaiYao( const tagAnalyseItem *pAnalyseItem )
{
	//检查牌眼
	U8 cbCardValue = pAnalyseItem->cbCardEye&MASK_VALUE;
	if( cbCardValue != 1 && cbCardValue != 9 ) return false;

	for( U8 i = 0; i < CountArray(pAnalyseItem->cbWeaveKind); i++ )
	{
		if( pAnalyseItem->cbWeaveKind[i]&(WIK_LEFT|WIK_CENTER|WIK_RIGHT) )
		{
			U8 j = 0;
			for(; j < 3; j++ )
			{
				cbCardValue = pAnalyseItem->cbCardData[i][j]&MASK_VALUE;
				if( cbCardValue == 1 || cbCardValue == 9 ) break;
			}
			if( j == 3 ) return false;
		}
		else
		{
			cbCardValue = pAnalyseItem->cbCenterCard[i]&MASK_VALUE;
			if( cbCardValue != 1 && cbCardValue != 9 ) return false;
		}
	}
	return true;
}

//将对
bool CChessLogic::IsJiangDui( const tagAnalyseItem *pAnalyseItem )
{
	//是否大对子
	if( !IsPengPeng(pAnalyseItem) ) return false;

	//检查牌眼
	U8 cbCardValue = pAnalyseItem->cbCardEye&MASK_VALUE;
	if( cbCardValue != 2 && cbCardValue != 5 && cbCardValue != 8 ) return false;

	for( U8 i = 0; i < CountArray(pAnalyseItem->cbWeaveKind); i++ )
	{
		if( pAnalyseItem->cbWeaveKind[i]&(WIK_LEFT|WIK_CENTER|WIK_RIGHT) )
		{
			U8 j = 0;
			for(; j < 3; j++ )
			{
				cbCardValue = pAnalyseItem->cbCardData[i][j]&MASK_VALUE;
				if( cbCardValue == 2 || cbCardValue == 5 || cbCardValue == 8 ) break;
			}
			if( j == 3 ) return false;
		}
		else
		{
			cbCardValue = pAnalyseItem->cbCenterCard[i]&MASK_VALUE;
			if( cbCardValue != 2 && cbCardValue != 5 && cbCardValue != 8 ) return false;
		}
	}
	return true;
}

//是否花猪
bool CChessLogic::IsHuaZhu( const U8 cbCardIndex[], const tagWeaveItem WeaveItem[], U8 cbWeaveCount )
{
	U8 cbColor[3] = { 0,0,0 };
	for( U8 i = 0; i < MAX_INDEX; i++ )
	{
		if( cbCardIndex[i] > 0 )
		{
			U8 cbCardColor = SwitchToCardData(i)&MASK_COLOR;
			cbColor[cbCardColor>>4]++;

			i = (i/9+1)*9-1;
		}
	}
	for( U8 i = 0; i < cbWeaveCount; i++ )
	{
		U8 cbCardColor = WeaveItem[i].cbCenterCard&MASK_COLOR;
		cbColor[cbCardColor>>4]++;
	}
	//缺一门就不是花猪
	for( U8 i = 0; i < CountArray(cbColor); i++ )
		if( cbColor[i] == 0 ) return false;

	return true;
}


U8 CChessLogic::GetPickNiaoCount(U8 cbCardData[MAX_NIAO_CARD],U8 cbCardNum)
{
	U8 cbPickNum = 0;
	for (int i=0;i<cbCardNum;i++)
	{
		ASSERT(IsValidCard(cbCardData[i]));

		U8 nValue = cbCardData[i]&MASK_VALUE;
		if (nValue == 1 || nValue == 5 || nValue == 9)
		{
			cbPickNum++;
		}
		
	}
	return cbPickNum;
}



//////////////////////////////////////////////////////////////////////////
