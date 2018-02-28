#ifndef GAME_DATA_H
#define GAME_DATA_H

#include <string>
#include <hash_map>
#include <list>
#include "DatBuilder\ConfigStruct.h"

class GameDataClass;

#pragma warning( disable: 4291 )

//---------------------------------------------------------------
// GameData 类型
//---------------------------------------------------------------
struct GameDataPropertyTypes
{
	enum Type{
		TypeNone = 0,

		TypeString = DType_string,

		TypeS8 = DType_S8,
		TypeU8 = DType_U8,

		TypeS16 = DType_S16,
		TypeU16 = DType_U16,

		TypeS32 = DType_S32,
		TypeU32 = DType_U32,

		TypeFloat32 = DType_F32,
		TypeFloat64 = DType_F64,

		TypeEnum8 = DType_enum8,
		TypeEnum16 = DType_enum16,


	} ;
};

/*
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
*/

#ifdef new
#undef new
#endif

//---------------------------------------------------------------
// GameData 属性
//---------------------------------------------------------------
class GameDataProperty
{
public:
	GameDataProperty( int offset, GameDataPropertyTypes::Type type );

	virtual ~GameDataProperty();

	template< typename T, typename cls > T& value( cls* pThis )
	{
		return *((T*)( (__int64)pThis + mOffset ) );
	}

	void* operator new( size_t size, GameDataClass* pClass, const char* name );

	int mOffset;

	GameDataPropertyTypes::Type mType;
};

//---------------------------------------------------------------
// GameData 运行时类
//---------------------------------------------------------------
class GameDataClass
{
public:
	typedef void ( *STATIC_CONSTRUCTOR )();

	GameDataClass( std::string name, STATIC_CONSTRUCTOR staticConstructor );
	virtual ~GameDataClass();

	void addProperty( std::string name, GameDataProperty* pProperty );
	GameDataProperty* getProperty( std::string name );
	GameDataProperty* getProperty( int index );

private:
	typedef stdext::hash_map< std::string, GameDataProperty* > PropertyMap;
	typedef stdext::hash_map< int, GameDataProperty*> PropertyIndexMap;
	typedef std::list< std::string > PropertyNameList;

	PropertyMap mPropertyMap;
	PropertyNameList mPropertyNameList;
	PropertyIndexMap mPropertyIndexMap;

	int mIndex;
};

//---------------------------------------------------------------
// GameData 声明
//---------------------------------------------------------------
#define DECLARE_GAMEDATA_CLASS( c ) \
	private: \
	typedef c ThisClass; \
	public: \
	static GameDataClass* getClass(); \
	static void staticConstructor_##c(); \
	private:

//---------------------------------------------------------------
// GameData 实现
//---------------------------------------------------------------
#define IMPLEMENT_GAMEDATA_CLASS( c ) \
	GameDataClass* c::getClass() { static GameDataClass mDataClass_##c( #c, (GameDataClass::STATIC_CONSTRUCTOR)&c::staticConstructor_##c ); return &mDataClass_##c; } \
	void c::staticConstructor_##c()

#define GAMEDATA_OFFSET(p) ((unsigned int)((const char *)&(((ThisClass *)1)->p)-(const char *)1))

#endif