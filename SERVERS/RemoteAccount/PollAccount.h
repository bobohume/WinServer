#ifndef _POLL_ACCOUNT_H_
#define _POLL_ACCOUNT_H_

#include "Common\Common.h"
#include <memory.h>
#include "base\locker.h"

#define GAME_PASS9INI_FILE "conf/Pass9.ini"

struct PollInfo
{
	char loginType[COMMON_STRING_LENGTH];
	char name[ACCOUNT_NAME_LENGTH];
	char pass[PASSWORD_LENGTH];
	char userIP[COMMON_STRING_LENGTH];
	int sId;
	int areaId;
	int socket;

	PollInfo()
	{
		Clear();
	}

	PollInfo(const PollInfo& other)
	{
		*this = other;
	}

	void Clear()
	{
		memset(this, 0, sizeof(PollInfo));
	}

	PollInfo& operator=(const PollInfo& other)
	{
		memcpy(this, &other, sizeof(PollInfo));
		return *this;
	}
};

#endif