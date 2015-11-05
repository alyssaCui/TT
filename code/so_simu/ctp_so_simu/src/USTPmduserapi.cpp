#include "USTPFtdcMduserApi.hpp"

#define SIG SIGRTMAX-1

CLog g_log;
//extern int cpuId;
//struct timeval * g_SeqTime_PushOrder;
//struct timeval * g_SeqTime_PopQuote;
//struct timeval * g_SeqTime_PopOrder;
//struct timeval * g_SeqTime_SendOrder;


/*struct timeval g_timeRcvQuote[MAX_TIME_STATISTICS_NUM] = {0};
long long g_rearTimeRcvQuote = 0;*/

struct tvTime_t * g_pTimeRcvQuote = NULL;
struct tvTime_t * g_pRearTimeRcvQuote = NULL;
long long g_llNumRcvQuote = 0;

extern bool g_bTradeReady;
//struct timeval g_lastTimeRcvQuote;
//StaticQuoteInteval g_staticQuoteInteval;


/**************************************************************
*
* CUstpFtdcMduserSpi      父类不做接口实现，由子类实现
*
***************************************************************/

CUstpFtdcMduserSpi::CUstpFtdcMduserSpi()
{

}
CUstpFtdcMduserSpi::~CUstpFtdcMduserSpi()
{

}

void CUstpFtdcMduserSpi::OnFrontConnected()
{

}
void CUstpFtdcMduserSpi::OnFrontDisconnected(int nReason)
{

}

void CUstpFtdcMduserSpi::OnHeartBeatWarning(int nTimeLapse)
{
}
	

void CUstpFtdcMduserSpi::OnRspUserLogin(CUstpFtdcRspUserLoginField *pRspUserLogin, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
}


void CUstpFtdcMduserSpi::OnRspUserLogout(CUstpFtdcRspUserLogoutField *pRspUserLogout, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
}


void CUstpFtdcMduserSpi::OnRspError(CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
}


void CUstpFtdcMduserSpi::OnRspSubMarketData(CUstpFtdcSpecificInstrumentField *pSpecificInstrument, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
}


void CUstpFtdcMduserSpi::OnRspUnSubMarketData(CUstpFtdcSpecificInstrumentField *pSpecificInstrument, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
}

void CUstpFtdcMduserSpi::OnRtnDepthMarketData(CUstpFtdcDepthMarketDataField *pDepthMarketData)
{
}

/****************************************************************************
*
* CThostFtdcMdApi         需要做接口实现，供应用开发者调用
*
***************************************************************************/
int CUstpFtdcMduserApi::m_iMdApis_used = 0;
CUstpFtdcMduserApi CUstpFtdcMduserApi::m_stMdApis[CUstpFtdcMduserApi::MAX_MDAPIS_NUM];

int AddQuoteCnt()
{
	struct tvTime_t *pTime = new(struct tvTime_t);
	pTime->pNext = NULL;
	pTime->tv = time_get_now();
	
	//g_log.debug("Rcv Quote %lld.\n",g_llNumRcvQuote);
	g_llNumRcvQuote++;
	
	if(NULL == g_pTimeRcvQuote)
	{
		g_pRearTimeRcvQuote = pTime;
		g_pTimeRcvQuote = pTime;
	}
	else
	{
		g_pRearTimeRcvQuote->pNext = pTime;
		g_pRearTimeRcvQuote = g_pRearTimeRcvQuote->pNext;
	}

	return ATF_SUCC;
}



void * ThostFtdcMdApiProc(void* para)
{
	CUstpFtdcMduserApi* pstMdApi = (CUstpFtdcMduserApi*)para;
	while(1)
	{
		switch(pstMdApi->m_iState)
		{
			case STATUS_QUOTE_DEFAULT:
				{
					//usleep(100);	
				}
				break;			
			case STATUS_QUOTE_INIT:
				{
					pstMdApi->HandleInit();
				}
				break;
			case STATUS_QUOTE_REQ_LOGIN:
				{					
					pstMdApi->HandleLogin();
				}
				break;
			case STATUS_QUOTE_REQ_SUBSCRIBE:
				{
					pstMdApi->HandleSubmarket();
					
					pstMdApi->SetState(STATUS_QUOTE_DEFAULT);
				}
				break;
			case STATUS_QUOTE_REQ_UNSUBSCRIBE:
				{
					pstMdApi->HandleUnsubmarket();
					
					pstMdApi->SetState(STATUS_QUOTE_DEFAULT);
				}
				break;
			default:
				g_log.debug("current status = %d \n",pstMdApi->m_iState);
				break;
		}


		//check if is a msg come from TT
		pstMdApi->HandleMsgFromATF();

		usleep(50);
	}

	return NULL;
}

CUstpFtdcMduserApi::CUstpFtdcMduserApi():m_Inited(0),m_iState(STATUS_QUOTE_DEFAULT)
{
	printf("Create CUstpFtdcMduserApi instance.\n");
	//m_iUseUdp = 0;
	memset(&m_userLoginField,0,sizeof(m_userLoginField));

	g_log.setFileLogFilter(LOG_LEVEL_DEBUG);
	g_log.setPrintLogFilter(LOG_LEVEL_DEBUG); 
}

CUstpFtdcMduserApi::~CUstpFtdcMduserApi()
{
	g_log.debug("\n");

	if(m_fdConnQuoteSvr >= 0)
		close(m_fdConnQuoteSvr);
}

/*非线程安全，不要多个线程同时调这个接口*/
CUstpFtdcMduserApi * CUstpFtdcMduserApi::CreateFtdcMduserApi(const char * pszFlowPath)
{
	if(m_iMdApis_used < MAX_MDAPIS_NUM)
	{
		if(pszFlowPath!=NULL)
		{
			strcpy(m_stMdApis[m_iMdApis_used].m_iLocalFile,pszFlowPath);
		}
		//m_stMdApis[m_iMdApis_used].m_iUseUdp = iUdp;

		return &m_stMdApis[m_iMdApis_used++];
	}
	else
	{
		return NULL;
	}
}

void CUstpFtdcMduserApi::RegisterSpi(CUstpFtdcMduserSpi* pSpi)
{
	g_log.debug("\n");
	m_FtdcMdSpi_instance = pSpi;
	
	return;
}

void CUstpFtdcMduserApi::RegisterFront(char *pszFrontAddress)  
{
	char url[32] = {0};
	char * pIp = NULL;
	char *pPort = NULL;

	g_log.debug("\n");

	if(NULL == pszFrontAddress)
	{
		g_log.error("Input param is NULL !");
		return;
	}

	strcpy(url,pszFrontAddress);
	
	if((pIp = strstr(url,"://"))!=NULL)
	{
		pIp = pIp + 3;
	}
	else
	{
		g_log.error("Front Address is invalid: no find \"://\" !");
       	return;
	}
	
	if((pPort = strchr(pIp,':'))!=NULL)
	{
		*pPort = '\0';
		
		pPort = pPort + 1;
		
		m_shListenPort = atoi(pPort) ;

		//strncpy(m_quoteSvrIP,pIp,pPort-pIp-1);
		strcpy(m_quoteSvrIP,pIp);

		g_log.debug("Input IP %s, port %d\n",m_quoteSvrIP,m_shListenPort);
	}
	else
	{
		g_log.error("Front Address is invalid: no find \":\" !");
	}
	
	return;
}

void CUstpFtdcMduserApi::Init()
{
	/*g_log.debug("UDP server: Port = %d\n",UDP_PORT_FOR_RCV_QUOTE);

	if( ATF_SUCC != m_UdpQuoteReceiver.Init(UDP_PORT_FOR_RCV_QUOTE,&m_QuoteQueue,sizeof(CThostFtdcDepthMarketDataField)) )
	{
		g_log.error("Failed to create UDP server.\n");
		return; 	
	}
	m_UdpQuoteReceiver.Start();
	*/
	//m_iNumInstr = 0;
	m_Inited = 1;
	m_bLogined = false;
	m_bQuoteReady = false;
	m_autoReconn = false;
	m_bQuoteOver = false;
	
	pthread_t tid;
	int ret = pthread_create(&tid,NULL,ThostFtdcMdApiProc,this);
	if (ret != 0)
	{	
		perror("Fail to create thread");
		return ;
	}
	
	SetState(STATUS_QUOTE_INIT);
	
	return;
}

void CUstpFtdcMduserApi::Reinit()
{
	close(m_fdConnQuoteSvr);

	m_bLogined = false;
	m_bQuoteReady = false;
	m_autoReconn = true;
	m_bQuoteOver = false;
	
	//clear the symbols subscribed
	SubscribeInstrument *pTmp = m_pInstrSubSucc;
	while(NULL != pTmp && NULL != pTmp->pNext)
	{
		m_pInstrSubSucc = pTmp->pNext;
		delete pTmp;
		pTmp = m_pInstrSubSucc;
	}
	delete m_pInstrSubSucc;
	m_pInstrSubSucc = NULL;

	timer_delete(m_timerid);
}

int CUstpFtdcMduserApi::ReqUserLogin(CUstpFtdcReqUserLoginField * pstUserLoginField,int nRequestID)
{
	//MdApiReqLoginPara stLoginPara;
	
	if(m_Inited==0)
	{
		g_log.error("not inited\n");
		return -1;
	}

    //交易接口会在响应或回报中返回与该请求相同的请求编号
	m_userLoginField.nRequestID = nRequestID;
	memcpy(&m_userLoginField.reqUserLogin,pstUserLoginField,sizeof(CUstpFtdcReqUserLoginField));
		
	SetState(STATUS_QUOTE_REQ_LOGIN);
	g_log.debug(" \n");

	return 0;
}

void CUstpFtdcMduserApi::SubscribeMarketDataTopic(int nTopicID, USTP_TE_RESUME_TYPE nResumeType)
{
	g_log.debug("SubscribeMarketDataTopic().....................\n");
}

int CUstpFtdcMduserApi::ReqSubscribeTopic(CUstpFtdcDisseminationField *pDissemination, int nRequestID)
{
	g_log.debug("ReqSubscribeTopic().......................\n");

}
int CUstpFtdcMduserApi::SubMarketData(char *ppInstrumentID[], int nCount)
{
	g_log.debug("\n");
	
	//目前trader3.0订阅一个合约，调用一次该接口
	strcpy(m_acInstrSubReq,ppInstrumentID[0]);
	//DEBUG_QUOTESO("recv Subscribe ppInstrumentID[0]:%s\n",m_acInstrSubReq);
	//m_iNumInstr++;
	SetState(STATUS_QUOTE_REQ_SUBSCRIBE);
	return 0;
}

int CUstpFtdcMduserApi::UnSubMarketData(char *ppInstrumentID[], int nCount)
{
	DEBUG_QUOTESO("\n");
	
	strcpy(m_acInstrUnsubReq,ppInstrumentID[0]);
	DEBUG_QUOTESO("recv UnSubscribe ppInstrumentID[0]:%s\n",m_acInstrSubReq);

	SetState(STATUS_QUOTE_REQ_UNSUBSCRIBE);
	
	return 0;
}


void CUstpFtdcMduserApi::Release()
{
	g_log.debug("\n");
}

const char *CUstpFtdcMduserApi::GetTradingDay()
{
	char day[12] = {0};
	GotCurrentDate(day);
	return day;
}

int CUstpFtdcMduserApi::Join()
{
	g_log.debug("\n");
	return 0;
}


int CUstpFtdcMduserApi::ReqUserLogout(CUstpFtdcReqUserLogoutField *pUserLogout, int nRequestID)
{
	DEBUG_QUOTESO("\n");
	return 0;
}

void CUstpFtdcMduserApi::SetState(FtdcMdApiState iState)
{
	m_iState = iState;
}

bool CUstpFtdcMduserApi::IsQuoteReady()
{
	//DEBUG_TRADERSO("\n");

	return m_bQuoteReady;
}

int CUstpFtdcMduserApi::ParseQuotePara(char * pPara)
{
	char *pHead = NULL;
	char *pEnd = NULL;
	char str[64] = {0};

	if(NULL == pPara)
	{
		g_log.error("Input parameter pPara = NULL !\n");
		return ATF_FAIL;
	}

	//find quote file to read
	pHead = strstr(pPara,"file=");
	if(NULL != pHead)
	{
		pHead = pHead + strlen("file=");
		pEnd = strchr(pHead,'|');
		if(NULL == pEnd)
			strcpy(str,pHead);
		else
			strncpy(str,pHead,pEnd-pHead);

		strcpy(m_acQuoteFile,str);
	}

	//find quote interval
	pHead = strstr(pPara,"interval=");
	if(NULL != pHead)
	{
		pHead = pHead + strlen("interval=");
		pEnd = strchr(pHead,'|');
		if(NULL == pEnd)
			strcpy(str,pHead);
		else
			strncpy(str,pHead,pEnd-pHead);
							
		m_iQuoteInterval = atoi(str);
	}

	return ATF_SUCC;
}

int CUstpFtdcMduserApi::QuoteProc()
{
	//pthread_t m_tid;
    //pthread_attr_t pthread_attr;
	
	//read quote from file
	m_quoteFetcher.SetFilePath(m_acQuoteFile);	
	m_quoteFetcher.Init(&m_QuoteQueue);   //init quote fetch thread
	m_quoteFetcher.Start();

	//create a thread to send quote
	/*pthread_attr_init(&pthread_attr);
	pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_DETACHED);
	if (0 != pthread_create(&m_tid,&pthread_attr,SendQuoteThreadFunc,this))
	{	
		g_log.error("Fail to create thread to send quote:%s\n", strerror(errno));  
		return ATF_FAIL;
	}*/

	CreateTimer();
	
	
	return ATF_SUCC;
}


int CUstpFtdcMduserApi::CreateTimer()
{
	struct sigaction sa;
	//sigset_t mask;
	struct sigevent sev;
	struct itimerspec its;

	long long freq_nanosecs = m_iQuoteInterval *1000;
	
	/* Establish handler for timer signal */
	sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = SendOneQuote;
    sigemptyset(&sa.sa_mask);
	if (sigaction(SIG, &sa, NULL) == -1)
	{
		g_log.error("sigaction\n");
		return ATF_FAIL;
	}

	  /* Create the timer */
	 sev.sigev_notify = SIGEV_SIGNAL;
	 sev.sigev_signo = SIG;
	 sev.sigev_value.sival_ptr = this;
	 if (timer_create(CLOCK_MONOTONIC, &sev, &m_timerid) == -1)
	 {
		 g_log.error("timer_create ,in startTimer()\n");
		 return ATF_FAIL;
	 }
	 g_log.debug("start timer ID is 0x%lx\n", (long)m_timerid);	 
	 
	 
	 /* Start the timer */
	 its.it_value.tv_sec = freq_nanosecs / 1000000000;
	 its.it_value.tv_nsec = freq_nanosecs % 1000000000;
	 its.it_interval.tv_sec = its.it_value.tv_sec;
	 its.it_interval.tv_nsec = its.it_value.tv_nsec;
	 if (timer_settime(m_timerid, 0, &its, NULL) == -1)
	 {
		 g_log.error("timer_settime, in startTimer()\n");
		 return ATF_FAIL;
	 }	

	 return ATF_SUCC;
}

void CUstpFtdcMduserApi::SendOneQuote(int sig, siginfo_t *si, void *uc)
{
	int ret = 0;
	int iOldErr;
	int iOverRun;
	char sendBuf[BUFSIZE] = {0};
	MsgHead stHead; 				
	int iSendLen = 0;	
	char * pSendBuf = NULL;
	CUstpFtdcDepthMarketDataField data;

    //在信号处理函数开始时保存errno
	iOldErr = errno;

	CUstpFtdcMduserApi* pObj = (CUstpFtdcMduserApi*)si->si_value.sival_ptr;
	
    iOverRun = timer_getoverrun(pObj->m_timerid);
    if (-1 == iOverRun)
    {
		 printf_blue("timer_getoverrun() return %d.\n",iOverRun);//g_log.error
    }
    else
    {
		 printf_blue("timer_getoverrun() return %d.\n",iOverRun);
    }
	

	//no instrument subscribed
	if(NULL == pObj->m_pInstrSubSucc)
	{
		errno = iOldErr;  //在信号处理函数结束的时候恢复被中断线程的 errno 值
		g_log.error("No instrument subscribed !\n");	
		return ;
	}
		
	if(ATF_SUCC == pObj->m_QuoteQueue.PopOneData(&data))
	{
		AddQuoteCnt();
		
		SubscribeInstrument *pTmp = pObj->m_pInstrSubSucc;
		while(pTmp != NULL)
		{
			printf_green("ready to send a quote\n");
			strcpy(data.InstrumentID,pTmp->instrument);
			pObj->m_FtdcMdSpi_instance->OnRtnDepthMarketData(&data);
				
			pTmp = pTmp->pNext;
		}
	}
	else
	{
	    //read quote over
		if((1 == pObj->m_quoteFetcher.GetFlagQuoteReadOver()))
		{
			if(true == pObj->m_bQuoteOver)
			{			
			    errno = iOldErr;  //在信号处理函数结束的时候恢复被中断线程的 errno 值
				return;
			}
			printf_green("read quote over\n");
			pObj->m_bQuoteOver = true;
				
			pObj->m_quoteFetcher.Terminate();
				
			memset(sendBuf,0,sizeof(sendBuf));					
			memset(&stHead,0,sizeof(stHead));
				
			//send msg to ATF
			ResultOfSendQuote result;
			result.isSucc = true;
				
			pSendBuf = sendBuf;
			stHead.iMsgID = htons(MSGID_C2S_QUOTE_RESULT_SEND_QUOTE);
			stHead.iMsgBodyLen = htons(sizeof(ResultOfSendQuote));
				
			memcpy(pSendBuf , &stHead, sizeof(stHead));
			memcpy(pSendBuf+sizeof(stHead),&result,sizeof(result));
			iSendLen = sizeof(stHead) + sizeof(result);
				
			ret = TcpSendData(pObj->m_fdConnQuoteSvr,(char*)pSendBuf,iSendLen);
			if(ret != iSendLen)
			{
				g_log.error("[MSGID_C2S_QUOTE_RESULT_SEND_QUOTE]TCP send error,expect %dB,actual %dB.\n",iSendLen,ret);
				
				errno = iOldErr;  //在信号处理函数结束的时候恢复被中断线程的 errno 值
				
				return ;
			}
			g_log.debug("[MSGID_C2S_QUOTE_RESULT_SEND_QUOTE]TCP send %dB \n",ret);
		}
		else
		{
			g_log.error("quote pop error!\n");	
		}
	}
	
	errno = iOldErr;  //在信号处理函数结束的时候恢复被中断线程的 errno 值

	return ;
}

int CUstpFtdcMduserApi::HandleInit()
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

	m_fdConnQuoteSvr = TcpConnect(m_quoteSvrIP,m_shListenPort);	
	if(m_fdConnQuoteSvr < 0)
	{
		g_log.error("Failed to connect QUOTE SERVER(ip:%s, port:%d, socket:%d)\n",m_quoteSvrIP,m_shListenPort,m_fdConnQuoteSvr);
		sleep(5);//客户端与服务端连接断开后，交易接口会自动尝试重新连接，频率是每5 秒一次。
		return ATF_FALSE;
	}
	g_log.info("Succeed to connect QUOTE SERVER(ip:%s, port:%d, socket:%d)\n",m_quoteSvrIP,m_shListenPort,m_fdConnQuoteSvr);
	

	//send msg to ATF
	stHead.iMsgID = htons(MSGID_C2S_QUOTE_REQ_CONN);
	stHead.iMsgBodyLen = htons(0);					
	pSendBuf = sendBuf; 	
	iSendLen = sizeof(stHead);
	memcpy(pSendBuf, &stHead, iSendLen);
						
	ret = TcpSendData(m_fdConnQuoteSvr,(char*)pSendBuf,iSendLen);
	if(ret != iSendLen)
	{
		g_log.error("[STATUS_QUOTE_INIT]TCP send error! \n");
		return ATF_FAIL;
	}
	g_log.debug("[STATUS_QUOTE_INIT]TCP send out %d bytes\n",ret);
	
	
	//receive msg from ATF					  
	iExpectRcvLen = sizeof(MsgHead) + sizeof(RspInfoFromATF);
	pRcvBuf = rcvBuf;
	ret = TcpRecvData(m_fdConnQuoteSvr,pRcvBuf,iExpectRcvLen);
	if(ret != iExpectRcvLen)
	{					
		g_log.error("[STATUS_QUOTE_INIT]TCP receive error! \n");
		return ATF_FAIL;
	}
	g_log.debug("[STATUS_QUOTE_INIT]recv rsp %d bytes\n",ret);
						
	pMsg = (MsgHead *)pRcvBuf;	 
	if(MSGID_S2C_QUOTE_RSP_CONN == ntohs(pMsg->iMsgID))
	{
		RspInfoFromATF *pRsp = (RspInfoFromATF *)(pRcvBuf + sizeof(MsgHead));
							
		//tell client 
		if(0 == pRsp->ErrorID)
		{
			m_FtdcMdSpi_instance->OnFrontConnected();
			
			//if(true == m_autoReconn)
				//SetState(STATUS_QUOTE_REQ_LOGIN);
		}
		else
			m_FtdcMdSpi_instance->OnFrontDisconnected(-1);

		return ATF_SUCC;
	}
	else
	{
		g_log.debug("[STATUS_QUOTE_INIT]msgID error, %d,%d\n",pMsg->iMsgID,ntohs(pMsg->iMsgID));
		return ATF_FAIL;
	}
}

int CUstpFtdcMduserApi::HandleLogin()
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
	
	//send msg to ATF
	QuoteReqUserLoginToATF * pReqUserLoginField = &m_userLoginField;					
	stHead.iMsgID = htons(MSGID_C2S_QUOTE_REQ_LOGIN);
	stHead.iMsgBodyLen = htons(sizeof(*pReqUserLoginField));
	pSendBuf = sendBuf;
	memcpy(pSendBuf, &stHead, sizeof(stHead));
	memcpy(pSendBuf+sizeof(stHead),pReqUserLoginField,sizeof(*pReqUserLoginField));
	iSendLen = sizeof(stHead) + sizeof(*pReqUserLoginField);
	
	ret = TcpSendData(m_fdConnQuoteSvr,pSendBuf,iSendLen);
	if(ret != iSendLen)
	{
		g_log.error("[STATUS_QUOTE_REQ_LOGIN]TCP send error,expect %dB, actual %dB !\n",iSendLen,ret);
		return ATF_FAIL;
	}
	g_log.debug("[STATUS_QUOTE_REQ_LOGIN]TCP send %dB.\n",iSendLen);
	
	//rcv msg from ATF					
	pRcvBuf = rcvBuf;
	iExpectRcvLen = sizeof(MsgHead) + sizeof(QuoteRspUserLoginFromATF);
	ret = TcpRecvData(m_fdConnQuoteSvr,pRcvBuf,iExpectRcvLen);
	if(ret != iExpectRcvLen)
	{					
		g_log.error("[STATUS_QUOTE_REQ_LOGIN]TCP rcv error,expect %dB, actual %dB !\n",iExpectRcvLen,ret);
		return ATF_FAIL;
	}
	g_log.debug("[STATUS_QUOTE_REQ_LOGIN]TCP rcv %dB.\n",ret);
						
	pMsg = (MsgHead *)pRcvBuf;					   
	if(MSGID_S2C_QUOTE_RSP_LOGIN == ntohs(pMsg->iMsgID))
	{
		QuoteRspUserLoginFromATF *pRsp = (QuoteRspUserLoginFromATF *)(pRcvBuf + sizeof(MsgHead)); 		
							
		m_FtdcMdSpi_instance->OnRspUserLogin(&pRsp->rspUserLogin,&pRsp->rspInfo,pReqUserLoginField->nRequestID, true);
							
		m_bLogined = true;

		/*if(true == m_autoReconn)
			SetState(STATUS_QUOTE_REQ_SUBSCRIBE);*/
		
		return ATF_SUCC;
	}
	else
	{
		g_log.debug("[STATUS_QUOTE_REQ_LOGIN]msgID error, %d,%d\n",pMsg->iMsgID,ntohs(pMsg->iMsgID));

		return ATF_FAIL;
	}
}

int CUstpFtdcMduserApi::HandleSubmarket()
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
	
	//send msg to ATF
	ReqSubMarketToATF subInfo;
	strcpy(subInfo.instr,m_acInstrSubReq);
						
	stHead.iMsgID = htons(MSGID_C2S_QUOTE_REQ_SUBSCRIBE);
	stHead.iMsgBodyLen = htons(sizeof(subInfo));
	
	pSendBuf = sendBuf;
	memcpy(pSendBuf , &stHead, sizeof(stHead));
	memcpy(pSendBuf+sizeof(stHead),&subInfo,sizeof(subInfo));
	iSendLen = sizeof(stHead) + sizeof(subInfo);
						
	ret = TcpSendData(m_fdConnQuoteSvr,(char*)pSendBuf,iSendLen);
	if(ret != iSendLen)
	{
		g_log.error("[STATUS_QUOTE_REQ_SUBSCRIBE]TCP send error,expect %dB,actual %dB.\n",iSendLen,ret);
		return ATF_FAIL;
	}
	g_log.debug("[STATUS_QUOTE_REQ_SUBSCRIBE]TCP send %dB \n",ret);
	
	//rcv msg from ATF						
	iExpectRcvLen = sizeof(MsgHead) + sizeof(RspSubMarketFromATF);
	pRcvBuf = rcvBuf;
	ret = TcpRecvData(m_fdConnQuoteSvr,pRcvBuf,iExpectRcvLen);
	if(ret != iExpectRcvLen)
	{					
		g_log.error("[STATUS_QUOTE_REQ_SUBSCRIBE]TCP rcv error,expect %dB,actual %dB.\n",iExpectRcvLen,ret);
		return ATF_FAIL;
	}
	g_log.debug("[STATUS_QUOTE_REQ_SUBSCRIBE]TCP rcv %dB \n",iExpectRcvLen);
						
	pRcvBuf = rcvBuf;
	pMsg = (MsgHead *)pRcvBuf;					   
	if(MSGID_S2C_QUOTE_RSP_SUBSCRIBE == ntohs(pMsg->iMsgID))
	{
		RspSubMarketFromATF *pRsp = (RspSubMarketFromATF *)(pRcvBuf + sizeof(MsgHead)); 	
							
		if(0 == pRsp->rspInfo.ErrorID)
		{	
			//对于同时订阅多个合约的场景
			//目前采用从一个单合约文件中读取行情,每个行情数据被发送多次,每次根据订阅信息改变合约名,发送给trader3.0
			//仅根据第一次收到的行情参数来进行配置
			if((false == m_bQuoteReady))
			{
				if(ATF_SUCC == ParseQuotePara(pRsp->QuoteSendPara))
				{
					m_bQuoteReady = true;
					
					QuoteProc();					
				}
			}
	
			//insert new instrument to m_pInstrSubSucc
			InsertNewInstr(&pRsp->specificInstrument);
		}
						
		m_FtdcMdSpi_instance->OnRspSubMarketData(&pRsp->specificInstrument,&pRsp->rspInfo,m_iRequestID,true);

		return ATF_SUCC;
	}
	else
	{
		g_log.error("[STATUS_QUOTE_REQ_SUBSCRIBE]TCP rcv error,expect %dB,actual %dB.\n",MSGID_S2C_QUOTE_RSP_SUBSCRIBE,ntohs(pMsg->iMsgID));
		return ATF_FAIL;
	}
}

int CUstpFtdcMduserApi::HandleUnsubmarket()
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
	RspUnsubMarketFromATF stRsp;
	RspUnsubMarketFromATF *pRsp = NULL;

	//no a instrument has been subscribed
	if(NULL == m_pInstrSubSucc)
	{
		g_log.error("No an instrument subscribed !\n");
		strcpy(stRsp.specificInstrument.InstrumentID,m_acInstrUnsubReq);
		stRsp.rspInfo.ErrorID = 16;
		pRsp = &stRsp;
	}
	else
	{
		SubscribeInstrument *pTmp = m_pInstrSubSucc;
		while(NULL != pTmp)
		{
			//the instrument unsubscribed has been subscribed
			if(0 == strcasecmp(pTmp->instrument,m_acInstrUnsubReq))
			{
				//send msg to ATF
				ReqUnsubMarketToATF unsubInfo;
				strcpy(unsubInfo.instr,m_acInstrUnsubReq);
									
				stHead.iMsgID = htons(MSGID_C2S_QUOTE_REQ_UNSUBSCRIBE);
				stHead.iMsgBodyLen = htons(sizeof(unsubInfo));
				
				pSendBuf = sendBuf;
				memcpy(pSendBuf , &stHead, sizeof(stHead));
				memcpy(pSendBuf+sizeof(stHead),&unsubInfo,sizeof(unsubInfo));
				iSendLen = sizeof(stHead) + sizeof(unsubInfo);
									
				ret = TcpSendData(m_fdConnQuoteSvr,(char*)pSendBuf,iSendLen);
				if(ret != iSendLen)
				{
					g_log.error("[STATUS_QUOTE_REQ_UNSUBSCRIBE]TCP send error,expect %dB,actual %dB.\n",iSendLen,ret);
					return ATF_FAIL;
				}
				g_log.debug("[STATUS_QUOTE_REQ_UNSUBSCRIBE]TCP send %dB \n",ret);


				//rcv msg from ATF						
				iExpectRcvLen = sizeof(MsgHead) + sizeof(RspUnsubMarketFromATF);
				pRcvBuf = rcvBuf;
				ret = TcpRecvData(m_fdConnQuoteSvr,pRcvBuf,iExpectRcvLen);
				if(ret != iExpectRcvLen)
				{					
					g_log.error("[STATUS_QUOTE_REQ_UNSUBSCRIBE]TCP rcv error,expect %dB,actual %dB.\n",iExpectRcvLen,ret);
					return ATF_FAIL;
				}
				g_log.debug("[STATUS_QUOTE_REQ_UNSUBSCRIBE]TCP rcv %dB \n",iExpectRcvLen);
									
				pMsg = (MsgHead *)pRcvBuf;		
				if(MSGID_S2C_QUOTE_RSP_UNSUBSCRIBE == ntohs(pMsg->iMsgID))
				{
					pRsp = (RspUnsubMarketFromATF *)(pRcvBuf + sizeof(MsgHead)); 	

					if(0 == pRsp->rspInfo.ErrorID)
					{
						//remove the instrument unsubscribed from m_pInstrSubSucc
						RemoveInstr(&pRsp->specificInstrument);
					}
				}
				else
				{
					g_log.error("[STATUS_QUOTE_REQ_UNSUBSCRIBE]TCP rcv error,expect %dB,actual %dB.\n",MSGID_S2C_QUOTE_RSP_UNSUBSCRIBE,ntohs(pMsg->iMsgID));

					strcpy(stRsp.specificInstrument.InstrumentID,m_acInstrUnsubReq);
					stRsp.rspInfo.ErrorID = 16;
					pRsp = &stRsp;
				}

				break;
			}
			else
			{
				pTmp = pTmp->pNext;
			}
		}

		//the instrument unsubscribed had not been found in the subscribe list
		if(NULL == pTmp)  
		{
			strcpy(stRsp.specificInstrument.InstrumentID,m_acInstrUnsubReq);
			stRsp.rspInfo.ErrorID = 16;
			pRsp = &stRsp;
			
			g_log.error("the instrument unsubscribed had not been found in the subscribe list !\n");
		}

	}

	//PrintInstr();

	//return rsp to trader3.0
	m_FtdcMdSpi_instance->OnRspUnSubMarketData(&pRsp->specificInstrument,&pRsp->rspInfo,m_iRequestID,true);

	return ATF_SUCC;
}

void CUstpFtdcMduserApi::PrintInstr()
{
	SubscribeInstrument *pInstr = m_pInstrSubSucc;
	
	while(pInstr != NULL)
	{
		DEBUG_RED("######################%s\n",pInstr->instrument);
		pInstr = pInstr->pNext; 
	}
}

int CUstpFtdcMduserApi::RemoveInstr(CUstpFtdcSpecificInstrumentField * pInst)
{
	if(NULL == pInst)
	{
		g_log.error("Input parameter pInst = NULL !\n");
		return ATF_FAIL;
	}
	
	SubscribeInstrument *pFront = m_pInstrSubSucc;
	SubscribeInstrument *pEnd = m_pInstrSubSucc;
	while(NULL != pFront)
	{
		if(0 == strcasecmp(pFront->instrument,pInst->InstrumentID))
		{
			if(pFront == m_pInstrSubSucc)
			{
				m_pInstrSubSucc = pFront->pNext;
				delete pFront;	
				pFront = NULL;
			}
			else
			{
				pEnd->pNext = pFront->pNext;
				delete pFront;
				pFront = NULL;
			}
			
			break;
		}
		else
		{
			pEnd = pFront;
			pFront = pFront->pNext;
		}
	}
}

int CUstpFtdcMduserApi::InsertNewInstr(CUstpFtdcSpecificInstrumentField * pInst)
{
	if(NULL == pInst)
	{
		g_log.error("Input parameter pInst = NULL !\n");
		return ATF_FAIL;
	}
	
	SubscribeInstrument *pNew = new SubscribeInstrument;
	strcpy(pNew->instrument,pInst->InstrumentID);
	pNew->pNext = NULL;
						
	SubscribeInstrument *pTmp = m_pInstrSubSucc;
	if(NULL == pTmp)
	{
		m_pInstrSubSucc = pNew;
	}
	else
	{
		while(pTmp != NULL)
		{
			if(0 == strcmp(pTmp->instrument,pNew->instrument))
			{
				g_log.warn("Have subscribed %s\n",pNew->instrument);
				return ATF_FAIL;
			}
	
			if(pTmp->pNext != NULL)
			{
				pTmp = pTmp->pNext;
			}
			else
			{
				pTmp->pNext = pNew;
				break;
			}
		}
	}

	return ATF_SUCC;
}




int CUstpFtdcMduserApi::HandleMsgFromATF()
{
	char rcvBuf[BUFSIZE] = {0};
	char * pRcvBuf = NULL;
	MsgHead *pMsg = NULL;

	/*======check if is a msg(MSGID_S2C_QUOTE_DISCONNECT) come from ATF======*/
	if(true == m_bLogined)
	{
		pRcvBuf = rcvBuf;
		int iExpectRcvLen = sizeof(MsgHead);
		if(iExpectRcvLen == TcpTimedRecvData(m_fdConnQuoteSvr,pRcvBuf,iExpectRcvLen,50))
		{
			pMsg = (MsgHead *)pRcvBuf;					   
			if(MSGID_S2C_QUOTE_DISCONNECT == ntohs(pMsg->iMsgID))
			{
				g_log.info("Received  MSGID_S2C_QUOTE_DISCONNECT, %dB\n",iExpectRcvLen);
				
				m_FtdcMdSpi_instance->OnFrontDisconnected(0x1001);
				
				Reinit();
				
				SetState(STATUS_QUOTE_INIT);

				return ATF_SUCC;
			}
		}
		
	}
	else
		return ATF_FAIL;

	return ATF_SUCC;
}
