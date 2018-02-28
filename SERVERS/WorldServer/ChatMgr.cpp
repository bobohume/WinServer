#include "ChatMgr.h"

#include "..\ChatServer\PlayerMgr.h"
#include "..\ChatServer\ServerMgr.h"
#include "..\ChatServer\ChatServer.h"
#include "WINTCP/AsyncSocket.h"
#include <string>

#include <time.h>
#include "Common/MemGuard.h"
#include "Common/FilterOpt.h"
#include "Common/BanInfoData.h"
#include "AtlConv.h"

#define CHAT_PACKET_SIZE (sizeof(stChatMessage) + 32)

enum CharTarget
{
	CHAT_TARGET_PERSON = 0, //个人
	CHAT_TARGET_TEAM,       //组队
	CHAT_TARGET_GUILD,      //公会
	CHAT_TARGET_ZONE,       //地图
	CHAT_TARGET_ALLZONE,    //全服
};

static char* _province_city[40][30];

CChatManager::CChatManager()
{
	m_chatLog.SetLogName("chatrec", false);
}

CChatManager::~CChatManager()
{
}

const char* _sep = "%#!@$^";

void CChatManager::writeLog(int playerId, int type, const char* msg, int playerIdTo)
{
	try
	{
		if (type >= CHAT_MSG_TYPE_UNKNOWN)
		{
			type = CHAT_MSG_TYPE_UNKNOWN;
		}

		if (0 == msg)
		{
			msg = "";
		}

		if (playerIdTo < 0)
		{
			ChatPlayerInfo srcPlayer;
			bool hasSrc = SERVER->getPlayerMgr()->GetPlayerInfo(playerId, srcPlayer);

			m_chatLog.WriteLog("%s%s%d%s%s%s%d%s%s%s%s", hasSrc ? srcPlayer.accountName : "?",
				_sep,
				playerId,
				_sep,
				hasSrc ? srcPlayer.playerName : "?",
				_sep,
				0,
				_sep,
				"全服跑马",
				_sep,
				msg);
		}
		else if (playerIdTo == 0 || playerIdTo == playerId)
		{
			ChatPlayerInfo srcPlayer;
			bool hasSrc = SERVER->getPlayerMgr()->GetPlayerInfo(playerId, srcPlayer);

			m_chatLog.WriteLog("%s%s%d%s%s%s%d%s%s%s%s", hasSrc ? srcPlayer.accountName : "?",
				_sep,
				playerId,
				_sep,
				hasSrc ? srcPlayer.playerName : "?",
				_sep,
				0,
				_sep,
				"?",
				_sep,
				msg);

		}
		else
		{
			ChatPlayerInfo srcPlayer;
			ChatPlayerInfo dstPlayer;
			bool hasSrc = SERVER->getPlayerMgr()->GetPlayerInfo(playerId, srcPlayer);
			bool hasDst = SERVER->getPlayerMgr()->GetPlayerInfo(playerIdTo, dstPlayer);

			m_chatLog.WriteLog("%s%s%d%s%s%s%d%s%s%s%s", hasSrc ? srcPlayer.accountName : "?",
				_sep,
				playerId,
				_sep,
				hasSrc ? srcPlayer.playerName : "?",
				_sep,
				playerIdTo,
				_sep,
				hasDst ? dstPlayer.playerName : "?",
				_sep,
				msg);
		}
	}
	catch (...)
	{
	}
}

void CChatManager::Initialize()
{
	//注册8个门宗频道
	for (int i = 0; i < 8; i++)
	{
		m_FamilyChannels[i] = GetChannelManager()->RegisterChannel(CHAT_MSG_TYPE_RACE, "");
	}

	initCityChannels();
	g_BanInfoRepository.read();
}

CChannelManager* CChatManager::GetChannelManager()
{
	return &m_channelManager;
}

void CChatManager::HandleZoneSendMessage(int lineId, stPacketHead* pHead, Base::BitStream* pBitStream)
{
	stChatMessage msg;
	stChatMessage::UnpackMessage(msg, *pBitStream);

	std::string sendName = CChatServer::getInstance()->getPlayerMgr()->getPlayerName(msg.nSender);

	if (sendName != "")
	{
		dStrcpy(msg.szSenderName, sizeof(msg.szSenderName), sendName.c_str());
	}

	switch (msg.btMessageType)
	{
	case CHAT_MSG_TYPE_SYSTEM:
	case CHAT_MSG_TYPE_SYSPLUS:
	{
		if (msg.nRecver == 0)
		{
			SendMessageToLine(msg, lineId);
			g_Log.WriteLog("全线跑马内容:%s, 玩家ID:%d, 玩家名字:%s", msg.szMessage, msg.nSender, msg.szSenderName);
		}
		else
		{
			SendMessageToWorld(msg);
			g_Log.WriteLog("全服跑马内容:%s, 玩家ID:%d, 玩家名字:%s", msg.szMessage, msg.nSender, msg.szSenderName);
		}
	}
	break;
	case CHAT_MSG_TYPE_LINE:
		SendMessageToLine(msg, lineId);
		break;
	case CHAT_MSG_TYPE_WORLD:
		SendMessageToWorld(msg);
		break;
	case CHAT_MSG_TYPE_WORLDPLUS:
	{
		SendMessageToWorld(msg);
		writeLog(msg.nSender, msg.btMessageType, msg.szMessage, -1);
		break;
	}
	case CHAT_MSG_TYPE_MSGBOX:
		SendMessageToWorld(msg);
		break;
	case CHAT_MSG_TYPE_SUPER_WORLD:
		SendMessageToWorld(msg);
		break;
	case CHAT_MSG_TYPE_SUPER_WORLD2:
		SendMessageToWorld(msg);
		break;
	case CHAT_MSG_TYPE_RACE:
	{
		if (msg.nRecver >= 1 && msg.nRecver <= 8)
		{
			SendMessageToChannel(msg, m_FamilyChannels[msg.nRecver - 1]);
		}
	}
	break;
	case CHAT_MSG_TYPE_TEAM:
	{
		U32 channelId = GetChannelManager()->GetChannelId(msg.nRecver, msg.btMessageType);
		SendMessageToChannel(msg, channelId);
	}
	break;
	case CHAT_MSG_TYPE_SQUAD:
	{
		U32 channelId = GetChannelManager()->GetChannelId(msg.nRecver, msg.btMessageType);
		SendMessageToChannel(msg, channelId);
	}
	break;
	case CHAT_MSG_TYPE_AWARD:
	{
		SendMessageToWorld(msg);
	}
	break;
	default:
		break;
	}
}

void CChatManager::SendChatResponse(int nSocket, int nAccountId, char cType, int value, BYTE nChannel)
{
	char buf[64];
	Base::BitStream SendPacket(buf, sizeof(buf));
	stPacketHead* pHead = IPacket::BuildPacketHead(SendPacket, "CLIENT_CHAT_SendMessageResponse_Notify");

	pHead->DestServerType = SERVICE_CLIENT;
	pHead->Id = nAccountId;
	pHead->DestZoneId = value;
	pHead->SrcZoneId = cType;
	pHead->LineServerId = nChannel;

	pHead->PacketSize = SendPacket.getPosition() - sizeof(stPacketHead);

	CChatServer::getInstance()->getServerSocket()->Send(nSocket, SendPacket);
}

void CChatManager::initCityChannels()
{
	_province_city[0][0] = "1";
	_province_city[0][1] = "北京";


	_province_city[1][0] = "1";
	_province_city[1][1] = "上海";


	_province_city[2][0] = "1";
	_province_city[2][1] = "天津";


	_province_city[3][0] = "12";
	_province_city[3][1] = "河北";
	_province_city[3][2] = "石家庄";
	_province_city[3][3] = "唐山";
	_province_city[3][4] = "秦皇岛";
	_province_city[3][5] = "邯郸";
	_province_city[3][6] = "邢台";
	_province_city[3][7] = "保定";
	_province_city[3][8] = "张家口";
	_province_city[3][9] = "承德";
	_province_city[3][10] = "沧州";
	_province_city[3][11] = "廊坊";
	_province_city[3][12] = "衡水";

	_province_city[4][0] = "12";
	_province_city[4][1] = "山西";
	_province_city[4][2] = "太原";
	_province_city[4][3] = "大同";
	_province_city[4][4] = "阳泉";
	_province_city[4][5] = "长治";
	_province_city[4][6] = "晋城";
	_province_city[4][7] = "朔州";
	_province_city[4][8] = "晋中";
	_province_city[4][9] = "运城";
	_province_city[4][10] = "忻州";
	_province_city[4][11] = "临汾";
	_province_city[4][12] = "吕梁";


	_province_city[5][0] = "13";
	_province_city[5][1] = "内蒙古";
	_province_city[5][2] = "呼和浩特";
	_province_city[5][3] = "包头";
	_province_city[5][4] = "乌海";
	_province_city[5][5] = "赤峰";
	_province_city[5][6] = "通辽";
	_province_city[5][7] = "鄂尔多斯";
	_province_city[5][8] = "呼伦贝尔";
	_province_city[5][9] = "巴彦淖尔";
	_province_city[5][10] = "乌兰察布";
	_province_city[5][11] = "兴安";
	_province_city[5][12] = "锡林郭勒";
	_province_city[5][13] = "阿拉善";


	_province_city[6][0] = "15";
	_province_city[6][1] = "辽宁";
	_province_city[6][2] = "沈阳";
	_province_city[6][3] = "大连";
	_province_city[6][4] = "鞍山";
	_province_city[6][5] = "抚顺";
	_province_city[6][6] = "本溪";
	_province_city[6][7] = "丹东";
	_province_city[6][8] = "锦州";
	_province_city[6][9] = "营口";
	_province_city[6][10] = "阜新";
	_province_city[6][11] = "辽阳";
	_province_city[6][12] = "盘锦";
	_province_city[6][13] = "铁岭";
	_province_city[6][14] = "朝阳";
	_province_city[6][15] = "葫芦岛";

	_province_city[7][0] = "10";
	_province_city[7][1] = "吉林";
	_province_city[7][2] = "长春";
	_province_city[7][3] = "吉林";
	_province_city[7][4] = "四平";
	_province_city[7][5] = "辽源";
	_province_city[7][6] = "通化";
	_province_city[7][7] = "白山";
	_province_city[7][8] = "松原";
	_province_city[7][9] = "白城";
	_province_city[7][10] = "延边";

	_province_city[8][0] = "13";
	_province_city[8][1] = "黑龙江";
	_province_city[8][2] = "哈尔滨";
	_province_city[8][3] = "齐齐哈尔";
	_province_city[8][4] = "牡丹江";
	_province_city[8][5] = "佳木斯";
	_province_city[8][6] = "大庆";
	_province_city[8][7] = "鸡西";
	_province_city[8][8] = "双鸭山";
	_province_city[8][9] = "伊春";
	_province_city[8][10] = "七台河";
	_province_city[8][11] = "鹤岗";
	_province_city[8][12] = "黑河";
	_province_city[8][13] = "绥化";


	_province_city[9][0] = "14";
	_province_city[9][1] = "江苏";
	_province_city[9][2] = "南京";
	_province_city[9][3] = "镇江";
	_province_city[9][4] = "常州";
	_province_city[9][5] = "无锡";
	_province_city[9][6] = "苏州";
	_province_city[9][7] = "南通";
	_province_city[9][8] = "扬州";
	_province_city[9][9] = "盐城";
	_province_city[9][10] = "淮安";
	_province_city[9][11] = "连云港";
	_province_city[9][12] = "徐州";
	_province_city[9][13] = "泰州";
	_province_city[9][14] = "宿迁";


	_province_city[10][0] = "12";
	_province_city[10][1] = "浙江";
	_province_city[10][2] = "杭州";
	_province_city[10][3] = "宁波";
	_province_city[10][4] = "温州";
	_province_city[10][5] = "绍兴";
	_province_city[10][6] = "嘉兴";
	_province_city[10][7] = "湖州";
	_province_city[10][8] = "金华";
	_province_city[10][9] = "衢州";
	_province_city[10][10] = "舟山";
	_province_city[10][11] = "台州";
	_province_city[10][12] = "丽水";

	_province_city[11][0] = "18";
	_province_city[11][1] = "安徽";
	_province_city[11][2] = "合肥";
	_province_city[11][3] = "蚌埠";
	_province_city[11][4] = "芜湖";
	_province_city[11][5] = "淮南";
	_province_city[11][6] = "马鞍";
	_province_city[11][7] = "淮北";
	_province_city[11][8] = "铜陵";
	_province_city[11][9] = "安庆";
	_province_city[11][10] = "黄山";
	_province_city[11][11] = "阜阳";
	_province_city[11][12] = "宿州";
	_province_city[11][13] = "滁州";
	_province_city[11][14] = "六安";
	_province_city[11][15] = "宣城";
	_province_city[11][16] = "巢湖";
	_province_city[11][17] = "池州";
	_province_city[11][18] = "亳州";

	_province_city[12][0] = "10";
	_province_city[12][1] = "福建";
	_province_city[12][2] = "福州";
	_province_city[12][3] = "厦门";
	_province_city[12][4] = "莆田";
	_province_city[12][5] = "三明";
	_province_city[12][6] = "泉州";
	_province_city[12][7] = "漳州";
	_province_city[12][8] = "南平";
	_province_city[12][9] = "龙岩";
	_province_city[12][10] = "宁德";

	_province_city[13][0] = "12";
	_province_city[13][1] = "江西";
	_province_city[13][2] = "南昌";
	_province_city[13][3] = "九江";
	_province_city[13][4] = "景德镇";
	_province_city[13][5] = "萍乡";
	_province_city[13][6] = "新余";
	_province_city[13][7] = "鹰潭";
	_province_city[13][8] = "赣州";
	_province_city[13][9] = "宜春";
	_province_city[13][10] = "上饶";
	_province_city[13][11] = "吉安";
	_province_city[13][12] = "抚州";


	_province_city[14][0] = "18";
	_province_city[14][1] = "山东";
	_province_city[14][2] = "济南";
	_province_city[14][3] = "青岛";
	_province_city[14][4] = "淄博";
	_province_city[14][5] = "枣庄";
	_province_city[14][6] = "东营";
	_province_city[14][7] = "烟台";
	_province_city[14][8] = "潍坊";
	_province_city[14][9] = "济宁";
	_province_city[14][10] = "泰安";
	_province_city[14][11] = "威海";
	_province_city[14][12] = "日照";
	_province_city[14][13] = "莱芜";
	_province_city[14][14] = "临沂";
	_province_city[14][15] = "德州";
	_province_city[14][16] = "聊城";
	_province_city[14][17] = "滨州";
	_province_city[14][18] = "菏泽";

	_province_city[15][0] = "19";
	_province_city[15][1] = "河南";
	_province_city[15][2] = "郑州";
	_province_city[15][3] = "开封";
	_province_city[15][4] = "洛阳";
	_province_city[15][5] = "平顶山";
	_province_city[15][6] = "安阳";
	_province_city[15][7] = "鹤壁";
	_province_city[15][8] = "新乡";
	_province_city[15][9] = "焦作";
	_province_city[15][10] = "濮阳";
	_province_city[15][11] = "许昌";
	_province_city[15][12] = "漯河";
	_province_city[15][13] = "三门峡";
	_province_city[15][14] = "南阳";
	_province_city[15][15] = "商丘";
	_province_city[15][16] = "信阳";
	_province_city[15][17] = "周口";
	_province_city[15][18] = "驻马店";
	_province_city[15][19] = "济源";

	_province_city[16][0] = "17";
	_province_city[16][1] = "湖北";
	_province_city[16][2] = "武汉";
	_province_city[16][3] = "黄石";
	_province_city[16][4] = "十堰";
	_province_city[16][5] = "宜昌";
	_province_city[16][6] = "襄樊";
	_province_city[16][7] = "鄂州";
	_province_city[16][8] = "荆门";
	_province_city[16][9] = "孝感";
	_province_city[16][10] = "荆州";
	_province_city[16][11] = "黄冈";
	_province_city[16][12] = "咸宁";
	_province_city[16][13] = "随州";
	_province_city[16][14] = "恩施";
	_province_city[16][15] = "仙桃";
	_province_city[16][16] = "潜江";
	_province_city[16][17] = "天门";

	_province_city[17][0] = "15";
	_province_city[17][1] = "湖南";
	_province_city[17][2] = "长沙";
	_province_city[17][3] = "株洲";
	_province_city[17][4] = "湘潭";
	_province_city[17][5] = "衡阳";
	_province_city[17][6] = "邵阳";
	_province_city[17][7] = "岳阳";
	_province_city[17][8] = "常德";
	_province_city[17][9] = "张家界";
	_province_city[17][10] = "益阳";
	_province_city[17][11] = "郴州";
	_province_city[17][12] = "永州";
	_province_city[17][13] = "怀化";
	_province_city[17][14] = "娄底";
	_province_city[17][15] = "湘西";

	_province_city[18][0] = "22";
	_province_city[18][1] = "广东";
	_province_city[18][2] = "广州";
	_province_city[18][3] = "韶关";
	_province_city[18][4] = "深圳";
	_province_city[18][5] = "珠海";
	_province_city[18][6] = "汕头";
	_province_city[18][7] = "佛山";
	_province_city[18][8] = "江门";
	_province_city[18][9] = "湛江";
	_province_city[18][10] = "茂名";
	_province_city[18][11] = "肇庆";
	_province_city[18][12] = "惠州";
	_province_city[18][13] = "梅州";
	_province_city[18][14] = "汕尾";
	_province_city[18][15] = "河源";
	_province_city[18][16] = "阳江";
	_province_city[18][17] = "清远";
	_province_city[18][18] = "东莞";
	_province_city[18][19] = "中山";
	_province_city[18][20] = "潮州";
	_province_city[18][21] = "揭阳";
	_province_city[18][22] = "云浮";

	_province_city[19][0] = "15";
	_province_city[19][1] = "广西";
	_province_city[19][2] = "南宁";
	_province_city[19][3] = "柳州";
	_province_city[19][4] = "桂林";
	_province_city[19][5] = "梧州";
	_province_city[19][6] = "北海";
	_province_city[19][7] = "防城港";
	_province_city[19][8] = "钦州";
	_province_city[19][9] = "贵港";
	_province_city[19][10] = "玉林";
	_province_city[19][11] = "百色";
	_province_city[19][12] = "贺州";
	_province_city[19][13] = "河池";
	_province_city[19][14] = "来宾";
	_province_city[19][15] = "崇左";


	_province_city[20][0] = "3";
	_province_city[20][1] = "海南";
	_province_city[20][2] = "海口";
	_province_city[20][3] = "三亚";


	_province_city[21][0] = "22";
	_province_city[21][1] = "四川";
	_province_city[21][2] = "成都";
	_province_city[21][3] = "自贡";
	_province_city[21][4] = "攀枝花";
	_province_city[21][5] = "泸州";
	_province_city[21][6] = "德阳";
	_province_city[21][7] = "绵阳";
	_province_city[21][8] = "广元";
	_province_city[21][9] = "遂宁";
	_province_city[21][10] = "内江";
	_province_city[21][11] = "乐山";
	_province_city[21][12] = "南充";
	_province_city[21][13] = "眉山";
	_province_city[21][14] = "宜宾";
	_province_city[21][15] = "广安";
	_province_city[21][16] = "达州";
	_province_city[21][17] = "雅安";
	_province_city[21][18] = "巴中";
	_province_city[21][19] = "资阳";
	_province_city[21][20] = "阿坝";
	_province_city[21][21] = "甘孜";
	_province_city[21][22] = "凉山";

	_province_city[22][0] = "9";
	_province_city[22][1] = "贵州";
	_province_city[22][2] = "贵阳";
	_province_city[22][3] = "六盘水";
	_province_city[22][4] = "遵义";
	_province_city[22][5] = "安顺";
	_province_city[22][6] = "铜仁";
	_province_city[22][7] = "黔西南";
	_province_city[22][8] = "黔东南";
	_province_city[22][9] = "黔南";

	_province_city[23][0] = "17";
	_province_city[23][1] = "云南";
	_province_city[23][2] = "昆明";
	_province_city[23][3] = "曲靖";
	_province_city[23][4] = "玉溪";
	_province_city[23][5] = "保山";
	_province_city[23][6] = "昭通";
	_province_city[23][7] = "丽江";
	_province_city[23][8] = "普洱";
	_province_city[23][9] = "临沧";
	_province_city[23][10] = "楚雄";
	_province_city[23][11] = "红河";
	_province_city[23][12] = "文山";
	_province_city[23][13] = "西双版纳";
	_province_city[23][14] = "大理";
	_province_city[23][15] = "德宏";
	_province_city[23][16] = "怒江";
	_province_city[23][17] = "迪庆";


	_province_city[24][0] = "8";
	_province_city[24][1] = "西藏";
	_province_city[24][2] = "拉萨";
	_province_city[24][3] = "昌都";
	_province_city[24][4] = "山南";
	_province_city[24][5] = "日喀则";
	_province_city[24][6] = "那曲";
	_province_city[24][7] = "阿里";
	_province_city[24][8] = "林芝";


	_province_city[25][0] = "11";
	_province_city[25][1] = "陕西";
	_province_city[25][2] = "西安";
	_province_city[25][3] = "铜川";
	_province_city[25][4] = "宝鸡";
	_province_city[25][5] = "咸阳";
	_province_city[25][6] = "渭南";
	_province_city[25][7] = "延安";
	_province_city[25][8] = "汉中";
	_province_city[25][9] = "榆林";
	_province_city[25][10] = "安康";
	_province_city[25][11] = "商洛";

	_province_city[26][0] = "15";
	_province_city[26][1] = "甘肃";
	_province_city[26][2] = "兰州";
	_province_city[26][3] = "嘉峪关";
	_province_city[26][4] = "金昌";
	_province_city[26][5] = "白银";
	_province_city[26][6] = "天水";
	_province_city[26][7] = "武威";
	_province_city[26][8] = "张掖";
	_province_city[26][9] = "平凉";
	_province_city[26][10] = "酒泉";
	_province_city[26][11] = "庆阳";
	_province_city[26][12] = "定西";
	_province_city[26][13] = "陇南";
	_province_city[26][14] = "临夏";
	_province_city[26][15] = "甘南";

	_province_city[27][0] = "9";
	_province_city[27][1] = "青海";
	_province_city[27][2] = "西宁";
	_province_city[27][3] = "海东";
	_province_city[27][4] = "海北";
	_province_city[27][5] = "黄南";
	_province_city[27][6] = "海南";
	_province_city[27][7] = "果洛";
	_province_city[27][8] = "玉树";
	_province_city[27][9] = "海西";


	_province_city[28][0] = "6";
	_province_city[28][1] = "宁夏";
	_province_city[28][2] = "银川";
	_province_city[28][3] = "石嘴山";
	_province_city[28][4] = "吴忠";
	_province_city[28][5] = "固原";
	_province_city[28][6] = "中卫";


	_province_city[29][0] = "19";
	_province_city[29][1] = "新疆";
	_province_city[29][2] = "乌鲁木齐";
	_province_city[29][3] = "克拉玛依";
	_province_city[29][4] = "吐鲁番";
	_province_city[29][5] = "哈密";
	_province_city[29][6] = "昌吉";
	_province_city[29][7] = "博尔塔拉";
	_province_city[29][8] = "巴音郭楞";
	_province_city[29][9] = "阿克苏  ";
	_province_city[29][10] = "克孜勒苏";
	_province_city[29][11] = "喀什";
	_province_city[29][12] = "和田";
	_province_city[29][13] = "伊犁哈萨克";
	_province_city[29][14] = "塔城";
	_province_city[29][15] = "阿勒泰";
	_province_city[29][16] = "石河子";
	_province_city[29][17] = "阿拉尔";
	_province_city[29][18] = "图木舒克";
	_province_city[29][19] = "五家渠";

	_province_city[30][0] = "1";
	_province_city[30][1] = "香港";


	_province_city[31][0] = "1";
	_province_city[31][1] = "澳门";


	_province_city[32][0] = "1";
	_province_city[32][1] = "台湾";

	_province_city[33][0] = "1";
	_province_city[33][1] = "其他";


	for (int i = 0; i <= 33; i++)
	{
		int count = atoi(_province_city[i][0]);

		for (int j = 1; j <= count; j++)
		{
			char* name = _province_city[i][j];
			mChannelMap[i * 100 + j] = GetChannelManager()->RegisterChannel(CHAT_MSG_TYPE_REALCITY, name);
		}
	}
}

void CChatManager::HandleJoinCity(int nSocket, stPacketHead* pHead, Base::BitStream* pBitStream)
{
	int playerId = pHead->Id;

	if (pHead->DestZoneId >= MAX_CITY_CHANNEL)
		return;

	U32 channelId = GetChannelManager()->GetChannelId(playerId, CHAT_MSG_TYPE_REALCITY);

	if (0 != channelId)
	{
		GetChannelManager()->RemovePlayer(playerId, channelId);
	}

	GetChannelManager()->AddPlayer(playerId, mChannelMap[pHead->DestZoneId]);
}

void CChatManager::HandleIncomingMsg(int nSocket, stPacketHead* pHead, Base::BitStream* pBitStream)
{
	int nPlayerId = pHead->Id;

	int lineId = SERVER->getServerMgr()->getLineId(nSocket);
	int accountId = SERVER->getPlayerMgr()->getAccountId(nPlayerId);

	if (0 == accountId)
		return;

	int nameRecver = 0;

	// 增加通过名字直接发送游戏消息
	if (pHead->DestZoneId == 1)
	{
		char playerName[COMMON_STRING_LENGTH];
		pBitStream->readString(playerName, COMMON_STRING_LENGTH);

		int namePlayerId = SERVER->getPlayerMgr()->getPlayerId(playerName);

		if (0 == namePlayerId)
		{
			SendChatResponse(nSocket, accountId, SEND_CHAT_ACK_NOPLAYER);
			return;
		}

		nameRecver = namePlayerId;
	}

	// 用于发送附近消息	
	std::vector<int> nearPlayerList;

	if (pHead->DestZoneId == 2)
	{
		int playerCount = pBitStream->readInt(16);

		for (int i = 0; i < playerCount; i++)
		{
			int pid = pBitStream->readInt(32);
			nearPlayerList.push_back(pid);
		}
	}

	stChatMessage msg;
	stChatMessage::UnpackMessage1(msg, *pBitStream);
	msg.nSender = nPlayerId;

	if (nameRecver != 0)
		msg.nRecver = nameRecver;

	// 检查消息是否合法
	if (IsBaned(msg))
	{
		SendChatResponse(nSocket, accountId, SEND_CHAT_ACK_REFUSED);
		return;
	}

	stChatMessage sendMsg;
	std::string tempStr = Util::Utf8ToMbcs(msg.szMessage);
	//std::string tempStr = msg.szMessage;
	if (!FilterOpt::replaceBanWord(sendMsg.szMessage, 512, tempStr.c_str())){
		dStrcpy(sendMsg.szMessage, 512, tempStr.c_str());
	}

	std::string str = SERVER->getPlayerMgr()->getPlayerName(msg.nSender);

	// 如果服务器上取不到玩家名字，则从发送消息内取得玩家名字
	if (str == "")
		dStrcpy(sendMsg.szSenderName, 32, msg.szSenderName);
	else
		dStrcpy(sendMsg.szSenderName, 32, str.c_str());

	sendMsg.btMessageType = msg.btMessageType;
	sendMsg.nSender = msg.nSender;
	sendMsg.nSenderLineID = msg.nSenderLineID;
	sendMsg.btPrivilegeFlagType = msg.btPrivilegeFlagType;

	// 检查发送时间间隔
	U64 nPendingTime = GetPlayerChatPendingTime(nPlayerId, msg.btMessageType);
	U64 nLastTime = GetPlayerChatLastTime(nPlayerId, msg.btMessageType);

	if (::GetTickCount64() - nLastTime < nPendingTime)
	{
		SendChatResponse(nSocket, accountId, SEND_CHAT_ACK_TOOFAST, msg.btMessageType == CHAT_MSG_TYPE_P2P ? msg.nRecver : 0, msg.btMessageType);
		return;
	}

	SetPlayerChatLastTime(nPlayerId, msg.btMessageType, ::GetTickCount64());

	// 检查发送消息种类
	if (msg.btMessageType < CHAT_MSG_TYPE_LINE)
	{
		// 玩家不能通过发送消息的方式发送大规模消息
		SendChatResponse(nSocket, accountId, SEND_CHAT_ACK_ERROR);
		return;
	}

	writeLog(msg.nSender, msg.btMessageType, msg.szMessage, msg.nRecver);

	U32 channelId = GetChannelManager()->GetChannelId(nPlayerId,msg.btMessageType);

	if(0 == channelId)
	{
		// 如果是群消息
		if( msg.btMessageType == CHAT_MSG_TYPE_GROUP)
		{
			channelId = GetChannelManager()->GetChannelId(msg.nRecver);
		}

		if( msg.btMessageType == CHAT_MSG_TYPE_RACE && msg.nRecver != 0 )
		{
			if(msg.nRecver >= 1 && msg.nRecver <= 8)
			{
				channelId = GetChannelManager()->GetChannelId(m_FamilyChannels[msg.nRecver-1]);
			}
		}
	}

	//可疑字符
	if (isSusWord(sendMsg.szMessage))
	{
		writeBadWordPlayer(sendMsg.nSender, sendMsg.szMessage, ChatBanInfo::SUSP);
	}

	//敏感字符
	if (isSensWord(sendMsg.szMessage))
	{
		writeBadWordPlayer(sendMsg.nSender, sendMsg.szMessage, ChatBanInfo::SENS);
		SendChatResponse(nSocket, accountId, SEND_CHAT_ACK_ILLEGAL);
		return;
	}


	if ((msg.btMessageType == CHAT_MSG_TYPE_PRIVATE /*|| msg.btMessageType == CHAT_MSG_TYPE_P2P */) && msg.nRecver != msg.nSender) // 不能给自己发点对点消息
	{
		std::string recvName = SERVER->getPlayerMgr()->getPlayerName(msg.nRecver);

		if (recvName == "")
			dStrcpy(sendMsg.szRecverName, 32, msg.szRecverName);
		else
			dStrcpy(sendMsg.szRecverName, 32, recvName.c_str());

		sendMsg.nRecver = msg.nRecver;
		int accountId = SERVER->getPlayerMgr()->getAccountId(msg.nRecver);

		if (accountId)
		{
			SendMessageTo(sendMsg, msg.nRecver);

			if (msg.btMessageType == CHAT_MSG_TYPE_PRIVATE || msg.btMessageType == CHAT_MSG_TYPE_P2P)
				SendMessageTo(sendMsg, msg.nSender);
		}
		else
			SendChatResponse(nSocket, accountId, SEND_CHAT_ACK_NOPLAYER, msg.nRecver);
	}
	else if (msg.btMessageType == CHAT_MSG_TYPE_LINE)
	{
		SendMessageToWorld(sendMsg);
	}
	else if (msg.btMessageType == CHAT_MSG_TYPE_AWARD)
	{
	SendMessageToWorld( sendMsg );
	}
	else
	{
		// 这是一个无效的消息
		if(0 == channelId)
		{
			if( msg.btMessageType == CHAT_MSG_TYPE_NEARBY )
			{
				SendMessage( sendMsg, nearPlayerList );
			}
			else
			{
				SendChatResponse( nSocket, accountId, SEND_CHAT_ACK_ERROR );
			}

			return;
		}

		SendMessageToChannel(sendMsg,channelId);
	}
}

void CChatManager::HandleGMMessage(stChatMessage& chatMessage)
{
	// 测试用，当前仅把GM发来的消息作为世界消息发送
	chatMessage.szMessage[0] = ' ';
	chatMessage.btMessageType = CHAT_MSG_TYPE_SYSTEM;
	SendMessageToWorld(chatMessage);
}

// 没有很强的时效性，不与客户端同步处理
void CChatManager::HandleAddToFamily(int nSocket, stPacketHead* pHead, Base::BitStream* pBitStream)
{
	int playerId = pHead->Id;
	int family = pHead->DestZoneId;

	if (family == 0 || family > MAX_FAMILY_COUNT)
		return;

	GetChannelManager()->AddPlayer(playerId, m_FamilyChannels[family - 1]);
}

// 向全世界发送全局消息
void CChatManager::SendMessageToWorld(stChatMessage& chatMessage)
{
	char Buffer[CHAT_PACKET_SIZE];
	Base::BitStream SendPacket(Buffer, sizeof(Buffer));
	stPacketHead *pSendHead = IPacket::BuildPacketHead(SendPacket, "SERVER_CHAT_SendMessageAll", 0, SERVICE_GATESERVER);

	stChatMessage::PackMessage(chatMessage, SendPacket);
	pSendHead->PacketSize = SendPacket.getPosition() - sizeof(stPacketHead);

	int gates[100];
	int count = 100;
	CChatServer::getInstance()->getServerMgr()->getGates(gates, count);

	for (int i = 0; i < count; ++i)
	{
		CChatServer::getInstance()->getServerSocket()->Send(gates[i], SendPacket);
	}
}

//向一组人群内发送消息
void CChatManager::SendMessage(stChatMessage& chatMessage, const std::vector<int>& playerList)
{
	if (0 == playerList.size())
		return;

	std::vector<SendInfo> sends;
	sends.reserve(playerList.size());

	U32 gates[MAX_LINE_COUNT] = { 0 };

	for (size_t i = 0; i < playerList.size(); ++i)
	{
		SendInfo sendInfo = SERVER->getPlayerMgr()->GetSendInfo(playerList[i]);

		if (0 == sendInfo.accountId || sendInfo.lineId >= MAX_LINE_COUNT)
			continue;

		gates[sendInfo.lineId]++;
		sends.push_back(sendInfo);
	}

	char Buffer[MAX_PACKET_SIZE];

	for (U32 i = 1; i < MAX_LINE_COUNT; ++i)
	{
		if (0 == gates[i] || gates[i] >= 60000)
			continue;

		int socketId = 0;

		Base::BitStream SendPacket(Buffer, sizeof(Buffer));
		stPacketHead *pSendHead = IPacket::BuildPacketHead(SendPacket, "SERVER_CHAT_SendMessage", 0, SERVICE_GATESERVER);

		SendPacket.writeInt(gates[i], 16);

		for (size_t iSend = 0; iSend < sends.size(); ++iSend)
		{
			if (sends[iSend].lineId == i)
			{
				SendPacket.writeInt(sends[iSend].accountId, 32);

				if (0 == socketId)
				{
					socketId = sends[iSend].socketId;
				}
			}
		}

		stChatMessage::PackMessage(chatMessage, SendPacket);
		pSendHead->PacketSize = SendPacket.getPosition() - sizeof(stPacketHead);

		SERVER->getServerSocket()->Send(socketId, SendPacket);
	}
}

void CChatManager::SendMessageTo(stChatMessage& chatMessage, int nPlayerId)
{
	SendInfo sendInfo = SERVER->getPlayerMgr()->GetSendInfo(nPlayerId);

	if (0 == sendInfo.accountId || 0 == sendInfo.socketId)
		return;

	char Buffer[CHAT_PACKET_SIZE];
	Base::BitStream SendPacket(Buffer, sizeof(Buffer));
	stPacketHead *pSendHead = IPacket::BuildPacketHead(SendPacket, "SERVER_CHAT_SendMessage", sendInfo.accountId, SERVICE_CLIENT);

	stChatMessage::PackMessage(chatMessage, SendPacket);
	pSendHead->PacketSize = SendPacket.getPosition() - sizeof(stPacketHead);

	SERVER->getServerSocket()->Send(sendInfo.socketId, SendPacket);
}

void CChatManager::SendMessageToLine(stChatMessage& chatMessage, int nLineId)
{
	char Buffer[CHAT_PACKET_SIZE];
	Base::BitStream SendPacket(Buffer, sizeof(Buffer));
	stPacketHead *pSendHead = IPacket::BuildPacketHead(SendPacket, "SERVER_CHAT_SendMessageAll", 0, SERVICE_GATESERVER);

	stChatMessage::PackMessage(chatMessage, SendPacket);
	pSendHead->PacketSize = SendPacket.getPosition() - sizeof(stPacketHead);

	std::vector<int> gateList;
	SERVER->getServerMgr()->getAllGates(gateList, nLineId);

	for (size_t i = 0; i < gateList.size(); ++i)
	{
		CChatServer::getInstance()->getServerSocket()->Send(gateList[i], SendPacket);
	}
}

void CChatManager::SendMessageToChannel(stChatMessage& chatMessage, int nChannelId)
{
	if (0 == nChannelId)
		return;

	std::vector<int> playerList;
	GetChannelManager()->GetPlayerList(nChannelId, playerList);

	SendMessage(chatMessage, playerList);
}

// 用于验证消息合法性
bool CChatManager::IsBaned(stChatMessage& chatMessage)
{

	for (size_t i = 0; i < m_banList.size(); i++)
	{
		switch (m_banList[i].type)
		{
		case ChatBanInfo::TARGET_TYPE_ACC:
		{
			if (0 == strcmp(m_banList[i].name, SERVER->getPlayerMgr()->getAccountName(chatMessage.nSender).c_str()))
			{
				int curTime = (int)time(0);
				return m_banList[i].IsChatBaned(curTime);
			}
		}
		break;
		case ChatBanInfo::TARGET_TYPE_ACTOR:
		{
			if (0 == strcmp(m_banList[i].name, SERVER->getPlayerMgr()->getPlayerName(chatMessage.nSender).c_str()))
			{
				int curTime = (int)time(0);
				return m_banList[i].IsChatBaned(curTime);
			}
		}
		break;
		case ChatBanInfo::TARGET_TYPE_IP:
		{
			if (0 == strcmp(m_banList[i].name, SERVER->getPlayerMgr()->getPlayerIp(chatMessage.nSender).c_str()))
			{
				int curTime = (int)time(0);
				return m_banList[i].IsChatBaned(curTime);
			}
		}
		break;
		}
	}

	return false;
}

U64 CChatManager::GetPlayerChatPendingTime(int nPlayerId, char cMessageType)
{
	__int64 v;
	v = nPlayerId;
	v = (v << 8) | cMessageType;

	OLD_DO_LOCK(m_cs);

	stPlayerChatRecord& rec = m_playerChatMap[v];

	if (rec.nPendingTime == 0)
	{
		switch (cMessageType)
		{
		case CHAT_MSG_TYPE_PRIVATE:
			rec.nPendingTime = CHAT_PENDING_TIME_PRIVATE;
			break;
		case CHAT_MSG_TYPE_WORLDPLUS:
			rec.nPendingTime = CHAT_PENDING_TIME_WORLDPLUS;
			break;
		case CHAT_MSG_TYPE_REALCITY:
			rec.nPendingTime = CHAT_PENDING_TIME_REALCITY;
			break;
		case CHAT_MSG_TYPE_LINE:
			rec.nPendingTime = CHAT_PENDING_TIME_LINE;
			break;
		default:
			rec.nPendingTime = CHAT_PENDING_TIME_NORAML;
			break;
		}
	}

	return rec.nPendingTime;
}

void CChatManager::SetPlayerChatLastTime(int nPlayerId, char cMessageType, U64 nTime)
{
	__int64 v;
	v = nPlayerId;
	v = (v << 8) | cMessageType;

	OLD_DO_LOCK(m_cs);

	m_playerChatMap[v].nLastTime = nTime;
}

U64 CChatManager::GetPlayerChatLastTime(int nPlayerId, char cMessageType)
{
	__int64 v;
	v = nPlayerId;
	v = (v << 8) | cMessageType;

	OLD_DO_LOCK(m_cs);

	CHATREC_MAP::iterator iter = m_playerChatMap.find(v);

	if (iter == m_playerChatMap.end())
		return 0;

	return iter->second.nLastTime;
}

void CChatManager::Update(void)
{
	OLD_DO_LOCK(m_cs);

	__time64_t curTime = time(0);

	for (std::list<AutoMsg>::iterator iter = m_autoMsgs.begin(); iter != m_autoMsgs.end();)
	{
		AutoMsg& autoMsg = *iter;

		if (curTime < autoMsg.startTime)
		{
			++iter;
			continue;
		}

		bool isEnded = false;

		if (autoMsg.freq <= 0 && autoMsg.times <= 0)
		{
			isEnded = true;
		}

		//判断时间是否结束
		if (autoMsg.startTime != autoMsg.endTime)
		{
			if (curTime >= autoMsg.endTime)
			{
				isEnded = true;
			}
		}
		else
		{
			if (autoMsg.freq <= 0 || autoMsg.times <= 0)
				isEnded = true;
		}

		//判断发送次数是否完毕
		if (0 == autoMsg.times)
		{
			isEnded = true;
		}

		if (isEnded)
		{
			iter = m_autoMsgs.erase(iter);
			continue;
		}

		//推进时间
		bool isSend = false;

		if (curTime - autoMsg.lastSendTime >= autoMsg.freq)
		{
			isSend = true;
		}

		if (isSend)
		{
			if (autoMsg.times > 0)
				autoMsg.times--;

			autoMsg.lastSendTime = curTime;

			//发送消息
			if (0 == autoMsg.playerId)
			{
				SendMessageToWorld(autoMsg.msg);
			}
			else
			{
				switch (autoMsg.channel)
				{
				case CHAT_TARGET_PERSON:
				{
					autoMsg.msg.btMessageType = CHAT_MSG_TYPE_PRIVATE;
					SendMessageTo(autoMsg.msg, autoMsg.playerId);
				}
				break;
				// 现在不能自动发送消息到TEAM
				//case CHAT_TARGET_TEAM:
				//	{
				//		//获取玩家所在的team

				//		TeamHandle pTeam = SERVER->GetTeamManager()->GetTeamByPlayer(autoMsg.playerId);

				//		if (0 == pTeam)
				//			return;

				//		autoMsg.msg.btMessageType = CHAT_MSG_TYPE_TEAM;
				//		SendMessageToChannel(autoMsg.msg,pTeam->GetChatChannelId());
				//	}   
				//	break;
				}
			}
		}

		++iter;
	}
}

void CChatManager::AddAutoMsg(AutoMsg& msg)
{
	OLD_DO_LOCK(m_cs);
	m_autoMsgs.push_back(msg);
}

void CChatManager::GetAutoMsg(int sid)
{
	DO_LOCK(Lock::Chat);
	for (std::list<AutoMsg>::iterator iter = m_autoMsgs.begin(); iter != m_autoMsgs.end(); ++iter)
	{
		MAKE_PACKET(sendPacket, "WORLD_CHAT_AutoMsgResponse", sid, SERVICE_CHATSERVER);
		iter->WriteData(&sendPacket);
		SEND_PACKET_NSOCK(sendPacket, SERVER->getWorldSocket());
	}
}

void CChatManager::DelAutoMsg(AutoMsg& msg)
{
	DO_LOCK(Lock::Chat);
	for (std::list<AutoMsg>::iterator iter = m_autoMsgs.begin(); iter != m_autoMsgs.end(); ++iter)
	{
		if (strcmp(iter->msg.szMessage, msg.msg.szMessage) == 0 && iter->channel == msg.channel && iter->endTime == msg.endTime)
		{
			m_autoMsgs.erase(iter);
			break;
		}
	}
}


void CChatManager::CancelAutoMsg()
{
	DO_LOCK(Lock::Chat);
	m_autoMsgs.clear();
}

void CChatManager::writeBadWordPlayer(int playerId, char* pWord, int type)
{
	OLD_DO_LOCK(m_cs);

	char inBuf[256];
	GetCurrentDirectoryA(256, inBuf);
	char path[255];
	sprintf_s(path, "%s\\BadWordPlayer.ini", inBuf);

	char cplayerId[10];
	sprintf_s(cplayerId, "%d", playerId);

	if (ChatBanInfo::SENS == type)
	{
		WritePrivateProfileStringA(cplayerId, pWord, "敏感字符", path);
	}
	else if (ChatBanInfo::SUSP == type)
	{
		WritePrivateProfileStringA(cplayerId, pWord, "可疑字符", path);
	}

}

void CChatManager::readFile(const char* path, int type)
{

	FILE* pf;
	char buff[MAX_BADWORDS_LENGTH];

	fopen_s(&pf, path, "rb");

	if (pf == NULL)
	{
		fopen_s(&pf, path, "wb");

		if (pf == NULL)
		{
			g_Log.WriteError("无法创建%s", path);
			assert(0);
			return;
		}
	}
	else
	{
		g_Log.WriteLog("打开%s成功!", path);

		if (ChatBanInfo::SENS == type)
		{
			ZeroMemory(buff, sizeof(buff));
			while (!feof(pf))
			{
				fgets(buff, sizeof(buff), pf);
				for (int i = 0; i < MAX_BADWORDS_LENGTH; ++i)
				{
					if (13 == buff[i])
						buff[i] = 0;
				}

				std::wstring str;
				A2W_s(buff, str);
				m_sensWords.push_back(str);
			}
		}
		else if (ChatBanInfo::SUSP == type)
		{
			ZeroMemory(buff, sizeof(buff));
			while (!feof(pf))
			{
				fgets(buff, sizeof(buff), pf);
				for (int i = 0; i < MAX_BADWORDS_LENGTH; ++i)
				{
					if (13 == buff[i])
						buff[i] = 0;
				}


				std::wstring str;
				A2W_s(buff, str);
				m_susWords.push_back(str);
			}
		}

	}

	fclose(pf);

}

bool CChatManager::isSusWord(const char* pName)
{
	std::wstring wName;
	A2W_s(pName, wName);

	VEC_WSTR_CONITER iterBegin = m_susWords.begin();
	VEC_WSTR_CONITER iterEnd = m_susWords.end();

	for (; iterBegin != iterEnd; ++iterBegin)
	{
		if (iterBegin->size() > 0 && wcsstr(wName.c_str(), iterBegin->c_str()) != NULL)
			return true;
	}

	return false;
}


bool CChatManager::isSensWord(const char* pName)
{
	std::wstring wName;
	A2W_s(pName, wName);

	VEC_WSTR_CONITER iterBegin = m_sensWords.begin();
	VEC_WSTR_CONITER iterEnd = m_sensWords.end();

	for (; iterBegin != iterEnd; ++iterBegin)
	{
		if (iterBegin->size() > 0 && wcsstr(wName.c_str(), iterBegin->c_str()) != NULL)
			return true;
	}

	return false;
}


ServerEventFunction(CChatServer, HandleAddAutoMsg, "WORLD_CHAT_AddAutoMsg")
{
	AutoMsg msg;
	msg.ReadData(pPack);

	SERVER->getChatMgr()->AddAutoMsg(msg);
	return true;
}

ServerEventFunction(CChatServer, HandleGetAutoMsg, "WORLD_CHAT_GetAutoMsg")
{
	int sid = phead->Id;
	SERVER->getChatMgr()->GetAutoMsg(sid);
	return true;
}

ServerEventFunction(CChatServer, HandleDelAutoMsg, "WORLD_CHAT_DelAutoMsg")
{
	AutoMsg msg;
	msg.ReadData(pPack);

	SERVER->getChatMgr()->DelAutoMsg(msg);
	return true;
}

ServerEventFunction(CChatServer, HandleCancelAutoMsg, "WORLD_CHAT_CancelAutoMsg")
{

	SERVER->getChatMgr()->CancelAutoMsg();
	return true;
}

/*ServerEventFunction( CChatServer, HandleSendMsgToLine, WORLD_CHAT_SendMessageToLine )
{
int lineId = phead->Id;

stChatMessage msg;
stChatMessage::UnpackMessage( msg, *pPack );

SERVER->getChatMgr()->SendMessageToLine(msg, lineId);
return true;
}*/

ServerEventFunction(CChatServer, HandleSendMsgToWorld, "WORLD_CHAT_SendMessageToWorld")
{
	stChatMessage msg;
	stChatMessage::UnpackMessage(msg, *pPack);

	SERVER->getChatMgr()->SendMessageToWorld(msg);
	return true;
}

ServerEventFunction(CChatServer, HandleSendMsgToPlayer, "WORLD_CHAT_SendMessageToPlayer")
{
	stChatMessage msg;
	stChatMessage::UnpackMessage(msg, *pPack);

	SERVER->getChatMgr()->SendMessageTo(msg, phead->Id);
	return true;
}

/*ServerEventFunction( CChatServer, HandleSendMsgToChannel, WORLD_CHAT_SendMessageToChannel )
{
int channelId = phead->Id;

stChatMessage msg;
stChatMessage::UnpackMessage( msg, *pPack );

SERVER->getChatMgr()->SendMessageToChannel( msg, channelId );

return true;
}

ServerEventFunction( CChatServer, HandleJoinCityRequest, CLIENT_CHAT_JoinCityRequest )
{
SERVER->getChatMgr()->HandleJoinCity( socketId, phead, pPack );
return true;
}*/

ServerEventFunction(CChatServer, HandleSendMsg, "CLIENT_CHAT_SendMessageRequest")
{
	SERVER->getChatMgr()->HandleIncomingMsg(socketId, phead, pPack);
	return true;
}

ServerEventFunction(CChatServer, HandleRegisterChannel, "WORLD_CHAT_RegisterChannelRequest")
{
	int type = phead->Id;
	int channelId = SERVER->getChatMgr()->GetChannelManager()->RegisterChannel(type, "");

	if (0 == channelId)
		return true;

	MAKE_PACKET(sendPacket, "WORLD_CHAT_RegisterChannelResponse", phead->SrcZoneId, SERVICE_WORLDSERVER, channelId, phead->DestZoneId, phead->Id);
	SEND_PACKET_NSOCK(sendPacket, SERVER->getWorldSocket());
	return true;
}

ServerEventFunction(CChatServer, HandleUnregisterChannel, "WORLD_CHAT_UnregisterChannelRequest")
{
	SERVER->getChatMgr()->GetChannelManager()->UnregisterChannel(phead->Id);
	return true;
}

ServerEventFunction(CChatServer, HandleAddPlayerToChannel, "WORLD_CHAT_ChannelAddPlayer")
{
	int playerId = phead->Id;
	int channelId = phead->DestZoneId;

	SERVER->getChatMgr()->GetChannelManager()->AddPlayer(playerId, channelId);
	return true;
}

ServerEventFunction(CChatServer, HandleRemovePlayerFromChannel, "WORLD_CHAT_ChannelRemovePlayer")
{
	int playerId = phead->Id;
	int channelId = phead->DestZoneId;

	SERVER->getChatMgr()->GetChannelManager()->RemovePlayer(playerId, channelId);
	return true;
}

/*
ServerEventFunction( CChatServer, HandleZoneAddFamily, ZONE_CHAT_AddToFamily )
{
SERVER->getChatMgr()->HandleAddToFamily(socketId,phead,pPack);
return true;
}

ServerEventFunction( CChatServer, HandleZoneSendMsg, ZONE_CHAT_SendMessageRequest )
{
int lineId = pPack->readInt(Base::Bit32);
SERVER->getChatMgr()->HandleZoneSendMessage( lineId, phead, pPack );
return true;
}


ServerEventFunction( CChatServer, HandleMdfChatBanList, WORLD_CHAT_ChatBanList )
{
int size = pPack->readInt(Base::Bit32);
ChatBanInfo chatBanInfo;
std::vector<ChatBanInfo>& vBanList = SERVER->getChatMgr()->m_banList;

vBanList.clear();
for (int i=0; i<size; i++)
{
chatBanInfo.unpack(pPack);
vBanList.push_back(chatBanInfo);
}

return true;
}*/