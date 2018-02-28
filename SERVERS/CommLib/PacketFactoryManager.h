#ifndef _PACKETFACTORYMANAGER_H_
#define _PACKETFACTORYMANAGER_H_

#include "PacketFactory.h"
#include "Common/PacketType.h"
#include "base/Locker.h"
#include <hash_map>

class PacketFactoryManager 
{
public :	
	PacketFactoryManager(void);
	~PacketFactoryManager(void);

	static PacketFactoryManager* Instance(void)
	{
		static PacketFactoryManager local;
		return &local;
	}

	void        AddFactory(PacketFactory* pFactory);
	
	PacketBase* CreatePacket(int packetID);
	void        RemovePacket(PacketBase* pPacket);
private :
	//PacketFactory* m_Factories[END_NET_MESSAGE];
	stdext::hash_map<int, PacketFactory*> m_Factories;
	
	CMyCriticalSection m_cs;
};

#define SAFE_REMOVE_PACKET(x) PacketFactoryManager::Instance()->RemovePacket(x);
#define CREATE_PACKET(x) PacketFactoryManager::Instance()->CreatePacket(x);
#define ADD_PACKET_FACTORY(x) PacketFactoryManager::Instance()->AddFactory(new x);

#endif /*_PACKETFACTORYMANAGER_H_*/
