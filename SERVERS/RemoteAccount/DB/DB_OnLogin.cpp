//#include "stdafx.h"
#include "DB_OnLogin.h"
#include "DBContext.h"
#include "DBLayer/Common/Define.h"
#include "DBLayer/Common/DBUtility.h"
#include <sstream>
#include <iomanip>
#include "Common/Common.h"

DB_OnLogin::DB_OnLogin(const PollInfo& info)
:info(info)
{
}

int DB_OnLogin::Execute(int ctxId, void* param)
{
	if (DB_CONTEXT_NOMRAL == ctxId)
	{
		CDBConn* pDBConn = (CDBConn*)param;
		IF_ASSERT(pDBConn == NULL)
			return PACKET_ERROR; //TODO: 检查是否释放资源

		ERROR_CODE error = UNKNOW_ERROR;
		int accountId    = 0;
        char result[32]  = {0};
		int flag         = 0;
		int status       = 0;
		MatrixPos matrixPos;
		char seed[MSTRIX_SEED_LENGTH+1] = {0};
		bool bRename = false;
		int TPRFlag = 0, PROMAccountId = 0;
		int AccountFlag[ACCOUNTFLAG_MAX] = {0,0,0,0,0};//帐号的特殊标志(Flag[0]用于记录是否允许创建新门宗,其它备用)

		//推广员的所有下线帐号Id
		std::vector<int> presenteeAccount;

		if (!StrSafeCheck(info.name, 256))
		{
			error = ACCOUNT_ID_ERROR;
			g_Log.WriteWarn("<<WARN>>Account name incorrect(name=%s)", info.name);
		}
		else if (strcmp(info.loginType, "normal")==0 && !StrSafeCheck(info.pass, 256))
		{
			error = PASS9_PWD_ERROR;
			g_Log.WriteWarn("<<WARN>>Account password incorrect(name=%s, password=%s)", info.name, info.pass);
		}
		else
		{			
			try
			{
				M_SQL(pDBConn, "EXEC USP_CHECKACCOUNT_SXZ '%s', 'G%03d', '0'", info.name, CRemoteAccount::getInstance()->GetRemoteID());

				if (pDBConn->More())
                {
					strcpy_s(result, sizeof(result), pDBConn->GetString());
                }
				g_Log.WriteLog("Check account[%s] result = %s.", info.name, result);

				if (strcmp(result, "0001") == 0)
				{
					PostLogicThread(this);
					return PACKET_NOTREMOVE;
				}
				else if(strcmp(result, "0000") == 0)
				{
					if (strcmp(info.loginType, "normal") == 0)
					{
						std::stringstream s;
						s << "EXEC USP_LOGIN_SXZ '";
						s << info.name << "','";
						s << info.pass << "','G";
						s << std::setfill('0') << std::setw(3) << CRemoteAccount::getInstance()->GetRemoteID();
						s << "','',0";					
						pDBConn->SQLExt(s.str().c_str());
						//M_SQL(pDBConn, "EXEC USP_LOGIN_SXZ '%s', '%s', 'G%03d', '', 0 ", name, pass, CRemoteAccount::getInstance()->GetRemoteID());
					}
					else
					{
						std::stringstream s;
						s << "EXEC USP_THIRDLOGIN_SXZ '";
						s << info.name << "','G";
						s << std::setfill('0') << std::setw(3) << CRemoteAccount::getInstance()->GetRemoteID();
						s << "','',0";					
						pDBConn->SQLExt(s.str().c_str());
					}
					if (pDBConn->More())
					{
						strcpy_s(result, sizeof(result), pDBConn->GetString());
						flag = pDBConn->GetInt();
					}
					g_Log.WriteLog("Account[%s] login result = %s.", info.name, result);

					if (strcmp(result, "0000") == 0)
					{
						error = NONE_ERROR;

						M_SQL(pDBConn, "EXECUTE USP_ACCOUNT_QUERYINFO '%s', %d", info.name, info.areaId);
						if (pDBConn->More())
						{
							accountId = pDBConn->GetInt();
							status = pDBConn->GetInt();
							bRename = pDBConn->GetInt() == 1? true : false;
							PROMAccountId = pDBConn->GetInt();
							TPRFlag = pDBConn->GetInt() == 0 ? 1 : 0;
							for(int ii = 0; ii < ACCOUNTFLAG_MAX; ii++)
								AccountFlag[ii] = pDBConn->GetInt();
						}
						g_Log.WriteLog("Account[%s:%d] status is %d.", info.name, accountId, status);

						if (accountId)
						{
							//查询当前帐号的所有下线帐号Id(推广员奖励的上下线关系)
							M_SQL(pDBConn, "EXECUTE USP_QUERYALLPMACCOUNT %d, %d", accountId, info.areaId);
							while(pDBConn->More())
							{
								presenteeAccount.push_back(pDBConn->GetInt());
							}
						}

						if (strcmp(info.loginType, "normal") == 0)
						{
						}
					}
					else if (strcmp(result, "0001") == 0)
                    {
                        g_Log.WriteLog("帐号[%s]登录失败,帐号被冻结",info.name);
                        error = ACCOUNT_FREEZE;
                    }
                    else
					{
                        g_Log.WriteLog("帐号[%s]登录失败,帐号ID错误",info.name);
						error = ACCOUNT_ID_ERROR;
					}
				}
				else
				{
					g_Log.WriteError("帐号检查返回一个未预期的错误结果[%s]", result);
					error = ACCOUNT_ID_ERROR;
				}
			}
			catch (CDBException &e)
			{
				error = ACCOUNT_ID_ERROR;
				int len = strlen(info.pass)+1;
				TempAlloc<char> tmpPass(len*2+1);
				ConvertHex(tmpPass.ptr, len*2+1, (const unsigned char*)info.pass, len);
				len = strlen(info.name)+1;
				TempAlloc<char> tmpName(len*2+1);
				ConvertHex(tmpName.ptr, len*2+1, (const unsigned char*)info.name, len);
				g_Log.WriteFocus("<<ERROR>>msg=%s, dberr=%s, ecode=%d, name=%s, pass=%s, func="__FUNCTION__, e.w_msgtext, e.m_dberrstr, e.m_dberr, tmpName.ptr, tmpPass.ptr);
			}
			catch (...)
			{
				error = ACCOUNT_ID_ERROR;
				g_Log.WriteError("帐号检查数据操作获得一个未知异常");
			}
		}

        if (0 == accountId && NONE_ERROR == error)
        {
            error = ACCOUNT_ID_ERROR;
		    g_Log.WriteError("帐号ID获取失败");
        }

		if (error==MATRIX_CARD_PROTECT || error==MATRIX_CARD_ERROR)
		{
			CMemGuard buf(4096 MEM_GUARD_PARAM);
			Base::BitStream sendPacket(buf.get(), 4096);
			stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, RA_LoginAck, error, SERVICE_ACCOUNTSERVER, info.sId);
			sendPacket.writeString(seed, sizeof(seed));
			matrixPos.WriteData(&sendPacket);
			pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
			CRemoteAccount::getInstance()->getServerSocket()->Send(info.socket, sendPacket);
		}
		else
		{
            g_Log.WriteLog("向帐号[%s]返回登录OK",info.name);

			CMemGuard buf(8192 MEM_GUARD_PARAM);
			Base::BitStream sendPacket(buf.get(), 8192);
			stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, RA_LoginAck, error, SERVICE_ACCOUNTSERVER, info.sId);
			sendPacket.writeString(info.name);
			sendPacket.writeString(info.pass);
			sendPacket.writeInt(accountId, Base::Bit32);
			sendPacket.writeInt(flag, Base::Bit32);
			sendPacket.writeInt(status, Base::Bit32);
			sendPacket.writeInt(GetLoginType(info.loginType), Base::Bit8);
			sendPacket.writeFlag(bRename);
			sendPacket.writeInt(TPRFlag, Base::Bit8);
			sendPacket.writeInt(PROMAccountId, Base::Bit32);
			for(int ii = 0; ii < ACCOUNTFLAG_MAX; ii++)
				sendPacket.writeInt(AccountFlag[ii], Base::Bit8);
			//网络提供商支持


			//发送推广员的所有下线帐号Id
			size_t presenteeCount = presenteeAccount.size();
			if(presenteeCount > MAX_QUERY_PRESENTEE)
				presenteeCount = MAX_QUERY_PRESENTEE;//嵌制到最多600个
			sendPacket.writeInt(presenteeCount, Base::Bit32);
			for(size_t i = 0; i < presenteeCount; i++)
			{
				sendPacket.writeInt(presenteeAccount[i], Base::Bit32);
			}

			pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
			CRemoteAccount::getInstance()->getServerSocket()->Send(info.socket, sendPacket);
		}

		return PACKET_OK;
	}
	else
	{
		return PACKET_OK;
	}
}