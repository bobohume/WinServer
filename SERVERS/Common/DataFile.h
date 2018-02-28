#pragma once

#include <stdio.h>
#include <vector>
#include "DatBuilder/DatBuilder.h"

class CDataFile
{
public:
	int  RecordNum;  //记录数量
	int  ColumNum;   //列数量

	CDataFile();
	~CDataFile();
	//
	bool readDataFile   (const char* name);    //读取data文件
	bool GetData        (RData&);              //得到具体的内容
	//
	void ReadDataInit   ();
	void ReadDataClose  ();
private:
	FILE* pfdata;
	int       readstep;   //控制读的总的数量
	//

public:
	std::vector<U8> m_DataTypes;
	int	m_CurrentColumnIndex;
};