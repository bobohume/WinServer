#include "BanList.h"
#include "DBLib/dbLib.h"
#include "WorldServer.h"
#include <time.h>
#include "DBLayer\Data\TBLBanList.h"
#include "ServerMgr.h"
#include "wintcp/dtServerSocket.h"


bool CBanlist::Init(void)
{
    OLD_DO_LOCK(m_cs);

	TBLBanList tb_ban(0);
    tb_ban.AttachConn(CGMRecver::Instance()->GetDBConn());

	int curTime = int(time(0));

	DBError err = tb_ban.DeleteForOverTime(curTime);

	if(err != DBERR_NONE)
		return false;

	std::vector<TBLBanList::stBan> banList;
	err = tb_ban.Load(banList);

	BanInfo banInfo;

	for(size_t i = 0; i < banList.size(); i++)
	{
		banInfo.name           = __TranslateName(banList[i].target);
		int type               = banList[i].type;
		banInfo.loginBanExpire = banList[i].loginBan;
		banInfo.chatBanExpire  = banList[i].chatBan;
		banInfo.loginReason    = banList[i].loginBanReason;
		banInfo.chatReason     = banList[i].chatBanReason;

		if (type >= TARGET_TYPE_COUNT)
			continue;

		switch(type)
		{
		case TARGET_TYPE_ACC:
			{   
				m_accBans.insert(std::make_pair(banInfo.name,banInfo));
			}
			break;
		case TARGET_TYPE_ACTOR:
			{
				m_actorBans.insert(std::make_pair(banInfo.name,banInfo));
			}
			break;
		case TARGET_TYPE_IP:
			{
				m_ipBans.insert(std::make_pair(banInfo.name,banInfo));
			}
			break;
		case TARGET_TYPE_MAC:
			{
				m_macBans.insert(std::make_pair(banInfo.name, banInfo));
			}
			break;
		}
	}

    return true;
}

void CBanlist::BanIpLogin(const char* name,int lastTime,const char* reason)
{
    if (0 == name)
        return;

    __DoBan(m_ipBans,TARGET_TYPE_IP,name,BAN_TYPE_LOGIN,lastTime,reason);
}

void CBanlist::BanIpChat(const char* name,int lastTime,const char* reason)
{
    if (0 == name)
        return;

    __DoBan(m_ipBans,TARGET_TYPE_IP,name,BAN_TYPE_CHAT,lastTime,reason);
}

void CBanlist::BanAccLogin(const char* name,int lastTime,const char* reason)
{
    if (0 == name)
        return;

    __DoBan(m_accBans,TARGET_TYPE_ACC,name,BAN_TYPE_LOGIN,lastTime,reason);
}

void CBanlist::BanAccChat(const char* name,int lastTime,const char* reason)
{
    if (0 == name)
        return;

    __DoBan(m_accBans,TARGET_TYPE_ACC,name,BAN_TYPE_CHAT,lastTime,reason);
}

void CBanlist::BanActorLogin(const char* name,int lastTime,const char* reason)
{
    if (0 == name)
        return;

    __DoBan(m_actorBans,TARGET_TYPE_ACTOR,name,BAN_TYPE_LOGIN,lastTime,reason);
}

void CBanlist::BanActorChat(const char* name,int lastTime,const char* reason)
{
    if (0 == name)
        return;

    __DoBan(m_actorBans,TARGET_TYPE_ACTOR,name,BAN_TYPE_CHAT,lastTime,reason);
}

void CBanlist::BanMACLogin(const char *name, int lastTime, const char *reason)
{
	if (0 == name)
		return;

	__DoBan(m_macBans, TARGET_TYPE_MAC, name, BAN_TYPE_LOGIN, lastTime, reason);
}

void CBanlist::UnBanIpLogin(const char* name)
{
    __DelBan(m_ipBans,name,BAN_TYPE_LOGIN);
}

void CBanlist::UnBanIpChat(const char* name)
{
    __DelBan(m_ipBans,name,BAN_TYPE_CHAT);
}

void CBanlist::UnBanAccLogin(const char* name)
{
    __DelBan(m_accBans,name,BAN_TYPE_LOGIN);
}

void CBanlist::UnBanAccChat(const char* name)
{
    __DelBan(m_accBans,name,BAN_TYPE_CHAT);
}

void CBanlist::UnBanActorLogin(const char* name)
{
    __DelBan(m_actorBans,name,BAN_TYPE_LOGIN);
}

void CBanlist::UnBanActorChat(const char* name)
{
    __DelBan(m_actorBans,name,BAN_TYPE_CHAT);
}

void CBanlist::UnBanMacLogin(const char *name)
{
	__DelBan(m_macBans, name, BAN_TYPE_LOGIN);
}

bool CBanlist::IsIpLoginBan(const char* name,BanInfo* pInfo)
{
    return __IsBaned(m_ipBans,name,BAN_TYPE_LOGIN,pInfo);
}

bool CBanlist::IsIpChatBan(const char* name,BanInfo* pInfo)
{
    return __IsBaned(m_ipBans,name,BAN_TYPE_CHAT,pInfo);
}

bool CBanlist::IsAccLoginBan(const char* name,BanInfo* pInfo)
{
    return __IsBaned(m_accBans,name,BAN_TYPE_LOGIN,pInfo);
}

bool CBanlist::IsAccChatBan(const char* name,BanInfo* pInfo)
{
    return __IsBaned(m_accBans,name,BAN_TYPE_CHAT,pInfo);
}

bool CBanlist::IsActorLoginBan(const char* name,BanInfo* pInfo)
{   
    return __IsBaned(m_actorBans,name,BAN_TYPE_LOGIN,pInfo);
}

bool CBanlist::IsActorChatBan(const char* name,BanInfo* pInfo)
{
    return __IsBaned(m_actorBans,name,BAN_TYPE_CHAT,pInfo);
}

bool CBanlist::IsMacLoginBan(const char *name, BanInfo * pInfo /* = 0 */)
{
	return __IsBaned(m_macBans, name, BAN_TYPE_LOGIN, pInfo);
}

void CBanlist::__DoBan(BAN_MAP& banMap,int target,const char* name,int banType,int banTime,const char* reason)
{
    if (0 == name || 0 == reason)
        return;

    OLD_DO_LOCK(m_cs);

    name = __TranslateName(name);

    BAN_MAP::iterator iter = banMap.find(name);

    if (iter == banMap.end())
    {
        iter = banMap.insert(std::make_pair(name,BanInfo())).first;
    }

    BanInfo& info = iter->second;

    info.name = name;
    info.type = target;

    //永久冻结帐号，给个足够大的年数
    if (0 == banTime)
    {
        banTime = 622080000; //10年
    }
    
    switch(banType)
    {
    case BAN_TYPE_CHAT:
        {
            info.chatBanExpire = (int)time(0) + banTime;
            info.chatReason    = reason ? reason : "";
        }
        break;
    case BAN_TYPE_LOGIN:
        {
            info.loginBanExpire = (int)time(0) + banTime;
            info.loginReason    = reason ? reason : "";
        }
        break;
    }

    //save to db
    __SaveBanToDB(info);
}

bool CBanlist::__IsBaned(BAN_MAP& banMap,const char* name,int type,BanInfo* pInfo)
{
    if (0 == name)
        return false;

    OLD_DO_LOCK(m_cs);

    name = __TranslateName(name);

    BAN_MAP::iterator iter = banMap.find(name);

    if (iter == banMap.end())
        return false;

    BanInfo& info = iter->second;

    if (0 != pInfo)
        *pInfo = info;

    int curTime = (int)time(0);

    switch(type)
    {
    case BAN_TYPE_CHAT:
        {
            return info.IsChatBaned(curTime);
        }
    case BAN_TYPE_LOGIN:
        {
            return info.IsLoginBaned(curTime);
        }
    }

    return false;
}

void CBanlist::__DelBan(BAN_MAP& banMap,const char* name,int type)
{
    if (0 == name)
        return;

    OLD_DO_LOCK(m_cs);

    name = __TranslateName(name);

    BAN_MAP::iterator iter = banMap.find(name);

    if (iter != banMap.end())
    {
        BanInfo& info = iter->second;

        switch(type)
        {
        case BAN_TYPE_CHAT:
            {
                info.chatBanExpire = 0;
                info.chatReason    = "";
            }
            break;
        case BAN_TYPE_LOGIN:
            {
                info.loginBanExpire = 0;
                info.loginReason    = "";
            }
            break;
        }

        __SaveBanToDB(info);

        if (!__HasBaned(info))
        {
            banMap.erase(iter);
        }
    }
}

void CBanlist::__SaveBanToDB(BanInfo& info)
{
	//首先删除这条记录
	TBLBanList tb_ban(0);
    tb_ban.AttachConn(CGMRecver::Instance()->GetDBConn());

	int curTime = int(time(0));
	DBError err = tb_ban.Delete(info.name.c_str(),info.type);

	if(err != DBERR_NONE)
		return;

    if (__HasBaned(info))
    {
		TBLBanList::stBan ban;
		dStrcpy(ban.target, 32, info.name.c_str());
		ban.type		= info.type;
		ban.loginBan	= info.loginBanExpire;
		ban.chatBan		= info.chatBanExpire;
		dStrcpy(ban.loginBanReason, 50, info.loginReason.c_str());
		dStrcpy(ban.chatBanReason, 50, info.chatReason.c_str());
		tb_ban.Update(ban);
    }
}

const char* CBanlist::__TranslateName(const char* name)
{   
    static char tmp[256];
    dStrcpy(tmp,sizeof(tmp),name);
    _strupr_s(tmp,sizeof(tmp));
    return tmp;
}

bool CBanlist::__HasBaned(BanInfo& info)
{
    int curTime = (int)time(0);

    return info.IsChatBaned(curTime) || info.IsLoginBaned(curTime);
}

void CBanlist::SendBanListToChatServer()
{
	BAN_MAP::iterator it_acc = m_accBans.begin();
	BAN_MAP::iterator it_act = m_actorBans.begin();
	BAN_MAP::iterator it_ip = m_ipBans.begin();

	std::vector<BanInfo> Infos;

	while (it_acc != m_accBans.end())
	{
		BanInfo& info = it_acc->second;
		if (0 == info.loginBanExpire)
			Infos.push_back(info);
		it_acc++;
	}

	while (it_act != m_actorBans.end())
	{
		BanInfo& info = it_act->second;
		if (0 == info.loginBanExpire)
			Infos.push_back(info);
		it_act++;
	}

	while (it_ip != m_ipBans.end())
	{
		BanInfo& info = it_ip->second;
		if (0 == info.loginBanExpire)
			Infos.push_back(info);
		it_ip++;
	}

	char buf[MAX_PACKET_SIZE];
	Base::BitStream sendPacket(buf, sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead( sendPacket, WORLD_CHAT_ChatBanList, 0, SERVICE_CHATSERVER );

	int size = Infos.size();
	for (int i=0; i<size; i++)
	{
		sendPacket.writeInt(size,Base::Bit32);
		Infos[i].pack(&sendPacket);
	}

	pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
	int svrSocket = SERVER->GetServerManager()->GetServerSocket( SERVICE_CHATSERVER );

	SERVER->GetServerSocket()->Send( svrSocket, sendPacket  );

}