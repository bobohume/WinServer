#pragma once

#include <string>

class NETLIB_CLASS CPacketStream
{
    static const int MaxByteCount = 512 * 512;
public:
    CPacketStream( char* pBuffer, int byteCount )
    {
        mBuffer = pBuffer;
        mByteCount = byteCount;
        mIsDeleteBufer = false;
        mPostion = 0;
    }

	CPacketStream( int byteCount = MaxByteCount )
    {
        mBuffer = mStackBuf;
        mIsDeleteBufer = false;
        mByteCount = byteCount;
        mPostion = 0;
    }

	~CPacketStream(void)
    {
        if( mIsDeleteBufer )
            delete[] mBuffer;
    }
public:
	void writeBytes( char* pBuf, int length )
    {
        // check overflow
        if( mByteCount - mPostion < length )
            return ;

        memcpy( mBuffer + mPostion, pBuf, length );
        mPostion += length;
    }

	void readBytes( char* pBuf, int length )
    {
        // check overflow
        if( mByteCount - mPostion < length )
            return ;

        memcpy( pBuf, mBuffer + mPostion, length );
        mPostion += length;
    }

	int getPosition()
    {
        return mPostion;
    }

	void setPosition( int position )
    {
        // check overflow
        if( position >= mByteCount || position < 0 )
            return ;

        mPostion = position;
    }

	void* getBuffer()
    {
        return mBuffer;
    }

	template< typename _Ty >
	CPacketStream& operator << ( _Ty& value )
	{
		writeBytes( (char*)&value, sizeof(_Ty) );
		return *this;
	}

	template< typename _Ty >
	CPacketStream& operator >> ( _Ty& value )
	{
		readBytes( (char*)&value, sizeof(_Ty) );
		return *this;
	}

	CPacketStream& operator << ( std::string& str )
    {
        int length = str.length();

        /*if( length > 255 )
        length = 255;*/

        //unsigned char size = (unsigned char)length;

        writeBytes( (char*)&length, 4 );

        if (0 != length)
            writeBytes( (char*)str.c_str(), length );	

        return *this;
    }

	CPacketStream& operator >> ( std::string& str )
    {
        //char buf[256] = { 0 };

        int size = 0;
        readBytes( (char*)&size, 4 );

        if (0 != size)
        {
            char* pBuf = new char[size + 1];
            readBytes( pBuf, size );
            pBuf[size]= 0;
            str = pBuf;

            delete []pBuf;
        }

        return *this;
    }

private:
	char*		mBuffer;
    char        mStackBuf[MaxByteCount];
	bool		mIsDeleteBufer;
	int			mByteCount;
	int			mPostion;
};


