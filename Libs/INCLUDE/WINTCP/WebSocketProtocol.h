#ifndef __WEBSOCKETPROTOCOL_H_
#define __WEBSOCKETPROTOCOL_H_
#include <map>
#include <string>
#include "Base/base64.h"
#include "Base/sha1.h"
#include "ISocket.h"

#define MAGIC_KEY "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

enum WEBSOCKET_STATUS {
	WEBSOCKET_UNCONNECT = 0,
	WEBSOCKET_HANDSHARKED = 1,
};

class CWebSocketProtocol
{
	typedef std::map<std::string, std::string> HEADER_MAP;
public:
	enum FrameType 
	{
		// 下一帧与结束
		NEXT_FRAME = 0x0,
		END_FRAME = 0x80,

		ERROR_FRAME = 0xFF00,
		INCOMPLETE_FRAME = 0xFE00,

		OPENING_FRAME = 0x3300,
		CLOSING_FRAME = 0x3400,

		// 未完成的帧
		INCOMPLETE_TEXT_FRAME = 0x01,
		INCOMPLETE_BINARY_FRAME = 0x02,

		// 文本帧与二进制帧
		TEXT_FRAME = 0x81,
		BINARY_FRAME = 0x82,

		PING_FRAME = 0x19,
		PONG_FRAME = 0x1A,

		// 关闭连接
		CLOSE_FRAME = 0x08
	};

	static bool isWebSocketProtocol(char* pData, int nSize);
	CWebSocketProtocol(	int fd);
	~CWebSocketProtocol();
private:
	int fetch_websocket_info(char *msg);
	void print();
	void reset();

	//解析HTTP消息头
	int fetch_fin(char *msg, int &pos);
	int fetch_opcode(char *msg, int &pos);
	int fetch_mask(char *msg, int &pos);
	int fetch_masking_key(char *msg, int &pos);
	int fetch_payload_length(char *msg, int &pos);
	int fetch_payload(char *msg, int &pos);

public:
	int process(OVERLAPPED_PLUS* ov);
	int makeFrame(char *msg,int len);
	int handshark(char* head);
	bool isWebSocketClose();
	bool isWebSocektPing();
private:
	void parse_str(char *request);
	int fetch_http_info(char* head);
	int send_data(char *buff);

private:
	unsigned char mask_;
	unsigned char fin_;
	unsigned char opcode_;
	unsigned char masking_key_[4];
	unsigned int  payload_length_;
	HEADER_MAP header_map_;
	int fd_;
};

#endif //__WEBSOCKETPROTOCOL_H_