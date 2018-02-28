#ifndef CPP_TYPE_INFO
#define CPP_TYPE_INFO

#include <typeinfo>
#include <string>
#include <hash_map>
#include <hash_set>
#include <list>
#include <deque>
#include <vector>

#include <WinSock2.h>
#include <windows.h>

namespace CTI {

	class Object;
	class ClassMember;
	class TypeInfo;
	struct IMember;
	struct IStream;


	class Object
	{
	public:
		virtual TypeInfo* getTypeInfo() = 0;
		virtual void* getMember( std::string name ) = 0;
		virtual std::string toString() { return "Object"; }
		virtual void serialize( IStream& stream ) = 0;
		virtual void unserialize( IStream& stream ) = 0;
	};

	typedef Object* (*CreateObjectFunction)();
	typedef void (*StaticConstructorFunction)();

	typedef void (*SerializeFunction)( IStream& stream, void* object );
	typedef void (*UnSerializeFunction)( IStream& stream, void* object );

	class TypeInfo
	{
	public:
		typedef stdext::hash_map< std::string, TypeInfo* > TypeInfoMap;
		typedef stdext::hash_map< std::string, ClassMember* > MemberMap;
		typedef std::list< ClassMember* > MemberList;

		TypeInfo( std::string className, CreateObjectFunction function, StaticConstructorFunction staticConstructor );

		virtual ~TypeInfo();

		Object* createObject() const;

		void addMember( ClassMember* classMember );

		ClassMember* getMember( std::string name );
		MemberList* getMemberList();

		static TypeInfo* getStaticType( std::string className );

		CreateObjectFunction mCreateFunction;
		std::string mClassName;

		MemberMap mMemberMap;
		MemberList mMemberList;

		static TypeInfoMap& getTypeInfoMap();
	};

	template< typename _Ty, typename _Parent = Object >
	class ClassTypeBase : public _Parent
	{
	protected:
		friend class TypeInfo;
	public:
		template<class T> 
		T* getMember(std::string name)
		{
			ClassMember* member = getTypeInfo()->getMember(name);
			if (member != NULL)
				return (T*)member->get(this);

			return NULL;
		}

		void* getMember( std::string name )
		{
			ClassMember* member = getTypeInfo()->getMember( name );
			if( member != NULL )
				return member->get( this );

			return NULL;
		}

		void serializeMember( std::string name, IStream& stream )
		{
			ClassMember* member = getTypeInfo()->getMember( name );
			if( member == NULL )
				return ;

			member->getSerializeFunction()( stream, member->get( this ) );
		}

		void unserializeMember( std::string name, IStream& stream )
		{
			ClassMember* member = getTypeInfo()->getMember( name );
			if( member == NULL )
				return ;

			member->getUnSerializeFunction()( stream, member->get( this ) );
		}

		void serialize( IStream& stream )
		{
			CTI::TypeInfo::MemberList::iterator it;

			for( it = getTypeInfo()->getMemberList()->begin(); it != getTypeInfo()->getMemberList()->end(); it++ )
			{
				ClassMember* member = *it;

				member->getSerializeFunction()( stream, member->get( this ) );
			}
		}

		void operator << ( IStream& stream )
		{
			unserialize( stream );
		}

		void operator >> ( IStream& stream )
		{
			serialize( stream );
		}

		void unserialize( IStream& stream )
		{
			CTI::TypeInfo::MemberList::iterator it;

			for( it = getTypeInfo()->getMemberList()->begin(); it != getTypeInfo()->getMemberList()->end(); it++ )
			{
				ClassMember* member = *it;

				member->getUnSerializeFunction()( stream, member->get( this ) );
			}
		}

		std::string toString()
		{
			return typeid( _Ty ).name();
		}
	};


	template< typename _Ty, typename _Parent = Object >
	class ClassType : public ClassTypeBase< _Ty, _Parent >
	{
	public:
		TypeInfo* getTypeInfo()
		{
			return getStaticType();
		}

		static TypeInfo* getStaticType()
		{
			return &mClassInfo;
		}

		typedef _Ty ThisClass;
		typedef _Parent Parent;

	private:
		static TypeInfo mClassInfo;

	private:
		static void _$class()
		{
			__if_exists( _Ty::$class )
			{
				_Ty::$class();
			}
		}
	};

	template< typename _Ty, typename _Parent > TypeInfo ClassType<_Ty, _Parent>::mClassInfo( typeid( _Ty ).name(), _createObject<_Ty>, ClassType<_Ty, _Parent>::_$class );

	template< typename _Ty, typename _Parent = Object >
	class AbstractClassType : public ClassTypeBase< _Ty, _Parent >
	{
	public:
		TypeInfo* getTypeInfo()
		{
			return getStaticType();
		}

		static TypeInfo* getStaticType()
		{
			return &mClassInfo;
		}
	private:
		static TypeInfo mClassInfo;

	private:
		static void _$class()
		{
			__if_exists( _Ty::$class )
			{
				_Ty::$class();
			}
		}
	};

	template< typename _Ty, typename _Parent > TypeInfo AbstractClassType<_Ty, _Parent>::mClassInfo( typeid( _Ty ).name(), NULL, AbstractClassType<_Ty, _Parent>::_$class );

	struct IStream : public AbstractClassType< IStream >
	{
		virtual void write( char* buf, int size ) = 0;
		virtual void read( char* buf, int size ) = 0;
	};

	class ClassMember : public AbstractClassType< ClassMember >
	{
		friend class TypeInfo;
	public:
		ClassMember( TypeInfo* typeInfo, std::string name, int offset, SerializeFunction serializeFunct, UnSerializeFunction unserializeFunct ) : mName(name), mOffset( offset ), mSerializeFunction( serializeFunct ), mUnSerializeFunction( unserializeFunct ) { typeInfo->addMember( this ); }
		virtual ~ClassMember() {}

		std::string getName() { return mName; }
		int getOffset() { return mOffset; }
		SerializeFunction getSerializeFunction() { return mSerializeFunction; }
		UnSerializeFunction getUnSerializeFunction() { return mUnSerializeFunction; }

		void* get( void* pThis ) { return (void*)((char*)pThis + mOffset ); }
	private:
		SerializeFunction mSerializeFunction;
		UnSerializeFunction mUnSerializeFunction;
		std::string mName;
		int mOffset;
	} ;

	template< typename _Ty >
	Object* _createObject()
	{
		return new _Ty();
	}

	struct MemoryStream : public ClassType< MemoryStream, IStream >
	{
		void write( char* buf, int size );

		void read( char* buf, int size );
	};

#define MEMBER_OFFSET(p,cls) ((size_t)( &(((cls*)0)->p) ))

	class FileStream : public ClassType< FileStream, IStream >
	{
		FILE* fp;
	public:
		enum OpenType
		{
			Write,
			Read,
		};

		FileStream(); 

		FileStream( std::string fileName, OpenType type );

		void open( std::string fileName, OpenType type );

		bool isReady();

		virtual void close();

		virtual ~FileStream();

		void write( char* buf, int size );

		void read( char* buf, int size );
	};

	

	/*template< typename _Ty >
	IStream& operator << ( _Ty& value, IStream& stream )
	{
		value.unserialize( stream );
		return stream;
	}

	template< typename _Ty >
	IStream& operator >> ( _Ty& value, IStream& stream )
	{
		value.serialize( stream );
		return stream;
	}*/

	template<class _Type>
	static IStream& operator << (_Type& value, IStream& stream)
	{ 
		stream.read((char*)&value, sizeof(_Type)); 
		return stream; 
	} 

	template<class _Type>
	static IStream& operator >> (_Type& value, IStream& stream)
	{
		stream.write((char*)&value, sizeof(_Type)); 
		return stream; 
	}

#define DECLARE_SERIALIZE_METHOD( _Type ) \
	template<> \
	static IStream& operator << ( _Type& value, IStream& stream ) \
	{ \
	stream.read( (char*)&value, sizeof( _Type ) ); \
	return stream; \
	} \
	template<> \
	static IStream& operator >> ( _Type& value, IStream& stream )\
	{\
	stream.write( (char*)&value, sizeof( _Type ) );\
	return stream;\
	}

	DECLARE_SERIALIZE_METHOD(bool);
	DECLARE_SERIALIZE_METHOD(int);
	DECLARE_SERIALIZE_METHOD(char);
	DECLARE_SERIALIZE_METHOD(long);
	DECLARE_SERIALIZE_METHOD(short);
	DECLARE_SERIALIZE_METHOD(__int64);
	DECLARE_SERIALIZE_METHOD(float);
	DECLARE_SERIALIZE_METHOD(double);
	DECLARE_SERIALIZE_METHOD(unsigned int);
	DECLARE_SERIALIZE_METHOD(unsigned char);
	DECLARE_SERIALIZE_METHOD(unsigned long);
	DECLARE_SERIALIZE_METHOD(unsigned short);
	DECLARE_SERIALIZE_METHOD(unsigned __int64);

	template<>
	static IStream& operator << <std::string>( std::string& value, IStream& stream )
	{
		size_t size;
		size << stream;
		char* buf = new char[size + 1];
		stream.read( buf, (int)size );
		buf[size] = 0;
		value = buf;	
		delete[] buf;
		return stream;
	}

	template<>
	static IStream& operator >> <std::string>( std::string& value, IStream& stream )
	{
		size_t size = value.length();
		size >> stream;
		char* buf = (char*)( value.c_str() );
		stream.write( buf, (int)size );
		return stream;
	}

	template< int count >
	IStream& operator << ( char (&value)[count], IStream& stream )
	{
		stream.read( value, count );

		return stream;
	}

	template< int count >
	IStream& operator >> ( char (&value)[count], IStream& stream )
	{
		stream.write( value, count );
		return stream;
	}

	template< typename _Ty >
	IStream& operator << ( std::deque<_Ty>& value, IStream& stream )
	{
		int size;
		size << stream;

		for( int i = 0; i < size; i++ )
		{
			_Ty v;
			v << stream;
			value.push_back( v );
		}

		return stream;
	}

	template< typename _Ty >
	IStream& operator >> ( std::deque<_Ty>& value, IStream& stream  )
	{
		int size = value.size();

		size >> stream;
		std::deque<_Ty>::iterator it;
		for( it = value.begin(); it != value.end(); it++ )
			*it >> stream;

		return stream;
	}

	template< typename _Ty >
	IStream& operator >> ( stdext::hash_set<_Ty>& value, IStream& stream  )
	{
		size_t size = value.size();

		size >> stream;
		stdext::hash_set<_Ty>::iterator it;
		for( it = value.begin(); it != value.end(); it++ )
			*it >> stream;

		return stream;
	}

	template< typename _Ty >
	IStream& operator << ( stdext::hash_set<_Ty>& value, IStream& stream )
	{
		int size;
		size << stream;

		for( int i = 0; i < size; i++ )
		{
			_Ty v;
			v << stream;
			value.insert( v );
		}

		return stream;
	}

	template< typename _Key, typename _Ty >
	IStream& operator >> (  stdext::hash_map<_Key,_Ty>& value, IStream& stream )
	{
		size_t size = value.size();
		size >> stream;

		stdext::hash_map<_Key,_Ty>::iterator it;

		for( it = value.begin(); it != value.end(); it++ )
		{
			((_Key)it->first) >> stream;
			((_Ty)it->second) >> stream;
		}

		return stream;
	}

	template< typename _Key, typename _Ty >
	IStream& operator << ( stdext::hash_map<_Key,_Ty>& value, IStream& stream )
	{
		int size;
		size << stream;

		for( int i = 0; i < size; i++ )
		{
			_Key k;
			_Ty v;
			k << stream;
			v << stream;
			value[k] = v;
		}

		return stream;
	}

	template< typename _Ty >
	IStream& operator >> (  std::list<_Ty>& value, IStream& stream )
	{
		size_t size = value.size();

		size >> stream;
		std::list<_Ty>::iterator it;
		for( it = value.begin(); it != value.end(); it++ )
			*it >> stream;

		return stream;
	}

	template< typename _Ty >
	IStream& operator << ( std::list<_Ty>& value, IStream& stream )
	{
		int size;
		size << stream;

		for( int i = 0; i < size; i++ )
		{
			_Ty v;
			v << stream;
			value.push_back( v );
		}

		return stream;
	}

	template< typename _Ty >
	IStream& operator >> (  std::vector<_Ty>& value, IStream& stream )
	{
		size_t size = value.size();

		size >> stream;
		std::vector<_Ty>::iterator it;
		for( it = value.begin(); it != value.end(); it++ )
			*it >> stream;

		return stream;
	}

	template< typename _Ty >
	IStream& operator << ( std::vector<_Ty>& value, IStream& stream )
	{
		int size;
		size << stream;

		for( int i = 0; i < size; i++ )
		{
			_Ty v;
			v << stream;
			value.push_back( v );
		}

		return stream;
	}


	template< typename _Ty >
	void serialize( IStream& stream, void* object )
	{
		( *(_Ty*)(object) ) >> stream  ;
	}

	template< typename _Ty >
	void unserialize( IStream& stream, void* object )
	{
		( *(_Ty*)(object) ) << stream;
	}

	

	struct _EmptyType {} ;

	template< typename _Ret, typename _A = _EmptyType, typename _B = _EmptyType, typename _C = _EmptyType, typename _D = _EmptyType, typename _E = _EmptyType >
	class Delegate : AbstractClassType< Delegate<_Ret,_A,_B,_C,_D, _E> >
	{
	private:
		template<typename __A, typename __B, typename __C, typename __D, typename __E > 
		struct ICallable
		{
			virtual _Ret operator () (__A& a, __B& b, __C& c, __D& d, __E& e ) = 0;
		};

		template<typename __A, typename __B, typename __C, typename __D > 
		struct ICallable<__A, __B, __C, __D, _EmptyType >
		{
			virtual _Ret operator () (__A& a, __B& b, __C& c, __D& d ) = 0;
		};

		template<typename __A, typename __B, typename __C >
		struct ICallable<__A, __B, __C, _EmptyType, _EmptyType >
		{
			virtual _Ret operator () (__A& a, __B& b, __C& c ) = 0;
		};

		template< typename __A, typename __B >
		struct ICallable<__A, __B, _EmptyType, _EmptyType, _EmptyType >
		{
			virtual _Ret operator () (__A& a, __B& b ) = 0;
		};

		template<typename __A >
		struct ICallable<__A, _EmptyType, _EmptyType, _EmptyType, _EmptyType >
		{
			virtual _Ret operator () (__A& a ) = 0;
		};

		template<>
		struct ICallable<_EmptyType, _EmptyType, _EmptyType, _EmptyType, _EmptyType >
		{
			virtual _Ret operator () () = 0;
		};



		template< typename _Cls, typename _Funct >
		class MethodCall : public ICallable<_A, _B, _C, _D, _E >
		{
			_Cls* mThis;
			_Funct mFunct;
		public:
			MethodCall( _Cls* pThis, _Funct funct ) : mThis( pThis ), mFunct( funct ) { }

			_Ret operator () ()	{ return (mThis->*mFunct)(); }
			_Ret operator () ( _A& a )	{return (mThis->*mFunct)( a );}
			_Ret operator () ( _A& a, _B& b )	{	return (mThis->*mFunct)( a, b ); }
			_Ret operator () ( _A& a, _B& b, _C& c ) {	return (mThis->*mFunct)( a, b, c );	}
			_Ret operator () ( _A& a, _B& b, _C& c, _D& d ) {	return (mThis->*mFunct)( a, b, c, d );	}
			_Ret operator () ( _A& a, _B& b, _C& c, _D& d, _E& e ) {	return (mThis->*mFunct)( a, b, c, d, e );	}
		};

		template< typename _Funct >
		class FunctionCall : public ICallable<_A,_B,_C,_D,_E>
		{
			_Funct mFunct;
		public:
			FunctionCall( _Funct funct ) : mFunct( funct ) {};

			_Ret operator () ()	{ return mFunct(); }
			_Ret operator () ( _A& a )	{return mFunct( a );}
			_Ret operator () ( _A& a, _B& b )	{	return mFunct( a, b ); }
			_Ret operator () ( _A& a, _B& b, _C& c ) {	return mFunct( a, b, c );	}
			_Ret operator () ( _A& a, _B& b, _C& c, _D& d ) {	return mFunct( a, b, c, d );	}
			_Ret operator () ( _A& a, _B& b, _C& c, _D& d, _E& e ) {	return mFunct( a, b, c, d, e );	}

		};

		ICallable<_A,_B,_C,_D,_E>* mFunction;

	public:

		Delegate()
		{
			mFunction = NULL;
		}

		~Delegate()
		{
			if( mFunction != NULL )
				delete mFunction;
		}

		bool isValidable()
		{
			return mFunction != NULL;
		}

		template< typename __Cls, typename __Funct >
		void hook ( __Cls* pThis, __Funct funct )
		{
			if( mFunction != NULL )
				delete mFunction;

			mFunction = new MethodCall<__Cls, __Funct>( pThis, funct );
		}

		template< typename __Funct >
		void hook( __Funct funct )
		{
			if( mFunction != NULL )
				delete mFunction;

			mFunction = new FunctionCall<__Funct>( funct );
		}

		void unhook()
		{
			if( mFunction != NULL )
				delete mFunction;
		}

#define delegate_check_function() if( mFunction == NULL ) return _Ret();

		_Ret operator () () { delegate_check_function(); return (*mFunction)();}
		_Ret operator () ( _A& a ) {delegate_check_function(); return (*mFunction)( a );}
		_Ret operator () ( _A& a, _B& b ){	delegate_check_function(); return (*mFunction)( a, b );}
		_Ret operator () (_A& a, _B& b, _C& c ){delegate_check_function(); return (*mFunction)( a, b, c );}
		_Ret operator () (_A& a, _B& b, _C& c, _D& d )	{	delegate_check_function(); return (*mFunction)( a, b, c, d );	}
		_Ret operator () (_A& a, _B& b, _C& c, _D& d, _E& e )	{	delegate_check_function(); return (*mFunction)( a, b, c, d, e );	}	
	};


	class Thread : public ClassType< Thread >
	{
		HANDLE mThreadHandle;

	public:
		Delegate<void, Thread* > WorkMethod;

		Thread();
		virtual ~Thread();

		void start();
		void stop();
		void wait();

		HANDLE getHandle() { return mThreadHandle; }

		void* tag;

	private:
		static void workThread( void* param )
		{
			Thread* pThis = static_cast< Thread* >( param );

			if( pThis->WorkMethod.isValidable() )
				pThis->WorkMethod( pThis );
		}
	};

	class AsyncFileStream : public ClassType< AsyncFileStream, FileStream >
	{
		HANDLE mIoComplatePort;
		Thread mThread;

		struct WriteWork
		{
			char* buf;
			int size;
		};
	public:

		AsyncFileStream();

		AsyncFileStream( std::string fileName, OpenType type );

		virtual ~AsyncFileStream();

		void write( char* buf, int size );

		void close();
	private:
		void init();
		void writeWork( Thread* thread );
	};
}

#define member(_Type,Name) _Type Name; \
	class MemberPointerTo_##Name : CTI::AbstractClassType< MemberPointerTo_##Name > { public: static void $class() { static CTI::ClassMember _member( _Type::ThisClass::getStaticType(), #Name, MEMBER_OFFSET(Name, _Type::ThisClass), CTI::serialize<_Type>, CTI::unserialize<_Type> ); } };

//#define member_array(_Type,Name, Length) _Type Name[Length]; \
	class MemberPointerTo_##Name : CTI::AbstractClassType< MemberPointerTo_##Name > { public: static void $class() { static CTI::ClassMember _member( _Type::ThisClass::getStaticType(), #Name, MEMBER_OFFSET(Name, _Type::ThisClass), CTI::serialize<_Type>, CTI::unserialize<_Type> ); } };

#define member_array(_Type,Name,Index) _Type Name##Index; \
	class MemberPointerTo_##Name##Index : CTI::AbstractClassType< MemberPointerTo_##Name##Index > { public: static void $class() { static CTI::ClassMember _member( _Type::ThisClass::getStaticType(), #Name###Index , MEMBER_OFFSET(Name[Index], _Type::ThisClass), CTI::serialize<_Type>, CTI::unserialize<_Type> ); } };

#define method(_Ret,Name, ... ) \
	_Ret Name( __VA_ARGS__ ); \
	CTI::Delegate<_Ret, __VA_ARGS__ > MethodTo_##Name;

#endif /*CPP_TYPE_INFO*/