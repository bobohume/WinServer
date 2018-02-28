
#include "WinSM.h"

SM_Object::SM_Object()
{
	m_Handle = NULL;
	m_pView = NULL;
	m_pData = NULL;
	m_Protect = 0;
	m_Access = 0;
	m_open = true;
}

SM_Object::~SM_Object()
{
	releaseSM();
}

void SM_Object::releaseSM()
{
	if(m_pView)
	{
		UnmapViewOfFile(m_pView);
	}

	if(m_Handle)
	{
		CloseHandle(m_Handle);
	}

	m_pView = NULL;
	m_Handle = NULL;
}

void *SM_Object::openSM(const char *name,int desiredAccess)
{
	releaseSM();

	int flProtect = FILE_MAP_ALL_ACCESS;
	if(desiredAccess == FILE_MAP_READ)
		flProtect = FILE_MAP_READ;

	m_Handle = OpenFileMappingA(flProtect, FALSE ,name);
	if(!m_Handle)
		return NULL;

	m_pView = MapViewOfFile(m_Handle, desiredAccess, 0, 0, 0);

	SM_BLOCK_HEAD *pBlockHead = (SM_BLOCK_HEAD*)m_pView;
	SM_BLOCK_DATA *pBlockData = (SM_BLOCK_DATA*)((char*)m_pView+sizeof(SM_BLOCK_HEAD));
	m_pData = (char *)(pBlockData+pBlockHead->blockNum);

	return m_pView;
}

void *SM_Object::createSM(int blockSize,int blockNum,const char *name,int desiredAccess,int flProtect)
{
	if(openSM(name,desiredAccess))
		return m_pView;

	int headSize = sizeof(SM_BLOCK_HEAD) + sizeof(SM_BLOCK_DATA) * blockNum;
	int totalSize = blockSize * blockNum + headSize;
	
	if( totalSize & 0x0FFF )
		totalSize = ( totalSize & ~0x0FFF ) + PAGE_SIZE;

	m_Handle = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, flProtect, 0, totalSize, name);
	if(!m_Handle)
	{
		int a = GetLastError();
		return NULL;
	}

	m_pView = MapViewOfFile(m_Handle, desiredAccess, 0, 0, 0);
	initSM(blockSize,blockNum);
	
	m_open = false;

	return m_pView;
}

void SM_Object::initSM(int blockSize,int blockNum)
{
	SM_BLOCK_HEAD *pBlockHead = (SM_BLOCK_HEAD*)m_pView;
	pBlockHead->blockNum = blockNum;
	pBlockHead->blockSize = blockSize;
	pBlockHead->pUsed = NULL;

	SM_BLOCK_DATA *pBlockData = (SM_BLOCK_DATA*)((char*)m_pView+sizeof(SM_BLOCK_HEAD));
	pBlockHead->pFree = pBlockData;

	m_pData = (char *)(pBlockData+pBlockHead->blockNum);
	for(int i=0;i<pBlockHead->blockNum;i++)
	{
		pBlockData[i].blockData = m_pData+(i*pBlockHead->blockSize);
		pBlockData[i].pNext = &pBlockData[i+1];
	}

	pBlockData[pBlockHead->blockNum-1].pNext = NULL;

	memset(m_pData,0,blockSize * blockNum);
}

void *SM_Object::getData()
{
	return m_pData;
}












