#ifndef __DBERROR_H__
#define __DBERROR_H__

enum DBError
{
	DBERR_NONE,							//无错误

	DBERR_DB_INITIALIZE,				//初始化数据库连接失败
	DBERR_DB_INTERNALERR,				//数据操作层内部异常
	DBERR_DB_BEGINTRAN,					//启动事务处理失败
	DBERR_DB_COMMITTRAN,				//提交事务处理失败
	DBERR_DB_SQLPARAMETER,				//SQL语句参数错误

	DBERR_IO_READFILE,					//读取本地文件数据失败
	DBERR_IO_WRITEFILE,					//写入本地文件数据失败
	DBERR_IO_CREATEFILE,				//创建本地文件失败
	DBERR_IO_OPENFILE,					//打开本地文件失败
	DBERR_IO_CHECKCRC,					//检查本地文件数据CRC错误

	DBERR_ACCOUNT_CREATE,				//创建帐号失败
	DBERR_ACCOUNT_NOFOUND,				//未找到此帐号用户
	DBERR_ACCOUNT_IDNAMENOMATCH,		//帐号ID与帐号名称不匹配
	DBERR_ACCOUNT_IDEXISTS,				//帐号ID存在
	DBERR_ACCOUNT_NAMEEXISTS,			//帐号名称存在
	DBERR_ACCOUNT_UPDATEGOLD,			//帐号元宝扣值失败

	DBERR_PLAYER_NOTNEWEST,				//角色数据不能转换到最新版本
	DBERR_PLAYER_CONVERTPARAMINVAILD,	//角色数据转换参数不合法
	DBERR_PLAYER_CONVERTFAILED,			//角色数据转换失败
	DBERR_PLAYER_DEFINEERROR,			//角色结构版本定义不正确
	DBERR_PLAYER_NOFOUND,				//未找到指定的角色数据
	DBERR_PLAYER_BLOBUNCOMPRESS,		//玩家离线数据解压错误
	DBERR_PLAYER_BLOBCOMPRESS,			//玩家离线数据压缩错误
	DBERR_PLAYER_BLOBTOOBIG,			//二进制数据过大
	DBERR_PLAYER_BLOBLENGTH,			//二进制数据长度无法确定
	DBERR_PLAYER_NAMEEXISTS,			//角色名已经被使用
	DBERR_PLAYER_TOOMANY,				//角色数太多(超过5个)
	DBERR_PLAYER_NOASSIGNID,			//无法分配角色ID
	DBERR_PLAYER_CREATE,				//创建角色数据失败
	DBERR_PLAYER_UNDELETE,				//恢复删除角色失败

	DBERR_MAIL_UPDATEFAILED,			//更新邮件数据失败
	DBERR_MAIL_RECVERID,				//收件人ID错误
	DBERR_MAIL_RECVERNAME,				//收件名称错误
	DBERR_MAIL_TOOMANY,					//邮件数量达到上限120封
	DBERR_MAIL_NOFOUND,					//未找到此邮件数据

	DBERR_INVENTORY_UPDATE,				//更新包裹栏物品数据失败
	DBERR_EQUIPLIST_UPDATE,				//更新装备栏物品数据失败
	DBERR_BANKLIST_UPDATE,				//更新银行仓库栏品数据失败

	DBERR_BUFF_UPDATEBUFF,				//更新增益BUFF数据失败
	DBERR_BUFF_UPDATEDEBUFF,			//更新减益BUFF数据失败
	DBERR_BUFF_UPDATESYSBUFF,			//更新系统BUFF数据失败
	DBERR_BUFF_UPDATESCHEDULEBUFF,		//更新调度BUFF数据失败

	DBERR_COOLDOWN_UPDATE,				//更新冷却时间表数据失败
	DBERR_FAME_UPDATE,					//更新声望数据失败
	DBERR_LIVESKILL_UPDATE,				//更新生活技能数据失败
	DBERR_PRESCRIPTION_UPDATE,			//更新配方数据失败

	DBERR_MISSION_UPDATE,				//更新任务数据错误
	DBERR_MISSION_UPDATEFLAG,			//更新任务旗标数据错误

	DBERR_PET_UPDATE,					//更新灵兽数据失败
	DBERR_MOUNT_UPDATE,					//更新骑宠数据失败
	
	DBERR_PANELLIST_UPDATE,				//更新快捷栏数据失败
	DBERR_SPLITLIST_UPDATE,				//更新物品分解栏数据失败
	DBERR_COMPOSELIST_UPDATE,			//更新物品合成栏数据失败

	DBERR_SKILLLIST_UPDATE,				//更新技能栏数据失败
	DBERR_SOCIAL_UPDATE,				//更新社会关系数据失败
	DBERR_SPIRIT_UPDATE,				//更新元神数据失败

	DBERR_TIMER_UPDATEITEM,				//更新赠送物品计时器失败
	DBERR_TIMER_UPDATEPETSTUDY,			//更新宠物修行计时器失败

	DBERR_TIMESET_UPDATE,				//更新活动及任务时效设置数据失败
	DBERR_MPLIST_UPDATE,				//保存出师数据失败
	DBERR_LOGICEVENT_SAVE,				//保存系统逻辑事件失败
	DBERR_ZONESETTING_SAVE,				//保存地图公共配置失败

	DBERR_ORG_MASTEREXIST,				//家族族长已经存在
	DBERR_ORG_NAMEEXIST,				//家族名已经存在
	DBERR_ORG_DISBAND,					//解散家族操作失败

	DBERR_TOPPETBLOB_NOFOUND,			//排行榜异兽blob数据未查找到
	DBERR_TOPITEMBLOB_NOFOUND,			//排行榜奇珍blob数据未查找到

	DBERR_LEAGUE_MASTEREXIST,			//联盟盟主已经存在
	DBERR_LEAGUE_NAMEEXIST,				//联盟名称已经存在
	DBERR_LEAGUE_DISBAND,				//解散家族操作失败

	DBERR_GOLDTRADE_REDUCEGOLD,			//元宝交易挂单预扣元宝失败
	DBERR_GOLDTRADE_RETURNGOLD,			//元宝挂单撤消返回元宝失败
	DBERR_GOLDTRADE_NOFOUND,			//元宝挂单已撤消

	DBERR_GOLDEXCHAGE_NOFOUND,			//元宝兑换数据查询未果
	DBERR_GOLDEXCHAGE_CONFIG_NOFOUND,	//元宝兑换配置数据查询未果
	DBERR_GOLDEXCHANGE_DRAWFAILED,		//领取元宝兑换失败

	DBERR_AUCTION_NOFOUND,				//拍卖行商品查询未果
	DBERR_AUCTION_DONTBUYSELF,			//物主不能对自己商品购买
	DBERR_AUCTION_FIXEDPRICEERR,		//购买价格不匹配

	DBERR_UNKNOWERR,					//未知的错误
	DBERR_MAX,
};

extern const char* getDBErr(DBError e);

class ExceptionResult
{
public:
	ExceptionResult(DBError e) { code = e;}
	virtual ~ExceptionResult() {}
	const char* str() {return getDBErr(code);}
	DBError code;			//返回错误码
};

#endif//__DBERROR_H__
