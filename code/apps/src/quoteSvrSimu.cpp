//#include "MsgDef.hpp"
//#include "TcpConnector.h"
#include "quoteSvrSimu.hpp"
#include "thostsoDef.hpp"

extern int g_timeout_case;

QuoteServer::QuoteServer(struct QuoteConf conf)
{
	m_uiActualProviderTotal = 0;
	int iReusePortFlag = 1;
	int  old_flags;

	strcpy(m_quoteSvrIP,conf.quoteSvrIP);
	m_quoteSvrPort = conf.quoteSvrPort;

	if(m_quoteSvrPort<0 || m_quoteSvrIP[0]==0)
	{
		g_log.error("Invalid param: m_quoteSvrIP = %s,m_quoteSvrPort = %d\n",m_quoteSvrIP,m_quoteSvrPort);
		return;
	}

	/*Create Quote TCP Server*/
	m_quoteSvrMsgConnFd = TcpCreateServer(m_quoteSvrIP,m_quoteSvrPort);
	g_log.info("[quoteSvr]socket %d(IP = %s, Port = %d)\n",m_quoteSvrMsgConnFd,m_quoteSvrIP,m_quoteSvrPort);
	if(m_quoteSvrMsgConnFd <= 0)
	{
		g_log.error("Failed to create quote server !\n");
		return;
	}
	
	old_flags = fcntl(m_quoteSvrMsgConnFd, F_GETFL, 0);
	fcntl(m_quoteSvrMsgConnFd, F_SETFL, old_flags | O_NONBLOCK );
	setsockopt(m_quoteSvrMsgConnFd, SOL_SOCKET, SO_REUSEADDR, &iReusePortFlag, sizeof(iReusePortFlag));

	//start thread loop
	Start();

}
QuoteServer::~QuoteServer()
{

}

void QuoteServer::StartNewOrder(QuoteOrder *vOrder)
{
	m_vOrder = vOrder;	
	m_uiExpectProviderTotal = m_vOrder->uiProvidersTotal;
	m_uiActualProviderTotal = 0;
	time(&m_tOrderStartTime);
	InitProvider();

	m_status = DOING_TASK;
	
	return ;
}

void QuoteServer::GetOrderStatus()
{
	unsigned int numDone = 0;
	
	for(unsigned int i=0;i<m_uiActualProviderTotal;i++)
	{
		if(DONE == m_aProviders[i].IsFinished())
			numDone++;
	}
	
	if(numDone == m_uiExpectProviderTotal)
	{
		m_status = NO_TASK;
		DestroyProvider();
	}
	else
	{
		time_t tNow;
		time(&tNow);
		if(tNow - m_tOrderStartTime >= g_timeout_case)
		{
			m_status = NO_TASK;
			DestroyProvider();
			g_log.error("case is timeout, will be stopped !\n");
		}
	}

}

int QuoteServer::InitProvider()
{
	for(unsigned int i=0;i<m_uiExpectProviderTotal;i++)
	{
		ProviderInitData data;
		data.iFrontID = i;
		data.pOpSet = &m_vOrder->quoteOp[i];
		
		m_aProviders[i].Init(data);
	}
	
	return ATF_SUCC;
}

void QuoteServer::DestroyProvider()
{
	for(unsigned int i=0;i<m_uiExpectProviderTotal;i++)
	{
		m_aProviders[i].Destroy();
	}
}

void* QuoteServer::Run(void *)
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
			int maxFd = m_quoteSvrMsgConnFd;
	
			time_out.tv_sec = 1;
			time_out.tv_usec = 0;
				
			FD_ZERO(&readfds);
			FD_SET(m_quoteSvrMsgConnFd,&readfds); 
			for(unsigned int i=0;i<m_uiActualProviderTotal;i++)
			{
				int cli_fd = m_aProviders[i].GetFd();
				if(cli_fd > 0)
				{
					FD_SET(cli_fd,&readfds); 
					maxFd = cli_fd > maxFd ? cli_fd: maxFd;
				}
			}
			
			ret = select(maxFd+1,&readfds,NULL,NULL,&time_out);
			if(ret < 0)
			{
				g_log.error("QuoteServer select error !\n");
				close(m_quoteSvrMsgConnFd);
				break;
			}
			else if (0 == ret)
			{
				g_log.debug("QuoteServer select timeout !\n");
			}
			else
			{
				if(FD_ISSET(m_quoteSvrMsgConnFd, &readfds))
				{
					if ((client_fd = accept(m_quoteSvrMsgConnFd, (struct sockaddr *)&cli_addr, &length))==-1)
					{
						g_log.error("QuoteServer accept error !\n");
						continue;
					}
					else
					{
						if(m_uiActualProviderTotal<m_uiExpectProviderTotal)
							m_aProviders[m_uiActualProviderTotal++].SetFd(client_fd);
						else
							continue;
					}
		
				}
				else
				{
					for(unsigned int i=0;i<m_uiActualProviderTotal;i++)
					{
						if(DONE == m_aProviders[i].IsFinished())
						{
							continue;
						}
						
						int cli_fd = m_aProviders[i].GetFd();
						if(FD_ISSET(cli_fd, &readfds))
						{
							m_aProviders[i].HandleMsgFromClient();//非阻塞
						}
					}			
				}
			}

			GetOrderStatus();
		}
		sleep(1);
		
		printf("quote NO TASK\n");
	}
	return NULL;
}

CProvider::CProvider()
{
	m_clientSock = -1;
	m_frontID = -1;
	m_maxOrderRef = INITIAL_VALUE;
	m_sessionID = INITIAL_VALUE;

	//m_waitBeforeSendQuote = CAN_NOT_SEND_QUOTE;
	m_connect = WAIT_CONNECTED;
	m_login = NOT_LOGIN;
	//m_sendInterval = 500;  //default 500 msec

	memset(m_agentIP,0,sizeof(m_agentIP));

	m_tcpbuf = new char[TCP_BUFSIZE];
	memset(&m_userLogined,0,sizeof(m_userLogined));
}

CProvider::~CProvider()
{
	delete [] m_tcpbuf;
}

void CProvider::Init(ProviderInitData sData)
{
	m_frontID = sData.iFrontID;
	m_vOperations = sData.pOpSet;	
	m_iterCur = m_vOperations->begin();
	m_iterEnd = m_vOperations->end();
	
	m_iProgress = TODO;
	
	return;
}

int CProvider::IsFinished()
{
	return m_iProgress;
}

void CProvider::Destroy()
{
	m_vOperations = NULL;
	m_clientSock = -1;
	m_frontID = -1;
}

void CProvider::SetFd(int sockfd)
{
	m_clientSock = sockfd;
}

int CProvider::GetFd()
{
	return m_clientSock;
}

int CProvider::CloseFd()
{
	if(-1 == m_clientSock)
	{
		g_log.error("fd is not exist !");
		return ATF_FAIL;
	}
		
	close(m_clientSock);
	return ATF_SUCC;
}


//除了一档行情需要调用ctp的login接口外，别的行情不需要进行相关的login/logout工作。
void CProvider::HandleMsgFromClient()
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
		if(ATF_FAIL == expectLen)
			return;
		
		rcvMsgSize = TcpRecvData(m_clientSock, pBuf,expectLen);
		if(rcvMsgSize < 1)
		{
			g_log.error("[QuoteServer]TCP rcv %s error,expect %dB, actual %dB.\n",msgName,rcvOpe.msgSize,rcvMsgSize);
			rcvOpe.result = OPERATE_FAIL;
			sprintf(rcvOpe.errReason,"%s: TCP rcv error, expect %dB, actual %dB.",msgName,rcvOpe.msgSize,rcvMsgSize);
			m_iterCur = m_iterEnd - 2;
		}
		else
		{
			if(ATF_FAIL == CheckMsg(rcvOpe.pMsg, rcvOpe.msgSize, m_tcpbuf, rcvMsgSize))
			{
				rcvOpe.result = OPERATE_FAIL;
				sprintf(rcvOpe.errReason,"%s",m_curOpErrReason);
				m_iterCur = m_iterEnd - 2;
			}
			else
			{
				rcvOpe.result = OPERATE_SUCC;
				g_log.info("[QuoteServer]Receive %s(%dB), right.\n",msgName,rcvMsgSize);
				
				if(IS_DEBUG)
				{
					printf_green("[QuoteServer]Receive %s(%dB), right.\n",msgName,rcvMsgSize);
				}
			}
		}


		m_iterCur++;
		while(m_iterCur != m_iterEnd)
		{
			memset(m_curOpErrReason,0,sizeof(m_curOpErrReason));
			
			OP &sendOpe = *m_iterCur;
			

			MsgIDToMsgName(sendOpe.msgID,msgName,sizeof(msgName));		
			
			if(SEND == sendOpe.direction)
			{
				if(MSGID_S2C_QUOTE_RSP_LOGIN == sendOpe.msgID)
				{
					QuoteRspUserLoginFromATF *pRsp = (QuoteRspUserLoginFromATF *)(sendOpe.pMsg + sizeof(MsgHead));
					FillMsgOnRspLogin(pRsp);
				}
				else if(MSGID_S2C_QUOTE_DISCONNECT == sendOpe.msgID)
				{
					m_connect = DISCONNECTED;
					m_login = NOT_LOGIN;
					m_sessionID++;
				}
				else if(MSGID_S2C_QUOTE_RSP_SUBSCRIBE == sendOpe.msgID)
				{
					RspSubMarketFromATF *pRsp = (RspSubMarketFromATF *)(sendOpe.pMsg + sizeof(MsgHead));
					strcpy(pRsp->specificInstrument.InstrumentID,m_acInstrSubReq);
					printf_blue("InstrumentID:%s,%s\n,",m_acInstrSubReq,pRsp->specificInstrument.InstrumentID);
				}				
				else if(MSGID_S2C_QUOTE_RSP_UNSUBSCRIBE == sendOpe.msgID)
				{
					RspUnsubMarketFromATF *pRsp = (RspUnsubMarketFromATF *)(sendOpe.pMsg + sizeof(MsgHead));
					strcpy(pRsp->specificInstrument.InstrumentID,m_acInstrUnsubReq);
					printf_blue("InstrumentID:%s,%s\n,",m_acInstrUnsubReq,pRsp->specificInstrument.InstrumentID);
				}
								
				ret = TcpSendData(m_clientSock,sendOpe.pMsg,sendOpe.msgSize);
				if(ret != sendOpe.msgSize)	
				{
					g_log.error("[QuoteServer]TCP send error,expect %dB, actual %dB.\n",sendOpe.msgSize,ret);
					sendOpe.result = OPERATE_FAIL;
					sprintf(sendOpe.errReason,"%s: TCP send error",msgName);
					m_iterCur = m_iterEnd - 2;
				}
				else
				{
					sendOpe.result = OPERATE_SUCC;
					g_log.info("[QuoteServer]Send %s(%dB).\n",msgName,ret);

					if(IS_DEBUG)
					{
						printf_green("[QuoteServer]Send %s(%dB)\n",msgName,rcvMsgSize);
					}
				}

				if(MSGID_S2C_QUOTE_RSP_CONN == sendOpe.msgID)
					m_connect = CONNECTED;
				
				if(DISCONNECTED == m_connect)
					CloseFd();
				
				m_iterCur++;
			}
			else if(LOCAL == sendOpe.direction)
			{
				if(IS_DEBUG)
				{
					printf_green("[QuoteServer]handle %s\n",msgName);	
				}
								
				if(MSGID_LOCAL_SLEEP == sendOpe.msgID)
				{
					SleepTime *pMsgBody = (SleepTime *)(sendOpe.pMsg + sizeof(MsgHead));
					sleep(pMsgBody->time);
									
					sendOpe.result = OPERATE_SUCC;
				}
				m_iterCur++;
			}
			else if(RCV == sendOpe.direction)
			{
				return;
			}
			else
			{
				g_log.error("[QuoteServer]op.direction is not defined !\n");
			
				sendOpe.result = OPERATE_FAIL;
			
				sprintf(sendOpe.errReason,"%s: op.direction is not defined.",msgName);
			
				m_iterCur = m_iterEnd - 2;
			}

			if(m_iterCur == m_iterEnd)
			{
				m_iProgress = DONE;
				
				printf_blue("provider %u had finished his job.\n",m_frontID);
				//g_log.info("[quoteServer]task done.\n");
			}
			
		}

	}
	else
		return;
}

void CProvider::RcvMsgFromClient(char * buf, int iBufSize, int *msgSize, int clientSock)
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
		   printf("QuoteServer::RcvMsgFromClient file=%s,select error\n",__FILE__);
		   close(clientSock);
		   break;
		}
		else if (ret==0)
		{
		   //printf("file=%s,%s,timeout\n",__FILE__,__FUNCTION__);
		}
		else
		{	
			if(FD_ISSET(clientSock, &readfds))
			{
				memset(buf,0,iBufSize);
				int recvbytes = 0;
				int n=0;
				char * pBuf = &buf[0];
				while(1)
				{
					n = recv(clientSock, (void*)pBuf, iNotRcvSize, 0);
					if(n<0)
					{
						break;
					}
					else if(n==0)
					{
						printf("QuoteServer::RcvMsgFromClient file:%s,  client called close()\n",__FILE__);
						close(clientSock);
						recvbytes = 0;
						break;
					}
					else
					{
						recvbytes += n;
						if(recvbytes > iBufSize)
						{
							recvbytes = -1;
							break;
						}
						pBuf += n;
						iNotRcvSize -= n;
					}
				}

				*msgSize = recvbytes;
			}
		}	
				
	}while(0);
		 

}

int CProvider::MsgIDToMsgName(MSG_ID id,char *pMsgName,int len)
{
	if(NULL == pMsgName)
	{
		g_log.error("[%s]Pointer pMsgName is null !\n",__FUNCTION__);
		return ATF_FAIL;
	}

	memset(pMsgName,0,len);
	
	switch(id)
	{
		case	MSGID_C2S_QUOTE_REQ_CONN:
			{
				strcpy(pMsgName,"MSGID_C2S_QUOTE_REQ_CONN");
			}
			break;
		case	MSGID_S2C_QUOTE_RSP_CONN:	
			{
				strcpy(pMsgName,"MSGID_S2C_QUOTE_RSP_CONN");
			}
			break;
		case	MSGID_C2S_QUOTE_REQ_LOGIN:
			{
				strcpy(pMsgName,"MSGID_C2S_QUOTE_REQ_LOGIN");
			}
			break;
		case	MSGID_S2C_QUOTE_RSP_LOGIN:	
			{
				strcpy(pMsgName,"MSGID_S2C_QUOTE_RSP_LOGIN");
			}
			break;
		case	MSGID_C2S_QUOTE_REQ_SUBSCRIBE:	
			{
				strcpy(pMsgName,"MSGID_C2S_QUOTE_REQ_SUBSCRIBE");
			}
			break;
		case	MSGID_S2C_QUOTE_RSP_SUBSCRIBE:	
			{
				strcpy(pMsgName,"MSGID_S2C_QUOTE_RSP_SUBSCRIBE");
			}
			break;
		case	MSGID_C2S_QUOTE_REQ_UNSUBSCRIBE:	
			{
				strcpy(pMsgName,"MSGID_C2S_QUOTE_REQ_UNSUBSCRIBE");
			}
			break;
		case	MSGID_S2C_QUOTE_RSP_UNSUBSCRIBE:	
			{
				strcpy(pMsgName,"MSGID_S2C_QUOTE_RSP_UNSUBSCRIBE");
			}
			break;
		case MSGID_LOCAL_SLEEP:
			{
				strcpy(pMsgName,"MSGID_LOCAL_SLEEP");
			}		
			break;
		case	MSGID_S2C_QUOTE_DISCONNECT:
			{
				strcpy(pMsgName,"MSGID_S2C_QUOTE_DISCONNECT");
			}
			break;
		case MSGID_C2S_QUOTE_RESULT_SEND_QUOTE:
			{
				strcpy(pMsgName,"MSGID_C2S_QUOTE_RESULT_SEND_QUOTE");
			}
			break;
		default:
			{
				g_log.error("[%s]Undefined msgID(%d) in QuoteServer !\n",__FUNCTION__,id);
				return ATF_FAIL;
			}
			break;
	}
	return 0;
}

int CProvider::GotMsgSize(MSG_ID id)
{
	switch(id)
	{
		case MSGID_C2S_QUOTE_REQ_CONN:
			{
				return sizeof(MsgHead);
			}
			break;
		case MSGID_S2C_QUOTE_RSP_CONN:
			{
				return sizeof(MsgHead) + sizeof(RspInfoFromATF);	
			}
			break;
		case MSGID_S2C_QUOTE_DISCONNECT: 
			{
				return sizeof(MsgHead);
			}
			break;
		case MSGID_C2S_QUOTE_REQ_LOGIN:
			{
				return sizeof(MsgHead) + sizeof(QuoteReqUserLoginToATF);
			}
			break;
		case MSGID_S2C_QUOTE_RSP_LOGIN:
			{
				return sizeof(MsgHead) + sizeof(QuoteRspUserLoginFromATF);
			}
			break;
		case MSGID_C2S_QUOTE_REQ_SUBSCRIBE:
			{
				return sizeof(MsgHead) + sizeof(ReqSubMarketToATF);
			}
			break;
		case MSGID_S2C_QUOTE_RSP_SUBSCRIBE: 
			{
				return sizeof(MsgHead) + sizeof(RspSubMarketFromATF);
			}
			break;
		case MSGID_C2S_QUOTE_REQ_UNSUBSCRIBE:
			{
				return sizeof(MsgHead) + sizeof(ReqUnsubMarketToATF);
			}
			break;
		case MSGID_S2C_QUOTE_RSP_UNSUBSCRIBE: 
			{
				return sizeof(MsgHead) + sizeof(RspUnsubMarketFromATF);
			}
			break;
		case MSGID_C2S_QUOTE_RESULT_SEND_QUOTE:
			{
				return sizeof(MsgHead) + sizeof(ResultOfSendQuote);
			}
			break;
		default:
			{
				g_log.error("[%s]Undefined msgID(%d) in QuoteServer !\n",__FUNCTION__,id);
				return ATF_FAIL;
			}
			break;
	}
	return ATF_SUCC;
}


int CProvider::CheckMsg(char * pExpectMsg, int expectMsgLen, char *pActualMsg, int actualMsgLen)
{
	//对于订阅行情的请求要特殊处理
	/*if(expectMsgLen != actualMsgLen)
	{
		g_log.error("Msg length is error,expect %d, actual %d",expectMsgLen,actualMsgLen);
		return ATF_FAIL;
	}*/
	
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

int CProvider::CmpMsg(int iMsgID, char * pExpectMsg, char *pActualMsg)
{
	if(NULL == pActualMsg)
	{
		sprintf(m_curOpErrReason,"msg(%d) body is null !",iMsgID);
		
		return ATF_FAIL;
	}
	
	switch(iMsgID)
	{
		case MSGID_C2S_QUOTE_REQ_CONN:
			{
				return ATF_SUCC;
			}
			break;
			
		case MSGID_C2S_QUOTE_REQ_LOGIN:
			{
				QuoteReqUserLoginToATF *pExpect,*pActual;
				pExpect = (QuoteReqUserLoginToATF *)pExpectMsg;
				pActual = (QuoteReqUserLoginToATF *)pActualMsg;
			
				return CmpMsgReqUserLogin(pExpect,pActual);
			}
			break;
			
		case MSGID_C2S_QUOTE_REQ_SUBSCRIBE:
			{
				ReqSubMarketToATF  *pExpect,*pActual;
				
				pExpect = (ReqSubMarketToATF *)pExpectMsg;
				pActual = (ReqSubMarketToATF *)pActualMsg;
			
				return CmpReqSubscribeMarketMsg(pExpect,pActual);
			}
			break;
		case MSGID_C2S_QUOTE_REQ_UNSUBSCRIBE:
			{
				ReqUnsubMarketToATF  *pExpect,*pActual;
					
				pExpect = (ReqUnsubMarketToATF *)pExpectMsg;
				pActual = (ReqUnsubMarketToATF *)pActualMsg;
				
				return CmpReqUnsubscribeMarketMsg(pExpect,pActual);
			}
			break;

		case MSGID_C2S_QUOTE_RESULT_SEND_QUOTE:
			{
				ResultOfSendQuote *pMsg = (ResultOfSendQuote *)pActualMsg;				
				if(true == pMsg->isSucc)
					return ATF_SUCC;
				else
				{
					sprintf(m_curOpErrReason,"Failed to send quote!");
					return ATF_FALSE;
				}
			}
			break;
		default:
			{
				g_log.error("Invalid msgID:%d\n",iMsgID);
				
				sprintf(m_curOpErrReason,"Invalid msgID:%d",iMsgID);
				return ATF_FAIL;
			}
			break;
	};
	
	
}

int CProvider::CmpMsgReqUserLogin(QuoteReqUserLoginToATF * pExpectMsg,QuoteReqUserLoginToATF *pActualMsg)
{
	int ret = ATF_SUCC;
	CUstpFtdcReqUserLoginField *pExpect, *pActual;
	pExpect = &pExpectMsg->reqUserLogin;
	pActual = &pActualMsg->reqUserLogin;

	//保存以填入回复中
	strcpy(m_userLogined.BrokerID,pActual->BrokerID);
	strcpy(m_userLogined.UserID,pActual->UserID);
	
    if(0 != strcasecmp(pExpect->UserID,pActual->UserID))
    {
		g_log.error("[QuoteSvr]Invalid UserID:expect %s,actual %s\n",pExpect->UserID,pActual->UserID);
		
		sprintf(m_curOpErrReason,"[QuoteSvr]Invalid UserID:expect %s,actual %s",pExpect->UserID,pActual->UserID);

		m_userLogined.ErrorID = 11;
		
		ret = ATF_FAIL;
	}

	if(0 != strcasecmp(pExpect->Password,pActual->Password))
	{
		g_log.error("[QuoteSvr]Invalid Password:expect %s,actual %s\n",pExpect->Password,pActual->Password);
		sprintf(m_curOpErrReason,"[QuoteSvr]Invalid Password:expect %s,actual %s",pExpect->Password,pActual->Password);
		m_userLogined.ErrorID = 14;
		ret = ATF_FAIL;
	}

	/*if(0 != strcasecmp(pExpect->BrokerID,pActual->BrokerID))
	{
		g_log.error("Invalid BrokerID:expect %s,actual %s\n",pExpect->BrokerID,pActual->BrokerID);
		sprintf(m_curOpErrReason,"[QuoteSvr]Invalid BrokerID:expect %s,actual %s",pExpect->BrokerID,pActual->BrokerID);
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

int CProvider::CmpReqSubscribeMarketMsg(ReqSubMarketToATF * pExpectMsg,ReqSubMarketToATF *pActualMsg)
{
	if(NULL == pExpectMsg || NULL == pActualMsg)
	{
		g_log.error("Input para is NULL !\n");
		return ATF_FAIL;
	}

	if(0 == strcmp(pExpectMsg->instr,pActualMsg->instr))
	{
		strcpy(m_acInstrSubReq,pActualMsg->instr);
		return ATF_SUCC;
	}
	else
	{
		g_log.error("expect instrument:%s, actual instrument:%s\n",pExpectMsg->instr,pActualMsg->instr);
		sprintf(m_curOpErrReason,"expect instrument:%s, actual instrument:%s\n",pExpectMsg->instr,pActualMsg->instr);
		return ATF_FAIL;
	}

}

int CProvider::CmpReqUnsubscribeMarketMsg(ReqUnsubMarketToATF * pExpectMsg,ReqUnsubMarketToATF *pActualMsg)
{
	if(NULL == pExpectMsg || NULL == pActualMsg)
	{
		g_log.error("Input para is NULL !\n");
		return ATF_FAIL;
	}

	if(0 == strcmp(pExpectMsg->instr,pActualMsg->instr))
	{
		strcpy(m_acInstrUnsubReq,pActualMsg->instr);
		return ATF_SUCC;
	}
	else
	{
		g_log.error("expect instrument:%s, actual instrument:%s\n",pExpectMsg->instr,pActualMsg->instr);
		sprintf(m_curOpErrReason,"expect instrument:%s, actual instrument:%s\n",pExpectMsg->instr,pActualMsg->instr);
		return ATF_FAIL;
	}

}


int CProvider::FillMsgOnRspLogin(QuoteRspUserLoginFromATF *pRsp)
{
	GotCurrentDate(pRsp->rspUserLogin.TradingDay);
	GotCurrentTime(pRsp->rspUserLogin.LoginTime);
	
	//GotCurrentTime(pRsp->rspUserLogin.SHFETime);
	//GotCurrentTime(pRsp->rspUserLogin.CZCETime);
	//GotCurrentTime(pRsp->rspUserLogin.DCETime);
	//GotCurrentTime(pRsp->rspUserLogin.FFEXTime);

	//strcpy(pRsp->rspUserLogin.SystemName,"QuoteHosting");
	
	//pRsp->rspUserLogin.FrontID = m_frontID;
	//pRsp->rspUserLogin.SessionID = m_sessionID;	
	//sprintf(pRsp->rspUserLogin.MaxOrderRef,"%lld", m_maxOrderRef);
	
	memcpy(&pRsp->rspUserLogin.UserID[0] , &m_userLogined.UserID[0], sizeof(pRsp->rspUserLogin.UserID));
	memcpy(&pRsp->rspUserLogin.BrokerID[0] , &m_userLogined.BrokerID[0], sizeof(pRsp->rspUserLogin.BrokerID));
	//pRsp->rspInfo.ErrorID = m_userLogined.ErrorID;
	
	m_connect = CONNECTED;	
	m_login = LOGINED;

	return ATF_SUCC;
}

int CProvider::GotAgentIp(char * ip)
{
	if(NULL == ip)
	{
		g_log.error("Invalid param !\n");
		return ATF_FAIL;
	}

	if(0 == strlen(m_agentIP))
	{
		g_log.error("m_agentIP is NULL !\n");
		return ATF_FAIL;
	}

	strcpy(ip,m_agentIP);
	return ATF_SUCC;
}

