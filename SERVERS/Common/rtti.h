#ifndef __RTTI_H__
#define __RTTI_H__

#include <string>
#include <sstream>
#include <hash_map>

#define RTTI_OFFSET( x, y ) (int)( (char*)( &(  (y*)(0) )->x  ) ) /* - (char*)( (y*)(0) ) */

#define RTTI_DECLARE_CLASS(p)			static RTTI::ClassRegisterBuilder<p> crb_##p( typeid(p).name() )
#define RTTI_DECLARE_FIELD(p, f, c, t)	static RTTI::ClassFieldBuilder<p,t> cfb_##p##_##f( typeid(p).name() , c, RTTI_OFFSET( f, p ))

namespace RTTI
{
	struct ToStringBase
	{
		virtual std::string operator () (void* value) = 0;
	};

	template< typename _Ty >
	struct ToString	: public ToStringBase
	{
		std::string operator () ( void* value )
		{
			std::stringstream ss;
			ss << *((_Ty*)value);
			return ss.str();
		}
	};

	struct ClassTypeBase
	{
		virtual ~ClassTypeBase() {} 
	};

	template< typename _Ty >
	class ClassType : ClassTypeBase
	{
	public:
		struct FieldStruct
		{
			int offset;
			int size;
		};

		typedef stdext::hash_map< std::string, int > FieldMap;
		typedef stdext::hash_map< std::string, ToStringBase* > ToStringMap;

		~ClassType()
		{
			ToStringMap::iterator it;
			for( it = mToStringMap.begin(); it != mToStringMap.end(); it++ )
			{
				delete it->second;
			}
		}

		template< typename T >
		void addField( std::string fieldName, int offset )
		{
			mFieldMap[fieldName] = offset;
			mToStringMap[fieldName] = new ToString<T>;
		}

		template< typename T >
		void getValue( std::string fieldName, void* pObject, T& value )
		{
			FieldMap::iterator it = mFieldMap.find( fieldName );
			if( it == mFieldMap.end() )
				return ;

			int offset = it->second;

			value = *((T*)( (char*)pObject + offset ));
		}

		std::string getValue( std::string fieldName, void* pObject )
		{
			ToStringMap::iterator it = mToStringMap.find( fieldName );
			if( it == mToStringMap.end() )
				return "";

			ToStringBase& toString = *( it->second );

			FieldMap::iterator itf = mFieldMap.find( fieldName );
			if( itf == mFieldMap.end() )
				return "";

			int offset = itf->second;

			return toString( ((char*)pObject + offset ) );
		}


	private:
		FieldMap mFieldMap;
		ToStringMap mToStringMap;
	};

	template< typename _Ty, typename T >
	void getValue( _Ty* pObject, std::string fieldName, T& value )
	{
		ClassType<_Ty>* pClass = (ClassType<_Ty>*)ClassRegister::getInstance()->getClass( typeid( _Ty ).name() );
		if( pClass )
		{
			pClass->getValue( fieldName, pObject, value );
		}
	}

	template< typename _Ty >
	std::string getStringValue( _Ty* pObject, std::string fieldName )
	{
		ClassType<_Ty>* pClass = (ClassType<_Ty>*)ClassRegister::getInstance()->getClass( typeid( _Ty ).name() );
		if( pClass )
		{
			return pClass->getValue( fieldName, pObject );
		}

		return "";
	}


	typedef stdext::hash_map< std::string, ClassTypeBase* > ClassMap;

	struct ClassRegister
	{
		static ClassRegister* getInstance()
		{
			static ClassRegister instance;
			return &instance;
		}

		template<typename _Ty>
		void registerClass( std::string className )
		{
			ClassMap::iterator it = mClassMap.find( className );
			if( it == mClassMap.end())
			{
				_Ty* p = new _Ty;
				mClassMap[className] = (ClassTypeBase*)p;
			}
		}

		~ClassRegister()
		{
			ClassMap::iterator it;
			for( it = mClassMap.begin(); it != mClassMap.end(); it++ )
			{
				delete it->second;
			}
		}

		void* getClass( std::string className )
		{
			return mClassMap[className];
		}

		ClassMap mClassMap;
	} ;

	template< typename _Ty >
	struct ClassRegisterBuilder
	{
		ClassRegisterBuilder( std::string className )
		{
			RTTI::ClassRegister::getInstance()->registerClass< RTTI::ClassType<_Ty> >( className );
		}
	};

	template< typename _Ty, typename T  >
	struct ClassFieldBuilder
	{
		ClassFieldBuilder( std::string className, std::string fieldName, int offset )
		{
			void* pObj = RTTI::ClassRegister::getInstance()->getClass( className.c_str() ); 

			if( pObj ) 
			{
				(( RTTI::ClassType<_Ty>* )pObj)->addField<T>( fieldName, offset );
			}
		}
	};
}


#endif