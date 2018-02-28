#ifndef SYNC_OBJECT_H
#define SYNC_OBJECT_H

#include "Common/CppTypeInfo.h"
#include "base/bitStream.h"
#include "Wintcp/IPacket.h"

#include <memory>

#ifdef WORLDSERVER
#include "WorldServer\WorldServer.h"
#include "wintcp/dtServerSocket.h"
#include "base/Locker.h"
#include "WorldServer/ManagerBase.h"
#include "WorldServer/LockCommon.h"
#endif

typedef unsigned int SyncMask;

//const unsigned int SyncMaskFull = 0xffffffff;
enum{ SyncMaskFull = 0xffffffff,};

class SyncConnection;

class SyncEvent : public CTI::AbstractClassType< SyncEvent >
{
public:
	int m_nSrcID;//起始玩家ID

	virtual void packData( Base::BitStream& stream ){ stream.writeInt(m_nSrcID,32); };//Class->参数
	virtual void unpackData( Base::BitStream& stream ){	m_nSrcID = stream.readInt( 32 ); };//参数->Class

	virtual void process( SyncConnection& conn ) {}
} ;

class SyncConnection : public CTI::AbstractClassType< SyncConnection >
{
public:
	virtual void postEvent( SyncEvent& event ) = 0;
};

class SyncObject : public CTI::ClassType< SyncObject >
{
public:
	SyncObject(){ mSyncMask = 0;}
	virtual ~SyncObject(){}

	enum masks //: SyncMask
	{
		InitMask = 1,
		NextFree = 1 << 1,
	};

	virtual void packUpdate( Base::BitStream& stream/*, SyncMask syncMask*/ );//Class->参数,WorldServer

	virtual void unpackUpdate( Base::BitStream& stream );//参数->Class,ZoneSerVer||Client
#if defined( NTJ_SERVER ) || defined( NTJ_CLIENT )
	virtual void process( SyncConnection& conn ){}
#endif

	void registerObject();
	void unregisterObject();

	int GetSyncID(){ return mSyncObjectId; }
	void SetSyncID( int nID )
	{
		mSyncObjectId = nID; 
		//int oldId = mSyncObjectId;

		//while ( InterlockedExchangeAcquire( &mSyncObjectId, nID ) != oldId )
		//	oldId = mSyncObjectId;
	}

	virtual bool onAdd(){return true;}
	virtual void onRemove(){}

	void setSyncMask( SyncMask syncMask );
	void clearSyncMask();
	SyncMask getSyncMask(){ return mSyncMask; }

private:
	SyncMask mSyncMask;
	volatile int mSyncObjectId;
};

//class SyncException
//{
//public:
//	SyncException( std::string text );
//	virtual ~SyncException();
//
//private:
//	std::string mText;
//} ;

class SyncManager
#ifdef WORLDSERVER
	: public ILockable, public CManagerBase< SyncManager, MSG_SYNC_BEGIN, MSG_SYNC_END >
#endif	//WORLDSERVER
//#ifdef NTJ_SERVER
//
//#endif	//NTJ_SERVER
{
public:
	SyncManager();
	virtual ~SyncManager(){}

	void registerObject( SyncObject& object );
	void unregisterObject( SyncObject& object );

	SyncObject* getObject( int id );

	static SyncManager* getInstance()
	{
		return &instance;
	}

private:
	typedef stdext::hash_map< int, SyncObject* > SyncObjectMap;//WorldServer||Client中First=SyncObject::mSyncObjectId
	SyncObjectMap mSyncObjectMap;
	static SyncManager instance;

#if defined( NTJ_SERVER ) || defined( NTJ_CLIENT )
public:
	void onReceive( stPacketHead* pHead, Base::BitStream& stream );

	class WorldConnection : public CTI::ClassType< WorldConnection, SyncConnection >
	{
	public:
		void postEvent( SyncEvent& event );
	};
	WorldConnection& getConnection()
	{
		return mWorldConn;
	}
private:
	WorldConnection mWorldConn;
#endif //defined( NTJ_SERVER ) || defined( NTJ_CLIENT )

#ifdef WORLDSERVER
public:
	void setSyncMask( SyncObject* obj, SyncMask mask );
	void clearSyncMask( SyncObject* obj );

	void OnSyncEvent(int SocketHandle,stPacketHead *pHead,Base::BitStream *Packet);

	// 开始同步对象, 结束同步对象
	void startSync(){ mWorkThread.start(); }
	void stopSync(){ mWorkThread.stop(); }

	void syncThread( CTI::Thread* thread );

	CTI::Thread mWorkThread;

	struct clientSocket : public CTI::AbstractClassType< clientSocket, SyncConnection >
	{
		int socketId;

		static volatile LONG mIdSeed;

		int mId;

		clientSocket(){	socketId = 0; mId = InterlockedIncrement( &mIdSeed );}

		void postEvent( SyncEvent& event );

		virtual void send( Base::BitStream& stream ) = 0;

		typedef stdext::hash_map< SyncObject*, SyncMask > ObjectSet;

		ObjectSet mObjectSet;
	};

	typedef std::tr1::shared_ptr< clientSocket > ClientSocketRef;

	struct playerSocket : public CTI::ClassType< playerSocket, clientSocket >
	{
		int accountId;

		playerSocket(){	accountId = 0; }

		void send( Base::BitStream& stream );
	};

	typedef std::tr1::shared_ptr< playerSocket > PlayerSocketRef;

	struct zoneSocket : public CTI::ClassType< zoneSocket, clientSocket >
	{
		int zoneId;

		zoneSocket(){ zoneId = 0; }

		void send( Base::BitStream& stream );
	};

	typedef std::tr1::shared_ptr< zoneSocket > ZoneSocketRef;

	void addClient( ClientSocketRef client );
	void removeClient( ClientSocketRef client );

	void addScope( ClientSocketRef client, SyncObject* obj );
	//void removeScope( clientSocket* client, SyncObject* obj );
	void sendObject( SyncObject* obj );

	void scopeToAllZone( SyncObject& obj );
	void scopeAllObjects( ClientSocketRef client );

#endif //WORLDSERVER

#ifdef WORLDSERVER
private:
	int mSyncObjIdSeed;
	CMyCriticalSection m_cs;

	typedef stdext::hash_map< int, ClientSocketRef > ClientSet;
	ClientSet mClientSet;

	typedef stdext::hash_map< SyncObject*, ClientSet > ScopeMap;//second:发往各个ZoneServer的信息||发往各个Player客户端的信息
	ScopeMap mScopeMap;
#endif //WORLDSERVER

} ;

#endif /*SYNC_OBJECT_H*/