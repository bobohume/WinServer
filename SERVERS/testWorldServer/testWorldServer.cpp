// testWorldServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "diffDetector.h"
struct buff
{
	int m_nRoleID;
	int m_nBuffID;
	buff():m_nBuffID(0),m_nRoleID(0)
	{

	}
};


int _tmain(int argc, _TCHAR* argv[])
{
	buff a[10];
	buff b[10];
	a[0].m_nBuffID = 1; a[1].m_nBuffID=2; a[2].m_nBuffID=3;
	a[0].m_nRoleID = 2; a[1].m_nRoleID=2; a[2].m_nRoleID=2;

	b[0].m_nBuffID = 1; b[1].m_nBuffID=2; b[2].m_nBuffID=4;
	b[0].m_nRoleID = 1; b[1].m_nRoleID=2; b[2].m_nRoleID=3;
	Diff dif =DiffDetector<buff,10,int>::GetDiff(a,b,&buff::m_nBuffID);
	
	
	return 0;
}

