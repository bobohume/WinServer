#pragma once 
#include <vector>
#include <string>

#define  MAXStrLen 128
#define  IntLen     32

enum  DataType
{
	DType_none    = 0,
	
	DType_string,
	
	DType_enum8,
	DType_enum16,
	
	DType_S8,
	DType_S16,
	DType_S32,
	
	DType_U8,
	DType_U16,
	DType_U32,
	
	DType_F32,
	DType_F64,
};
//
struct elementData
{
	std::string elem_name;      //枚举值的字符值
	int         elem_value;     //枚举值的逻辑值
};
//
class Cconfig
{
public:
	char       m_ColName[MAXStrLen];	//列名
	DataType   m_DType;                 //数据类型
	int        elem_num;                //一共有几个元素
	std::vector<elementData> m_enumVT;  //枚举类型的内容

	Cconfig();
	~Cconfig();
	void  Init();	
};