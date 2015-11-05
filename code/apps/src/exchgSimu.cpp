#include "exchgSimu.hpp"
#include "thostsoDef.hpp"

#include <sys/select.h> 

#define MRK_PRICE 2

extern int g_timeout_case;
long long gOrderSysID = 1;


ExchgServer::ExchgServer(struct ExchgConf conf)
{
	int iReusePortFlag = 1;
	int	 old_flags = 0;
	
	m_status = NO_TASK;
	m_uiActualClientTotal = 0;

	strcpy(m_exchgSvrIP,conf.exchgSvrIP);
	m_exchgSvrPort = conf.exchgSvrPort;
	strcpy(m_exchangeID,conf.exchangeID);

	if(m_exchgSvrPort<0||m_exchgSvrIP[0]==0)
	{
		g_log.error("Invalid param: m_exchgSvrIP = %s, m_exchgSvrPort = %d\n",m_exchgSvrIP,m_exchgSvrPort);
		return ;
	}
	
	m_exchgConnFd = TcpCreateServer(m_exchgSvrIP,m_exchgSvrPort);
	if(m_exchgConnFd<0)
	{
		g_log.error("Failed to create exchange server !\n");
		return ;
	}
	g_log.info("[ExchgServer]socket %d(IP = %s, Port = %d)\n",m_exchgConnFd,m_exchgSvrIP,m_exchgSvrPort);
	
	old_flags = fcntl(m_exchgConnFd, F_GETFL, 0);
	fcntl(m_exchgConnFd, F_SETFL, old_flags | O_NONBLOCK );
	setsockopt(m_exchgConnFd, SOL_SOCKET, SO_REUSEADDR, &iReusePortFlag, sizeof(iReusePortFlag));
	
	//start thread loop
	Start();
}

ExchgServer::~ExchgServer()
{

}

void ExchgServer::StartNewOrder(ExchgOrder *vOrder)
{
	m_vOrder = vOrder;
	m_uiExpectClientTotal = m_vOrder->uiClientsTotal;
	m_uiActualClientTotal = 0;
	time(&m_tOrderStartTime);
	
	InitAccounts();
	
	m_status = DOING_TASK;
	
	return;
}

void ExchgServer::GetOrderStatus()
{
	unsigned int numDone = 0;
	
	for(unsigned int i=0;i<m_uiActualClientTotal;i++)
	{
		if(DONE == m_aClients[i].IsFinished())
			numDone++;
	}
	
	if(numDone == m_uiExpectClientTotal)
	{
		m_status = NO_TASK;
		DestroyAccounts();
	}
	else
	{
		time_t tNow;
		time(&tNow);
		if(tNow - m_tOrderStartTime >= g_timeout_case)
		{
			m_status = NO_TASK;
			DestroyAccounts();
			g_log.error("case is timeout, will be stopped !\n");
		}
	}
}

int ExchgServer::InitAccounts()
{
	for(unsigned int i=0;i<m_uiExpectClientTotal;i++)
	{
		AccountInitData data;
		data.iFrontID = i;
		data.pOpSet = &m_vOrder->exchgOp[i];		
		strcpy(data.exchangeID,m_exchangeID);
		sprintf(data.traderID,"%u",i);
		
		m_aClients[i].Init(data);
	}
	
	return ATF_SUCC;
}

void ExchgServer::DestroyAccounts()
{
	for(unsigned int i=0;i<m_uiExpectClientTotal;i++)
	{
		m_aClients[i].Destroy();
	}
}

void* ExchgServer::Run(void *)
{
	int ret = 0;	
	int client_fd = 0;	
	struct sockaddr_in cli_addr;	
    struct timeval time_out;

	socklen_t length = (socklen_t )sizeof(struct sockaddr_in);
	
	while(1)
	{
		while(m_status == DOING_TASK)
		{
			fd_set readfds;
			int maxFd = m_exchgConnFd;

			time_out.tv_sec = 1;
			time_out.tv_usec = 0;
			
			FD_ZERO(&readfds);
			FD_SET(m_exchgConnFd,&readfds); 
			for(unsigned int i=0;i<m_uiActualClientTotal;i++)
			{
				int cli_fd = m_aClients[i].GetFd();
				
				if(cli_fd > 0)
				{
					FD_SET(cli_fd,&readfds); 
					maxFd = cli_fd > maxFd ? cli_fd: maxFd;
				}
			}
			
			ret = select(maxFd+1,&readfds,NULL,NULL,&time_out);
			if(ret < 0)
			{
				g_log.error("ExchgServer select error !\n");
				close(m_exchgConnFd);
				break;
			}
			else if (0 == ret)
			{
				g_log.debug("ExchgServer select timeout !\n");
			}
			else
			{
				if(FD_ISSET(m_exchgConnFd, &readfds))
				{
					if ((client_fd = accept(m_exchgConnFd, (struct sockaddr *)&cli_addr, &length))==-1)
					{
						g_log.error("accept error !\n");
						continue;
					}
					else
					{
						if(m_uiActualClientTotal < m_uiExpectClientTotal)
						{
							int  old_flags = fcntl(client_fd, F_GETFL, 0);
							fcntl(client_fd, F_SETFL, old_flags | O_NONBLOCK );
						
							m_aClients[m_uiActualClientTotal++].SetFd(client_fd);
						}
					}
				}
				else
				{
					for(unsigned int i=0;i<m_uiActualClientTotal;i++)
					{
						if(DONE == m_aClients[i].IsFinished())
						{
							continue;
						}
					
						int cli_fd = m_aClients[i].GetFd();
						if(FD_ISSET(cli_fd, &readfds))
						{
							m_aClients[i].HandleMsgFromClient();//非阻塞
						}
					}			
				}
			}

			GetOrderStatus();
		}
		printf("Exchange NO TASK\n");
		sleep(1);
	}
	return NULL;
}




CAccountTrade::CAccountTrade()
{
	m_clientSock = -1;
	m_vOperations = NULL;	
	
	m_connect = WAIT_CONNECTED;
	m_login = NOT_LOGIN;
	
	m_sessionID = INITIAL_VALUE;
	m_maxOrderRef = INITIAL_VALUE;
	m_orderLocalID = INITIAL_VALUE;
	
	m_tcpbuf = new char[TCP_BUFSIZE];
}

CAccountTrade::~CAccountTrade()
{
	delete [] m_tcpbuf;
}

void CAccountTrade::Init(AccountInitData sData)
{
	m_frontID = sData.iFrontID;	
	strcpy(m_exchangeID,sData.exchangeID);
	strcpy(m_acTraderID,sData.traderID);
	m_iActiveUserID = 1;
	
	m_vOperations = sData.pOpSet;
	m_iterCur = m_vOperations->begin();
	m_iterEnd = m_vOperations->end();

	m_iProgress = TODO;
	
	m_iOrderNum = 0;
	m_iCancelOrderNum = 0;
}

void CAccountTrade::Destroy()
{
	m_vOperations = NULL;
	m_clientSock = -1;

	m_mOrder.clear();
	m_mOrderRef2SysID.clear();
	m_mCancelOrder.clear();
}

void CAccountTrade::SetFd(int sockfd)
{
	m_clientSock = sockfd;
}

int CAccountTrade::GetFd()
{
	return m_clientSock;
}

int CAccountTrade::CloseFd()
{
	if(-1 == m_clientSock)
	{
		g_log.error("fd is not exist !");
		return ATF_FAIL;
	}
		
	close(m_clientSock);
	return ATF_SUCC;
}

int CAccountTrade::IsFinished()
{
	return m_iProgress;
}
void CAccountTrade::HandleMsgFromClient()
{
	int ret = 0;
	int rcvMsgSize = 0;
	char msgName[64] = {0};
	char *pBuf = m_tcpbuf;

	if(m_clientSock < 0)
		return;

	memset(m_curOpErrReason,0,sizeof(m_curOpErrReason));
	OP &rcvOpe = *m_iterCur;
					
	ret = MsgIDToMsgName(rcvOpe.msgID,msgName,sizeof(msgName));
	if(ATF_FAIL == ret)
		return;
	
	if(RCV == rcvOpe.direction)
	{
		int expectLen = GotMsgSize(rcvOpe.msgID);
		
		rcvMsgSize = TcpRecvData(m_clientSock, pBuf,expectLen);
		if(rcvMsgSize < 1)
		{
			rcvOpe.result = OPERATE_FAIL;			
			sprintf(rcvOpe.errReason,"[ExchgServer]Failed to rcv %s(%d)",msgName,rcvMsgSize);
			g_log.error("[ExchgServer]Recv %s(%d) error.\n",msgName,rcvMsgSize);
			
			m_iterCur = m_iterEnd - 2;
		}
		else
		{
			char *pBuf = m_tcpbuf;
			if(ATF_FAIL == CheckMsg(rcvOpe.pMsg,rcvOpe.msgSize,pBuf,rcvMsgSize))
			{
				rcvOpe.result = OPERATE_FAIL;
				sprintf(rcvOpe.errReason,"%s",m_curOpErrReason);
				g_log.error("[ExchgServer]Recv %s(%d) error.\n",msgName,rcvMsgSize);
				
				m_iterCur = m_iterEnd - 2;
			}
			else
			{
				if(IS_DEBUG)
				{
					printf_red("[ExchgServer]Recv %s(%dB), right.\n",msgName,rcvMsgSize);
				}
								
				g_log.info("[ExchgServer]Recv %s(%d), right.\n",msgName,rcvMsgSize);
							
				rcvOpe.result = OPERATE_SUCC;
			}
		}
		
		m_iterCur++;
		while((m_iterCur != m_iterEnd))
		{
			memset(m_curOpErrReason,0,sizeof(m_curOpErrReason));
			OP &sendOp = *m_iterCur;			
			MsgIDToMsgName(sendOp.msgID,msgName,sizeof(msgName));
			
			if(SEND == sendOp.direction)
			{
				int result = ATF_SUCC;
				
				switch(sendOp.msgID)
				{
				case MSGID_S2C_TRADE_RSP_CONN:
					result = OnFrontConnected(sendOp.pMsg, m_clientSock);
					break;
					
				case MSGID_S2C_TRADE_RSP_LOGIN:
					result = OnRspUserLogin(sendOp.pMsg, m_clientSock);
					break;
					
				case MSGID_S2C_TRADE_DISCONNECT:
					result = OnFrontDisconnected(sendOp.pMsg, m_clientSock);
					break;
					
				case MSGID_S2C_TRADE_OnRtnOrder:
					result = OnRtnOrder(sendOp.pMsg, m_clientSock);
					break;	
					
				case MSGID_S2C_TRADE_OnRtnTrade:
					result = OnRtnTrade(sendOp.pMsg, m_clientSock);
					break;
					
				case MSGID_S2C_TRADE_OnRspOrderInsert:
					result = OnRspOrderInsert(sendOp.pMsg, m_clientSock);
					break;
					
				case MSGID_S2C_TRADE_OnErrRtnOrderInsert:
					result = OnErrRtnOrderInsert(sendOp.pMsg, m_clientSock);
					break;
					
				case MSGID_S2C_TRADE_OnRspOrderAction:
					result = OnRspOrderAction(sendOp.pMsg, m_clientSock);
					break;
					
				case MSGID_S2C_TRADE_OnErrRtnOrderAction:
					result = OnErrRtnOrderAction(sendOp.pMsg, m_clientSock);
					break;
					
				default:
					{
					}
					break;
				}

				if(ATF_FAIL == result)
				{
					sendOp.result = OPERATE_FAIL;
																
					sprintf(sendOp.errReason,"%s",m_curOpErrReason);
																
					m_iterCur = m_iterEnd - 2;
				}
				else
				{
					sendOp.result = OPERATE_SUCC;
				}

				++m_iterCur;

				//all operations are over
				if(m_iterCur == m_iterEnd)
				{
					if(LIMITED == m_tradeMode)
					{
						m_iProgress = DONE;
										
						if(IS_DEBUG)
						{
							//printf_red("[ExchgServer]:task done \n");
							
							printf_blue("client %u had finished his job.\n",m_frontID);
						}
					}
				}
			}
			else
				return;
		}
	}
}

int CAccountTrade::MsgIDToMsgName(MSG_ID id,char *pMsgName,int len)
{
	if(NULL == pMsgName)
	{
		g_log.error("Pointer pMsgName is null !");
		return ATF_FAIL;
	}

	memset(pMsgName,0,len);
	
	switch(id)
	{
		case MSGID_C2S_TRADE_REQ_CONN:
			{
				strcpy(pMsgName,"MSGID_C2S_TRADE_REQ_CONN");
			}
			break;
		case MSGID_S2C_TRADE_RSP_CONN:
			{
				strcpy(pMsgName,"MSGID_S2C_TRADE_RSP_CONN");
			}
			break;
		case MSGID_C2S_TRADE_REQ_LOGIN:	
			{
				strcpy(pMsgName,"MSGID_C2S_TRADE_REQ_LOGIN");
			}
			break;
		case MSGID_S2C_TRADE_RSP_LOGIN:
			{
				strcpy(pMsgName,"MSGID_S2C_TRADE_RSP_LOGIN");
			}
			break;
		case MSGID_C2S_TRADE_ReqOrderInsert:
			{
				strcpy(pMsgName,"MSGID_C2S_TRADE_ReqOrderInsert");
			}
			break;
		case MSGID_S2C_TRADE_OnRtnOrder: 
			{
				strcpy(pMsgName,"MSGID_S2C_TRADE_OnRtnOrder");
			}
			break;
		case MSGID_S2C_TRADE_OnRtnTrade: 
			{
				strcpy(pMsgName,"MSGID_S2C_TRADE_OnRtnTrade");
			}
			break;
		case MSGID_S2C_TRADE_OnRspOrderInsert:	
			{
				strcpy(pMsgName,"MSGID_S2C_TRADE_OnRspOrderInsert");
			}
			break;
		case MSGID_S2C_TRADE_OnErrRtnOrderInsert:
			{
				strcpy(pMsgName,"MSGID_S2C_TRADE_OnErrRtnOrderInsert");
			}
			break;
		case MSGID_LOCAL_TRADE_RESULT:
			{
				strcpy(pMsgName,"MSGID_LOCAL_TRADE_RESULT");
			}
			break;
		case MSGID_C2S_TRADE_ReqOrderAction:
			{
				strcpy(pMsgName,"MSGID_C2S_TRADE_ReqOrderAction");
			}
			break;
		case MSGID_LOCAL_HANDLE_ORDER_ACTION:
			{
				strcpy(pMsgName,"MSGID_LOCAL_HANDLE_ORDER_ACTION");
			}
			break;
		case	MSGID_S2C_TRADE_DISCONNECT:
			{
				strcpy(pMsgName,"MSGID_S2C_TRADE_DISCONNECT");
			}
			break;
		default:
			{
				g_log.error("Undefined msgID(%d) is received by ExchgServer !\n",id);
				return ATF_FAIL;
			}
			break;
	}
	return 0;
}

void CAccountTrade::RcvMsgFromClient(char * buf, int iBufSize,int *msgSize, int clientSock)
{
	struct timeval time_out;
	fd_set readfds;
	int ret;
	int iNotRcvSize = iBufSize;
	
	do
	{
		time_out.tv_sec = TIMEOUT_SEC;      //s
		time_out.tv_usec = 0;   //us
		
		FD_ZERO(&readfds);
		FD_SET(clientSock,&readfds);

		ret = select(clientSock+1,&readfds,NULL,NULL,&time_out);
		
		if(ret<0)
		{
		   //g_log.error("file=%s,select error\n",__FILE__);
		   close(clientSock);
		   break;
		}
		else if (ret==0)
		{
		   //g_log.warn("file=%s,%s,timeout\n",__FILE__,__FUNCTION__);
		}
		else//can read
		{	
			if(FD_ISSET(clientSock, &readfds))
			{
				memset(buf,0,iNotRcvSize);
				int recvbytes = 0;
				int n=0;
				char * pBuf = &buf[0];
				while(1)
				{
					n = recv(clientSock, (void*)pBuf, iNotRcvSize, 0);
					if(n<0)
					{
							/*if(errno == EINTR||errno == EAGAIN)
							{
								continue;
							}*/

						break;
					}
					else if(n==0)
					{
						g_log.fatal("[ExchgServer]client socket %d closed.\n",clientSock);
						close(clientSock);
						recvbytes = 0;
						break;
					}
					else
					{
						recvbytes += n;
						if(recvbytes < iBufSize)
						{
							pBuf += n;
							iNotRcvSize -= n;
						}
						else if(recvbytes == iBufSize)
						{
							break;
						}
						else
						{
							recvbytes = -1;
							break;
						}
					}
				}

				*msgSize = recvbytes;
			}
		}	
				
	}while(0);
		 
	return ;
}


int CAccountTrade::GotMsgSize(MSG_ID id)
{
	switch(id)
	{
		case MSGID_C2S_TRADE_REQ_CONN:
			{
				return sizeof(MsgHead);
			}
			break;
		case MSGID_S2C_TRADE_RSP_CONN:
			{
				return sizeof(MsgHead);
			}
			break;
		case MSGID_C2S_TRADE_REQ_LOGIN: 
			{
				return sizeof(MsgHead) + sizeof(TradeReqUserLoginToATF);
			}
			break;
		case MSGID_S2C_TRADE_RSP_LOGIN:
			{
				return sizeof(MsgHead) + sizeof(TradeRspUserLoginFromATF);
			}
			break;
		case MSGID_C2S_TRADE_ReqOrderInsert:
			{
				return sizeof(MsgHead) + sizeof(ReqOrderInsertToATF);
			}
			break;
		case MSGID_S2C_TRADE_OnRtnOrder: 
			{
				return sizeof(MsgHead) + sizeof(OnRtnOrderFromATF);
			}
			break;
		case MSGID_S2C_TRADE_OnRtnTrade: 
			{
				return sizeof(MsgHead) + sizeof(OnRtnTradeFromATF);
			}
			break;
		case MSGID_S2C_TRADE_OnRspOrderInsert:	
			{
				return sizeof(MsgHead) + sizeof(OnRspOrderInsertFromATF);
			}
			break;
		case MSGID_S2C_TRADE_OnErrRtnOrderInsert:
			{
				return sizeof(MsgHead) + sizeof(OnErrRtnOrderInsertFromATF);
			}
			break;
		case MSGID_C2S_TRADE_ReqOrderAction:
			{
				return sizeof(MsgHead) + sizeof(ReqOrderActionToATF);
			}
			break;
		case	MSGID_S2C_TRADE_DISCONNECT:
			{
				return sizeof(MsgHead);
			}
			break;
		default:
			{
				g_log.error("Undefined msgID(%d) is received by ExchgServer !\n",id);
				return ATF_FAIL;
			}
			break;
	}
	return ATF_SUCC;
}

int CAccountTrade::CheckMsg(char * pExpectMsg, int expectMsgLen, char *pActualMsg, int actualMsgLen)
{
	char expectMsgName[64] = {0};
	char actualMsgName[64] = {0};

	MsgHead * pExpectHead = (MsgHead *)pExpectMsg;
	MsgHead * pActualHead = (MsgHead *)pActualMsg;

	int expectMsgID = pExpectHead->iMsgID;
	int actualMsgID = ntohs(pActualHead->iMsgID);

	if(expectMsgID != actualMsgID)
	{
		MsgIDToMsgName((MSG_ID)expectMsgID,expectMsgName,sizeof(expectMsgName));
		MsgIDToMsgName((MSG_ID)actualMsgID,actualMsgName,sizeof(actualMsgName));
		
		//g_log.error("MsgID is error,expect %d, actual %d\n",expectMsgID,actualMsgID);		
		//sprintf(m_curOpErrReason,"MsgID is error,expect %d, actual %d",expectMsgID,actualMsgID);

		g_log.error("MsgID is error,expect %s, actual %s\n",expectMsgName,actualMsgName);
		sprintf(m_curOpErrReason,"MsgID is error,expect %s, actual %s",expectMsgName,actualMsgName);
		
		return ATF_FAIL;
	}
	
	char *pExpectMsgBody = pExpectMsg + sizeof(MsgHead);
	char *pActualMsgBody = pActualMsg + sizeof(MsgHead);
	if(ATF_FAIL == CmpMsg(expectMsgID, pExpectMsgBody, pActualMsgBody))
	{
		g_log.error("Msg body is error\n");
		return ATF_FAIL;
	}

	return ATF_SUCC;
}

int CAccountTrade::OnFrontConnected(char *pMsg, int client_fd)
{
	int ret = 0;
	int iSendLen = sizeof(MsgHead) + sizeof(RspInfoFromATF);
	
	ret = TcpSendData(m_clientSock,pMsg,iSendLen);
	if(ret != iSendLen)	
	{
		g_log.error("[ExchgServer]TCP send OnFrontConnected error,expect %dB, actual %dB.\n",iSendLen,ret);

		memset(m_curOpErrReason,0,sizeof(m_curOpErrReason));
		sprintf(m_curOpErrReason,"TCP send OnFrontConnected error,expect %dB, actual %dB.",iSendLen,ret);
								
		return ATF_FAIL;
	}
	else
	{
		if(IS_DEBUG)
		{
			printf_red("[ExchgServer]Send OnFrontConnected(%dB)\n",ret);
		}
		g_log.info("[ExchgServer]Send OnFrontConnected(%dB)\n",ret);
	}
	
	m_connect = CONNECTED;

	return ATF_SUCC;
}


int CAccountTrade::OnFrontDisconnected(char *pMsg,int client_fd)
{
	int ret = 0;
	int iSendLen = sizeof(MsgHead);
	
	ret = TcpSendData(m_clientSock,pMsg,iSendLen);
	if(ret != iSendLen)	
	{
		g_log.error("[ExchgServer]TCP send OnFrontDisconnected error,expect %dB, actual %dB.\n",iSendLen,ret);

		memset(m_curOpErrReason,0,sizeof(m_curOpErrReason));
		sprintf(m_curOpErrReason,"TCP send OnFrontDisconnected error,expect %dB, actual %dB.",iSendLen,ret);
								
		return ATF_FAIL;
	}
	else
	{
		if(IS_DEBUG)
		{
			printf_red("[ExchgServer]Send OnFrontDisconnected(%dB)\n",ret);
		}
								
		g_log.info("[ExchgServer]Send OnFrontDisconnected(%dB)\n",ret);
	}
	
	m_connect = DISCONNECTED;
	m_login = NOT_LOGIN;
	m_sessionID++;

	CloseFd();

	return ATF_SUCC;
}

int CAccountTrade::OnRspUserLogin(char *pMsg,int client_fd)
{
	int ret = 0;
	int iSendLen = sizeof(MsgHead) + sizeof(TradeRspUserLoginFromATF);
	
	TradeRspUserLoginFromATF *pRsp = (TradeRspUserLoginFromATF *)(pMsg + sizeof(MsgHead));
	FillMsgOnRspLogin(pRsp);

	ret = TcpSendData(m_clientSock,pMsg,iSendLen);
	if(ret != iSendLen)	
	{
		g_log.error("[ExchgServer]TCP send OnRspUserLogin error,expect %dB, actual %dB.\n",iSendLen,ret);

		memset(m_curOpErrReason,0,sizeof(m_curOpErrReason));
		sprintf(m_curOpErrReason,"TCP send OnRspUserLogin error,expect %dB, actual %dB.",iSendLen,ret);
								
		return ATF_FAIL;
	}
	else
	{
		if(IS_DEBUG)
		{
			printf_red("[ExchgServer]Send OnRspUserLogin(%dB)\n",ret);
		}
								
		g_log.info("[ExchgServer]Send OnRspUserLogin(%dB)\n",ret);
								
		return ATF_SUCC;
	}
}

int CAccountTrade::OnRtnOrder(char *pMsg,int client_fd)
{
	int iSendLen = 0;
	int ret = 0;

	OnRtnOrderFromATF *pRsp = (OnRtnOrderFromATF *)(pMsg+sizeof(MsgHead));
	int iOrderID = pRsp->order.RequestID;	//利用RequestID存放对应的报单ID				
	std::map<int, ReqOrderInsertToATF>::iterator iter = m_mOrder.find(iOrderID);
	if(iter == m_mOrder.end())
	{
		g_log.error("No order:%d\n",iOrderID);
		
		memset(m_curOpErrReason,0,sizeof(m_curOpErrReason));
		sprintf(m_curOpErrReason,"[%s]No order:%d !\n",__FUNCTION__,iOrderID);
		return ATF_FAIL;
	}
	
	ReqOrderInsertToATF *pReq = &(iter->second);
	FillMsgOnRtnOrder(&pReq->order,&pRsp->order);
	
	iSendLen = sizeof(MsgHead) + sizeof(OnRtnOrderFromATF);	
	ret = TcpSendData(client_fd,pMsg,iSendLen);
	if(ret != iSendLen)
	{
		memset(m_curOpErrReason,0,sizeof(m_curOpErrReason));
		sprintf(m_curOpErrReason,"[%s]send OnRtnOrder,expect %dB, actual %dB.\n",__FUNCTION__,iSendLen,ret);

		g_log.error("[%s]send OnRtnOrder,expect %dB, actual %dB.\n",__FUNCTION__,iSendLen,ret);
		return ATF_FAIL;
	}
	else
	{
		g_log.info("[%s]send OnRtnOrder %dB.\n",__FUNCTION__,ret);
		return ATF_SUCC;
	}
}

int CAccountTrade::OnRtnTrade(char *pMsg,int client_fd)
{
	int iSendLen = 0;
	int ret = 0;

	OnRtnTradeFromATF *pRsp = (OnRtnTradeFromATF *)(pMsg+sizeof(MsgHead));
	int iOrderID = pRsp->trade.SequenceNo;	//利用SequenceNo存放对应的报单ID				
	std::map<int, ReqOrderInsertToATF>::iterator iter = m_mOrder.find(iOrderID);
	if(iter == m_mOrder.end())
	{
		g_log.error("No order:%d\n",iOrderID);
		
		memset(m_curOpErrReason,0,sizeof(m_curOpErrReason));
		sprintf(m_curOpErrReason,"[%s]No order:%d !\n",__FUNCTION__,iOrderID);
		return ATF_FAIL;
	}
	
	ReqOrderInsertToATF *pReq = &(iter->second);
	FillMsgOnRtnTrade(&pReq->order,&pRsp->trade);
	
	iSendLen = sizeof(MsgHead) + sizeof(OnRtnTradeFromATF);	
	ret = TcpSendData(client_fd,pMsg,iSendLen);
	if(ret != iSendLen)
	{
		memset(m_curOpErrReason,0,sizeof(m_curOpErrReason));
		sprintf(m_curOpErrReason,"[%s]send OnRtnTrade,expect %dB, actual %dB.\n",__FUNCTION__,iSendLen,ret);

		g_log.error("[%s]send OnRtnTrade,expect %dB, actual %dB.\n",__FUNCTION__,iSendLen,ret);
		return ATF_FAIL;
	}
	else
	{
		g_log.info("[%s]send OnRtnTrade %dB.\n",__FUNCTION__,ret);
		return ATF_SUCC;
	}
}

int CAccountTrade::OnRspOrderInsert(char *pMsg, int client_fd)
{
	int ret = 0;
	int iSendLen = sizeof(MsgHead) + sizeof(OnRspOrderInsertFromATF);	
	
	OnRspOrderInsertFromATF *pRsp = (OnRspOrderInsertFromATF *)(pMsg+sizeof(MsgHead));
	int iOrderID = pRsp->inputOrder.RequestID;	//利用RequestID存放对应的报单ID				
	std::map<int, ReqOrderInsertToATF>::iterator iter = m_mOrder.find(iOrderID);
	if(iter == m_mOrder.end())
	{
		g_log.error("[%s]No order:%d\n",__FUNCTION__,iOrderID);
		
		memset(m_curOpErrReason,0,sizeof(m_curOpErrReason));
		sprintf(m_curOpErrReason,"[%s]No order:%d !\n",__FUNCTION__,iOrderID);
		
		return ATF_FAIL;
	}

	ReqOrderInsertToATF *pReq = &(iter->second);
	memcpy(&pRsp->inputOrder,pReq,sizeof(pRsp->inputOrder));
		
	ret = TcpSendData(client_fd,pMsg,iSendLen);
	if(ret != iSendLen)
	{
		memset(m_curOpErrReason,0,sizeof(m_curOpErrReason));
		sprintf(m_curOpErrReason,"[%s]send OnRspOrderInsert,expect %dB, actual %dB.\n",__FUNCTION__,iSendLen,ret);

		g_log.error("[%s]send OnRspOrderInsert,expect %dB, actual %dB.\n",__FUNCTION__,iSendLen,ret);
		return ATF_FAIL;
	}
	else
	{
		g_log.info("[%s]send OnRspOrderInsert %dB.\n",__FUNCTION__,ret);
		return ATF_SUCC;
	}
}

int CAccountTrade::OnErrRtnOrderInsert(char *pMsg,int client_fd)
{
	int ret = 0;
	int iSendLen = sizeof(MsgHead) + sizeof(OnErrRtnOrderInsertFromATF);	
	
	OnErrRtnOrderInsertFromATF *pRsp = (OnErrRtnOrderInsertFromATF *)(pMsg+sizeof(MsgHead));
	int iOrderID = pRsp->inputOrder.RequestID;	//利用RequestID存放对应的报单ID				
	std::map<int, ReqOrderInsertToATF>::iterator iter = m_mOrder.find(iOrderID);
	if(iter == m_mOrder.end())
	{
		g_log.error("[%s]No order:%d\n",__FUNCTION__,iOrderID);
		
		memset(m_curOpErrReason,0,sizeof(m_curOpErrReason));
		sprintf(m_curOpErrReason,"[%s]No order:%d !\n",__FUNCTION__,iOrderID);
		
		return ATF_FAIL;
	}

	ReqOrderInsertToATF *pReq = &(iter->second);
	memcpy(&pRsp->inputOrder,pReq,sizeof(pRsp->inputOrder));
		
	ret = TcpSendData(client_fd,pMsg,iSendLen);
	if(ret != iSendLen)
	{
		memset(m_curOpErrReason,0,sizeof(m_curOpErrReason));
		sprintf(m_curOpErrReason,"[%s]send OnErrRtnOrderInsert,expect %dB, actual %dB.\n",__FUNCTION__,iSendLen,ret);

		g_log.error("[%s]send OnErrRtnOrderInsert,expect %dB, actual %dB.\n",__FUNCTION__,iSendLen,ret);
		return ATF_FAIL;
	}
	else
	{
		g_log.info("[%s]send OnErrRtnOrderInsert %dB.\n",__FUNCTION__,ret);
		return ATF_SUCC;
	}
}

int CAccountTrade::OnRspOrderAction(char *pMsg,int client_fd)
{
	int ret = 0;
	int iSendLen = sizeof(MsgHead) + sizeof(OnRspOrderActionFromATF);	

	OnRspOrderActionFromATF *pRsp = (OnRspOrderActionFromATF *)(pMsg+sizeof(MsgHead));
	int iCancelID = pRsp->inputOrder.RequestID;	//利用RequestID存放对应的撤单ID				
	std::map<int, ReqOrderActionToATF>::iterator iter = m_mCancelOrder.find(iCancelID);
	if(iter == m_mCancelOrder.end())
	{
		g_log.error("[%s]No cancel order:%d\n",__FUNCTION__,iCancelID);
		
		memset(m_curOpErrReason,0,sizeof(m_curOpErrReason));
		sprintf(m_curOpErrReason,"[%s]No cancel order:%d !\n",__FUNCTION__,iCancelID);
		
		return ATF_FAIL;
	}

	ReqOrderActionToATF *pReq = &(iter->second);
	memcpy(&pRsp->inputOrder,pReq,sizeof(pRsp->inputOrder));
		
	ret = TcpSendData(client_fd,pMsg,iSendLen);
	if(ret != iSendLen)
	{
		memset(m_curOpErrReason,0,sizeof(m_curOpErrReason));
		sprintf(m_curOpErrReason,"[%s]send OnRspOrderAction,expect %dB, actual %dB.\n",__FUNCTION__,iSendLen,ret);

		g_log.error("[%s]send OnRspOrderAction,expect %dB, actual %dB.\n",__FUNCTION__,iSendLen,ret);
		return ATF_FAIL;
	}
	else
	{
		g_log.info("[%s]send OnRspOrderAction %dB.\n",__FUNCTION__,ret);
		return ATF_SUCC;
	}

}

int CAccountTrade::OnErrRtnOrderAction(char *pMsg,int client_fd)
{
	int ret = 0;
	int iSendLen = sizeof(MsgHead) + sizeof(OnErrRtnOrderActionFromATF);	

	OnErrRtnOrderActionFromATF *pRsp = (OnErrRtnOrderActionFromATF *)(pMsg+sizeof(MsgHead));
	int iCancelID = pRsp->inputOrder.RequestID;	//利用RequestID存放对应的撤单ID				
	std::map<int, ReqOrderActionToATF>::iterator iter = m_mCancelOrder.find(iCancelID);
	if(iter == m_mCancelOrder.end())
	{
		g_log.error("[%s]No cancel order:%d\n",__FUNCTION__,iCancelID);
		
		memset(m_curOpErrReason,0,sizeof(m_curOpErrReason));
		sprintf(m_curOpErrReason,"[%s]No cancel order:%d !\n",__FUNCTION__,iCancelID);
		
		return ATF_FAIL;
	}

	ReqOrderActionToATF *pReq = &(iter->second);
	FillMsgOnErrRtnOrderAction(&pReq->order,&pRsp->inputOrder);
	//memcpy(&pRsp->inputOrder,pReq,sizeof(pRsp->inputOrder));
		
	ret = TcpSendData(client_fd,pMsg,iSendLen);
	if(ret != iSendLen)
	{
		memset(m_curOpErrReason,0,sizeof(m_curOpErrReason));
		sprintf(m_curOpErrReason,"[%s]send OnErrRtnOrderAction,expect %dB, actual %dB.\n",__FUNCTION__,iSendLen,ret);

		g_log.error("[%s]send OnErrRtnOrderAction,expect %dB, actual %dB.\n",__FUNCTION__,iSendLen,ret);
		return ATF_FAIL;
	}
	else
	{
		g_log.info("[%s]send OnErrRtnOrderAction %dB.\n",__FUNCTION__,ret);
		return ATF_SUCC;
	}
}

int CAccountTrade::CmpMsg(int iMsgID, char * pExpectMsg, char *pActualMsg)
{
	switch(iMsgID)
	{
		case MSGID_C2S_TRADE_REQ_CONN:
			{
				return ATF_SUCC;
			}
			break;
			
		case MSGID_C2S_TRADE_REQ_LOGIN:
			{
				TradeReqUserLoginToATF *pExpect,*pActual;
				pExpect = (TradeReqUserLoginToATF *)pExpectMsg;
				pActual = (TradeReqUserLoginToATF *)pActualMsg;
			
				return CmpMsgReqUserLogin(pExpect,pActual);
			}
			break;
			
		case MSGID_C2S_TRADE_ReqOrderInsert:
			{
				ReqOrderInsertToATF *pExpect = (ReqOrderInsertToATF *)pExpectMsg;
				ReqOrderInsertToATF *pActual = (ReqOrderInsertToATF *)pActualMsg;
				
				return CmpMsgReqOrderInsert(pExpect,pActual);
			}
			break;

		case MSGID_C2S_TRADE_ReqOrderAction:
			{
				ReqOrderActionToATF *pExpect = (ReqOrderActionToATF *)pExpectMsg;
				ReqOrderActionToATF *pActual = (ReqOrderActionToATF *)pActualMsg;
				
				return CmpMsgReqOrderAction(pExpect,pActual);

				//printf_red("*******OrderSysID:%s\n",pActual->order.OrderSysID);
				//printf_red("*******ExchangeID:%s\n",pActual->order.ExchangeID);
	
				return ATF_SUCC;
			}
			break;
		default:
			{
				g_log.error("Invalid msg received !:%d\n",iMsgID);
				
				sprintf(m_curOpErrReason,"Invalid msg received !:%d",iMsgID);
				return ATF_FAIL;
			}
			break;
	};
	
	
}

int CAccountTrade::CmpMsgReqUserLogin(TradeReqUserLoginToATF * pExpectMsg,TradeReqUserLoginToATF *pActualMsg)
{

	int ret = ATF_SUCC;
	CThostFtdcReqUserLoginField *pExpect, *pActual;
	pExpect = &pExpectMsg->reqUserLogin;
	pActual = &pActualMsg->reqUserLogin;

	//保存以填入回复中
	strcpy(m_userLogined.BrokerID,pActual->BrokerID);
	strcpy(m_userLogined.UserID,pActual->UserID);
	
    if(0 != strcasecmp(pExpect->UserID,pActual->UserID))
    {
		g_log.error("Invalid UserID:expect %s,actual %s\n",pExpect->UserID,pActual->UserID);
		sprintf(m_curOpErrReason,"Invalid UserID:expect %s,actual %s",pExpect->UserID,pActual->UserID);
		m_userLogined.ErrorID = 11;
		ret = ATF_FAIL;
	}

	if(0 != strcasecmp(pExpect->Password,pActual->Password))
	{
		g_log.error("Invalid Password:expect %s,actual %s\n",pExpect->Password,pActual->Password);
		sprintf(m_curOpErrReason,"Invalid Password:expect %s,actual %s",pExpect->Password,pActual->Password);
		m_userLogined.ErrorID = 14;
		ret = ATF_FAIL;
	}

	/*if(0 != strcasecmp(pExpect->BrokerID,pActual->BrokerID))
	{
		g_log.error("Invalid BrokerID:expect %s,actual %s\n",pExpect->BrokerID,pActual->BrokerID);
		sprintf(m_curOpErrReason,"Invalid BrokerID:expect %s,actual %s",pExpect->BrokerID,pActual->BrokerID);
		m_userLogined.ErrorID = 12;
		ret = ATF_FAIL;
	}*/

	if(ATF_FAIL == ret)
		return ATF_FAIL;
	else
	{
		m_userLogined.ErrorID = 0;
		return ATF_SUCC;
	}
}

int CAccountTrade::CmpMsgReqOrderInsert(ReqOrderInsertToATF * pExpectMsg,ReqOrderInsertToATF *pActualMsg)
{
	int ret = ATF_SUCC;

    if(pExpectMsg->order.CombOffsetFlag[0] != pActualMsg->order.CombOffsetFlag[0])
	{
		g_log.error("Invalid CombOffsetFlag:expect %s,actual %s\n",pExpectMsg->order.CombOffsetFlag,pActualMsg->order.CombOffsetFlag);
		sprintf(m_curOpErrReason,"Invalid CombOffsetFlag:expect %s,actual %s",pExpectMsg->order.CombOffsetFlag,pActualMsg->order.CombOffsetFlag);
		ret = ATF_FAIL;
	}

    if(pExpectMsg->order.OrderPriceType != pActualMsg->order.OrderPriceType)
	{
		g_log.error("Invalid OrderPriceType:expect %d,actual %d\n",pExpectMsg->order.OrderPriceType,pActualMsg->order.OrderPriceType);
		sprintf(m_curOpErrReason,"Invalid OrderPriceType:expect %d,actual %d",pExpectMsg->order.OrderPriceType,pActualMsg->order.OrderPriceType);
		ret = ATF_FAIL;
	}

    if(pExpectMsg->order.TimeCondition != pActualMsg->order.TimeCondition)
	{
		g_log.error("Invalid TimeCondition:expect %d,actual %d\n",pExpectMsg->order.TimeCondition,pActualMsg->order.TimeCondition);
		sprintf(m_curOpErrReason,"Invalid TimeCondition:expect %d,actual %d",pExpectMsg->order.TimeCondition,pActualMsg->order.TimeCondition);
		ret = ATF_FAIL;
	}

    if(pExpectMsg->order.VolumeCondition != pActualMsg->order.VolumeCondition)
	{
		g_log.error("Invalid VolumeCondition:expect %ds,actual %d\n",pExpectMsg->order.VolumeCondition,pActualMsg->order.VolumeCondition);
		sprintf(m_curOpErrReason,"Invalid VolumeCondition:expect %d,actual %d",pExpectMsg->order.VolumeCondition,pActualMsg->order.VolumeCondition);
		ret = ATF_FAIL;
	}

    if(pExpectMsg->order.ContingentCondition != pActualMsg->order.ContingentCondition)
	{
		g_log.error("Invalid ContingentCondition:expect %d,actual %d\n",pExpectMsg->order.ContingentCondition,pActualMsg->order.ContingentCondition);
		sprintf(m_curOpErrReason,"Invalid ContingentCondition:expect %d,actual %d",pExpectMsg->order.ContingentCondition,pActualMsg->order.ContingentCondition);
		ret = ATF_FAIL;
	}

    if(pExpectMsg->order.LimitPrice!= pActualMsg->order.LimitPrice)
	{
		g_log.error("Invalid LimitPrice:expect %f,actual %f\n",pExpectMsg->order.LimitPrice,pActualMsg->order.LimitPrice);
		sprintf(m_curOpErrReason,"Invalid LimitPrice:expect %f,actual %f",pExpectMsg->order.LimitPrice,pActualMsg->order.LimitPrice);
		ret = ATF_FAIL;
	}

    if(pExpectMsg->order.VolumeTotalOriginal != pActualMsg->order.VolumeTotalOriginal)
	{
		g_log.error("Invalid VolumeTotalOriginal:expect %d,actual %d\n",pExpectMsg->order.VolumeTotalOriginal,pActualMsg->order.VolumeTotalOriginal);
		sprintf(m_curOpErrReason,"Invalid VolumeTotalOriginal:expect %d,actual %d",pExpectMsg->order.VolumeTotalOriginal,pActualMsg->order.VolumeTotalOriginal);
		ret = ATF_FAIL;
	}

    if(pExpectMsg->order.Direction != pActualMsg->order.Direction)
	{
		g_log.error("Invalid Direction:expect %d,actual %d\n",pExpectMsg->order.Direction,pActualMsg->order.Direction);
		sprintf(m_curOpErrReason,"Invalid Direction:expect %d,actual %d",pExpectMsg->order.Direction,pActualMsg->order.Direction);
		ret = ATF_FAIL;
	}

	if(ATF_SUCC == ret)
	{
		m_mOrder.insert(pair<int,ReqOrderInsertToATF>(m_iOrderNum++,*pActualMsg));
		
		++gOrderSysID;
		char sysID[64] = {0};
		sprintf(sysID,"%lld",gOrderSysID);
		m_mOrderRef2SysID.insert(pair<std::string,std::string>(pActualMsg->order.OrderRef,sysID));
		
		if(IS_DEBUG)
		{
			static long long iOrderIn = 0;
			printf_red("order pushed into vector %lld\n",iOrderIn);
			iOrderIn++;
		}
		
		return ATF_SUCC;
	}
	else
	{
		return ATF_FAIL;
	}
}

int CAccountTrade::CmpMsgReqOrderAction(ReqOrderActionToATF * pExpectMsg,ReqOrderActionToATF *pActualMsg)
{
	//OrderRef最初保存的是该次用例的报单ID,
	//根据报单ID从m_mOrder中获得撤单对应的报单OrderRef,然后比较
	int iOrderID = atoi(pExpectMsg->order.OrderRef);
	std::map<int, ReqOrderInsertToATF>::iterator iter = m_mOrder.find(iOrderID);
	if(iter == m_mOrder.end())
	{
		g_log.error("[%s]No order:%d\n",__FUNCTION__,iOrderID);
		
		memset(m_curOpErrReason,0,sizeof(m_curOpErrReason));
		sprintf(m_curOpErrReason,"[%s]No order:%d !\n",__FUNCTION__,iOrderID);
		return ATF_FAIL;
	}

	if(0 != strcmp(iter->second.order.OrderRef,pActualMsg->order.OrderRef))
	{
		g_log.error("[%s]wrong OrderRef,expect %s,actual %s.\n",__FUNCTION__,iter->second.order.OrderRef,pActualMsg->order.OrderRef);
		
		memset(m_curOpErrReason,0,sizeof(m_curOpErrReason));
		sprintf(m_curOpErrReason,"[%s]wrong OrderRef,expect %s,actual %s.\n",__FUNCTION__,iter->second.order.OrderRef,pActualMsg->order.OrderRef);
		return ATF_FAIL;
	}

	//根据报单ID从m_mOrderRef2SysID中获得撤单对应的报单OrderSysID,然后比较
	std::map<std::string,std::string>::iterator iter2 = m_mOrderRef2SysID.find(iter->second.order.OrderRef);
	if(iter2 == m_mOrderRef2SysID.end())
	{
		g_log.error("[%s]No order(OrderRef = %s)\n",__FUNCTION__,iter->second.order.OrderRef);
		
		memset(m_curOpErrReason,0,sizeof(m_curOpErrReason));
		sprintf(m_curOpErrReason,"[%s]No order(OrderRef = %s)\n",__FUNCTION__,iter->second.order.OrderRef);
		return ATF_FAIL;
	}

	if(0 != strcmp(iter2->second.c_str(),pActualMsg->order.OrderSysID))
	{
		g_log.error("[%s]wrong OrderSysID,expect %s,actual %s.\n",__FUNCTION__,iter2->second.c_str(),pActualMsg->order.OrderRef);
		
		memset(m_curOpErrReason,0,sizeof(m_curOpErrReason));
		sprintf(m_curOpErrReason,"[%s]wrong OrderSysID,expect %s,actual %s.\n",__FUNCTION__,iter2->second.c_str(),pActualMsg->order.OrderRef);
		return ATF_FAIL;
	}
	
	m_mCancelOrder.insert(pair<int,ReqOrderActionToATF>(m_iCancelOrderNum++,*pActualMsg));
	
	return ATF_SUCC;
}

int CAccountTrade::FillMsgOnRspLogin(TradeRspUserLoginFromATF *pRsp)
{
	GotCurrentDate(pRsp->rspUserLogin.TradingDay);
	GotCurrentTime(pRsp->rspUserLogin.LoginTime);
	
	GotCurrentTime(pRsp->rspUserLogin.SHFETime);
	GotCurrentTime(pRsp->rspUserLogin.CZCETime);
	GotCurrentTime(pRsp->rspUserLogin.DCETime);
	GotCurrentTime(pRsp->rspUserLogin.FFEXTime);

	strcpy(pRsp->rspUserLogin.SystemName,"TradingHosting");
	
	pRsp->rspUserLogin.FrontID = m_frontID;
	pRsp->rspUserLogin.SessionID = m_sessionID;	
	sprintf(pRsp->rspUserLogin.MaxOrderRef,"%lld", m_maxOrderRef);
	
	memcpy(&pRsp->rspUserLogin.UserID[0] , &m_userLogined.UserID[0], sizeof(pRsp->rspUserLogin.UserID));
	memcpy(&pRsp->rspUserLogin.BrokerID[0] , &m_userLogined.BrokerID[0], sizeof(pRsp->rspUserLogin.BrokerID));
	
	//pRsp->rspInfo.ErrorID = m_userLogined.ErrorID;
	m_connect = CONNECTED;
	m_login = LOGINED;

	return ATF_SUCC;
}

int CAccountTrade::FillMsgOnRtnOrder(CThostFtdcInputOrderField *pReq,CThostFtdcOrderField *pRsp)
{
	if((NULL == pReq)||(NULL == pRsp))
	{
		g_log.error("[%s]Input param is NULL !\n",__FUNCTION__);
		return ATF_FAIL;
	}
	
	memcpy(pRsp->BrokerID,pReq->BrokerID,sizeof(pRsp->BrokerID));
	memcpy(pRsp->InvestorID,pReq->InvestorID,sizeof(pRsp->InvestorID));
	memcpy(pRsp->InstrumentID,pReq->InstrumentID,sizeof(pRsp->InstrumentID));
	memcpy(pRsp->OrderRef,pReq->OrderRef,sizeof(pRsp->OrderRef));
	memcpy(pRsp->UserID,pReq->UserID,sizeof(pRsp->UserID));
	pRsp->OrderPriceType = pReq->OrderPriceType;
	pRsp->Direction = pReq->Direction;
	memcpy(pRsp->CombOffsetFlag,pReq->CombOffsetFlag,sizeof(pRsp->CombOffsetFlag));
	memcpy(pRsp->CombHedgeFlag,pReq->CombHedgeFlag,sizeof(pRsp->CombHedgeFlag));
	pRsp->LimitPrice = pReq->LimitPrice;
	pRsp->TimeCondition = pReq->TimeCondition;
	memcpy(pRsp->GTDDate,pReq->GTDDate,sizeof(pRsp->GTDDate));
	pRsp->VolumeCondition = pReq->VolumeCondition;
	pRsp->MinVolume = pReq->MinVolume;
	pRsp->ContingentCondition = pReq->ContingentCondition;
	pRsp->StopPrice = pReq->StopPrice;
	pRsp->ForceCloseReason = pReq->ForceCloseReason;
	pRsp->IsAutoSuspend = pReq->IsAutoSuspend;
	memcpy(pRsp->BusinessUnit,pReq->BusinessUnit,sizeof(pRsp->BusinessUnit));
	
	pRsp->RequestID = pReq->RequestID;

    memcpy(pRsp->ExchangeID,m_exchangeID,sizeof(pRsp->ExchangeID));
	memcpy(pRsp->ExchangeInstID,pReq->InstrumentID,sizeof(pRsp->ExchangeInstID));

    GotCurrentDate(pRsp->InsertDate);
	GotCurrentDate(pRsp->TradingDay);
	GotCurrentTime(pRsp->InsertTime);
	
	pRsp->FrontID = m_frontID;
	pRsp->SessionID = m_sessionID;
	sprintf(pRsp->OrderLocalID,"%lld",m_orderLocalID);	
	memcpy(pRsp->TraderID,m_acTraderID,sizeof(pRsp->TraderID));
	
	pRsp->VolumeTotalOriginal = pReq->VolumeTotalOriginal;
	//pRsp->VolumeTotal = pReq->VolumeTotalOriginal - pRsp->VolumeTraded;

	if(THOST_FTDC_OST_Unknown == pRsp->OrderStatus)
	{
		memset(pRsp->OrderSysID,0,sizeof(pRsp->OrderSysID));
	}
	else if((THOST_FTDC_OST_AllTraded == pRsp->OrderStatus)||(THOST_FTDC_OST_NoTradeQueueing == pRsp->OrderStatus))
	{
		std::map<std::string, std::string>::iterator iter = m_mOrderRef2SysID.find(pReq->OrderRef);
		if(iter != m_mOrderRef2SysID.end())
		{
			strcpy(pRsp->OrderSysID,iter->second.c_str());
		}
		else
		{
			g_log.error("No found OrderRef:%s\n",pReq->OrderRef);
		}
	}

	return ATF_SUCC;
}

int CAccountTrade::FillMsgOnRtnTrade(CThostFtdcInputOrderField *pReq,CThostFtdcTradeField *pRsp)
{
	if((NULL == pReq)||(NULL == pRsp))
	{
		g_log.error("[%s]Input param is NULL !\n",__FUNCTION__);
		return ATF_FAIL;
	}
	
	memcpy(pRsp->BrokerID,pReq->BrokerID,sizeof(pRsp->BrokerID));
	memcpy(pRsp->InvestorID,pReq->InvestorID,sizeof(pRsp->InvestorID));
	memcpy(pRsp->InstrumentID,pReq->InstrumentID,sizeof(pRsp->InstrumentID));	
	memcpy(pRsp->OrderRef,pReq->OrderRef,sizeof(pRsp->OrderRef));
	memcpy(pRsp->UserID,pReq->UserID,sizeof(pRsp->UserID));
	memcpy(pRsp->ExchangeInstID,pReq->InstrumentID,sizeof(pRsp->ExchangeInstID));
	pRsp->Direction = pReq->Direction;
	memcpy(pRsp->ExchangeID,m_exchangeID,sizeof(pRsp->ExchangeID));
	
	sprintf(pRsp->OrderLocalID,"%lld",m_orderLocalID);
	memcpy(pRsp->TraderID,m_acTraderID,sizeof(pRsp->TraderID));
	
	std::map<std::string, std::string>::iterator iter = m_mOrderRef2SysID.find(pReq->OrderRef);	
	if(iter != m_mOrderRef2SysID.end())
	{
		strcpy(pRsp->OrderSysID,iter->second.c_str());
	}
	else
	{
		g_log.error("No found OrderRef:%s\n",pReq->OrderRef);
	}

	return ATF_SUCC;

}



int CAccountTrade::FillRtnOrderActionCommonPart(CThostFtdcInputOrderActionField *pReq,CThostFtdcOrderField *pRsp)
{
	memcpy(pRsp->BrokerID,pReq->BrokerID,sizeof(pRsp->BrokerID));
	memcpy(pRsp->InvestorID,pReq->InvestorID,sizeof(pRsp->InvestorID));
	memcpy(pRsp->UserID,pReq->UserID,sizeof(pRsp->UserID));
	memcpy(pRsp->InstrumentID,pReq->InstrumentID,sizeof(pRsp->InstrumentID));
	memcpy(pRsp->ExchangeInstID,pReq->InstrumentID,sizeof(pRsp->ExchangeInstID));
    GotCurrentDate(pRsp->InsertDate);
	GotCurrentDate(pRsp->TradingDay);
	GotCurrentTime(pRsp->InsertTime);
	
	sprintf(pRsp->OrderLocalID,"%lld",m_orderLocalID);
	
	pRsp->FrontID = m_frontID;
	pRsp->SessionID = pReq->SessionID;
	
	memcpy(pRsp->TraderID,m_acTraderID,sizeof(pRsp->TraderID));
	
	memcpy(pRsp->OrderRef,pReq->OrderRef,sizeof(pRsp->OrderRef));	

    memcpy(pRsp->ExchangeID,m_exchangeID,sizeof(pRsp->ExchangeID));

	return ATF_SUCC;

}

int CAccountTrade::FillMsgOnErrRtnOrderAction(CThostFtdcInputOrderActionField *pReq,CThostFtdcOrderActionField *pRsp)
{
	if(NULL == pReq || NULL == pRsp)
	{
		g_log.error("Input param is NULL !\n");
		return ATF_FAIL;
	}
	
	memcpy(pRsp->BrokerID,pReq->BrokerID,sizeof(pRsp->BrokerID));
	memcpy(pRsp->InvestorID,pReq->InvestorID,sizeof(pRsp->InvestorID));
	memcpy(pRsp->InstrumentID,pReq->InstrumentID,sizeof(pRsp->InstrumentID));
	memcpy(pRsp->OrderRef,pReq->OrderRef,sizeof(pRsp->OrderRef));
	memcpy(pRsp->UserID,pReq->UserID,sizeof(pRsp->UserID));
	pRsp->RequestID = pReq->RequestID;
	pRsp->FrontID = m_frontID;
	pRsp->SessionID = pReq->SessionID;
    memcpy(pRsp->ExchangeID,m_exchangeID,sizeof(pRsp->ExchangeID));
	pRsp->LimitPrice = pReq->LimitPrice;
	pRsp->OrderActionRef = pReq->OrderActionRef;
	memcpy(pRsp->OrderSysID,pReq->OrderSysID,sizeof(pRsp->OrderSysID));
	pRsp->ActionFlag = pReq->ActionFlag;
	pRsp->VolumeChange = pReq->VolumeChange;

	GotCurrentDate(pRsp->ActionDate);
	GotCurrentTime(pRsp->ActionTime);
	memcpy(pRsp->TraderID,m_acTraderID,sizeof(pRsp->TraderID));

	pRsp->OrderActionStatus = THOST_FTDC_OAS_Rejected;
	
	///安装编号
	//TThostFtdcInstallIDType	InstallID;
	///本地报单编号
	//TThostFtdcOrderLocalIDType	OrderLocalID;
	///操作本地编号
	//TThostFtdcOrderLocalIDType	ActionLocalID;
	///会员代码
	//TThostFtdcParticipantIDType	ParticipantID;
	///客户代码
	//TThostFtdcClientIDType	ClientID;
	///业务单元
	//TThostFtdcBusinessUnitType	BusinessUnit;

	return ATF_SUCC;	
}

void CAccountTrade::SetTradeMode(TRADE_MODE mode)
{
	m_tradeMode = mode;
}


int CAccountTrade::RspUserAuthenticate()
{
    return ATF_SUCC;
}






