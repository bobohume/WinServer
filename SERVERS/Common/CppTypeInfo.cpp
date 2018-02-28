#include "CppTypeInfo.h"
#include <process.h>
#include "Base\Log.h"

CTI::TypeInfo::TypeInfo( std::string className, CreateObjectFunction function, StaticConstructorFunction staticConstructor ) : mClassName( className ), mCreateFunction( function )
{
	getTypeInfoMap()[className] = this;

	staticConstructor();
}

CTI::TypeInfo::~TypeInfo()
{
	getTypeInfoMap().erase( mClassName );
}

CTI::Object* CTI::TypeInfo::createObject() const
{
	return mCreateFunction();
}

void CTI::TypeInfo::addMember( ClassMember* classMember )
{
	if( classMember == NULL )
		return ;

	mMemberMap[classMember->getName()] = classMember;

	mMemberList.push_back( classMember );
}

CTI::ClassMember* CTI::TypeInfo::getMember( std::string name )
{
	MemberMap::iterator it;
	it = mMemberMap.find( name );
	if( it == mMemberMap.end() )
		return NULL;
	else
		return it->second;
}

CTI::TypeInfo* CTI::TypeInfo::getStaticType( std::string className )
{
	TypeInfoMap::iterator it;

	it = getTypeInfoMap().find( className );
	if( it != getTypeInfoMap().end() )
		return it->second;

	return NULL;
}

CTI::TypeInfo::TypeInfoMap& CTI::TypeInfo::getTypeInfoMap()
{
	static TypeInfoMap mTypeInfoMap;
	return mTypeInfoMap;
}

CTI::TypeInfo::MemberList* CTI::TypeInfo::getMemberList()
{
	return &mMemberList;
}

void CTI::MemoryStream::write( char* buf, int size )
{
	throw std::exception( "Not implemented" );
}

void CTI::MemoryStream::read( char* buf, int size )
{
	throw std::exception( "Not implemented" );
}

CTI::FileStream::FileStream()
{
	fp = NULL;
}

CTI::FileStream::FileStream( std::string fileName, OpenType type )
{
	fp = NULL;
	open( fileName, type );
}

void CTI::FileStream::open( std::string fileName, OpenType type )
{
	try
	{
		if( fp )
			close();

		fopen_s( &fp, fileName.c_str(), type == Write ? "wb+" : "rb" );	

		if(NULL == fp)
			throw std::exception( "FileStream : open : Error on open the file" );
	}
	catch( ... )
	{

	}
}

void CTI::FileStream::close()
{
	if( fp )
		fclose( fp );

	fp = NULL;
}

CTI::FileStream::~FileStream()
{
	close();
}

void CTI::FileStream::write( char* buf, int size )
{
	if( fwrite( buf, 1, size, fp ) != size )
		g_Log.WriteError("FileStream : write : Error on writing ... ");
		//throw std::exception("FileStream : write : Error on writing ... ");
}

void CTI::FileStream::read( char* buf, int size )
{
	int s = fread( buf, 1, size, fp );
	if( s != size )
	{
		memset(buf, 0, size);
		g_Log.WriteError("FileStream : read : Error on reading ... ");
		//long m = ftell( fp );
		//throw std::exception("FileStream : read : Error on reading ... ");
	}
}

bool CTI::FileStream::isReady()
{
	return fp != NULL;
}

CTI::IStream& operator <<( CTI::IStream& stream, std::string& value )
{
	int size = value.size();
	size >> stream;
	stream.write( (char*)value.c_str(), size );
	return stream;
}

CTI::IStream& operator >>( CTI::IStream& stream, std::string& value )
{
	int size;
	size << stream;

	char* buf = new char[size + 1];
	stream.read( buf, size + 1 );

	buf[size] = 0;
	value = buf;
	delete[] buf;

	return stream;
}

template<>
void CTI::serialize<CTI::Object>( CTI::IStream& stream, void* object )
{
	((Object*)object)->serialize( stream );
}

template<>
void CTI::unserialize<CTI::Object>( CTI::IStream& stream, void* object )
{
	((Object*)object)->unserialize( stream );
}

CTI::Thread::Thread()
{
	mThreadHandle = INVALID_HANDLE_VALUE;
	tag = NULL;
}

CTI::Thread::~Thread()
{
	stop();
}

void CTI::Thread::start()
{
	mThreadHandle = (HANDLE)_beginthread( workThread, 0, this );
}

void CTI::Thread::stop()
{
	if( mThreadHandle !=INVALID_HANDLE_VALUE )
	{
		::TerminateThread( mThreadHandle, 0 );
		::WaitForSingleObject( mThreadHandle, 60*1000 );
		mThreadHandle = INVALID_HANDLE_VALUE;
	}
}

void CTI::Thread::wait()
{
	if( mThreadHandle != INVALID_HANDLE_VALUE )
		::WaitForSingleObject( mThreadHandle, -1 );
}

CTI::AsyncFileStream::AsyncFileStream()
{
	init();
}

CTI::AsyncFileStream::AsyncFileStream( std::string fileName, OpenType type )
{
	open( fileName, type );
	init();
}

void CTI::AsyncFileStream::init()
{
	mIoComplatePort = ::CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, NULL, 1 );

	mThread.WorkMethod.hook( this, &CTI::AsyncFileStream::writeWork );
	mThread.start();
}

CTI::AsyncFileStream::~AsyncFileStream()
{
	mThread.wait();
}

void CTI::AsyncFileStream::writeWork( Thread* thread )
{
	WriteWork* pWork;
	DWORD dwLength;
	LPOVERLAPPED* pData = NULL;
	
	while( 1 )
	{
		::GetQueuedCompletionStatus( mIoComplatePort, &dwLength, ( PULONG_PTR )&pWork, (LPOVERLAPPED*)&pData, -1 );

		if( pWork == NULL )
			break;

		__super::write( pWork->buf, pWork->size );

		delete[] pWork->buf;
		delete pWork;
	}
}

void CTI::AsyncFileStream::write( char* buf, int size )
{
	WriteWork* ww = new WriteWork();
	ww->buf = new char[size];
	ww->size = size;
	memcpy( ww->buf, buf, size );

	::PostQueuedCompletionStatus( mIoComplatePort, 0, (ULONG_PTR)ww, (LPOVERLAPPED)ww );
}

void CTI::AsyncFileStream::close()
{
	::PostQueuedCompletionStatus( mIoComplatePort, 0, (ULONG_PTR)NULL, (LPOVERLAPPED)NULL );
	
	mThread.wait();

	FileStream::close();
}
