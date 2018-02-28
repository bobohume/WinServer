#include "stdafx.h"

#include "PacketFactoryManager.h"

PacketFactoryManager::PacketFactoryManager(void)
{
	m_Factories.clear();
}

PacketFactoryManager::~PacketFactoryManager(void) 
{		
	/*for(int i = 0; i < sizeof(m_Factories); ++i) 
	{
		SAFE_DELETE(m_Factories[i]) ;
	}*/
}

void PacketFactoryManager::AddFactory( PacketFactory* pFactory ) 
{		
	if( m_Factories[pFactory->GetPacketID()] != NULL ) 
	{
		assert(0) ;
		return ;
	}
	
	m_Factories[pFactory->GetPacketID()] = pFactory ;
}

PacketBase* PacketFactoryManager::CreatePacket(int packetID) 
{
	if(m_Factories[packetID] == NULL ) 
	{
		//assert(0) ;
		return 0 ;
	}

	PacketBase* pPacket = NULL ;
	
	OLD_DO_LOCK(m_cs);
	
	_MY_TRY
	{
		pPacket = m_Factories[packetID]->CreatePacket();
	}
	_MY_CATCH
	{
		pPacket = NULL;
	}

	return pPacket;
}

void PacketFactoryManager::RemovePacket( PacketBase* pPacket )
{
	if( pPacket == 0)
	{
		assert(0) ;
		return ;
	}

	int packetID = pPacket->GetPacketID();

	if (packetID <= 0)
	{
		delete pPacket;
		return;
	}
	
	if( m_Factories[packetID] == NULL) 
	{
		assert(0) ;
		return;
	}

    OLD_DO_LOCK(m_cs);
	
	_MY_TRY
	{
		m_Factories[packetID]->DestoryPacket(pPacket);
	}
	_MY_CATCH
	{
	}
}



