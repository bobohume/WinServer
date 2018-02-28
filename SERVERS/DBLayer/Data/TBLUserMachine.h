#ifndef __TBLUSERMACHINE_H__
#define __TBLUSERMACHINE_H__

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif

#include <vector>

class CDBConn;

class TBLUserMachine: public TBLBase
{
public:
	typedef __int64 U64;
	
	struct stUserMahine
	{
		U64		id;							//唯一ID
		unsigned int accountId;
		char	IPAddress[32];				//IP地址
		char	MacAddress[64];				//Mac网卡物理地址
		char	OSVersion[64];				//操作系统版本
		char	CPUName[64];				//CPU名称
		U64		CPUFreq;					//CPU频率
		char	VideoCardName[64];			//显卡名称
		char	VideoDriverVersion[64];	    //显卡驱动版本
		char	VideoMemory[64];			//显存大小
		char	DirectXVersion[64];	        //DX版本
		U64		TotalPhysMemory;			//总物理内存大小
		U64		CurrentPhysMemory;			//当前物理内存大小
		U64		TotalVirtualMemory;			//总虚拟内存大小
		U64		CurrentVirtualMemory;		//当前虚拟内存大小
		U64		UsePhysMemory;				//当前进程使用物理内存
		U64		UseVirtualMemory;			//当前进程使用虚拟内存
		U64		DiskSize;					//硬盘大小
		U64		CurrentDriveSize;			//当前驱动器大小
		U64		CurrentDriveFreeSize;		//当前驱动器剩余大小
	};
	
	TBLUserMachine(DataBase* db):TBLBase(db) {}
	virtual ~TBLUserMachine() {}
	DBError Load(std::vector<stUserMahine>& Info);
	DBError Save(stUserMahine& Info);
	DBError QueryMAC(int AccountId, std::string& strMacAddress);
};

#endif//__TBLUSERMACHINE_H__