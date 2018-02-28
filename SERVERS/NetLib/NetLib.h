#pragma once

#ifdef NETLIB_INTERNAL
#define NETLIB_CLASS //__declspec( dllimport )
#else

//#ifdef _DEBUG
//
//#pragma comment(lib, "../Debug/NetLib.lib")
//
//#else
//
//#pragma comment(lib, "../Release/NetLib.lib")
//
//#endif

#define NETLIB_CLASS //__declspec( dllexport )

#endif

#include "Delegate.h"
#include "Packet.h"
#include "PacketStream.h"
#include "NetLib_Core.h"
#include "NetSession.h"
#include "NetEventHandler.h"
#include "NetServer.h"
#include "NetClient.h"
#include "StateMachine.h"
#include "Lock.h"
#include "HttpDownloader.h"


