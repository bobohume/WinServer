#ifndef __ACHIEVEMENTBASE_H__
#define __ACHIEVEMENTBASE_H__

#include "Base/BitStream.h"
#include "Common/PacketType.h"

//荣誉结构
struct stCredit
{
	U8 credit;					//荣誉值
	U8 drawflag;				//是否领取奖励
	U32 id;						//荣誉ID
	U32 uptime;					//达成时间
};

//荣誉统计值
struct stCreditCounter
{
	U32 id;						//荣誉统计值编号
	S32 count;					//荣誉统计值
};

struct stAchievement
{
	int	 nPlayerId;				//角色ID
	int  nCredit;				//成就累计总值
	int	 nCount;				//完成成就总数
	int* nIdList;				//成就ID列表
	int* nUpTimeList;			//成就达成时间列表

	stAchievement()
	{
		nPlayerId = nCredit = nCount = 0;
		nIdList = nUpTimeList = NULL;
	}
	~stAchievement()
	{
		Clear();
	}

	void Clear()
	{
		if(nIdList)
			delete[] nIdList;
		if(nUpTimeList)
			delete[] nUpTimeList;
		nUpTimeList = nIdList = NULL;
		nCredit = nCount = 0;
	}

	void Build(int count)
	{
		if(count <= 0)
			return;

		Clear();
		nCount = count;
		nIdList = new int[count];
		nUpTimeList = new int[count];
	}

	void ReBuild(int count)
	{
		if(count <= 0)
			return;
		int* idlist		= new int[count];
		int* countlist	= new int[count];
		int credit		= nCredit;
		memcpy(idlist, nIdList, sizeof(int) * nCount);
		memcpy(countlist, nUpTimeList, sizeof(int) * nCount);
		Clear();
		nCredit		= credit;
		nCount		= count;
		nIdList		= idlist;
		nUpTimeList = countlist;
	}

	void Sum(int Credit)
	{
		nCredit += Credit;
	}

	template<typename T>
	void PackData( T* stream)
	{
		stream->writeInt(nPlayerId, Base::Bit32);
		stream->writeInt(nCount,	Base::Bit32);
		if(nCount > 0 && nIdList && nUpTimeList)
		{
			for(int i = 0; i < nCount; i++)
			{
				stream->writeInt(nIdList[i], Base::Bit32);
				stream->writeInt(nUpTimeList[i], Base::Bit32);
			}
		}
	}

	template<typename T>
	void UnpackData(T* stream)
	{
		nPlayerId	= stream->readInt(Base::Bit32);
		nCount		= stream->readInt(Base::Bit32);
		if(nCount > 0)
		{
			Build(nCount);
			for(int i = 0; i < nCount; i++)
			{
				nIdList[i]		= stream->readInt(Base::Bit32);
				nUpTimeList[i]	= stream->readInt(Base::Bit32);
			}			
		}
	}
};

struct stAchRecord
{
	int	 nPlayerId;
	int	 nCount;
	int* nIdList;
	int* nCountList;

	stAchRecord()
	{
		nPlayerId = nCount = 0;
		nIdList = nCountList = NULL;
	}

	~stAchRecord()
	{
		Clear();
	}

	void Clear()
	{
		if(nIdList)
			delete[] nIdList;
		nIdList = NULL;
		if(nCountList)
			delete[] nCountList;
		nCountList = NULL;
	}

	void Build(int count)
	{
		if(count <= 0)
			return;

		Clear();
		nCount = count;
		nIdList = new int[count];
		nCountList = new int[count];
	}

	void ReBuild(int count)
	{
		if(count <= 0)
			return;
		int* idlist = new int[count];
		int* countlist = new int[count];
		memcpy(idlist, nIdList, sizeof(int) * nCount);
		memcpy(countlist, nCountList, sizeof(int) * nCount);
		Clear();
		nCount = count;
		nIdList = idlist;
		nCountList = countlist;
	}

	template<typename T>
	void PackData( T* stream)
	{
		stream->writeInt(nPlayerId, Base::Bit32);
		stream->writeInt(nCount,	Base::Bit32);
		if(nCount > 0 && nIdList && nCountList)
		{
			for(int i = 0; i < nCount; i++)
			{
				stream->writeInt(nIdList[i], Base::Bit32);
				stream->writeInt(nCountList[i], Base::Bit32);
			}
		}
	}

	template<typename T>
	void UnpackData(T* stream)
	{
		nPlayerId	= stream->readInt(Base::Bit32);
		nCount		= stream->readInt(Base::Bit32);
		if(nCount > 0)
		{
			Build(nCount);
			for(int i = 0; i < nCount; i++)
			{
				nIdList[i]		= stream->readInt(Base::Bit32);
				nCountList[i]	= stream->readInt(Base::Bit32);
			}			
		}
	}
};


#endif//__ACHIEVEMENTBASE_H__