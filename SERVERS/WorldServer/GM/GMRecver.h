#ifndef _GMRECVER_H_
#define _GMRECVER_H_

#include "base/Locker.h"
#include "base/Log.h"
#include "lua/LuaWarp.h"
#include "Common/PlayerStructEx.h"
#include "WINTCP/IPacket.h"
#include <hash_map>
#include <vector>

class CMyLog;

/************************************************************************/
/* ¥¶¿ÌGM√¸¡Ó
/************************************************************************/
class CGMRecver
{
public:
    CGMRecver(void);
    ~CGMRecver(void);

    bool Initialize(void);
    void Shutdown(void);

    CDBConn* GetDBConn(void) {return m_pDBConn;}

	static CGMRecver* Instance(void)
	{
		static CGMRecver local;
		return &local;
	}

	U32   AddGold(int PlayerId, S32 Gold, F32 Price);
    bool  HandleCommand(int SocketHandle,stPacketHead* pHead,Base::BitStream *Packet);
	const char* Format(const char* fmt,...);
private:
    CDBConn* m_pDBConn;
};

#endif /*_GMRECVER_H_*/