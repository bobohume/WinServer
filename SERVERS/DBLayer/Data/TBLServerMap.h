#ifndef __TBLSERVERMAP_H__
#define __TBLSERVERMAP_H__

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif

#include <vector>

class CDBConn;

class TBLServerMap: public TBLBase
{
public:
    //地图的类型
    enum MapType
    {
        MAP_TYPE_NORMAL = 0,
        MAP_TYPE_COPYMAP,
        
        MAP_TYPE_COUNT
    };
    
	struct stZoneInfo
	{
		int ZoneId;			//地图服务器编号
		int MapType;        //地图的类型(MapType)
		int MaxPlayerNum;	//最大玩家数量
		int ServicePort;    //服务端口号
		int Sequence;
	};
	
	TBLServerMap(DataBase* db):TBLBase(db) {}
	virtual ~TBLServerMap() {}
	
	DBError	Load(std::vector<stZoneInfo>& ZoneInfo);
};

#endif//__TBLSERVERMAP_H__