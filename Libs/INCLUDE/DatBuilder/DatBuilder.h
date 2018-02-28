
#pragma once
#include "base/types.h"
//#include "windows.h"
#include "ConfigStruct.h"
//#include "../../game/Engine/source/core/stringTable.h"
//#include "..\..\game\Engine\source\core\stringTable.h"


#define  MAX_FILE_PATH	256
#define  max_strLen  1024*30
struct FCheck 
{
	std::string  path;    //路径
	std::string title;    //标题
};

struct RData
{
	int       m_Type;   //数据类型
	//union
	struct
	{
		char	m_string[max_strLen];
		
		S8		m_S8;
		S16		m_S16;
		S32		m_S32;
		
		U8		m_U8;
		U16		m_U16;
		U32		m_U32;
		
		F32		m_F32;
		F64		m_F64;

		U8		m_Enum8;
		U16		m_Enum16;
	} m_Data;
};

#define		m_string		m_Data.m_string
#define		m_S8			m_Data.m_S8
#define		m_S16			m_Data.m_S16
#define		m_S32			m_Data.m_S32
#define		m_U8			m_Data.m_U8
#define		m_U16			m_Data.m_U16
#define		m_U32			m_Data.m_U32
#define		m_Enum8			m_Data.m_Enum8
#define		m_Enum16		m_Data.m_Enum16
#define		m_F32			m_Data.m_F32
#define		m_F64			m_Data.m_F64


class COperateData
{
public:
	//HWND       m_hWnd ;                          //工具窗口的HANDLE用于输出提示信息
    void       OutData();						//建立ata文件
	void       OutExlINi();                     //建立EXL和INI
	//用于读取数据的变量
	int  RecordNum;  //记录数量
	int  ColumNum;   //列数量

	char* ChangetxtUtf8ToAnsi(char* szu8);
	void ChangeTextEncode(const char *filename);
	void ReadDataInit   ();
	void ReadDataClose  ();
	bool readDataFile   (const char* name);    //读取data文件
	bool GetData        (RData&);              //得到具体的内容
	//
	COperateData();
	~COperateData();
	//
private:


	std::vector<Cconfig>  m_ConfigList;
	std::vector<FCheck>  m_IexFlieList;    //iex列表
	std::vector<FCheck>  m_IniFlieList;    //int列表
	std::vector<FCheck>  m_DataFlieList;   //Data列表
	//
	char			 m_Path[MAX_FILE_PATH];        //生成文件的路径
	char             m_FileName[MAX_FILE_PATH];    //要生成的文件名
	char             m_FileName2[MAX_FILE_PATH];   //
	char			 ErrorChr[256];
	//用于读取数据的变量
	FILE* pfdata;
	int   readstep;   //控制读的总的数量
	//
	std::string     GetFileExtenName(const char* name);  //得到文件的扩展名  
	std::string     GetFileTitleName(const char* name);
	void			RansackFile(const char* lpPath);     //遍历所有的文件 isOut 

	//
	void		BuildData(const char* fiex);       //输出Ddata文件
	void		BulidConfigVector(const char* fini); //建立必要的解析信息
	void		ParseData(const char* filename);   //解析Data文件
	
	//主键唯一性检查(主键只可能是string和int)
	std::vector<int>     iMainKeyVt;
	std::vector<std::string>  sMainKeyVt;

	DataType             MainKeyType;              //主键的类型
	int                  MainKeyIndex;

	bool       CheckmKeyExclusive(S32 itemp, std::string stemp);           //检查主键的唯一性 
	//
	void       ShowErrorMessage(const char* chr);  //显示错误信息
};
