#include "stdafx.h"

#include "ShareMemory.h"
#include <windows.h>

#define INVALID_SM_HANDLE ((void*)0)
 
bool ShareMemory::Create(int key,int Size)
{	
	bool rslt = __DoCreate(key,Size);

	if (rslt)
	{
		char keybuf[64] = {0};
		sprintf_s(keybuf,sizeof(keybuf),"smfinished_%d",key);

		m_createEvent = CreateEvent(0,0,TRUE,keybuf);
	}

	return rslt;
}

bool ShareMemory::__DoCreate(int key,int Size)
{
	if (__DoAttach(key,Size))
	{
		return true;
	}
	
    char keybuf[64] = {0};
    sprintf_s(keybuf,sizeof(keybuf),"smkey_%d",key);

    m_smHandle = CreateFileMappingA((HANDLE)0xFFFFFFFFFFFFFFFF, NULL, PAGE_READWRITE, 0, Size, keybuf);

    if(m_smHandle == INVALID_SM_HANDLE)
    {
        return false;
    }

	m_isNewCreated = true;

    m_pHeader = (DataHead*)MapViewOfFile(m_smHandle,FILE_MAP_ALL_ACCESS,0,0,0);

    if(m_pHeader)
    {
        m_pData = (char*)(m_pHeader + 1);

        m_pHeader->m_Key  =	key;
        m_pHeader->m_Size =	Size;
        m_Size = Size;

        //对数据进行清空
        memset(m_pData,0,m_Size - sizeof(ShareMemory::DataHead));

        return true;
    }

    return false;
}

void ShareMemory::Destory(void)
{
	if(0 != m_createEvent)
	{
		CloseHandle(m_createEvent);
		m_createEvent = 0;
	}

    if( m_pHeader )
    {
        UnmapViewOfFile(m_pHeader);
        m_pHeader = 0;
    }

    if( m_smHandle )
    {
        CloseHandle(m_smHandle);
        m_smHandle = 0;
    }

    m_Size = 0;
}

bool ShareMemory::Attach(int key,int Size)
{
	char keybuf[64] = {0};
	sprintf_s(keybuf,sizeof(keybuf),"smfinished_%d",key);

    HANDLE et = OpenEvent(EVENT_ALL_ACCESS,0,keybuf);

	if (0 == et)
		return false;

	CloseHandle(et);

	return __DoAttach(key,Size);
}

bool ShareMemory::__DoAttach(int key,int Size)
{
	char keybuf[64] = {0};
	sprintf_s(keybuf,sizeof(keybuf),"smkey_%d",key);
    m_smHandle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, true, keybuf);

    if(m_smHandle == INVALID_SM_HANDLE)
    {
        return false;
    }

    m_pHeader = (DataHead*)MapViewOfFile(m_smHandle,FILE_MAP_ALL_ACCESS,0,0,0);

    if(m_pHeader)
    {
        m_pData = (char*)(m_pHeader + 1);
        assert(m_pHeader->m_Key  == key);
        assert(m_pHeader->m_Size == Size);
        
        m_Size = Size;
        return true;
    }
    
    return false;
}

bool ShareMemory::DumpToFile(char* FilePath)
{
    if (0 == FilePath)
        return false;

    FILE* f	= 0;
    fopen_s(&f,FilePath,"wb");
    
    if(!f)	
        return false;
        
    fwrite(m_pHeader,1,m_Size,f);
    fclose(f);

    return true;
}

bool ShareMemory::MergeFromFile(char* FilePath)
{
    assert(FilePath);

    FILE* f = 0;
    fopen_s(&f,FilePath,"rb");
    
    if(!f)
        return false;
        
    fseek(f,0L,SEEK_END);
    INT FileLength =ftell(f);
    fseek(f,0L,SEEK_SET);
    fread(m_pHeader,FileLength,1,f);
    fclose(f);

    return true;
}

int ShareMemory::GetHeadVer(void)
{
    return m_pHeader->m_version;
}

void ShareMemory::SetHeadVer(int ver)
{
    m_pHeader->m_version = ver;
}

int  ShareMemory::GetHeadStatus(void)
{
    return m_pHeader->m_status;
}

void ShareMemory::SetHeadStatus(int status)
{
    m_pHeader->m_status = status;
}