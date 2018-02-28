#include "GameData.h"

void GameDataClass::addProperty( std::string name, GameDataProperty* pProperty )
{
	// 属性只能添加一次，且后面再次添加属性，不能覆盖之前的
	PropertyMap::iterator it = mPropertyMap.find( name );
	if( it != mPropertyMap.end() )
		return ;

	mPropertyMap[name] = pProperty;
	mPropertyIndexMap[mIndex++] = pProperty;
	mPropertyNameList.push_back( name );
}

GameDataClass::GameDataClass( std::string name, STATIC_CONSTRUCTOR staticConstructor )
{
	mIndex = 0;
	staticConstructor();
}

GameDataClass::~GameDataClass()
{
	for( PropertyMap::iterator it = mPropertyMap.begin(); it != mPropertyMap.end(); it++ )
	{
		//delete it->second;
		free(it->second);
	}
}

GameDataProperty* GameDataClass::getProperty( std::string name )
{
	PropertyMap::iterator it;

	it = mPropertyMap.find( name );
	if( it != mPropertyMap.end() )
		return it->second;

	return NULL;
}

GameDataProperty* GameDataClass::getProperty( int index )
{
	PropertyIndexMap::iterator it;

	it = mPropertyIndexMap.find( index );
	if( it != mPropertyIndexMap.end() )
		return it->second;

	return NULL;
}

void* GameDataProperty::operator new( size_t size, GameDataClass* pClass, const char* name )
{
	GameDataProperty* pProperty = (GameDataProperty*)malloc( size ); 
	pClass->addProperty( name, pProperty ); 
	return pProperty;
}

GameDataProperty::~GameDataProperty()
{

}

GameDataProperty::GameDataProperty( int offset, GameDataPropertyTypes::Type type )
{
	mOffset = offset;
	mType = type;
}