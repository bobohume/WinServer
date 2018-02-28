#pragma once

#include "Locker.h"
#include "memPool.h"
#include <hash_map>
#include <list>

#define MAX_TRY_COUNT 3

/*

template <typename T_DataPtr>
struct stKeyListBlock
{
	void Init()
	{
		Try = 0;
		LastTime = 0;
		Seq = 0;
	}

	U32	Try;
	U32 Seq;
	U32 LastTime;
	T_DataPtr *Info;
};

//Ray:建立这个模板是为了在顺序操作的队列中可以快速索引定位目标并操作
// 玩家角色保存列表，和玩家登陆排队列表都有这样的特性

template <typename T_Key,typename T_DataPtr>
class KeyList
{
	std::list< stKeyListBlock<T_DataPtr> *> m_List;				//工作链表
	stdext::hash_map<T_Key, stKeyListBlock<T_DataPtr> *> m_Map;	//Index

	CMemPool				*m_pMemPool;

public:
	HANDLE m_hEvent;

protected:
	CCriticalSection		m_cs;

private:
	stMapDequesBlock<T_DataPtr> *Alloc()
	{
		stMapDequesBlock<T_DataPtr> *pNew = NULL;
		if(m_pMemPool){
			pNew = m_pMemPool->Alloc(sizeof(stMapDequesBlock<T_DataPtr>));
		}else{
			pNew = new stMapDequesBlock<T_DataPtr>;
		}

		pNew->Init();
		return pNew;
	}

	void Release(stMapDequesBlock<T_DataPtr> *pOld)
	{
		//把节点分离
		pOld->Try = 0;
		if(pOld->pNext == NULL){
			m_pEnd = pOld->pPrev;
		}else{
			pOld->pNext->pPrev = pOld->pPrev;
		}
		if(pOld->pPrev == NULL){
			m_pHead = pOld->pNext;
		}else{
			pOld->pPrev->pNext = pOld->pNext;		
		}

		pOld->pNext = NULL;
		pOld->pPrev = NULL;

		if(m_pMemPool){
			m_pMemPool->Free(pOld);
		}else{
			delete pOld;
		}
	}

protected:
	void MoveToTail(stMapDequesBlock<T_Data> *pOld)
	{
		//把节点分离
		pOld->Try = 0;
		if(pOld->pNext == NULL){
			m_pEnd = pOld->pPrev;
		}else{
			pOld->pNext->pPrev = pOld->pPrev;
		}
		if(pOld->pPrev == NULL){
			m_pHead = pOld->pNext;
		}else{
			pOld->pPrev->pNext = pOld->pNext;		
		}

		pOld->pNext = NULL;
		pOld->pPrev = NULL;

		if(m_pEnd==NULL){
			m_pHead = pOld;
			m_pEnd  = pOld;
		}else{
			m_pEnd->pNext  = pOld;
			pOld->pPrev = m_pEnd;
			m_pEnd = pOld;
		}
	}

	BOOL Remove(T_Key Key,int Seq)
	{
		CLocker Lock(m_cs);

		stdext::hash_map<T_Key,stMapDequesBlock<T_Data> *>::iterator Pos;

		Pos = m_Map.find(Key);
		if(Pos != m_Map.end()){
			if(Pos->second->Seq == Seq){
				Release(Pos->second);
				m_Map.erase(Pos);
				return TRUE;
			}
		}

		return FALSE;
	}
	
	BOOL Remove(T_Key Key)
	{
		CLocker Lock(m_cs);

		stdext::hash_map<T_Key,stMapDequesBlock<T_Data> *>::iterator Pos;

		Pos = m_Map.find(Key);
		if(Pos != m_Map.end()){
			Release(Pos->second);
			m_Map.erase(Pos);
			return TRUE;
		}

		return FALSE;
	}

	stMapDequesBlock<T_Data> * GetWorksLink()
	{
		return m_pHead;
	}

	T_Data GetWorkData(T_Key Key)		//直接操作数据
	{
		stdext::hash_map<T_Key,stMapDequesBlock<T_Data> *>::iterator Pos;

		Pos = m_Map.find(Key);
		if(Pos != m_Map.end()){
			return Pos->second->Info;
		}

		return NULL;
	}

public:
	MapDeques(void)
	{
		m_pMemPool = CMemPool::GetInstance();
		m_hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	}

	virtual ~MapDeques(void)
	{
		CLocker dLock(m_cs);
		CloseHandle(m_hEvent);
	}

	BOOL Insert(T_Key Key,T_DataPtr pData,U32 Seq,bool EqualFlag=false)		//添加
	{
		CLocker dLock(m_cs);

		stdext::hash_map<T_Key,stMapDequesBlock<T_DataPtr> *>::iterator Pos;

		Pos = m_Map.find(Key);
		if(Pos != m_Map.end()){
			if((Pos->second->Seq<Seq) || (EqualFlag && Pos->second->Seq==Seq)){
				memcpy(Pos->second->Info,Data,sizeof(T_Data));
				Pos->second->Seq = Seq;
				Pos->second->Try = 0;
				return FALSE;
			}
		}else{
			stMapDequesBlock<T_Data> *pNew = Alloc();
			if(m_pEnd==NULL){
				m_pHead = pNew;
				m_pEnd  = pNew;
			}else{
				m_pEnd->pNext  = pNew;
				pNew->pPrev = m_pEnd;
				m_pEnd = pNew;
			}

			memcpy(&pNew->Info,pData,sizeof(T_Data));
			pNew->Seq = Seq;
			m_Map.insert(pair<T_Key,stMapDequesBlock<T_Data> *>(Key,pNew));
		}

		return TRUE;
	}

	BOOL Erase(T_Key Key,int Seq)
	{
		CLocker dLock(m_cs);

		return Remove(Key,Seq);
	}

	BOOL Detach(T_Key Key,T_Data *pData,int Seq)
	{
		CLocker dLock(m_cs);

		stdext::hash_map<T_Key,stMapDequesBlock<T_Data> *>::iterator Pos;

		Pos = m_Map.find(Key);
		if(Pos != m_Map.end()){
			if(Pos->second->Seq == Seq){
				if(pData != NULL)
					memcpy(pData,&Pos->second->Info,sizeof(T_Data));

				Remove(Key);
			}
			return TRUE;
		}

		return FALSE;
	}

	BOOL Copy(T_Key Key,T_Data *pData)		//从列表中复制
	{
		CLocker dLock(m_cs);

		stdext::hash_map<T_Key,stMapDequesBlock<T_Data> *>::iterator Pos;

		Pos = m_Map.find(Key);
		if(Pos != m_Map.end()){
			memcpy(pData,&Pos->second->Info,sizeof(T_Data));
			return TRUE;
		}

		return FALSE;
	}

	BOOL IsInQueue(T_Key Key)
	{
		CLocker dLock(m_cs);

		stdext::hash_map<T_Key,stMapDequesBlock<T_Data> *>::iterator Pos;

		Pos = m_Map.find(Key);
		if(Pos != m_Map.end()){
			return TRUE;
		}

		return FALSE;
	}

	size_t GetSize()
	{
		CLocker dLock(m_cs);

		return m_Map.size();
	}
};

*/