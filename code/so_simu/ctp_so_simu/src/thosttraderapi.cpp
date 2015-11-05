#include "thosttraderapi.hpp"


extern CLog g_log;
//extern int cpuId;

/*struct timeval g_timeRcvOrder[MAX_TIME_STATISTICS_NUM] = {0};
long long g_rearTimeRcvOrder = 0;
long long g_tradeNum = 0;

int maxTimeTradeProcess = 0;
int mixTimeTradeProcess = 0;
int avgTimeTradeProcess = 0;*/

/*extern struct timeval g_timeRcvQuote[MAX_TIME_STATISTICS_NUM];
extern long g_rearTimeRcvQuote;*/

extern struct tvTime_t * g_pTimeRcvQuote;
extern struct tvTime_t * g_pRearTimeRcvQuote;
extern long long g_llNumRcvQuote;

struct tvTime_t * g_pTimeRcvOrder = NULL;
struct tvTime_t * g_pRearTimeRcvOrder = NULL;
long long g_llNumRcvOrder = 0;
bool g_bTradeReady;

//extern struct timeval * g_SeqTime_PushOrder;
//extern struct timeval * g_SeqTime_PopQuote;
//extern struct timeval * g_SeqTime_PopOrder;
//extern struct timeval * g_SeqTime_SendOrder;

void * ThostFtdcTraderApiProc(void* pApi)
{
	CThostFtdcTraderApi* pstTraderApi = (CThostFtdcTraderApi*)pApi;

	while(1)
	{
		switch(pstTraderApi->m_iState)
		{
			case STATUS_TRADE_DEFAULT:
				{	
	
				}
				break;
				
			case STATUS_TRADE_INIT:
				{
					pstTraderApi->Handlelnit();
				}
				break;
				
			case STATUS_TRADE_REQ_LOGIN:
				{
					pstTraderApi->HandleLogin();
				}
				break;
				
			case STATUS_TRADE_REQ_SETTLEMENT_CONFIRM:
				{	
					pstTraderApi->HandleSettlementConfirm();
				}
				break;

			case STATUS_QRY_POSITION:
				{
					pstTraderApi->HandleQryPosition();
				}
				break;
				
			default:
				break;
		}

		/*======send order=========*/
		pstTraderApi->SendOrderToATT();
			
			
		/*======send order action=====*/
		pstTraderApi->SendCancelOrderToATT();
			
			
		/*====receive msg from ATF==*/
		pstTraderApi->HandleMsgFromATT();

		usleep(50);
 	}

	return NULL;
}

int AddOrderCnt()
{
	struct tvTime_t *pTime = new(struct tvTime_t);
	pTime->pNext = NULL;
	pTime->tv = time_get_now();
	
	//g_log.debug("Rcv order %lld.\n",g_llNumRcvOrder);
	
	g_llNumRcvOrder++;
	
	if(NULL == g_pTimeRcvOrder)
	{
		g_pRearTimeRcvOrder = pTime;
		g_pTimeRcvOrder = pTime;
	}
	else
	{
		g_pRearTimeRcvOrder->pNext = pTime;
		g_pRearTimeRcvOrder = g_pRearTimeRcvOrder->pNext;
	}

	return ATF_SUCC;

}

int StatTradeProcessTime(struct InfoTradeProcessTime & staticInfo)
{
	long long idx = 0;

	if(g_llNumRcvQuote >= g_llNumRcvOrder)
	{
		staticInfo.tradeNum = g_llNumRcvOrder;
		
		struct tvTime_t *pQuoteTime = g_pTimeRcvQuote;
		struct tvTime_t *pOrderTime = g_pTimeRcvOrder;
		
		while(pOrderTime != NULL)
		{
			long timeDiff = time_diff_us(pQuoteTime->tv,pOrderTime->tv);

			if(0 != idx)
			{
				if(timeDiff > staticInfo.maxTimeTradeProcess)
				{
					staticInfo.maxTimeTradeProcess = timeDiff;
					staticInfo.maxIdx = idx;
				}

				if(timeDiff < staticInfo.mixTimeTradeProcess)
					staticInfo.mixTimeTradeProcess = timeDiff;
				
				staticInfo.avgTimeTradeProcess = (staticInfo.avgTimeTradeProcess * idx + timeDiff)/(idx + 1);
			}
			else
			{
				staticInfo.maxTimeTradeProcess = timeDiff;
				staticInfo.mixTimeTradeProcess = timeDiff;
				staticInfo.avgTimeTradeProcess = timeDiff;
				staticInfo.maxIdx = idx;
			}

			idx++;
			pOrderTime = pOrderTime->pNext;
			pQuoteTime = pQuoteTime->pNext;
		}
	}
	else
	{
		g_log.error("g_llNumRcvOrder(%lld) is more than g_llNumRcvQuote(%lld)!\n",g_llNumRcvOrder,g_llNumRcvQuote);
		
		return -1;
	}
	
	g_log.debug("Quote received: %lld, Order received: %lld !\n",g_llNumRcvQuote,g_llNumRcvOrder);
	
	g_log.debug("Trade process time: max(%dusec), min(%dusec), average(%dusec) !\n",
		staticInfo.maxTimeTradeProcess,staticInfo.mixTimeTradeProcess,staticInfo.avgTimeTradeProcess);
	
	return 0;
}


int StaticQuoteInteval(StaticQuoteInterval_t &staticInfo)
{
	long long idx = 0;
	staticInfo.quoteNum = g_llNumRcvQuote;

	struct tvTime_t *pQuoteTime = g_pTimeRcvQuote;
	while(NULL != pQuoteTime && NULL != pQuoteTime->pNext)
	{
		int timeDiff = time_diff_us(pQuoteTime->tv,pQuoteTime->pNext->tv);
		
		if(0 != idx)
		{
			if(timeDiff > staticInfo.maxQuoteInterval)
			{
				staticInfo.maxQuoteInterval = timeDiff;
				staticInfo.maxIdx = idx;
			}
		
			if(timeDiff < staticInfo.minQuoteInterval)
				staticInfo.minQuoteInterval = timeDiff;
			
			staticInfo.avgQuoteInterval = (staticInfo.avgQuoteInterval * idx + timeDiff)/(idx + 1);
		}
		else
		{
			staticInfo.avgQuoteInterval = timeDiff;
			staticInfo.minQuoteInterval = timeDiff;
			staticInfo.maxQuoteInterval = timeDiff;
			staticInfo.maxIdx = idx;
		}

		pQuoteTime = pQuoteTime->pNext;
		idx++;
	}
	
	g_log.debug("Quote received: %lld, interval: max(%dusec), min(%dusec), average(%dusec) !\n",
		g_llNumRcvQuote,staticInfo.maxQuoteInterval,staticInfo.minQuoteInterval,staticInfo.avgQuoteInterval);

	return 0;
}

void * ATFA(void* pApi)
{
	char sendBuf[2048] = {0};
	char rcvBuf[2048] = {0};
	char *pRcvBuf = NULL;
	char *pSendBuf = NULL;
	MsgHead * pMsgHead = NULL;
	MsgHead stHead;
	int msgBodyLen = 0;
	int msgId;
	int ret = 0;
	int port = ATFA_PORT;
	int connSock = -1;
	char localIP[24] = "127.0.0.1";
	
	struct sockaddr_in cli_addr;
	socklen_t length = (socklen_t )sizeof(struct sockaddr_in);
	
	//CThostFtdcTraderApi* pstTraderApi = (CThostFtdcTraderApi*)pApi;

	
	/*Create Quote TCP Server*/
	int listenSock = TcpCreateServer(localIP,port);
	g_log.debug("ATF agent Port = %d\n",port);
	if(listenSock <= 0)
	{
		g_log.error("Failed to create ATF agent !\n");
		return NULL;
	}

	while(1)
	{
		if ((connSock = accept(listenSock, (struct sockaddr *)&cli_addr, &length)) == -1)
		{
			g_log.error("[%s]:accept error!\n",__FUNCTION__);
			continue;
		}

		//receive msg head
		pRcvBuf = rcvBuf;
		ret = TcpRecvData(connSock,pRcvBuf,sizeof(MsgHead));
		if(ret != sizeof(MsgHead))
		{					
			g_log.error("ATF agent recv msg head error! \n");
			return NULL;
		}
		//DEBUG_ATFA("ATF agent recv msg head %dB\n",ret);

		pMsgHead = (MsgHead *)pRcvBuf;
		msgId = ntohs(pMsgHead->iMsgID);
		msgBodyLen = ntohs(pMsgHead->iMsgBodyLen);

		//receive msg body
		pRcvBuf = rcvBuf;
		ret = TcpRecvData(connSock,pRcvBuf,msgBodyLen);
		if(ret != msgBodyLen)
		{					
			g_log.error("ATF agent recv msg body error!\n");
			return NULL;
		}
		//DEBUG_ATFA("ATF agent recv msg body %dB\n",ret);
		
		switch(msgId)
		{
			case MSGID_S2C_REQ_TRADE_PROCESS_STATIC_TIME:
				{
					//DEBUG_ATFA("Received  MSGID_S2C_REQ_TRADE_PROCESS_STATIC_TIME, %dB\n",ret);

					InfoTradeProcessTime info;
					/*info.avgTimeTradeProcess = avgTimeTradeProcess;
					info.maxTimeTradeProcess = maxTimeTradeProcess;
					info.mixTimeTradeProcess = mixTimeTradeProcess;
					info.tradeNum = g_rearTimeRcvOrder;*/
					StatTradeProcessTime(info);
					
					stHead.iMsgID = htons(MSGID_C2S_RSP_TRADE_PROCESS_STATIC_TIME);
					stHead.iMsgBodyLen = htons(sizeof(info));
					
					pSendBuf = sendBuf;
					memcpy(pSendBuf, &stHead, sizeof(stHead));
					memcpy(pSendBuf+sizeof(stHead),&info,sizeof(info));
					
					int iSendLen = sizeof(stHead) + sizeof(info);
					ret = TcpSendData(connSock,(char*)pSendBuf,iSendLen);
					if(iSendLen != ret)
					{
						g_log.error("ATFA send MSGID_C2S_RSP_TRADE_PROCESS_STATIC_TIME error,expect %dB,actual %dB\n",iSendLen,ret);
						return NULL;
					}
					//DEBUG_ATFA("ATFA send MSGID_C2S_RSP_TRADE_PROCESS_STATIC_TIME %dB\n",ret);
				}
				break;
			case MSGID_S2C_REQ_QUOTE_INTERVAL_STATIC_TIME:
				{
					//DEBUG_ATFA("Received  MSGID_S2C_REQ_QUOTE_INTERVAL_STATIC_TIME, %dB\n",ret);
					
					StaticQuoteInterval_t info;
					StaticQuoteInteval(info);
					
					stHead.iMsgID = htons(MSGID_C2S_RSP_QUOTE_INTERVAL_STATIC_TIME);
					stHead.iMsgBodyLen = htons(sizeof(info));
					
					pSendBuf = sendBuf;
					memcpy(pSendBuf, &stHead, sizeof(stHead));
					memcpy(pSendBuf+sizeof(stHead),&info,sizeof(info));
					
					int iSendLen = sizeof(stHead) + sizeof(info);
					ret = TcpSendData(connSock,(char*)pSendBuf,iSendLen);
					if(iSendLen != ret)
					{
						g_log.error("ATFA send MSGID_C2S_RSP_QUOTE_INTERVAL_STATIC_TIME error,expect %dB,actual %dB\n",iSendLen,ret);
						return NULL;
					}
					//DEBUG_ATFA("ATFA send MSGID_C2S_RSP_QUOTE_INTERVAL_STATIC_TIME %dB\n",ret);
				}
				break;
			case MSGID_S2C_REDDY_FOR_PERFORM_TEST:
				{
				}
				break;
			default:
			{
				g_log.error("Undefined msg !\n");
			}
			break;
			}
		close(connSock);
	}
}




CThostFtdcTraderSpi::~CThostFtdcTraderSpi()
{

}


int CThostFtdcTraderApi::m_iTraderApis_used = 0;
CThostFtdcTraderApi CThostFtdcTraderApi::m_stTraderApis[CThostFtdcTraderApi::MAX_TRADERAPIS_NUM];



CThostFtdcTraderApi::CThostFtdcTraderApi()
{
	g_log.debug("\n");

	m_iUseUdp = 0;
	m_bLogined = false;
	m_autoReconn = false;
	g_bTradeReady = false;

	m_llNumOrder = 0;
	m_llNumCancelOrder = 0;
	
	pthread_mutex_init(&m_mutexState,NULL);

	memset(&m_userLoginField,0,sizeof(m_userLoginField));
	
	SetState(STATUS_TRADE_DEFAULT);
}

CThostFtdcTraderApi::~CThostFtdcTraderApi()
{
	g_log.info("destroy ~CThostFtdcTraderApi\n");
	
	pthread_mutex_destroy(&m_mutexState);
	
	if(m_msgConnFd >= 0)
		close(m_msgConnFd);
}


CThostFtdcTraderApi *CThostFtdcTraderApi::CreateFtdcTraderApi(const char *pszFlowPath , const bool bIsUsingUdp)
{
	printf("Create CreateFtdcTraderApi instance.\n");
	g_log.debug("\n");
	if(m_iTraderApis_used < MAX_TRADERAPIS_NUM)
	{
		if(pszFlowPath!=NULL)
		{
			strcpy(m_stTraderApis[m_iTraderApis_used].m_iLocalFile,pszFlowPath);
		}
		m_stTraderApis[m_iTraderApis_used].m_iUseUdp = bIsUsingUdp;

		return &m_stTraderApis[m_iTraderApis_used++];
	}
	else
	{
		return NULL;
	}
}

void CThostFtdcTraderApi::RegisterSpi(CThostFtdcTraderSpi *pSpi)
{
	g_log.debug("\n");
	m_ftdcTraderSpi_instance = pSpi;

	return;
}

void CThostFtdcTraderApi::SubscribePrivateTopic(THOST_TE_RESUME_TYPE nResumeType)
{
	g_log.debug("\n");
	return;
}

void CThostFtdcTraderApi::SubscribePublicTopic(THOST_TE_RESUME_TYPE nResumeType)
{
	g_log.debug("\n");

	return;
}

void CThostFtdcTraderApi::RegisterFront(char *pszFrontAddress)
{
	char url[32] = {0};

	g_log.debug("\n");

	if(NULL == pszFrontAddress)
	{
		g_log.error("Input param is NULL !");
		return ;
	}

	strcpy(url,pszFrontAddress);
	char * pIp = url;
	char * pPort = NULL;
	
	if((pIp = strstr(url,"://"))!=NULL)
	{
		pIp = pIp + 3;
	}
	else
	{
		g_log.error("Front Address is invalid: no find \"://\" !\n");
		return ;
        //如果有错误如何返回错误信息
	}
	
	if((pPort = strstr(pIp,":"))!=NULL)
	{
		*pPort = '\0';
		
		pPort = pPort + 1;
		
		m_shListenPort = atoi(pPort);

		strcpy(m_traderSvrIP,pIp);

		g_log.debug("Exchange Input IP %s , port %d\n",m_traderSvrIP,m_shListenPort);
	}
	else
	{
		g_log.error("Front Address is invalid: no find \":\" !\n");
		return;
	}

}

void CThostFtdcTraderApi::Init()
{
	g_log.debug("\n");


	pthread_t tid;
	int ret = pthread_create(&tid,NULL,ThostFtdcTraderApiProc,this);
	if (ret != 0)
	{	
		perror("Fail to create thread thosttraderapi !");
		return ;
	}

	m_Inited = 1;
	SetState(STATUS_TRADE_INIT);

	//ATF agent thread
	pthread_t tid2;
	ret = pthread_create(&tid2,NULL,ATFA,this);
	if (ret != 0)
	{	
		perror("Fail to create thread ATFA !");
		return ;
	}
}


int CThostFtdcTraderApi::ReqUserLogin(CThostFtdcReqUserLoginField *pReqUserLoginField, int nRequestID)
{
	g_log.debug("Trade\n");

	if(m_Inited==0)
	{
		g_log.error("not inited\n");
		return -1;
	}

	//if(false == m_requestDone)
		//return -2;    //-2: 未处理请求队列总数量超限，目前上限为1		

    //交易接口会在响应或回报中返回与该请求相同的请求编号
    //m_requestDone = false;
	
	m_userLoginField.nRequestID = nRequestID;
	memcpy(&m_userLoginField.reqUserLogin,pReqUserLoginField,sizeof(CThostFtdcReqUserLoginField));

	SetState(STATUS_TRADE_REQ_LOGIN);

	return 0;
}

/*void CThostFtdcTraderApi::SetIsLogined(int iLogined)
{
	g_log.debug("\n");

	m_iLogined = iLogined;
}

int CThostFtdcTraderApi::IsLogined()
{
	//DEBUG_TRADERSO("\n");

	return m_iLogined;
}*/


int CThostFtdcTraderApi::ReqSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, int nRequestID)
{
	g_log.debug("Trade\n");

	m_iRequestID = nRequestID;
	memcpy(&m_settlementInfoConfirm,pSettlementInfoConfirm,sizeof(CThostFtdcSettlementInfoConfirmField));
	
	SetState(STATUS_TRADE_REQ_SETTLEMENT_CONFIRM);

	return 0;
}

int CThostFtdcTraderApi::ReqQryInvestorPosition(CThostFtdcQryInvestorPositionField *pQryInvestorPosition, int nRequestID)
{
	g_log.debug("Trade\n");

	m_qryInvestorPosition.nRequestID = nRequestID;
	memcpy(&m_qryInvestorPosition.qry,pQryInvestorPosition,sizeof(*pQryInvestorPosition));
	
	SetState(STATUS_QRY_POSITION);
	
	return 0;
}


int CThostFtdcTraderApi::ReqOrderInsert(CThostFtdcInputOrderField *pInputOrder, int nRequestID)
{
	g_log.debug("\n");

	AddOrderCnt();
	
	m_inputOrder.nRequestID = nRequestID;
	memcpy(&m_inputOrder.order,pInputOrder,sizeof(m_inputOrder));

	while(ATF_FALSE == m_OrderQueue.PushOneData(&m_inputOrder))
	{
		g_log.error("Order push error !\n");
		/*if(PerfTest)
		if(g_llNumRcvOrder > 0 && g_llNumRcvOrder < MAX_NUM_QUOTE)
		{
			static long long ifail = 0;
			g_log.fatal("order push error %lld!\n",ifail);
			ifail++;
		}*/
	}	
	g_log.debug("Succeed to push an order into queue.\n");

	m_llNumOrder++;
	
	/*static long long orderInsertNum = 0;
	DEBUG_TRADERSO("pushed an order into queue %lld\n",orderInsertNum);
	if(orderInsertNum%2000==0)
	{
		g_log.info("========== pushed an order into queue %lld\n",orderInsertNum);
	}
	//g_SeqTime_PushOrder[orderInsertNum%SEQTIME_COUNT]=time_get_now();
	orderInsertNum++;*/

	return 0;
}

int CThostFtdcTraderApi::ReqOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, int nRequestID)
{
	AddOrderCnt();
	

	if(THOST_FTDC_AF_Delete != pInputOrderAction->ActionFlag)
	{
		CThostFtdcInputOrderActionField inputOrderAction;
		memcpy(&inputOrderAction,pInputOrderAction,sizeof(inputOrderAction));
		
		CThostFtdcRspInfoField rspInfo;
		rspInfo.ErrorID = 15;
		strncpy(rspInfo.ErrorMsg,"CTP:BAD_FIELD",sizeof(rspInfo.ErrorMsg));
		//int nRequestID;
		//bool bIsLast = true;
		m_ftdcTraderSpi_instance->OnRspOrderAction(&inputOrderAction,&rspInfo,nRequestID,true);
	}

	m_inputOrderAction.nRequestID = nRequestID;
	
	//g_log.debug("++++++++++++++++++++OrderSysID:%s !\n",pInputOrderAction->OrderSysID);
	memcpy(&m_inputOrderAction.order,pInputOrderAction,sizeof(m_inputOrderAction.order));
	m_inputOrderAction.nRequestID = 2015;
	//g_log.debug("++++++++++++++after OrderSysID:%s !\n",m_inputOrderAction.order.OrderSysID);

	while(ATF_FAIL == m_OrderActionQueue.PushOneData(&m_inputOrderAction))
	{
		////usleep(1000);
		g_log.error("Cancel order push error !\n");
	}
	g_log.debug("Succeed to push an cancel order into queue.\n");
	
	m_llNumCancelOrder++;
	
	return 0;
}

void CThostFtdcTraderApi::SetState(FtdcTraderApiState iState)
{
	m_iState = iState;
}

int CThostFtdcTraderApi::Handlelnit()
{
	int ret = 0;
	char sendBuf[BUFSIZE] = {0};
	char rcvBuf[BUFSIZE] = {0};
	MsgHead stHead; 				
	int iSendLen = 0;	
	int iExpectRcvLen = 0;
	char * pSendBuf = NULL;
	char * pRcvBuf = NULL;
	MsgHead *pMsg = NULL;

	m_msgConnFd = TcpConnect(m_traderSvrIP,m_shListenPort);
	if(m_msgConnFd < 0)
	{
		g_log.error("Failed to connect exchange Server(ip:%s, port:%d, socket:%d)\n",m_traderSvrIP,m_shListenPort,m_msgConnFd);
		return ATF_FALSE;
	}
	g_log.info("Succeed to connect exchange Server(ip:%s, port:%d, socket:%d)\n",m_traderSvrIP,m_shListenPort,m_msgConnFd);
	
	tcpConnector.Init(m_msgConnFd);
	
	//send msg to ATF
	stHead.iMsgID = htons(MSGID_C2S_TRADE_REQ_CONN);
	stHead.iMsgBodyLen = htons(0);					
	pSendBuf = sendBuf;
	memcpy(pSendBuf, &stHead, sizeof(stHead));
	iSendLen = sizeof(stHead);
					
	ret = TcpSendData(m_msgConnFd,(char*)pSendBuf,iSendLen);
	if(iSendLen != ret)
	{
		g_log.error("[STATUS_TRADE_INIT]TCP send error,expect %dB,actual %dB\n",iSendLen,ret);
		return ATF_FAIL;
	}
	g_log.debug("[STATUS_TRADE_INIT]TCP send %dB\n",ret);
						
	
	//receive msg from ATF
	pRcvBuf = rcvBuf;					
	iExpectRcvLen = sizeof(MsgHead) + sizeof(RspInfoFromATF);
	ret = TcpRecvData(m_msgConnFd,pRcvBuf,iExpectRcvLen);
	if(ret != iExpectRcvLen)
	{					
		g_log.error("[STATUS_TRADE_INIT]TCP rcv error,expect %dB,actual %dB\n",iExpectRcvLen,ret);
		return ATF_FAIL;
	}
	g_log.debug("[STATUS_TRADE_INIT]TCP rcv %dB\n",ret);
						
	pMsg = (MsgHead *)pRcvBuf;					   
	if(MSGID_S2C_TRADE_RSP_CONN == ntohs(pMsg->iMsgID))
	{
		RspInfoFromATF *pRsp = (RspInfoFromATF *)(pRcvBuf + sizeof(MsgHead));
							
		//tell client 
		if(0 == pRsp->ErrorID)
		{
			m_ftdcTraderSpi_instance->OnFrontConnected();

			//if(true == m_autoReconn)
				//SetState(STATUS_TRADE_REQ_LOGIN);
		}
		else
			m_ftdcTraderSpi_instance->OnFrontDisconnected(-1);
	}

	return ATF_SUCC;
}

int CThostFtdcTraderApi::HandleLogin()
{
	int ret = 0;
	char sendBuf[BUFSIZE] = {0};
	char rcvBuf[BUFSIZE] = {0};
	MsgHead stHead; 				
	int iSendLen = 0;	
	int iExpectRcvLen = 0;
	char * pSendBuf = NULL;
	char * pRcvBuf = NULL;
	MsgHead *pMsg = NULL;
	
	TradeReqUserLoginToATF * pReqUserLoginField = &m_userLoginField;
	stHead.iMsgID = htons(MSGID_C2S_TRADE_REQ_LOGIN);
	stHead.iMsgBodyLen = htons(sizeof(*pReqUserLoginField));
	pSendBuf = sendBuf;
	memcpy(pSendBuf, &stHead, sizeof(stHead));
	memcpy(pSendBuf+sizeof(stHead),pReqUserLoginField,sizeof(*pReqUserLoginField));
	iSendLen = sizeof(stHead) + sizeof(*pReqUserLoginField);	
	
	ret = TcpSendData(m_msgConnFd,pSendBuf,iSendLen);
	if(ret != iSendLen)
	{
		g_log.error("[STATUS_TRADE_REQ_LOGIN]TCP send error,expect %dB, actual %dB !\n",iSendLen,ret);
		return ATF_FAIL;
	}
	g_log.debug("[STATUS_TRADE_REQ_LOGIN]TCP send %dB.\n",ret);
	
	//rcv msg from ATF					
	pRcvBuf = rcvBuf;					
	iExpectRcvLen = sizeof(MsgHead) + sizeof(TradeRspUserLoginFromATF);
	ret = TcpRecvData(m_msgConnFd,pRcvBuf,iExpectRcvLen);
	if(ret != iExpectRcvLen)
	{					
		g_log.error("[STATUS_TRADE_REQ_LOGIN]TCP rcv error,expect %dB, actual %dB !\n",iExpectRcvLen,ret);
		return ATF_FAIL;
	}
	g_log.debug("[STATUS_TRADE_REQ_LOGIN]TCP rcv %dB.\n",ret);
					
	pMsg = (MsgHead *)pRcvBuf;					   
	if(MSGID_S2C_TRADE_RSP_LOGIN == ntohs(pMsg->iMsgID))
	{
		TradeRspUserLoginFromATF *pRsp = (TradeRspUserLoginFromATF *)(pRcvBuf + sizeof(MsgHead)); 
		m_ftdcTraderSpi_instance->OnRspUserLogin(&pRsp->rspUserLogin,&pRsp->rspInfo,pReqUserLoginField->nRequestID, true);
	}

	m_bLogined = true;
	
	return ATF_SUCC;
}

int CThostFtdcTraderApi::HandleSettlementConfirm()
{	
	//这个请求暂不需要发送到ATF
	CThostFtdcRspInfoField rspInfo;
	rspInfo.ErrorID = 0;
					
	CThostFtdcSettlementInfoConfirmField settlementInfoConfirm;					
	memcpy(&settlementInfoConfirm,&m_settlementInfoConfirm,sizeof(CThostFtdcSettlementInfoConfirmField));
	
	SetState(STATUS_TRADE_DEFAULT);
	
	m_ftdcTraderSpi_instance->OnRspSettlementInfoConfirm(&settlementInfoConfirm,&rspInfo,m_iRequestID,true);

	g_bTradeReady = true;

	return ATF_SUCC;
}	

int CThostFtdcTraderApi::HandleQryPosition()
{
	ReqQryPosToATF * pReq = &m_qryInvestorPosition;
	RspQryPosFromATF rsp;
	
	memset(&rsp,0,sizeof(rsp));
	rsp.rspInfo.ErrorID = 0;
	strcpy(rsp.pos.BrokerID,pReq->qry.BrokerID);
	strcpy(rsp.pos.InvestorID,pReq->qry.InvestorID);
	strcpy(rsp.pos.InstrumentID,pReq->qry.InstrumentID);
	
	SetState(STATUS_TRADE_DEFAULT);
	
	m_ftdcTraderSpi_instance->OnRspQryInvestorPosition(&rsp.pos,&rsp.rspInfo,pReq->nRequestID, true);

	return ATF_SUCC;
}

int CThostFtdcTraderApi::SendOrderToATT()
{
	int ret = 0;
	char sendBuf[BUFSIZE] = {0};
	MsgHead stHead; 				
	int iSendLen = 0;	
	char * pSendBuf = NULL;
	ReqOrderInsertToATF order;

	if(false == g_bTradeReady)
	{
		return ATF_FAIL;
	}
	
	if(ATF_TRUE == m_OrderQueue.PopOneData(&order))
	{
		m_llNumOrder--;
	
		static long long orderSentNum = 0;
		//g_SeqTime_PopOrder[orderSentNum%SEQTIME_COUNT]=time_get_now();
		//if(orderSentNum%2000==0)
		//{
			g_log.debug("Succeed to pop an order from queue.\n");
		//}
		//memset(&stHead,0,sizeof(stHead));	
		//memset(sendBuf,0,sizeof(sendBuf));	
		//memset(rcvBuf,0,sizeof(rcvBuf)); 
		
		//send order to ATF
		//InputOrderToATF *pInOrder = &pstTraderApi->m_inputOrder;
		stHead.iMsgID = htons(MSGID_C2S_TRADE_ReqOrderInsert);					
		stHead.iMsgBodyLen = htons(sizeof(order));
		pSendBuf = sendBuf;
		memcpy(pSendBuf, &stHead, sizeof(stHead));
		memcpy(pSendBuf+sizeof(stHead),&order,sizeof(order));
		iSendLen = sizeof(stHead) + sizeof(order);					
		
		ret = TcpSendData(m_msgConnFd,pSendBuf,iSendLen);
		if(iSendLen != ret)
		{
			g_log.error("TCP send ReqOrderInsert error,expect %dB, actual %dB !\n",iSendLen,ret);
			return ATF_FAIL;
		}		
		g_log.info("Succeed to send ReqOrderInsert(%dB)\n",ret);
		
		//g_SeqTime_SendOrder[orderSentNum%SEQTIME_COUNT]=time_get_now();
		//if((orderSentNum)%2000==0)
		//{
		//	g_log.info("========================================	pop an order to send(end) %lld\n",orderSentNum);
			/*
			extern long * g_SeqTime_PopQuote;
			extern long * g_SeqTime_PushOrder;
			extern long * g_SeqTime_PopOrder;
			extern long * g_SeqTime_SendOrder;
	
			*/
		//	g_log.info("========================================   :%06d,%06d,%06d,%06d\n",
		//	g_SeqTime_PopQuote[orderSentNum%SEQTIME_COUNT].tv_usec,
		//		g_SeqTime_PushOrder[orderSentNum%SEQTIME_COUNT].tv_usec,
		//		g_SeqTime_PopOrder[orderSentNum%SEQTIME_COUNT].tv_usec,
		//		g_SeqTime_SendOrder[orderSentNum%SEQTIME_COUNT].tv_usec
	
		//	);
		//	g_log.info("========================================   :%lu,%lu,%lu\n",
		//		time_diff_us(g_SeqTime_PopQuote[orderSentNum%SEQTIME_COUNT],g_SeqTime_PushOrder[orderSentNum%SEQTIME_COUNT]),
		//		time_diff_us(g_SeqTime_PushOrder[orderSentNum%SEQTIME_COUNT],g_SeqTime_PopOrder[orderSentNum%SEQTIME_COUNT]),
		//		time_diff_us(g_SeqTime_PopOrder[orderSentNum%SEQTIME_COUNT],g_SeqTime_SendOrder[orderSentNum%SEQTIME_COUNT])
		//		);
		//}
		
		orderSentNum++;
	
	}
	else
	{
		/*if(PerfTest)
			if(g_llNumRcvOrder > 0 && g_llNumRcvOrder<MAX_NUM_QUOTE)
			{
				static long long ifail = 0;
				g_log.fatal("order pop error %lld!\n",ifail);
				ifail++;
			}*/
		if(m_llNumOrder > 0)
		{
			g_log.error("Failed to pop an order from queue.\n");
		}
	}

	return ATF_SUCC;
}

int CThostFtdcTraderApi::SendCancelOrderToATT()
{
	int ret = 0;
	char sendBuf[BUFSIZE] = {0};
	MsgHead stHead; 				
	int iSendLen = 0;	
	char * pSendBuf = NULL;
	ReqOrderActionToATF orderAction;

	if(false == g_bTradeReady)
	{
		return ATF_FAIL;
	}
	
	if(ATF_TRUE == m_OrderActionQueue.PopOneData(&orderAction))
	{
		m_llNumCancelOrder--;
		
		stHead.iMsgID = htons(MSGID_C2S_TRADE_ReqOrderAction);					
		stHead.iMsgBodyLen = htons(sizeof(orderAction));
		pSendBuf = sendBuf;
		memcpy(pSendBuf, &stHead, sizeof(stHead));
		memcpy(pSendBuf+sizeof(stHead),&orderAction,sizeof(orderAction));
		iSendLen = sizeof(stHead) + sizeof(orderAction);					
		
		ret = TcpSendData(m_msgConnFd,pSendBuf,iSendLen);
		if(iSendLen != ret)
		{
			g_log.error("TCP send ReqOrderAction error,expect %dB, actual %dB !\n",iSendLen,ret);
			return ATF_FAIL;
		}
		g_log.info("TCP send ReqOrderAction %dB\n",ret);
	}
	else
	{
		if(m_llNumCancelOrder > 0)
		{
			g_log.error("Failed to pop an cancel order from queue.\n");
		}
	}

	return ATF_SUCC;
}

int CThostFtdcTraderApi::HandleMsgFromATT()
{
	int  old_flags;
	MsgHead * pRspHead =  NULL;
	MsgHead rcvHead;
	int msgHeadLen = sizeof(MsgHead);
	int msgBodyLen = 0;
	int totalLen = 0;

	if(false == m_bLogined)
	{
		return ATF_FAIL;
	}
	
	old_flags = fcntl(m_msgConnFd, F_GETFL, 0);
	if (fcntl(m_msgConnFd, F_SETFL, old_flags | O_NONBLOCK) == -1)
	{
		printf("[STATUS_TRADE_RSP_ORDER_INSERT] set nonblock error:%d(%s)\n", errno,strerror(errno));
	}
		
	tcpConnector.RecvData();
		
	do
	{
		if(msgHeadLen != tcpConnector.GetData((char*&)pRspHead,msgHeadLen))
		{
			//g_log.error("can't get enough data of MsgHead(%dB)\n",msgHeadLen);
			return ATF_FAIL;
		}
		else
		{
			memcpy(&rcvHead,pRspHead,msgHeadLen);
			pRspHead = &rcvHead;
			tcpConnector.FlushData(msgHeadLen);
			
			msgBodyLen = ntohs(pRspHead->iMsgBodyLen);
			char * pRspBody = NULL;
			if(msgBodyLen != tcpConnector.GetData((char*&)pRspBody,msgBodyLen))
			{
				g_log.error("can't get enough data of MsgBody(%d)\n",msgBodyLen);
				return ATF_FAIL;
			}
	
			totalLen = msgBodyLen + msgHeadLen;
			//head ok ,body ok either
			if(MSGID_S2C_TRADE_OnRtnOrder == ntohs(pRspHead->iMsgID))
			{
				g_log.debug("TCP rcv MSGID_S2C_TRADE_OnRtnOrder %dB\n",totalLen);
				OnRtnOrderFromATF *pRspMsg = (OnRtnOrderFromATF *)(pRspBody);					
				m_ftdcTraderSpi_instance->OnRtnOrder(&pRspMsg->order);					
				tcpConnector.FlushData(msgBodyLen);
			}
			else if(MSGID_S2C_TRADE_OnRtnTrade == ntohs(pRspHead->iMsgID))
			{
				g_log.debug("TCP rcv MSGID_S2C_TRADE_OnRtnTrade %dB\n",totalLen);
				OnRtnTradeFromATF *pRspMsg = (OnRtnTradeFromATF *)(pRspBody);					
	
				//DEBUG_TRADERSO("MSGID_S2C_TRADE_OnRtnTrade++++++++++++OrderSysID: %s\n",pRspMsg->trade.OrderSysID);	
				//DEBUG_TRADERSO("MSGID_S2C_TRADE_OnRtnTrade++++++++++++Volume: %d\n",pRspMsg->trade.Volume);	
					
				m_ftdcTraderSpi_instance->OnRtnTrade(&pRspMsg->trade);
					
				tcpConnector.FlushData(msgBodyLen);
			}							
			else if(MSGID_S2C_TRADE_OnRspOrderInsert == ntohs(pRspHead->iMsgID))
			{
				g_log.debug("TCP rcv MSGID_S2C_TRADE_OnRspOrderInsert %dB\n",totalLen);
					
				OnRspOrderInsertFromATF *pRspMsg = (OnRspOrderInsertFromATF *)(pRspBody);					
				CThostFtdcInputOrderField *pInputOrder = &pRspMsg->inputOrder;
				CThostFtdcRspInfoField *pRspInfo =	&pRspMsg->rspInfo;
				m_ftdcTraderSpi_instance->OnRspOrderInsert(pInputOrder,pRspInfo,pInputOrder->RequestID,true);
										
				tcpConnector.FlushData(msgBodyLen);
			}
			else if(MSGID_S2C_TRADE_OnErrRtnOrderInsert == ntohs(pRspHead->iMsgID))
			{
				g_log.debug("TCP rcv MSGID_S2C_TRADE_OnErrRtnOrderInsert %dB\n",totalLen);
										
				OnErrRtnOrderInsertFromATF *pErrRtn = (OnErrRtnOrderInsertFromATF *)(pRspBody); 					
				CThostFtdcInputOrderField *pInputOrder = &pErrRtn->inputOrder;
				CThostFtdcRspInfoField *pRspInfo =	&pErrRtn->rspInfo;
				m_ftdcTraderSpi_instance->OnErrRtnOrderInsert(pInputOrder,pRspInfo);
										
				tcpConnector.FlushData(msgBodyLen);
			}
			else if(MSGID_S2C_TRADE_OnRspOrderAction == ntohs(pRspHead->iMsgID))
			{
				g_log.debug("TCP rcv MSGID_S2C_TRADE_OnRspOrderAction %dB\n",totalLen);
					
				OnRspOrderActionFromATF *pErrRtn = (OnRspOrderActionFromATF *)(pRspBody);					
				CThostFtdcInputOrderActionField *pInputOrder = &pErrRtn->inputOrder;
				CThostFtdcRspInfoField *pRspInfo =	&pErrRtn->rspInfo;
				m_ftdcTraderSpi_instance->OnRspOrderAction(pInputOrder,pRspInfo,pInputOrder->RequestID,true);
										
				tcpConnector.FlushData(msgBodyLen);
			}
			else if(MSGID_S2C_TRADE_OnErrRtnOrderAction == ntohs(pRspHead->iMsgID))
			{
				g_log.debug("TCP rcv MSGID_S2C_TRADE_OnErrRtnOrderAction %dB\n",totalLen);
					
				OnErrRtnOrderActionFromATF *pErrRtn = (OnErrRtnOrderActionFromATF *)(pRspBody); 					
				CThostFtdcOrderActionField *pInputOrder = &pErrRtn->inputOrder;
				CThostFtdcRspInfoField *pRspInfo =	&pErrRtn->rspInfo;
				m_ftdcTraderSpi_instance->OnErrRtnOrderAction(pInputOrder,pRspInfo);								
										
				tcpConnector.FlushData(msgBodyLen);
			}
			else if(MSGID_S2C_TRADE_DISCONNECT == ntohs(pRspHead->iMsgID))
			{
				g_log.info("TCP rcv MSGID_S2C_TRADE_DISCONNECT %dB\n",totalLen);
					
				m_ftdcTraderSpi_instance->OnFrontDisconnected(0x1001);
				tcpConnector.FlushData(msgHeadLen);
	
				m_bLogined = false;
				m_autoReconn = true;
				g_bTradeReady = false;
				close(m_msgConnFd);
					
				SetState(STATUS_TRADE_INIT);
			}
			else
			{
				g_log.error("Undefined msgID, msgID = %d !\n" , ntohs(pRspHead->iMsgID));
				break;
			}
		}
	}while(1);
	fcntl(m_msgConnFd, F_SETFL, old_flags);

	return ATF_SUCC;
}

void CThostFtdcTraderApi::LockState()
{
	pthread_mutex_lock(&m_mutexState);

}
void CThostFtdcTraderApi::UnLockState()
{
	pthread_mutex_unlock(&m_mutexState);

}

int CThostFtdcTraderApi::Join()
{
	return 0;
}

const char *CThostFtdcTraderApi::GetTradingDay()
{
	char day[12] = {0};
	GotCurrentDate(day);
	return day;

}

int CThostFtdcTraderApi::ReqAuthenticate(CThostFtdcReqAuthenticateField *pReqAuthenticateField, int nRequestID)
{
	DEBUG_TRADERSO("\n");

	return 0;
}

int CThostFtdcTraderApi::ReqUserLogout(CThostFtdcUserLogoutField *pUserLogout, int nRequestID)
{
	DEBUG_TRADERSO("\n");

	return 0;
}

int CThostFtdcTraderApi::ReqUserPasswordUpdate(CThostFtdcUserPasswordUpdateField *pUserPasswordUpdate, int nRequestID) 
{
	DEBUG_TRADERSO("\n");

	return 0;
}

int CThostFtdcTraderApi::ReqTradingAccountPasswordUpdate(CThostFtdcTradingAccountPasswordUpdateField *pTradingAccountPasswordUpdate, int nRequestID) 
{
	return 0;

}
int CThostFtdcTraderApi::ReqParkedOrderInsert(CThostFtdcParkedOrderField *pParkedOrder, int nRequestID) 
{
	DEBUG_TRADERSO("\n");

	return 0;
}

int CThostFtdcTraderApi::ReqParkedOrderAction(CThostFtdcParkedOrderActionField *pParkedOrderAction, int nRequestID)
{
	DEBUG_TRADERSO("\n");

	return 0;
}
int CThostFtdcTraderApi::ReqQueryMaxOrderVolume(CThostFtdcQueryMaxOrderVolumeField *pQueryMaxOrderVolume, int nRequestID) 
{
	DEBUG_TRADERSO("\n");

	return 0;
}
 int CThostFtdcTraderApi::ReqRemoveParkedOrder(CThostFtdcRemoveParkedOrderField *pRemoveParkedOrder, int nRequestID)
 {
	return 0;
 }

 int CThostFtdcTraderApi::ReqRemoveParkedOrderAction(CThostFtdcRemoveParkedOrderActionField *pRemoveParkedOrderAction, int nRequestID)
 {
	 DEBUG_TRADERSO("\n");
	 return 0;
 }

 int CThostFtdcTraderApi::ReqQryOrder(CThostFtdcQryOrderField *pQryOrder, int nRequestID)
 {
	 return 0;
 }

int CThostFtdcTraderApi::ReqQryTrade(CThostFtdcQryTradeField *pQryTrade, int nRequestID) 
 {
	DEBUG_TRADERSO("\n");
	return 0;
 }
int CThostFtdcTraderApi::ReqQryTradingAccount(CThostFtdcQryTradingAccountField *pQryTradingAccount, int nRequestID)
 {
 	DEBUG_TRADERSO("\n");
	return 0;
 }

int CThostFtdcTraderApi::ReqQryInvestor(CThostFtdcQryInvestorField *pQryInvestor, int nRequestID) 
{
	DEBUG_TRADERSO("\n");

	return 0;
}


int CThostFtdcTraderApi::ReqQryTradingCode(CThostFtdcQryTradingCodeField *pQryTradingCode, int nRequestID) 
{
	DEBUG_TRADERSO("\n");

	return 0;
}

 int CThostFtdcTraderApi::ReqQryInstrumentMarginRate(CThostFtdcQryInstrumentMarginRateField *pQryInstrumentMarginRate, int nRequestID)
 {
	 DEBUG_TRADERSO("\n");
	 return 0;
 }

 int CThostFtdcTraderApi::ReqQryInstrumentCommissionRate(CThostFtdcQryInstrumentCommissionRateField *pQryInstrumentCommissionRate, int nRequestID) 
 {
	 return 0;
 }


 int CThostFtdcTraderApi::ReqQryExchange(CThostFtdcQryExchangeField *pQryExchange, int nRequestID) 
 {
 	 DEBUG_TRADERSO("\n");
	 return 0;
 }

 int CThostFtdcTraderApi::ReqQryInstrument(CThostFtdcQryInstrumentField *pQryInstrument, int nRequestID) 
 {
	 DEBUG_TRADERSO("\n");
	 return 0;
 }
 
int CThostFtdcTraderApi::ReqQryDepthMarketData(CThostFtdcQryDepthMarketDataField *pQryDepthMarketData, int nRequestID) 
{
	DEBUG_TRADERSO("\n");
	return 0;
}
 
int CThostFtdcTraderApi::ReqQrySettlementInfo(CThostFtdcQrySettlementInfoField *pQrySettlementInfo, int nRequestID) 
{
	DEBUG_TRADERSO("\n");
	return 0;
}
 
int CThostFtdcTraderApi::ReqQryTransferBank(CThostFtdcQryTransferBankField *pQryTransferBank, int nRequestID)
{
	DEBUG_TRADERSO("\n");
	return 0;
}
 
int CThostFtdcTraderApi::ReqQryInvestorPositionDetail(CThostFtdcQryInvestorPositionDetailField *pQryInvestorPositionDetail, int nRequestID) 
{
	DEBUG_TRADERSO("\n");
	return 0;
}
 
int CThostFtdcTraderApi::ReqQryNotice(CThostFtdcQryNoticeField *pQryNotice, int nRequestID) 
{
	DEBUG_TRADERSO("\n");
	return 0;
}
int CThostFtdcTraderApi::ReqQrySettlementInfoConfirm(CThostFtdcQrySettlementInfoConfirmField *pQrySettlementInfoConfirm, int nRequestID) 
{
	DEBUG_TRADERSO("\n");

	return 0;
}
 
int CThostFtdcTraderApi::ReqQryInvestorPositionCombineDetail(CThostFtdcQryInvestorPositionCombineDetailField *pQryInvestorPositionCombineDetail, int nRequestID) 
{
	DEBUG_TRADERSO("\n");
	return 0;
}
 
int CThostFtdcTraderApi::ReqQryCFMMCTradingAccountKey(CThostFtdcQryCFMMCTradingAccountKeyField *pQryCFMMCTradingAccountKey, int nRequestID)
{
	DEBUG_TRADERSO("\n");
	return 0;
}
 
int CThostFtdcTraderApi::ReqQryEWarrantOffset(CThostFtdcQryEWarrantOffsetField *pQryEWarrantOffset, int nRequestID)
{
	DEBUG_TRADERSO("\n");

	return 0;
}
 
int CThostFtdcTraderApi::ReqQryTransferSerial(CThostFtdcQryTransferSerialField *pQryTransferSerial, int nRequestID) 
{
	DEBUG_TRADERSO("\n");
	return 0;
}
 
int CThostFtdcTraderApi::ReqQryAccountregister(CThostFtdcQryAccountregisterField *pQryAccountregister, int nRequestID) 
{
	DEBUG_TRADERSO("\n");

	return 0;
}
 
int CThostFtdcTraderApi::ReqQryContractBank(CThostFtdcQryContractBankField *pQryContractBank, int nRequestID) 
{
	DEBUG_TRADERSO("\n");

	return 0;
}

int CThostFtdcTraderApi::ReqQryParkedOrder(CThostFtdcQryParkedOrderField *pQryParkedOrder, int nRequestID) 
{
	DEBUG_TRADERSO("\n");

	return 0;
}

int CThostFtdcTraderApi::ReqQryParkedOrderAction(CThostFtdcQryParkedOrderActionField *pQryParkedOrderAction, int nRequestID) 
{
	DEBUG_TRADERSO("\n");

	return 0;
}

int CThostFtdcTraderApi::ReqQryTradingNotice(CThostFtdcQryTradingNoticeField *pQryTradingNotice, int nRequestID)
{
	DEBUG_TRADERSO("\n");

	return 0;
}

int CThostFtdcTraderApi::ReqQryBrokerTradingParams(CThostFtdcQryBrokerTradingParamsField *pQryBrokerTradingParams, int nRequestID) 
{
	DEBUG_TRADERSO("\n");

	return 0;
}

int CThostFtdcTraderApi::ReqQryBrokerTradingAlgos(CThostFtdcQryBrokerTradingAlgosField *pQryBrokerTradingAlgos, int nRequestID) 
{
	DEBUG_TRADERSO("\n");

	return 0;
}

int CThostFtdcTraderApi::ReqFromBankToFutureByFuture(CThostFtdcReqTransferField *pReqTransfer, int nRequestID) 
{
	DEBUG_TRADERSO("\n");

	return 0;
}

int CThostFtdcTraderApi::ReqFromFutureToBankByFuture(CThostFtdcReqTransferField *pReqTransfer, int nRequestID) 
{
	DEBUG_TRADERSO("\n");

	return 0;
}

int CThostFtdcTraderApi::ReqQueryBankAccountMoneyByFuture(CThostFtdcReqQueryAccountField *pReqQueryAccount, int nRequestID) 
{
	DEBUG_TRADERSO("\n");

	return 0;
}



