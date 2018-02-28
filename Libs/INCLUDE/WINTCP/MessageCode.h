#ifndef _MESSAGECODE_H_
#define _MESSAGECODE_H_

#include <hash_map>
#include <string>
#include "Base/bitStream.h"
#include "Base/functionLink.h"


struct stPacketHead;

extern void GetMessageCode(const char* szName,U32& m1,U32& m2); 
extern void GetMessageCodeSimple(const char* szName,U32& m1); 
extern U32  GetMessageCodeSimple(const char* szName);
extern const char* ShowMessageMsg(U32 message);


class MessageCode
{
public:
	static MessageCode* Instance(void)
	{
		static MessageCode local;
		return &local;
	}

	MessageCode();
	~MessageCode();
	void Register(const char* messageName,const char* callback);
	template<class T, class U>
	void Register(const char* messageName, T obj, U func)
	{
		U32 message = 0;
		//U32 m2 = 0;
		if (messageName == NULL || func == NULL || obj == NULL)
		{
			g_Log.WriteError("一股不翔的预感-[消息: %s]-[函数]-居然是NULL......", messageName==NULL?"NULL":messageName);
			return;
		}
		//GetMessageCode(messageName,m1,m2);
		GetMessageCodeSimple(messageName,message);
		//U64 message = m1;
		//message = ((message << 32) | m2);
		if (m_callbacks.find(message) != m_callbacks.end())
		{
			if (0 != m_callbacks[message])
			{
				g_Log.WriteWarn("消息[%s]重复注册!!!!!!!!!!!!!",messageName);
				return;
			}
		}

		FunctionLink* pNode =  CreateFunctionMethod<T, U, bool, int, stPacketHead*, Base::BitStream*>(obj, func);
		m_callbacks[message] = pNode;
	}

	template<class T>
	void Register(const char* messageName, T func)
	{
		U32 message = 0;
		//U32 m2 = 0;
		if (messageName == NULL || func == NULL)
		{
			g_Log.WriteError("一股不翔的预感-[消息: %s]-[函数]-居然是NULL......", messageName==NULL?"NULL":messageName);
			return;
		}
		//GetMessageCode(messageName,m1,m2);
		GetMessageCodeSimple(messageName,message);
		//U64 message = m1;
		//message = ((message << 32) | m2);
		if (m_callbacks.find(message) != m_callbacks.end())
		{
			if (0 != m_callbacks[message])
			{
				g_Log.WriteWarn("消息[%s]重复注册!!!!!!!!!!!!!",messageName);
				return;
			}
		}

		FunctionLink* pNode =  CreateFunctionGlobal<T, bool, int, stPacketHead*, Base::BitStream*>(func);
		m_callbacks[message] = pNode;
	}

	bool Trigger(int socketHandle,stPacketHead* pHead,Base::BitStream& RecvPacket);
	bool IsValid(U32 message);
private:
	//static void Export(struct lua_State* L);
	typedef stdext::hash_map<U32,FunctionLink*> MESSAGE_MAP;
	typedef stdext::hash_map<U32,std::string>   MESSAGE_MAPEX;
	MESSAGE_MAP		m_callbacks;
	MESSAGE_MAPEX	m_messages;
};

#define  REGISTER_EVENT_FUNCTION(name, func)	MessageCode::Instance()->Register(name, func)
#define  REGISTER_EVENT_METHOD(name, obj, func) MessageCode::Instance()->Register(name, obj, func)
#define  ISVALID_EVENT(msg) MessageCode::Instance()->IsValid(msg)
#define  TRIGGER_EVENT(socketHandle, pHead, RecvPacket) MessageCode::Instance()->Trigger(socketHandle, pHead, RecvPacket)
#define  SHOWMSG_EVENT(msg) ShowMessageMsg(msg)
#endif