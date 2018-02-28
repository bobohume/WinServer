#ifndef _CLIENTERRORCODE_H_
#define _CLIENTERRORCODE_H_

#include "base/types.h"
#include "Base/bitStream.h"
#include "CommLib/CommLib.h"

#define IMPLEMENT_GETTYPE(type,name)	template<>\
							static U8 getType(type v)\
							{\
								v;\
								return name;\
							}

class ClientErrorCode
{
public:
    ClientErrorCode(void):
      m_sendPacket(m_buf,255)
    {
          mParNum = 0;
    }

    ClientErrorCode(int MsgID) :
        m_sendPacket(m_buf,255)
	{
        mMsgID		= MsgID;
		mParNum     = 0;

        pack();
	}
    
	template<typename T1>
    ClientErrorCode(int MsgID,T1 p1):
        m_sendPacket(m_buf,255)
	{
        mMsgID		= MsgID;
		mParNum     = 1;
		mAny[0]     = p1;
		mParType[0] = any::getType(p1);

        pack();
	}

	template<typename T1,typename T2>
    ClientErrorCode(int MsgID,T1 p1,T2 p2):
        m_sendPacket(m_buf,255)
	{
		mMsgID		= MsgID;
        mParNum     = 2;
		mAny[0]     = p1;
		mParType[0] = any::getType(p1);

		mAny[1]     = p2;
		mParType[1] = any::getType(p2);

        pack();
	}

	template<typename T1,typename T2,typename T3>
    ClientErrorCode(int MsgID,T1 p1,T2 p2,T3 p3):
        m_sendPacket(m_buf,255)
	{
        mMsgID		= MsgID;
		mParNum     = 3;
		mAny[0]     = p1;
		mParType[0] = any::getType(p1);

		mAny[1]     = p2;
		mParType[1] = any::getType(p2);

		mAny[2]     = p3;
		mParType[2] = any::getType(p3);

        pack();
	}

	template<typename T1,typename T2,typename T3,typename T4>
    ClientErrorCode(int MsgID,T1 p1,T2 p2,T3 p3,T4 p4):
        m_sendPacket(m_buf,255)
	{
        mMsgID		= MsgID;
		mParNum     = 4;
		mAny[0]     = p1;
		mParType[0] = any::getType(p1);

		mAny[1]     = p2;
		mParType[1] = any::getType(p2);

		mAny[2]     = p3;
		mParType[2] = any::getType(p3);

		mAny[3]     = p4;
		mParType[3] = any::getType(p4);

        pack();
	}

    Base::BitStream& GetStream(void) {return m_sendPacket;}

    //客户端处理
    void process(Base::BitStream& SendPacket);
protected:
	enum
	{
		MSG_STRING_MAXLEN		= 255,		
		MSG_BIT4				= 4,
		MSG_BIT16				= 16,
	};

	enum
	{
		MAX_PARAMETER = 4,		//最大参数个数

		TYPE_UNKNOW,
		TYPE_U32,
		TYPE_S32,
		TYPE_STRING,
	};

	struct any
	{
		struct abstract_hold
		{
			virtual void* getValue() = 0;
			virtual void pack(Base::BitStream *bstream)   = 0;
			virtual	void unpack(Base::BitStream *bstream) = 0;
			virtual ~abstract_hold(){};
		};

		template<class T>
		struct value_hold : public abstract_hold
		{
			value_hold(T v):_value(v){}

			void pack(Base::BitStream *bstream)
			{
				bstream->write(sizeof(_value),&_value);
			}

			void unpack(Base::BitStream *bstream)
			{
				bstream->read(sizeof(_value),&_value);
			}

			virtual void* getValue()
			{
				return (void*)_value;
			}
		private:
			T _value;
		};

		template<>
		struct value_hold<const char*> : public abstract_hold
		{
			value_hold(const char* v)
			{
				dStrcpy(_value,sizeof(_value),v);
			}

			void pack(Base::BitStream *bstream)
			{
				bstream->writeString(_value,sizeof(_value));
			}

			void unpack(Base::BitStream *bstream)
			{
				bstream->readString(_value,sizeof(_value));
			}

			virtual void* getValue()
			{
				return (void*)&_value;
			}
		private:
			char _value[64];
		};

		template<>
		struct value_hold<char*> : public abstract_hold
		{
			value_hold(char* v)
			{
				dStrcpy(_value,sizeof(_value),v);
			}

			void pack(Base::BitStream *bstream)
			{
				bstream->writeString(_value,sizeof(_value));
			}

			void unpack(Base::BitStream *bstream)
			{
				bstream->readString(_value,sizeof(_value));
			}

			virtual void* getValue()
			{
				return (void*)&_value;
			}
		private:
			char _value[64];
		};

		any(){ pHold = NULL;}

		void setType(U8 uType)
		{
			SAFE_DELETE(pHold);

			switch(uType)
			{
			case TYPE_U32:
				pHold = new value_hold<U32>(0);
				break;
			case TYPE_S32:
				pHold = new value_hold<S32>(0);
				break;
			case TYPE_STRING:
				pHold = new value_hold<const char*>("");
				break;
			default:
				break;
			}
		}

		template<typename T>
		static U8 getType(T v)
		{
			//不支持的类型会产生一个编译时的错误
			void _ComplierError;
			v;
			return TYPE_UNKNOW;
		}

		IMPLEMENT_GETTYPE(U32,TYPE_U32);
		IMPLEMENT_GETTYPE(S32,TYPE_S32);
		IMPLEMENT_GETTYPE(const char*,TYPE_STRING);
		IMPLEMENT_GETTYPE(char*,TYPE_STRING);

		template<typename T>
		any(const T& v){ pHold = new value_hold<T>(v);}

		~any(){ SAFE_DELETE(pHold); }

		template<typename T>
		any& operator=(const T& v)
		{
			SAFE_DELETE(pHold);
			pHold = new value_hold<T>(v);
			return *this;
		}

		abstract_hold* pHold;
	};

	int		mMsgID;							// 固定的消息字符串ID（见BuildPrefix.h的enWarnMessage)

	U8		mParNum;
	U8		mParType[MAX_PARAMETER];
	any		mAny[MAX_PARAMETER];

    char    m_buf[256];
	Base::BitStream m_sendPacket;

	void pack();
	bool unpack(Base::BitStream *bstream);
};


#endif /*_CLIENTERRORCODE_H_*/