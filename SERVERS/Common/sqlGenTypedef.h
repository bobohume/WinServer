#ifndef sqlGenTypedef_H__
#define sqlGenTypedef_H__
#include <map>

enum DBDataType
{
	DBDataType_char_array,
	DBDataType_S8	,
	DBDataType_U8	,
	DBDataType_S16	,
	DBDataType_U16	,
	DBDataType_S32	,
	DBDataType_U32	,
	//DBDataType_S64	,
	DBDataType_U64	,
	DBDataType_bool	,
	DBDataType_F32 ,
	DBDataType_SecondPassword,
	DBDataType_PlayerDataFlag1,
	DBDataType_PlayerDataFlag2
};

typedef std::map<int,std::string> tMapOffset2ColumnName;
typedef std::map<int,DBDataType> tMapOffset2DataType;

#define _DECALRE_INITDB static void InitDBStruct(tMapOffset2ColumnName& m_mapOffset2ColumnName, tMapOffset2DataType& m_mapOffset2DataType, int nBaseOffset =0)

#define REGEDIT_DB_COLUMN(StructType/*主类型名*/, MemberaType/*成员类型名*/, MemberName/*成员名*/, ColumnNameStr/*数据库列名*/)\
{\
	MemberaType StructType::* p = &StructType::MemberName;\
	m_mapOffset2ColumnName.insert(std::make_pair ((int&)p+nBaseOffset,#ColumnNameStr));\
	m_mapOffset2DataType.insert(std::make_pair   ((int&)p+nBaseOffset, DBDataType_##MemberaType));\
}

//将char a[5]这样的结构映射到一个数据库column上
#define REGEDIT_DB_COLUMN_ARRAY(StructType/*主类型名*/, MemberaType/*成员类型名*/, MemberName/*成员名*/, nArrayLength/*数组长度*/ ,ColumnNameStr/*数据库列名*/){\
	MemberaType (StructType::* p )[nArrayLength]= &StructType::MemberName;\
	m_mapOffset2ColumnName.insert(std::make_pair ((int&)p+nBaseOffset,#ColumnNameStr));\
	m_mapOffset2DataType.insert(std::make_pair   ((int&)p+nBaseOffset, DBDataType_##MemberaType##_array));}



//映射一个二进制char字符串
#define REGEDIT_DB_COLUMN_SecondPassword(StructType/*主类型名*/, MemberaType/*成员类型名*/, MemberName/*成员名*/, nArrayLength/*数组长度*/ ,ColumnNameStr/*数据库列名*/){\
	MemberaType (StructType::* p )[nArrayLength]= &StructType::MemberName;\
	m_mapOffset2ColumnName.insert(std::make_pair ((int&)p+nBaseOffset,#ColumnNameStr));\
	m_mapOffset2DataType.insert(std::make_pair   ((int&)p+nBaseOffset, DBDataType_SecondPassword));}






//将 a[5]这样的结构分为 aname0, aname1, aname2,aname3这样的结构存储
#define REGEDIT_DB_COLUMN_ARRAY_SLICE(StructType/*主类型名*/, MemberaType/*成员类型名*/, MemberName/*成员名*/, nArrayLength/*数组长度*/ ,ColumnNameStr/*数据库列名*/){\
	MemberaType (StructType::* p )[nArrayLength]= &StructType::MemberName;\
	for(int i=0; i<nArrayLength; i++){\
	std::stringstream ss ; ss<<#ColumnNameStr<<i;\
	m_mapOffset2ColumnName.insert(std::make_pair ((int&)p+i*sizeof(MemberaType)+nBaseOffset,ss.str()));\
	m_mapOffset2DataType.insert(std::make_pair   ((int&)p+i*sizeof(MemberaType)+nBaseOffset, DBDataType_##MemberaType));\
	}}

enum End_Condition //结束条件
{
	keynum_is_zero, //数组元素key值为0
	array_endl		//数组结束
};

enum Index_Sensitive_Status //下标是否敏感
{
	index_sensitive, //下标敏感
	index_non_sensitive //下标不敏感
};

#endif