#pragma once

#include "PacketStream.h"
#include "NetSession.h"

struct NETLIB_CLASS t_IPacket
{
	virtual void onPack( CPacketStream& stream ) = 0;
	virtual void onUnpack( CPacketStream& stream ) = 0;

	static const int MAX_PACKET_SIZE = 1024;
};

struct NETLIB_CLASS t_packetHeader
{
	int PacketSize;
	int Id;
	int Message;
	int MessageEx;

	t_packetHeader()
	{
		memset( this, 0, sizeof( t_packetHeader ) );
	}

	static t_packetHeader* buildHeader( CPacketStream& stream, int message, int id = 0, int messageEx = 0 )
	{
		t_packetHeader header;
		header.Id = id;
		header.Message = message;
		header.MessageEx = messageEx;

		char protocol = CNetSession::LOGIC;

		stream << protocol << header;

		return (t_packetHeader*)((char*)stream.getBuffer() + 1);
	}
} ;



