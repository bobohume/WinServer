#ifndef _REG_COMMON_H
#define _REG_COMMON_H

struct RegError
{
	enum Types //: int
	{
		Unknown = -500,

		AlreadyExisted,			// 帐号已存在
		WrongFormat,			// 错误的格式
		InvaildIdCode,			// 错误的验证码

		Fail,
		Ok = 1,
	} ;
};

struct stActiveParam
{
	char msgId[256];
	char result[256];
	char sequence[256];
	char userName[256];
	char pass[256];
	char birthday[256];
	char gender[256];
	char regDT[256];	

	stActiveParam()
	{
		memset( this, 0, sizeof( stActiveParam ) );
	}

	template< typename _Ty >
	void WriteData( _Ty* packet )
	{
		packet->writeString( msgId, sizeof( msgId ) );
		packet->writeString( result, sizeof( result ) );
		packet->writeString( sequence, sizeof( sequence ) );
		packet->writeString( userName, sizeof( userName ) );
		packet->writeString( pass, sizeof( pass ) );
		packet->writeString( birthday, sizeof( birthday ) );
		packet->writeString( gender, sizeof( gender ) );
		packet->writeString( regDT, sizeof( regDT ) );
	}

	template< typename _Ty >
	void ReadData( _Ty* packet )
	{
		packet->readString( msgId, sizeof( msgId ) );
		packet->readString( result, sizeof( result ) );
		packet->readString( sequence, sizeof( sequence ) );
		packet->readString( userName, sizeof( userName ) );
		packet->readString( pass, sizeof( pass ) );
		packet->readString( birthday, sizeof( birthday ) );
		packet->readString( gender, sizeof( gender ) );
		packet->readString( regDT, sizeof( regDT ) );

	}
};

struct stRegister
{
	enum Types
	{
		Unknown = -1,
		Normal,
		Mail,
		Cellphone,

		Count
	} ;

	Types mRegType;

	int mIdCode;
	char mIdCodeText[32];

	char mRegName[128];
	char mRegPass[128];

	char mRealName[32];
	char mId[64];
	char email[256];

	stRegister()
	{
		memset( this, 0, sizeof(stRegister) );

		mRegType = Unknown;
	}

	stRegister( const stRegister& other )
	{
		copy( other );
	}

	stRegister& operator = ( const stRegister& other )
	{
		copy( other );

		return *this;
	}

	void copy( const stRegister& other )
	{
		mRegType = other.mRegType;
		mIdCode = other.mIdCode;

		strcpy_s( mRegName, sizeof( mRegName ), other.mRegName );
		strcpy_s( mRegPass, sizeof( mRegPass ), other.mRegPass );
		strcpy_s( mRealName, sizeof( mRealName ), other.mRealName );
		strcpy_s( mId, sizeof( mId ), other.mId );
		strcpy_s( mIdCodeText, sizeof( mIdCodeText ), other.mIdCodeText );
		strcpy_s( email, sizeof( email ), other.email );
	}

	template< typename _Ty >
	void WriteData( _Ty* packet )
	{
		packet->writeInt( mRegType, 8 );
		packet->writeString( mRegName, sizeof( mRegName ) );
		packet->writeString( mRegPass, sizeof( mRegPass ) );
		packet->writeString( mRealName, sizeof( mRealName ) );
		packet->writeString( mId, sizeof( mId ) );
		packet->writeString( mIdCodeText, sizeof( mIdCodeText ) );

		packet->writeInt( mIdCode, 32 );
		packet->writeString( email, sizeof( email ) );
	}

	template< typename _Ty >
	void ReadData( _Ty* packet )

	{
		mRegType = static_cast< Types >( packet->readInt( 8 ) );

		packet->readString( mRegName, sizeof( mRegName ) );
		packet->readString( mRegPass, sizeof( mRegPass ) );
		packet->readString( mRealName, sizeof( mRealName ) );
		packet->readString( mId, sizeof( mId ) );
		packet->readString( mIdCodeText, sizeof( mIdCodeText ) );

		mIdCode = packet->readInt( 32 );
		packet->readString( email, sizeof( email ) );
	}
};

#endif 