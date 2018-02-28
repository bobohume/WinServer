#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "TBLUserMachine.h"
#include "DBUtility.h"

// ----------------------------------------------------------------------------
// 保存用户机器配置数据
DBError TBLUserMachine::Save(stUserMahine& Info)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		SAFEDBSTRING(Info.IPAddress, IPAddress);
		SAFEDBSTRING(Info.MacAddress, MacAddress);
		SAFEDBSTRING(Info.CPUName, CPUName);
		SAFEDBSTRING(Info.VideoCardName, VideoCardName);
		SAFEDBSTRING(Info.OSVersion, OSVersion);
		SAFEDBSTRING(Info.VideoDriverVersion, VideoDriverVersion);
		SAFEDBSTRING(Info.VideoMemory, VideoMemory);
		SAFEDBSTRING(Info.DirectXVersion, DirectXVersion);

		M_SQL(GetConn(),"EXECUTE Sp_SaveUserMachine %d,'%s','%s','%s','%s',%I64d,'%s',"\
			"'%s','%s','%s',%I64d,%I64d,%I64d,%I64d,%I64d,%I64d,%I64d,%I64d,%I64d",
			Info.accountId,pIPAddress.ptr,pMacAddress.ptr, pOSVersion.ptr, pCPUName.ptr,
			Info.CPUFreq, pVideoCardName.ptr, pVideoDriverVersion.ptr, pVideoMemory.ptr,
			pDirectXVersion.ptr, Info.TotalPhysMemory, Info.CurrentPhysMemory,
			Info.TotalVirtualMemory, Info.CurrentVirtualMemory, Info.UsePhysMemory,
			Info.UseVirtualMemory, Info.DiskSize, Info.CurrentDriveSize, Info.CurrentDriveFreeSize);
		GetConn()->Exec();
		err = DBERR_NONE;
	}
	DBSCATCH(Info.IPAddress)
	return err;
}

// ----------------------------------------------------------------------------
// 查询用户机器配置数据
DBError TBLUserMachine::Load(std::vector<stUserMahine>& Info)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{		
		M_SQL(GetConn(),"SELECT * FROM Tbl_UserMachine");
		stUserMahine user;
		while(GetConn()->More())
		{			
			strcpy_s(user.IPAddress,			32, GetConn()->GetString());
			strcpy_s(user.MacAddress,			50, GetConn()->GetString());
			strcpy_s(user.OSVersion,			50, GetConn()->GetString());
			strcpy_s(user.CPUName,				50, GetConn()->GetString());
			user.CPUFreq = GetConn()->GetBigInt();
			strcpy_s(user.VideoCardName,		50, GetConn()->GetString());
			strcpy_s(user.VideoDriverVersion,	32, GetConn()->GetString());
			strcpy_s(user.VideoMemory,			32, GetConn()->GetString());
			strcpy_s(user.DirectXVersion,		32, GetConn()->GetString());
			user.TotalPhysMemory = GetConn()->GetBigInt();
			user.CurrentPhysMemory = GetConn()->GetBigInt();
			user.TotalPhysMemory = GetConn()->GetBigInt();
			user.TotalVirtualMemory = GetConn()->GetBigInt();
			user.CurrentVirtualMemory = GetConn()->GetBigInt();
			user.UsePhysMemory = GetConn()->GetBigInt();
			user.UseVirtualMemory = GetConn()->GetBigInt();
			user.DiskSize = GetConn()->GetBigInt();
			user.CurrentDriveSize = GetConn()->GetBigInt();
			user.CurrentDriveFreeSize = GetConn()->GetBigInt();
			Info.push_back(user);
		}
		err = DBERR_NONE;
	}
	DBECATCH()
	return err;
}

//查询户最近一次登录MAC
DBError TBLUserMachine::QueryMAC(int AccountId, std::string& strMacAddress)
{
	DBError err = DBERR_UNKNOWERR;
	try
	{
		M_SQL(GetConn(), "EXECUTE Sp_UserMachine_QueryMAC %d", AccountId);
		if (GetConn()->More())
		{
			strMacAddress = GetConn()->GetString();
			err = DBERR_NONE;
		}
	}
	DBCATCH(AccountId);
	return err;
}
