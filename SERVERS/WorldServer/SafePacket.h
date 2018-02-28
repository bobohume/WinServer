#ifndef _SAFEPACKET_H_
#define _SAFEPACKET_H_

#include <memory>
#include <hash_map>
#include <vector>
#include "base/Locker.h"
#include "base/Bitstream.h"

class Player;
typedef std::tr1::shared_ptr<Player> AccountRef;

//如果玩家在游戏状态中,但仍有可能处于传送或断线状态,导致消息包无法发送出去,SafePacket
//会对消息包进行缓存,并在玩家进入游戏后发送缓冲的消息包
class SafePacket
{
    struct DelayPacket
    {
        DelayPacket(void)
        {
            pData = 0;
            size  = 0;
        }

        char* pData;
        int   size;
    };

public:
    static SafePacket* Instance(void)
    {
        static SafePacket local;
        return &local;
    }
    
    bool Send(AccountRef spPlayer,Base::BitStream& packet);

    //强制发送所有的消息包
    void Flush(Player* pPlayer);
    
    //清理不需要发送的消息包
    void Update(void);
protected:
    SafePacket(void);

    bool DoSend(Player* pPlayer,Base::BitStream& packet,bool isBuffed);

    CMyCriticalSection m_cs;
    
    typedef stdext::hash_map<U32,std::vector<DelayPacket> > PACKETS_MAP;
    PACKETS_MAP m_packets;
};

#define SAFEPKT SafePacket::Instance()

#endif /*_SAFEPACKET_H_*/