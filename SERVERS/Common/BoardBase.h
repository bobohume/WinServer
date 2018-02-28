#ifndef BOARD_BASE_H
#define BOARD_BASE_H


#include <list>


enum eBoardMsgType
{
	BoardType_None = 0,
	BoardType_Marrige,			// 征婚
	BoardType_Prentice,			// 招徒
	BoardType_Proficient,		// 征贤

	BoardType_MPMessage,		// 师徒公告
	BoardType_Count,
};

struct stBoardMsg
{
	static const int MAX_TITLE_LENGTH = 64;
	static const int MAX_MSG_LENGTH = 128;

	int id;

	int playerId;
	
	int type;
	int family;
	int levelHigh;
	int levelLow;
	int gender;

	char title[MAX_TITLE_LENGTH];
	char msg[MAX_MSG_LENGTH];

    int postTime;

	int time;

	int hot;			// 人气
	int hot1;			// 关注 3
	int hot2;			// 路过 1

	stBoardMsg()
	{
		memset( this, 0, sizeof( stBoardMsg ) );
	}

	stBoardMsg( const stBoardMsg& other )
	{
		copy( other );
	}

	stBoardMsg& operator = ( const stBoardMsg& other )
	{
		copy( other );

		return *this;
	}

	void copy( const stBoardMsg& other )
	{
		id = other.id;
		playerId = other.playerId;
		type = other.type;
		family = other.family;
		levelHigh = other.levelHigh;
		levelLow = other.levelLow;
		gender = other.gender;
		time = other.time;
		hot = other.hot;
		hot1 = other.hot1;
		hot2 = other.hot2;
        postTime = other.postTime;

		strcpy_s( title, sizeof( title ), other.title );
		strcpy_s( msg, sizeof( msg ), other.msg );
	}
	
	template< typename _Ty >
	void readData( _Ty* packet )
	{
		id = packet->readInt( 32 );
		playerId = packet->readInt( 32 );
		type = packet->readInt( 8 );
		family = packet->readInt( 4 );
		gender = packet->readInt( 2 );
		levelHigh = packet->readInt( 8 );
		levelLow = packet->readInt( 8 );
		time = packet->readInt( 4 );
		hot = packet->readInt( 16 );
		hot1 = packet->readInt( 16 );
		hot2 = packet->readInt( 16 );
        postTime = packet->readInt( 32 );

		packet->readString( title, sizeof( title ) );
		packet->readString( msg, sizeof( msg ) );
	}

	template< typename _Ty >
	void writeData( _Ty* packet )
	{
		packet->writeInt( id, 32 );
		packet->writeInt( playerId, 32 );
		packet->writeInt( type, 8 );
		packet->writeInt( family, 4 );
		packet->writeInt( gender, 2 );
		packet->writeInt( levelHigh, 8 );
		packet->writeInt( levelLow, 8 );
		packet->writeInt( time, 4 );
		packet->writeInt( hot, 16 );
		packet->writeInt( hot1, 16 );
		packet->writeInt( hot2, 16 );
        packet->writeInt( postTime, 32 );

		packet->writeString( title );
		packet->writeString( msg );
	}

};



#endif