#ifndef CORPS_BASE_H
#define CORPS_BASE_H

enum CorpsError
{
	CORPSOP_OK,
	CORPSERR_Already,
	CORPSERR_NoTeam,
	CORPSERR_OtherNoTeam,
	CORPSERR_NotTeamLeader,
	CORPSERR_AlreadInCorps,
	CORPSERR_OtherAlreadInCorps,
	CORPSERR_NotInCorps,
	CORPSERR_NotAuthorize,
	CORPSERR_Offline,
	CORPSERR_TeamFull,
	CORPSERR_Reject,
	CORPSERR_TeamNotInCorps
};

static const char* strCorpsError[] =
{
	"成功!",
	"已经",
	"您没有队伍",
	"对方没有队伍",
	"您不是队长",
	"您已经在团队中了",
	"对方已经在团队中了",
	"您不在团队中",
	"您没有权限",
	"",
	"团队中队伍已满",
	"对方拒绝了你的邀请",
	"团队中没有此队伍"
};

#define MAX_CORPS_TEAM_NUM 6

struct ZoneClientCorps
{
	int masterTeam;

	ZoneClientCorps(int master)
		:masterTeam(master)
	{
	}

	ZoneClientCorps()
	{
		memset(this, 0, sizeof(ZoneClientCorps));
	}

	template<typename _Ty>
	void WriteData(_Ty* packet) const
	{
		packet->writeBits(sizeof(ZoneClientCorps)<<3, this);
	}

	template<typename _Ty>
	void ReadData(_Ty* packet)
	{
		packet->readBits(sizeof(ZoneClientCorps)<<3, this);
	}
};

#endif