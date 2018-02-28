//
//  CGameBase.h
//  HelloLua
//
//  Created by BaoGuo on 14-1-17.
//
//

#ifndef HelloLua_CGameBase_h
#define HelloLua_CGameBase_h
#include "Base/types.h"
#include <hash_map>
#include <hash_set>
#include "stdlib.h"
#include "string.h"

class BuffTable;


enum en_GameObjectType
{
	GAME_GRID  = BIT(0),//地表
	GAME_WALL  = BIT(1),//墙体
	GAME_WATER = BIT(2),//水面
	GAME_NOWAY = BIT(3),//虚空
	GAME_ITEM  = BIT(4),//物品
	GAME_TRAP  = BIT(5),//陷阱
	GAME_ROAD  = BIT(6),//通道
	GAME_NPC   = BIT(7),//npc
	GAME_MONSTER = BIT(8),//怪物
	GAME_PLAYER = BIT(9),//玩家
	GAME_EXPLORED = BIT(10),//是否探索过
	GAME_SEEAREA  = BIT(11),//视野，后面可能有火把这概念
};

enum en_RoadType
{
	ROAD_DOOR      = BIT(0),//门
	ROAD_PASSWAY   = BIT(1),//通道
};

const static U32 GAME_TSSTATIC = 0 | GAME_WALL | GAME_WATER | GAME_NOWAY;
const static U32 GAME_TRIGGER  = 0 | GAME_ITEM | GAME_TRAP | GAME_ROAD;
const static U32 GAME_OBJECT = 0 | GAME_NPC | GAME_MONSTER | GAME_PLAYER;

template <class T>
class SimObjectPtr;

struct stPoint
{
	stPoint():row(0), col(0)
	{
	}

	stPoint(U32 row, U32 col):row(row), col(col)
	{
	}

	U32 row;
	U32 col;

	bool operator <(const stPoint& other) const
	{
		if(row < other.row)
		{
			return true;
		}
		else if(row == other.row)
		{
			if(col < other.col)
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		return false;
	}

	bool operator ==(const stPoint& other) const
	{
		if(row == other.row && col == other.col)
			return true;
		else
			return false;
	}

	stPoint operator +=(const stPoint& other)
	{
		col += other.col;
		row += other.row;
		return *this;
	}

	bool operator !=(const stPoint& other) const
	{
		if(row != other.row || col != other.col)
			return true;
		else
			return false;
	}

	void operator =(const stPoint& other)
	{
		row = other.row;
		col = other.col;
	}

	stPoint operator +(const stPoint& other)
	{
		stPoint tempGrid;
		tempGrid.row = row + other.row;
		tempGrid.col = col + other.col;
		return tempGrid;
	}

	stPoint operator -(const stPoint& other)
	{
		stPoint tempGrid;
		tempGrid.row = row - other.row;
		tempGrid.col = col - other.col;
		return tempGrid;
	}

	F32 len()
	{
		return sqrt((float)(col*col + row*row));
	}

	static F32 getLengthen(const stPoint& start, const stPoint& end)
	{
		static int nCol  =  0;
		static int nRow  = 0;
		nCol = 0;
		nRow = 0;
		nCol = abs(((S32)start.col - (S32)end.col));
		nRow = abs(((S32)start.row - (S32)end.row));

		if(nRow >= nCol)
			return nRow;

		return nCol;
	}

	bool isVaild()
	{
		if(col == U32_MAX && row == U32_MAX)
			return false;
		return true;
	}
};

class CGameBase
{
public:
	virtual ~CGameBase() {};
	virtual U32 getObjectType() = 0;

	inline void setGridPosition(U32 row, U32 col)
	{
		m_grid.row = row;
		m_grid.col = col;
	}

	inline void setGridPosition(stPoint& grid)
	{
		m_grid = grid;
	}

	inline stPoint getGridPosition() const
	{
		return m_grid;
	}

protected:
	CGameBase():m_grid()
	{
	}

	stPoint m_grid;
};

template <class T>
class SimObjectPtr
{
private:
	CGameBase* mObj;

public:
	SimObjectPtr():mObj(NULL) {};
	SimObjectPtr(T* ptr):mObj(ptr)
	{
	}

	SimObjectPtr(const SimObjectPtr<T> &rhs)
	{
		mObj = const_cast<T *>(static_cast<T *>(rhs));
	}

	SimObjectPtr<T>& operator=(const SimObjectPtr<T>& rhs)
	{
		if(this == &rhs)
			return (*this);
		mObj = const_cast<T *>(static_cast<const T*>(rhs));

		return (*this);
	}



	~SimObjectPtr()
	{

	}

	SimObjectPtr<T>& operator = (T *ptr)
	{
		if(mObj != (CGameBase*)ptr)
		{
			mObj = (CGameBase*) ptr;
		}
		return (*this);
	}


	bool operator == (const CGameBase* ptr) {return  mObj == ptr; }
	bool operator == (CGameBase* ptr) { return mObj == ptr;  }
	bool operator != (const CGameBase* ptr) {return  mObj != ptr; }

	bool isNull() const { return  mObj == 0; }
	T* operator->() const { return  static_cast<T*>(mObj); }
	T& operator*() const { return static_cast<T*>(mObj); }
	operator T*() const { return static_cast<T*>(mObj)? static_cast<T*>(mObj) : NULL; }//强制转换重载
	T* getObject() const { return static_cast<T*>(mObj);}
};

#endif
