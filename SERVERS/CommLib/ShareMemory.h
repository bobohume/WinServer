#ifndef _SHAREMEMORY_H_
#define _SHAREMEMORY_H_

#include <assert.h>

/************************************************************************/
/* 封装share memory api操作
   header
   data
   
   header中的版本号,用来监控对应的客户端程序是否死机或异常退出
/************************************************************************/
class ShareMemory
{	
public:
    //当前内存块的状态
    enum Status
    {
        STATUS_NORMAL,
        STATUS_CLOSE,
        
        STATUS_COUNT
    };
    
    struct DataHead
    {
    public:
        int m_Key;
        int m_Size;
        int m_version;		
        int m_status;

        DataHead(void)
        {
            m_Size	  = 0;
            m_version = 0;
            m_status  = 0;
        }
    };
   
public:
	ShareMemory(void)
	{
		m_pData	    = 0;
		m_smHandle	= 0;
		m_Size		= 0;
		m_pHeader	= 0;	
		m_isNewCreated = 0;
		m_createEvent = 0;
	}
	
	~ShareMemory() { Destory(); };
	
	//创建或打开已经存在的
	bool	Create(int key,int Size);
	void	Destory(void);
	bool	Attach(int key,int Size);
	bool	Detach();

	bool	IsNewCreated(void) const {return m_isNewCreated;}
	
	int	    GetHeadVer(void);
	void    SetHeadVer(int ver);
	
	int     GetHeadStatus(void);
	void    SetHeadStatus(int status);
	
	//存储和从文件读取
	bool    DumpToFile(char* FilePath);
	bool    MergeFromFile(char* FilePath);

	char*	GetData(void) { return m_pData;}
	int	    GetSize(void) {return m_Size;}

	char*	GetTypePtr(int tSize,int tIndex)
	{
		assert(tSize > 0);
		assert(tSize*tIndex < m_Size);
		
		if( tSize<=0 || tIndex>=m_Size )
			return 0;
			
		return m_pData + tSize*tIndex;
	}
private:
	bool __DoCreate(int key,int Size);
	bool __DoAttach(int key,int Size);

	bool    m_isNewCreated;
	void*   m_smHandle;
	int     m_Size;
	char*   m_pData;
	
	DataHead* m_pHeader;
	
	//如果共享内存创建,并初始化好数据,则创建这个事件变量
	void*	m_createEvent;
};

#endif /*_SHAREMEMORY_H_*/