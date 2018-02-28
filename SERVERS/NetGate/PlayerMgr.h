#ifndef PLAYER_MANAGER_H
#define PLAYER_MANAGER_H

#include <string>
#include <hash_map>
#include <Time.h>
#include <list>

#include "Base/Locker.h"
#include "Common/PacketType.h"

class CPlayerManager
{
private:
	struct stAccountInfo
	{
		int LastTime;
		T_UID UID;
		int socketId;
		bool isAdult;
		char AccountName[ACCOUNT_NAME_LENGTH];

		stAccountInfo(T_UID uid, int socket)
		{
			LastTime	 = (int)time(NULL);
			UID			 = uid;
			socketId	 = socket;
			isAdult		 = true;
			AccountName[0] = 0;
		}
	};

	typedef stdext::hash_map<int, int> SocketMap;//first:socketId, second:AccountId
	typedef SocketMap::iterator SocketMapIter;
	typedef SocketMap::const_iterator SocketMapConIter;
	typedef stdext::hash_map<int, stAccountInfo> AccountMap;//first£ºAccountId
	typedef AccountMap::iterator AccountMapIter;

	SocketMap m_SocketMap;
	AccountMap m_AccountMap;

	static int m_uidSeed;

public:
	CMyCriticalSection m_cs;

	void TimeProcess(U32 bExit);

	void ReleaseSocketMap(int SocketId, bool postEvent=false);
	int GetAccountSocket(int AccountId);
	int GetSocketAccount(int nSocket, T_UID& UID);
	ERROR_CODE AddAccountMap(int SocketId, T_UID UID, int AccountId);
	void Broadcast(Base::BitStream& sendPacket, int ctrlType);

    void SendAccounts(void);
};

#endif