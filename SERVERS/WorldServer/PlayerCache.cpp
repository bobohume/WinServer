//#include "PlayerCache.h"
//#include "AccountHandler.h"
//#include <process.h>
//#include "Base\Log.h"
//#include "WorldServer.h"
//#include "ExportScript.h"
//#include "PlayerMgr.h"
//
//VOID CALLBACK s_apc( ULONG_PTR dwParam )
//{
//
//}
//
//CPlayerCache::~CPlayerCache()
//{
//	// 刷新数据缓存
//	flush();
//
//	mIsEnd = true;
//    QueueUserAPC(s_apc,mThreadHandle,0);
//	::WaitForSingleObject( mThreadHandle, -1 );
//}
//
//CPlayerCache::CPlayerCache()
//{
//	mIsEnd = false;
//
//	mLastCacheCheckTime = 0;
//	mHitPrecent = 0.0f;
//
//	// 开始一个工作进程用来保存数据
//	mThreadHandle = (HANDLE)_beginthread( WorkThread, 0, this );
//}
//
//void CPlayerCache::flush()
//{
//	CLocker l( mLoadLock );
//
//	PlayerDataList::NodeRef it = mPlayerDataList.getHead();
//	while( it )
//	{
//		CLocker lock( mSaveLock );
//		mSavePlayerDataList.push_back( PlayerDataList::NodeValueRef( it->mValue ) );
//		mDeleteSet[(*(it->mValue))->BaseData.PlayerId]++;
//		it = it->mNext;
//	}
//}
//
//PlayerDataRef CPlayerCache::loadPlayer( int playerId )
//{
//	mLoadLock.Lock();
//	
//	if( GetTickCount() - mLastCacheCheckTime > 1000 )
//	{
//		mHitPrecent = float( mHitCount ) / mVisitCount;
//		mHitCount = 0;
//		mVisitCount = 0;
//		mLastCacheCheckTime = GetTickCount();
//	}
//
//	mVisitCount++;
//
//	PlayerDataRef player;
//
//	HashPlayerIdMap::iterator it = mPlayerIdMap.find( playerId );
//	if( it != mPlayerIdMap.end() )
//	{
//		mHitCount++;
//		player = *(it->second->mValue);
//
//		mLoadLock.Unlock();
//	}
//	else
//	{
//		mSaveLock.Lock();
//
//		PlayerSetID::iterator it = mSaveSetID.find( playerId );
//		if( it != mSaveSetID.end() )
//		{
//			player = it->second;
//			mSaveLock.Unlock();
//			mLoadLock.Unlock();
//
//			return player;
//		}
//
//		mSaveLock.Unlock();
//
//		mLoadLock.Unlock();
//
//		player = PlayerDataRef( Parent::loadPlayer( playerId ) );
//		
//		mLoadLock.Lock();
//		
//		updatePlayer( player );
//
//		mLoadLock.Unlock();
//	}
//
//	return player;
//}
//
//PlayerDataRef CPlayerCache::loadPlayer( std::string playerName )
//{
//	mLoadLock.Lock();
//
//	if( GetTickCount() - mLastCacheCheckTime > 1000 )
//	{
//		mHitPrecent = float( mHitCount ) / mVisitCount;
//		mHitCount = 0;
//		mVisitCount = 0;
//		mLastCacheCheckTime = GetTickCount();
//	}
//
//	mVisitCount++;
//
//	PlayerDataRef player;
//
//	HashPlayerNameMap::iterator it = mPlayerNameMap.find( playerName );
//	if( it != mPlayerNameMap.end() )
//	{
//		mHitCount++;
//		player = *(it->second->mValue);
//
//		mLoadLock.Unlock();
//	}
//	else
//	{
//		mSaveLock.Lock();
//
//		PlayerSetName::iterator it = mSaveSetName.find( playerName );
//		if( it != mSaveSetName.end() )
//		{
//			player = it->second;
//			mSaveLock.Unlock();
//			mLoadLock.Unlock();
//
//			return player;
//		}
//
//		mSaveLock.Unlock();
//
//		mLoadLock.Unlock();
//#ifndef _NTJ_UNITTEST
//		player = PlayerDataRef( Parent::loadPlayer( playerName ) );
//#endif
//		mLoadLock.Lock();
//
//		updatePlayer( player );
//		
//		mLoadLock.Unlock();
//	}
//
//	return player;
//}
//
//int CPlayerCache::savePlayerData( PlayerDataRef pPlayerData )
//{
//	if( !pPlayerData )
//		return UNKNOW_ERROR;
//
//	updatePlayer( pPlayerData );
//
//	saveToDB( pPlayerData );
//
//	return NONE_ERROR;
//}
//
//void CPlayerCache::saveToDB( PlayerDataRef pPlayerData )
//{
//	CLocker lock( mSaveLock );
//
//	if( mSaveSetID.find( pPlayerData->BaseData.PlayerId ) != mSaveSetID.end() )
//		return ;
//
//	if( mSaveSetName.find( pPlayerData->BaseData.PlayerName ) != mSaveSetName.end() )
//		return ;
//
//	mSaveSetID[pPlayerData->BaseData.PlayerId] = pPlayerData;
//	mSaveSetName[pPlayerData->BaseData.PlayerName] = pPlayerData;
//
//	mSavePlayerDataList.push_back( PlayerDataList::NodeValueRef( new PlayerDataRef( pPlayerData ) ) );
//
//	QueueUserAPC(s_apc,mThreadHandle,0);
//}
//
//void CPlayerCache::WorkThread( void* pParam )
//{
//	CPlayerCache* pThis = static_cast< CPlayerCache* >( pParam );
//
//	SET_SHE_HANDLER;
//
//	while( 1 )
//	{
//		try
//		{
//			pThis->mLoadLock.Lock();
//			pThis->mSaveLock.Lock();
//
//			if( pThis->mSavePlayerDataList.size() > 0 )
//			{
//				PlayerDataRef pPlayerData = *( pThis->mSavePlayerDataList.getHead()->mValue );
//
//				if( pPlayerData )
//				{
//					int PlayerId = pPlayerData->BaseData.PlayerId;
//					{
//						// 如果这是对象是要删除的
//						if( pThis->mDeleteSet.find( PlayerId ) != pThis->mDeleteSet.end() )
//						{
//							pThis->mPlayerIdMap.erase( PlayerId );
//							pThis->mPlayerNameMap.erase( pPlayerData->BaseData.PlayerName );
//							pThis->mDeleteSet.erase( PlayerId );
//						}
//					}
//
//					g_Log.WriteLog("发送到保存线程[%s](Id:%d)",pPlayerData->BaseData.PlayerName,pPlayerData->BaseData.PlayerId);
//					pThis->savePlayer(pPlayerData.get());
//
//					pThis->mSavePlayerDataList.remove( pThis->mSavePlayerDataList.getHead() );
//
//					pThis->mSaveSetID.erase( PlayerId );
//					pThis->mSaveSetName.erase( pPlayerData->BaseData.PlayerName );
//
//
//					pThis->mSaveLock.Unlock();
//					pThis->mLoadLock.Unlock();
//
//					//pThis->mWorker.postWork( WorkMethod( CPlayerCache::doSave ), pThis, 1, new PlayerDataRef( pPlayerData ) );
//				}
//				else
//				{
//					pThis->mSavePlayerDataList.remove( pThis->mSavePlayerDataList.getHead() );
//
//					pThis->mSaveLock.Unlock();
//
//					pThis->mLoadLock.Unlock();
//				}
//			}
//			else
//			{
//				pThis->mSaveLock.Unlock();
//
//				pThis->mLoadLock.Unlock();
//
//				if( pThis->mIsEnd )
//					break;
//
//				SleepEx( -1, true );
//			}
//		}
//		catch( SHException she )
//		{
//		    pThis->mSaveLock.Unlock();
//            pThis->mLoadLock.Unlock();
//			g_dumper.writeDump( she.getExceptionPointer(),TINY_DUMP );
//			
//		}
//		catch( ... )
//		{
//		    pThis->mSaveLock.Unlock();
//            pThis->mLoadLock.Unlock();
//		}
//	}
//}
//
//void CPlayerCache::doSave( PlayerDataRef* player )
//{
//	// 保存到数据库中
//	savePlayer( (*player).get() );
//	delete player;
//}
//
//void CPlayerCache::processTick()
//{
//	
//}
//
////---------------------------------------------------------------
//// 用于更新玩家
////---------------------------------------------------------------
//void CPlayerCache::updatePlayer( PlayerDataRef pPlayerData )
//{
//	CLocker lock( mSaveLock );
//
//	if( !pPlayerData )
//		return ;
//
//	int playerId = pPlayerData->BaseData.PlayerId;
//
//	// 查询玩家数据是否在数据缓存中（越频繁访问的数据查询时间越短）
//	// 以下代码遗弃，采用自定义链表
//	//for( it=mPlayerDataList.begin(); it != mPlayerDataList.end(); ++it )
//	//{
//	//	if( ( *it )->BaseData.PlayerId == playerId )
//	//		break;
//	//}
//
//	PlayerDataList::NodeRef node;
//
//	HashPlayerIdMap::iterator it = mPlayerIdMap.find( playerId );
//
//	// 没有发现该数据在缓存中
//	if( it == mPlayerIdMap.end() )
//	{
//		// 缓存已满
//		if( mPlayerDataList.size() == CacheSize )
//		{
//			PlayerDataList::NodeRef node = mPlayerDataList.getTail();
//			
//			if( !node )
//				return ;
//			
//			PlayerDataRef pData = *node->mValue;
//
//			if( !pData )
//				return ;
//
//			{
//				CLocker lock( mSaveLock );
//
//				if( mSaveSetID.find( pData->BaseData.PlayerId ) == mSaveSetID.end() )
//				{
//					mSavePlayerDataList.push_back( PlayerDataList::NodeValueRef( new PlayerDataRef( pData ) ) );
//	
//					mDeleteSet[pData->BaseData.PlayerId]++;
//
//					mSaveSetID[pData->BaseData.PlayerId] = pData;
//
//					mSaveSetName[pData->BaseData.PlayerName] = pData;
//				}
//				else if( mSaveSetName.find( pData->BaseData.PlayerName ) == mSaveSetName.end() )
//				{
//					mSavePlayerDataList.push_back( PlayerDataList::NodeValueRef( new PlayerDataRef( pData ) ) );
//
//					mDeleteSet[pData->BaseData.PlayerId]++;
//
//					mSaveSetID[pData->BaseData.PlayerId] = pData;
//
//					mSaveSetName[pData->BaseData.PlayerName] = pData;
//				}
//			}
//
//			mPlayerIdMap.erase( pData->BaseData.PlayerId );
//			mPlayerNameMap.erase( pData->BaseData.PlayerName );
//
//			mPlayerDataList.remove( node );
//		}
//
//		PlayerDataList::NodeRef node = mPlayerDataList.push_front( PlayerDataList::NodeValueRef( new PlayerDataRef( pPlayerData ) ) );
//
//		mDeleteSet.erase( pPlayerData->BaseData.PlayerId );
//
//		mPlayerIdMap[pPlayerData->BaseData.PlayerId] = node;
//		mPlayerNameMap[pPlayerData->BaseData.PlayerName] = node;
//	}
//	else
//	{
//		// 将玩家数据移动到链表头
//
//		mPlayerDataList.remove( it->second );
//
//		mPlayerDataList.push_front( it->second );
//	}
//}
//
//int CPlayerCache::getSaveListCount()
//{
//	CLocker l( mLoadLock );
//	CLocker m( mSaveLock );
//
//	return (int)mSavePlayerDataList.size();
//}
//
//void CPlayerCache::showStatus()
//{
//	CLocker l( mLoadLock );
//	CLocker m( mSaveLock );
//
//	g_Log.WriteLog( "Cache size: %d", mPlayerDataList.size() );
//	g_Log.WriteLog( "Save queue size: %d", mSavePlayerDataList.size() );
//	g_Log.WriteLog( "Cache hit precent: %.2f%%", mHitPrecent * 100.0f );
//}
//
//CmdFunction( ShowCacheStatus )
//{
//	SERVER->GetPlayerManager()->showStatus();
//}