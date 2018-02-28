#ifndef LOG_SERVER_H
#define LOG_SERVER_H

#include "Common/ServerFramwork.h"

class CLogServer : public CServerFramework<CLogServer>
{
public:
	bool onInit();
	void onMainLoop();
	void onShutdown();
	void _loadParam( t_server_param &param );

    virtual bool IsClosed(void);
};

#endif