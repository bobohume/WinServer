
#ifndef WINSM_H
#define WINSM_H

#include <WinSock2.h>
#include <windows.h>
#include <string>

#define PAGE_SIZE 4096

struct SM_BLOCK_DATA
{
	void *blockData;
	SM_BLOCK_DATA *pNext;
};

struct SM_BLOCK_HEAD
{
	int blockNum;
	int blockSize;
	SM_BLOCK_DATA *pFree;
	SM_BLOCK_DATA *pUsed;
};

class SM_Object
{
	HANDLE m_Handle;
	void *m_pView;
	char *m_pData;
	int m_Protect;
	int m_Access;
	std::string m_Name;
	bool m_open;

	void initSM(int blockSize,int blockNum);
public:
	SM_Object();
	~SM_Object();

	void releaseSM();
	void *openSM(const char *name,int desiredAccess=FILE_MAP_READ);
	void *createSM(int blockSize,int blockNum,const char *name=NULL,int desiredAccess=FILE_MAP_ALL_ACCESS,int flProtect= PAGE_READWRITE);

	void *getData();
	bool isOpen(){return m_open;}
	int getBlockNum(){return ((SM_BLOCK_HEAD*)m_pView)->blockNum;}
};


#endif



