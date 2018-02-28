#ifndef KEY_MAP_BASE_H
#define KEY_MAP_BASE_H


/* creates a mask with `n' 1 bits at position `p' */
#define MASK1(n,p)	((~((~(int)0)<<n))<<p)

/* creates a mask with `n' 0 bits at position `p' */
#define MASK0(n,p)	(~MASK1(n,p))

struct stKeyMapBase
{
	struct key
	{
		char keyboard[2]; 
	};

	static stKeyMapBase* create()
	{
		int size = sizeof( stKeyMapBase );
		stKeyMapBase* theNew = (stKeyMapBase*)malloc( size );
		memset( theNew, 0, size );
		return theNew;
	}

    stKeyMapBase()
    {
        keyMapSoultion = 0;
        memset( keyMaps, 0, sizeof(keyMaps) );
    }

	int keyMapSoultion;
	enum
	{
		ChatMode_Pos = 0,
		ChatMode_Size = 1,
		SelfSpell_Pos = ChatMode_Pos + ChatMode_Size,
		SelfSpell_Size = 2,
		Soultion_Pos = SelfSpell_Pos + SelfSpell_Size,
		Soultion_Size = 2,
		BaseType_Pos = Soultion_Pos + Soultion_Size,
		BaseType_Size = 2,
	};

	static const int customKeyCounts = 150;
	
	key keyMaps[customKeyCounts];


	bool isChatMode() 
	{
		return ( keyMapSoultion >> ChatMode_Pos) & MASK1(ChatMode_Size,0);
	}

	int getSelfSpell()
	{
		return ( keyMapSoultion >> SelfSpell_Pos) & MASK1(SelfSpell_Size,0);
	}

	int getSoultion()
	{
		return ( keyMapSoultion >> Soultion_Pos) & MASK1(Soultion_Size,0);
	}
	int getBastType()
	{
		return ( keyMapSoultion >> BaseType_Pos) & MASK1(BaseType_Size,0);
	}

	void setChatMode( bool value )
	{
		keyMapSoultion = (keyMapSoultion & MASK0(ChatMode_Size,ChatMode_Pos)) | ((value << ChatMode_Pos) & MASK1(ChatMode_Size,ChatMode_Pos));
	}

	void setSelfSpell( int value )
	{
		keyMapSoultion = (keyMapSoultion & MASK0(SelfSpell_Size,SelfSpell_Pos)) | ((value << SelfSpell_Pos) & MASK1(SelfSpell_Size,SelfSpell_Pos));
	}

	void setSoultion( int value )
	{
		keyMapSoultion = (keyMapSoultion & MASK0(Soultion_Size,Soultion_Pos)) | ((value << Soultion_Pos) & MASK1(Soultion_Size,Soultion_Pos));
	}
	void setBastType( int value )
	{
		keyMapSoultion = (keyMapSoultion & MASK0(BaseType_Size,BaseType_Pos)) | ((value << BaseType_Pos) & MASK1(BaseType_Size,BaseType_Pos));
	}

	template< typename _Ty >
	void writeData( _Ty* stream )
	{
		stream->writeInt( keyMapSoultion, 32 );

		stream->writeBits( customKeyCounts * sizeof( key ) * 8, keyMaps );
	}

	template< typename _Ty >
	void readData( _Ty* stream )
	{
		keyMapSoultion = stream->readInt( 32 );

		stream->readBits( customKeyCounts * sizeof( key ) * 8, keyMaps );
	}
};


#endif /*KEY_MAP_BASE_H*/