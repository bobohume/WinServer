#ifndef _GUARANTEEDEVENT_
#define _GUARANTEEDEVENT_

#include <vector>

//Ray: 这个类是用来确保服务器之间一些重要操作的同步的

const int EVENT_VER = 100;
const int MAX_MAIL_ZIP_SIZE = 4096;

//下面这个不能删减，只能增加，否则影响数据库解释
enum GEventType
{
	invalidEvent = 0,

	deleteMailItem,		//删除邮件物品
	deleteLogicEvent,	//删除离线事件
	sendMail,			//发送邮件
	storeBuyItem,		//商城购物
	fixedStallBuyItem,	//商铺购物


	//需要转换的请求
	receiveMailItem,	//收取邮件物品
	receiveLogicEvent,  //收取事件

	totalEvent,
};

enum GEventErr
{
	GE_NONE_ERROR,				//没错误
	GE_MAX_SIZE_ERROR,			//超过最大队列尺寸
	GE_OVERLAP_ERROR,			//重复了一个请求
	GE_TIMEOUT_ERROR,			//请求过期,比如已经发起了一个正在执行的删除请求，又发起一个接收请求会返回这个错误
	GE_INVALID_EVENT,			//无效的事件请求
	GE_UNKNOW_ERROR,			//未知错误
};

struct stLogicData
{
	char mTypeA;
	int  mValueA1;
	int  mValueA2;
	char mTypeB;
	int  mValueB1;
	int  mValueB2;
};

struct stBlobData
{
	int mSize;
	char mBuf[MAX_MAIL_ZIP_SIZE];
};

struct GEventData
{
	int mSrcPlayerId;		//发起玩家
	int mDestPlayerId;		//目标玩家
	int	mLogicId;			//可能需要用到的ID，比如邮件ID，离线事件ID等

	GEventData()
	{
		memset(this,0,sizeof(GEventData));
	}

	union 
	{
		stLogicData mData;		//其他逻辑数据
		stBlobData  mMail;		//独立的发送邮件数据，包括邮件标题，内容，邮件物品，金钱等
	}uData;
};

//这个类不允许改写，防止逻辑被无意识窜改
class GEvent		
{
	friend class CGuaranteedEventMgr;

protected:
	int			ver;		//版本号，用来数据库数据升级用
	int			uid;		//事件的递增编号，这个种子在玩家身上
	int			sqid;		//事件重发的序列递增号
	GEventType	mType;

public:
	GEventData  mEventData;

public:
	GEvent();

public:
	//改写下面方法
	virtual bool packData(void *bitStream);
	virtual bool unpackData(void *bitStream,int ver);
	virtual void onSendData(void *obj) = 0;
	virtual bool onReceiveData(void *obj,int err) = 0;
};

#endif