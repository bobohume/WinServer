#include "ChatLog.h"
//#include "StringTable.h"
#include "DBLayer/common/DBUtility.h"
#include "JSON/JsonParser.h"
#include "Common/Base64.h"
#include "Common/UtilString.h"
#include "Common/Http/Param.h"
#include "JSON/JsonParser.h"
#include "Common/Http/Request.h"
#include "ChatServer.h"
#include "base/md5.h"

const std::string s_Key = "abc123!*tanwan_game000999";

ChatLog::ChatLog() : m_Error(0), gkey(""), tkey(""), server_id(""), qid(""), name(""), type(5),
				content(""), time(0), ip(""), toqid(""), toname(""), sign("")
{
}

// 返回的字符串需要手动delete
char* ChatLog::LetSpacesGo(const char *pStr)
{
	char* pStrTemp = new char[MAX_RECIEPT_LEN];
	int iPosSrc = 0, iPosDes = 0;
	ZeroMemory(pStrTemp, MAX_RECIEPT_LEN);

	while (*(pStr + iPosSrc) != '\0')
	{
		if (*(pStr + iPosSrc) != '\r' || *(pStr + iPosSrc) != '\n')
		{
			*(pStrTemp + iPosDes) = *(pStr + iPosSrc);
			iPosDes++;
		}

		iPosSrc++;
	}

	return pStrTemp;
}

int ChatLog::Execute(int ctxId,void* param)
{
	if(DB_CONTEXT_LOGIC != ctxId)
	{
		do{
			std::string Url = SERVER->GetChatLogUrl();

			std::ostringstream sMD5;
			sMD5 << gkey
				<< "."
				<< server_id
				<< "."
				<< qid
				<< "."
				<< time
				<< "."
				<< s_Key;
			sign = GetMD5_32(sMD5.str().c_str(), sMD5.str().length());

			int ret = dBase64::encode((unsigned char*)pDstUTF, pDstBass64, strlen(pDstUTF), MAX_RECIEPT_LEN);
			if (0 == ret)
			{
				g_Log.WriteError("[Http_Purchase] Base64 encode error...");
				m_Error = 1;
				break;
			}

			std::string result;
			JsonParser oJsonEncode(new Json::Value());
			oJsonEncode.AddNode("receipt-data", pDstBass64);
			HTTP_RESULTCODE resultCode = CRequest::GetInstance()->SyncHttpsRequest(Url, EHttpMethod_Post, oJsonEncode, result);
			if (resultCode != HTTPRESULT_OK)
			{
				g_Log.WriteError("[Http_Purchase] Fail to request resultCode:%d", resultCode);
				m_Error = 2;
				break;
			}

			//result = "{\"receipt\":{\"original_purchase_date_pst\":\"2013-12-02 23:28:14 America/Los_Angeles\", \"purchase_date_ms\":\"1386055694039\",\"unique_identifier\":\"287a7cd8e82820b98a6cefe69e14e05a62617b7e\", \"original_transaction_id\":\"1000000095350370\", \"bvrs\":\"1.0\", \"transaction_id\":\"1000000095350370\", \"quantity\":\"1\", \"unique_vendor_identifier\":\"1A535C88-A405-4646-856F-7DCA7F17C2FB\",\"item_id\":\"768469970\", \"product_id\":\"com.firerain.db1\", \"purchase_date\":\"2013-12-03 07:28:14 Etc/GMT\", \"original_purchase_date\":\"2013-12-03 07:28:14 Etc/GMT\", \"purchase_date_pst\":\"2013-12-02 23:28:14 America/Los_Angeles\", \"bid\":\"com.firerain.nzd\", \"original_purchase_date_ms\":\"1386055694039\"}, \"status\":0}";
			JsonParser oJsonDncode;
			if (!oJsonDncode.Parse(result.c_str()))
			{
				g_Log.WriteError("[Http_Purchase] Fail to parse jason String...", m_playerId, result.c_str());
				m_Error = 3;
				break;
			}

			if (oJsonDncode.GetValue("status").asInt() == 0)
			{
				JsonParser oReceipt = oJsonDncode.GetChild("receipt");
				m_Quantity			= oReceipt.GetValue("quantity").asString();
				m_Transaction_id	= oReceipt.GetValue("transaction_id").asString();
				m_Product_id		= oReceipt.GetValue("product_id").asString();
				m_Purchase_date		= oReceipt.GetValue("purchase_date_pst").asString();
				m_itemId			= oReceipt.GetValue("item_id").asString();
			}
			else
			{
				g_Log.WriteError("[Http_Purchase] Error status:%d", oJsonDncode.GetValue("status").asInt());
				m_Error = oJsonDncode.GetValue("status").asInt();
				break;
			}

		}while (false);

		SAFE_DELETE_ARRAY(pDstNoSpaces);
		SAFE_DELETE_ARRAY(pDstUTF);
		SAFE_DELETE_ARRAY(pDstBass64);

		PostLogicThread(this);
		return PACKET_NOTREMOVE;
	}
	else
	{
		if (m_Error == 0)
		{
			//获取当前内购的可获得的钱数
			DB_PurchaseRecord* pDBHandle = new DB_PurchaseRecord();
			pDBHandle->playerId = m_playerId;
			pDBHandle->quantity = 1;
			pDBHandle->transaction_id = m_Transaction_id;
			pDBHandle->product_id = m_Product_id;
			pDBHandle->item_id = m_itemId;
			pDBHandle->purchase_date = m_Purchase_date;
			SERVER->GetBillDBManager()->SendPacket(pDBHandle);
		}
		else
			g_Log.WriteWarn("[Http_Purchase] Purchase error!");
	}

	return PACKET_OK;
}

