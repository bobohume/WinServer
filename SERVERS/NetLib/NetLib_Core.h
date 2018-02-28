#pragma once

class NETLIB_CLASS CNetLib
{
public:
	static void Init()
    {
        WSAData data;

        int err = WSAStartup( MAKEWORD( 2, 0 ),&data );
    }

	static void Shutdown()
    {
        WSACleanup();
    }
};