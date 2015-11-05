#include <dirent.h>

#include "tinyxml.h"
#include "tinystr.h"

#include "task.hpp"

//extern char g_progDir[MAX_FILE_PATH_LEN];
extern int g_orderLocalID; 


CTask::CTask()
{
}

int CTask::Init(TaskConf conf)
{
	memcpy(&m_quoteConf,&conf.g_quoteConf,sizeof(m_quoteConf));
	memcpy(&m_exchgConf,&conf.g_exchgConf,sizeof(m_exchgConf));
	memcpy(&m_agentConf,&conf.g_agentConf,sizeof(m_agentConf));
	memcpy(&m_gtestConf,&conf.gtestConf,sizeof(m_gtestConf));
	
	strcpy(m_arrTestcasesPath,conf.m_arrTestcasesPath);
	strcpy(m_arrTestDataPath,conf.m_arrTestDataPath);
	strcpy(m_progPath,conf.progDir);

    m_pTestcasesFileList = NULL;

	m_agentSocket = 0;

	return ATF_TRUE;
}

CTask::~CTask()
{
	struct FileNode *pNode = NULL;

	g_log.debug("enter ~CTask deconstructor()\n");
	
	while(NULL != m_pTestcasesFileList)
	{
        pNode = m_pTestcasesFileList;
		m_pTestcasesFileList = m_pTestcasesFileList->next;

		delete pNode;
		pNode = NULL;
	}
	
	TestCase *pTmp = m_casesList;
	while(NULL != pTmp->next)
	{
		m_casesList = pTmp->next;
		CleanCase(pTmp);
		delete pTmp;
		pTmp = m_casesList;
	}

	
    if(NULL == m_pQuoteSvr)
    {
		delete m_pQuoteSvr;
		m_pQuoteSvr = NULL;
	}
	
    if(NULL == m_pExchgSvr)
    {
		delete m_pExchgSvr;
		m_pExchgSvr = NULL;
	}

	
	if(m_agentSocket != 0)
		close(m_agentSocket);
}

int CTask::ExecuteWorkItem()
{
	
	return ATF_SUCC;
}

int CTask::CleanCase(TestCase *pCase)
{
	OPERATE_SET::iterator iterCur;
	OPERATE_SET::iterator iterEnd;
	
	for(unsigned int i=0;i<pCase->quoteOrder.uiProvidersTotal;i++)
	{
		iterCur = pCase->quoteOrder.quoteOp[i].begin();
		iterEnd = pCase->quoteOrder.quoteOp[i].end();
		 
		for (; iterCur != iterEnd; ++iterCur)
		{
			OP op=*iterCur;
			if(NULL != op.pMsg)
				delete op.pMsg;
			//pItem->quoteOp.clean(op);
		}
	}

	for(unsigned int i=0;i<pCase->exchgOrder.uiClientsTotal;i++)
	{
		iterCur = pCase->exchgOrder.exchgOp[i].begin();
		iterEnd = pCase->exchgOrder.exchgOp[i].end();
		for (; iterCur != iterEnd; ++iterCur)
		{
			OP op=*iterCur;
			if(NULL != op.pMsg)
				delete op.pMsg;
			//pItem->exchgOp.clean(op);
		}
	}
	
	return ATF_SUCC;

}


int CTask::finishTransact()
{
	m_allTransact.finish();

	return ATF_SUCC;
}
int CTask::run()
{
	unsigned long timeDiff = 0;
	TestCase *pCase = NULL;
    unsigned int uiCaseDone = 0;
	CCasesParser cases;
	
    //load test cases 
	if(ATF_SUCC != cases.GetCasesList(m_arrTestcasesPath,m_casesList,&m_uiTotalCases))
	{
		g_log.error("Failed to get cases list !\n");
		return ATF_FAIL;
	}

	if(READABLE == m_exchgConf.isValid)
		m_pExchgSvr = new ExchgServer(m_exchgConf);
	if(READABLE == m_quoteConf.isValid)
		m_pQuoteSvr = new QuoteServer(m_quoteConf);
	
    pCase = m_casesList;
	while(NULL != pCase)
	{
		//g_log.info("[RUN       ] %s :\n",pItem->fileName);
		printf_green("[RUN       ] %s :\n",pCase->fileName);

		pCase->startTime = time_get_now();

		if(0 == uiCaseDone)
			m_stTimeStartTask = pCase->startTime;
		
		pCase->status = WORKITEM_DOING;

		//m_pExchgSvr->SetTradeMode(pCase->exchgOrder.tradeMode);
		
		//start exchange simulator
		if(READABLE == m_exchgConf.isValid)
			m_pExchgSvr->StartNewOrder(&pCase->exchgOrder);

		//start quote data server
		if(READABLE == m_quoteConf.isValid)
			m_pQuoteSvr->StartNewOrder(&pCase->quoteOrder);
		

		if(READABLE == m_quoteConf.isValid)
		{
			if(READABLE == m_exchgConf.isValid)
			{
				if(LIMITED == pCase->exchgOrder.tradeMode)
				{
					while(1)
					{
						sleep(1);	
						if(NO_TASK == m_pQuoteSvr->m_status && NO_TASK == m_pExchgSvr->m_status)
						{
							g_log.debug("%s done.\n",pCase->fileName);
							break;
						}
					}
				}
				/*else
				{
					while(!m_pQuoteSvr->GetQuoteSendFlag())
					{
						sleep(1);			
					}
					sleep(1);
					m_pExchgSvr->Finish();
				}*/
			}
			else
			{
				while(1)
				{
					sleep(1);
					if(NO_TASK == m_pQuoteSvr->m_status)
					{
						g_log.debug("%s done.\n",pCase->fileName);
						break;
					}
				}
			}
		}
		else
		{
			if(READABLE == m_exchgConf.isValid)
			{
				if(LIMITED == pCase->exchgOrder.tradeMode)
				{
					while(1)
					{
						sleep(1);	
						if(NO_TASK == m_pExchgSvr->m_status)
						{
							g_log.debug("%s done.\n",pCase->fileName);
							break;
						}
					}
				}
			}
		}

		
		pCase->endTime = time_get_now();
		if(uiCaseDone == m_uiTotalCases - 1)
			m_stTimeEndTask = pCase->endTime;
		timeDiff = time_diff_ms(pCase->startTime,pCase->endTime);
		
		GetCaseResult(pCase);		
		if(WORKITEM_DONE_SUCC == pCase->status)
		{
			//g_log.info("[==SUCCESS==]%s(%lu ms)\n\n",pItem->fileName,timeDiff);
			printf_green("[==SUCCESS==]%s(%lu ms)\n\n",pCase->fileName,timeDiff);
		}
		else if(WORKITEM_DONE_FAIL == pCase->status)
		{
			//g_log.info("[==FAILED==]%s(%lu ms)\n\n",pItem->fileName,timeDiff);
			printf_red("[==FAILED==]%s(%lu ms)\n\n",pCase->fileName,timeDiff);
		}

		pCase = pCase->next;
		uiCaseDone++;
	}


	finishTransact();

	//get static info from ATF agent
	GotStaticInfo();
	
	report();	

	printf("All test cases done.\n");

	return ATF_SUCC;

}

int CTask::GetCaseResult(TestCase *pCase)
{
	OPERATE_SET::iterator iterCur;
	OPERATE_SET::iterator iterEnd;
	
	if(READABLE == m_quoteConf.isValid)
	{
		for(unsigned int i=0;i<pCase->quoteOrder.uiProvidersTotal;i++)
		{
			iterCur = pCase->quoteOrder.quoteOp[i].begin();
			iterEnd = pCase->quoteOrder.quoteOp[i].end();

			for (; iterCur != iterEnd; ++iterCur)
			{
				printf_yellow("===============\n");
				printf_yellow("msg(%d):result(%d),reason(%s)\n",iterCur->msgID,iterCur->result,iterCur->errReason);

				if(OPERATE_SUCC != iterCur->result)
				{
					pCase->status = WORKITEM_DONE_FAIL;
					strcpy(pCase->errReason,iterCur->errReason);
					m_uiFailCases++;
					
					g_log.error("[==ERROR==]msg(%d):result(%d),reason(%s)\n",iterCur->msgID,iterCur->result,iterCur->errReason);
					return ATF_FAIL;
				}
				
			}
		}
	}
	
	if(READABLE == m_exchgConf.isValid)
	{
		for(unsigned int i=0;i<pCase->exchgOrder.uiClientsTotal;i++)
		{
			iterCur = pCase->exchgOrder.exchgOp[i].begin();
			iterEnd = pCase->exchgOrder.exchgOp[i].end();
			for (; iterCur != iterEnd; ++iterCur)
			{
				printf_blue("===============\n");
				printf_blue("msg(%d):result(%d),reason(%s)\n",iterCur->msgID,iterCur->result,iterCur->errReason);

				OP op =* iterCur;
				if(OPERATE_SUCC != op.result)
				{
					pCase->status = WORKITEM_DONE_FAIL;
					strcpy(pCase->errReason,op.errReason);
					m_uiFailCases++;
					
					g_log.info("[==ERROR==]msg(%d),result(%d),reason(%s)\n",op.msgID,op.result,op.errReason);
					//break;
					return ATF_FAIL;
				}
			}
		}
	}
	
	pCase->status = WORKITEM_DONE_SUCC;
	return ATF_SUCC;
}

int CTask::GotStaticInfo()
{
	CommuWithAgent(MSGID_S2C_REQ_TRADE_PROCESS_STATIC_TIME);
	
	CommuWithAgent(MSGID_S2C_REQ_QUOTE_INTERVAL_STATIC_TIME);

	return ATF_SUCC;
}

int CTask::report()
{
	TestCase *pCase = m_casesList;

	m_uiSuccCases = m_uiTotalCases - m_uiFailCases;
	m_ulTimeTaskDone = time_diff_ms(m_stTimeStartTask,m_stTimeEndTask);

	int sec = m_ulTimeTaskDone/1000;
	int msec = m_ulTimeTaskDone%1000;
	if(0 == sec)
	{
		//g_log.info("[==========]%u tese cases ran.(%lu ms total)\n", m_uiTotalWorkItem, m_ulTimeTaskDone);
		printf_green("[==========]%u tese cases ran.(%lu ms total)\n", m_uiTotalCases, m_ulTimeTaskDone);
	}
	else
	{
		//g_log.info("[==========]%u tese cases ran.(%d.%d s total)\n", m_uiTotalWorkItem, sec, msec);
		printf_green("[==========]%u tese cases ran.(%d.%d s total)\n", m_uiTotalCases, sec, msec);
	}
	//g_log.info("[==PASSED==]%u tese cases!\n",m_succWorkItem);
	printf_green("[==PASSED==]%u tese cases!\n",m_uiSuccCases);
	
	if(m_uiFailCases > 0)
	{	
		//g_log.info("[==FAILED==]%u tese cases,listed below:\n",m_failWorkItem);
		printf_red("[==FAILED==]%u tese cases,listed below:\n",m_uiFailCases);
		
		pCase = m_casesList;
		while(pCase !=  NULL)	
		{
			if(WORKITEM_DONE_FAIL == pCase->status)
			{
				//g_log.info("[==FAILED==]%s\n",pItem->fileName);
				printf_red("[==FAILED==]%s(Reason: %s)\n",pCase->fileName,pCase->errReason);
			}

			pCase = pCase->next;
		}
	}

	printf_yellow("[==STATIC==]transactionName\t\ttotal              \tlost               \tsuccess            \tavgTime(usec)\tminTime(usec)\tmaxTime(usec)\tmaxIdx\n");	
	
	printf_yellow("[==STATIC==]QuoteInterval  \t\t%-19lld\t%-19lld\t%-19lld\t%-10d\t%-10d\t%-10d\t%-19lld\n",
		m_stQuoteIntervalStatic.quoteNum,
		m_stQuoteIntervalStatic.quoteNum - m_stQuoteIntervalStatic.quoteNum, 
		m_stQuoteIntervalStatic.quoteNum, 
		m_stQuoteIntervalStatic.avgQuoteInterval,
		m_stQuoteIntervalStatic.minQuoteInterval,
		m_stQuoteIntervalStatic.maxQuoteInterval,
		m_stQuoteIntervalStatic.maxIdx);	

	printf_yellow("[==STATIC==]tradeProcTime  \t\t%-19lld\t%-19lld\t%-19lld\t%-10d\t%-10d\t%-10d\t%-19lld\n",
		m_stQuoteIntervalStatic.quoteNum,
		m_stQuoteIntervalStatic.quoteNum - m_stTradeProcTimeStatic.tradeNum, 
		m_stTradeProcTimeStatic.tradeNum,		
		m_stTradeProcTimeStatic.avgTimeTradeProcess,
		m_stTradeProcTimeStatic.mixTimeTradeProcess,
		m_stTradeProcTimeStatic.maxTimeTradeProcess,
		m_stTradeProcTimeStatic.maxIdx);


	PrintResultToXML();


	return ATF_SUCC;
}

int CTask::PrintResultToXML()
{
	char filepath[MAX_FILE_PATH_LEN] = {0}; 
	unsigned int failCaseNum = 0;

	TestCase *pCase = m_casesList;
	sprintf(filepath,"%s/report/atf_report.xml",m_progPath);
	
	TiXmlDocument doc(filepath);  
	bool loadOkay = doc.LoadFile();  
	if (!loadOkay) 
	{	  
		g_log.error( "Could not load file %s. Error='%s'. \n", filepath,doc.ErrorDesc() );	
		return ATF_FAIL;	
	}  
	  
	// get dom root of '*.xml', here root should be 'testsuites'.  
	TiXmlElement* root = doc.RootElement();  
	  
	// trace every testsuite below root.  
	TiXmlElement *pTestsuite = root->FirstChildElement("testsuite");
	TiXmlElement *pTestcase = pTestsuite->FirstChildElement("testcase");
	for( ; pTestcase && NULL != pCase; pTestcase = pTestcase->NextSiblingElement() ) 
	{  
		if(WORKITEM_DONE_SUCC != pCase->status)
		{
			TiXmlElement * pFailure = new TiXmlElement("failure");  
			pFailure->SetAttribute("message",pCase->errReason);  
        	pTestcase->LinkEndChild(pFailure);  

			failCaseNum++;
        }  
		else
		{
			TiXmlElement * pFailure = pTestcase->FirstChildElement("failure");
			if(0 == pFailure)
			{
				g_log.info("Testcase %s is success.",pTestcase->Value());
			}
			else
			{
				failCaseNum++;
			}
		}
		pCase = pCase->next;
	  
	}  

	if(m_uiFailCases != failCaseNum)
	{
		root->SetAttribute("failures",failCaseNum); 
		pTestsuite->SetAttribute("failures",failCaseNum);
	}
	
	doc.SaveFile();  
	g_log.info("Succeeded to print test report to atf_report.xml.\n");

	return ATF_SUCC;
}

int CTask::CommuWithAgent(MSG_ID id)
{
	char sendBuf[2048] = {0};
	char rcvBuf[2048] = {0};
	char *pSendBuf = sendBuf;
	char *pRcvBuf = rcvBuf;
	char agentIp[32] = {0};
	int agentPort = ATFA_PORT;
	int ret = 0;
	MsgHead stHead;
	MsgHead *pMsgHead = NULL;
	int iRcvLen = 0;
	
	/*if(ATF_FAIL == m_pQuoteSvr->GotAgentIp(agentIp))
	{
		g_log.error("Unknown ATF agent IP !)\n");
		return ATF_FAIL;
	}*/

	strncpy(agentIp,m_agentConf.agentIP,sizeof(agentIp));
	agentPort = m_agentConf.agentPort;

	m_agentSocket = TcpConnect(agentIp,agentPort);
	if(m_agentSocket < 0)
	{
		g_log.error("Failed to connect ATF agent(ip:%s, port:%d)\n",agentIp,agentPort);
		return ATF_FAIL;
	}

	switch(id)
	{
		case MSGID_S2C_REQ_TRADE_PROCESS_STATIC_TIME:
			{
				stHead.iMsgID = htons(MSGID_S2C_REQ_TRADE_PROCESS_STATIC_TIME);
				stHead.iMsgBodyLen = htons(0);
				
				pSendBuf = sendBuf;
				memcpy(pSendBuf, &stHead, sizeof(stHead));
				
				int iSendLen = sizeof(stHead);
				ret = TcpSendData(m_agentSocket,(char*)pSendBuf,iSendLen);
				if(iSendLen != ret)
				{
					g_log.error("Send MSGID_S2C_REQ_TRADE_PROCESS_STATIC_TIME error,expect %dB,actual %dB\n",iSendLen,ret);
					return ATF_FAIL;
				}
				g_log.info("Send MSGID_S2C_REQ_TRADE_PROCESS_STATIC_TIME %dB\n",ret);

				//recv msg head
				iRcvLen = sizeof(MsgHead);
				ret = TcpRecvData(m_agentSocket,pRcvBuf,iRcvLen);
				if(ret != iRcvLen)
				{					
					g_log.error("[%s]Recv msg head error,expect %dB, actual %dB!\n",__FUNCTION__,iRcvLen,ret);
					return ATF_FAIL;
				}
				g_log.info("[%s]Recv msg head %dB\n",__FUNCTION__,ret);

				pMsgHead = (MsgHead *)pRcvBuf;
				if(MSGID_C2S_RSP_TRADE_PROCESS_STATIC_TIME == ntohs(pMsgHead->iMsgID))
				{
					//rcv msg body
					iRcvLen = sizeof(m_stTradeProcTimeStatic);
					ret = TcpRecvData(m_agentSocket,pRcvBuf,iRcvLen);
					if(ret != iRcvLen)
					{					
						g_log.error("[%s]Recv msg body error,expect %dB, actual %dB!\n",__FUNCTION__,iRcvLen,ret);
						return ATF_FAIL;
					}
					g_log.info("[%s]Recv msg %dB\n",__FUNCTION__,ret);

					InfoTradeProcessTime *pInfo = (InfoTradeProcessTime *)pRcvBuf;
					m_stTradeProcTimeStatic.avgTimeTradeProcess = pInfo->avgTimeTradeProcess;
					m_stTradeProcTimeStatic.maxTimeTradeProcess = pInfo->maxTimeTradeProcess;
					m_stTradeProcTimeStatic.mixTimeTradeProcess = pInfo->mixTimeTradeProcess;
					m_stTradeProcTimeStatic.tradeNum =  pInfo->tradeNum;

					return ATF_SUCC;
				}
				else
				{
					g_log.error("[%s]Expect msg %d, actual %d!\n",__FUNCTION__,MSGID_C2S_RSP_TRADE_PROCESS_STATIC_TIME,pMsgHead->iMsgID);
					
					return ATF_FAIL;
				}
			}
			break;
		case MSGID_S2C_REQ_QUOTE_INTERVAL_STATIC_TIME:
			{
				stHead.iMsgID = htons(MSGID_S2C_REQ_QUOTE_INTERVAL_STATIC_TIME);
				stHead.iMsgBodyLen = htons(0);
						
				pSendBuf = sendBuf;
				memcpy(pSendBuf, &stHead, sizeof(stHead));
						
				int iSendLen = sizeof(stHead);
				ret = TcpSendData(m_agentSocket,(char*)pSendBuf,iSendLen);
				if(iSendLen != ret)
				{
					g_log.error("Send MSGID_S2C_REQ_QUOTE_INTERVAL_STATIC_TIME error,expect %dB,actual %dB\n",iSendLen,ret);
					return ATF_FAIL;
				}
				g_log.info("Send MSGID_S2C_REQ_QUOTE_INTERVAL_STATIC_TIME %dB\n",ret);
		
				//recv msg head
				iRcvLen = sizeof(MsgHead);
				ret = TcpRecvData(m_agentSocket,pRcvBuf,iRcvLen);
				if(ret != iRcvLen)
				{					
					g_log.error("[%s]Recv msg head error,expect %dB, actual %dB!\n",__FUNCTION__,iRcvLen,ret);
					return ATF_FAIL;
				}
				g_log.info("[%s]Recv msg head %dB\n",__FUNCTION__,ret);
		
				pMsgHead = (MsgHead *)pRcvBuf;
				if(MSGID_C2S_RSP_QUOTE_INTERVAL_STATIC_TIME == ntohs(pMsgHead->iMsgID))
				{
					//rcv msg body
					iRcvLen = sizeof(StaticQuoteInterval_t);
					ret = TcpRecvData(m_agentSocket,pRcvBuf,iRcvLen);
					if(ret != iRcvLen)
					{					
						g_log.error("[%s]Recv msg body error,expect %dB, actual %dB!\n",__FUNCTION__,iRcvLen,ret);
						return ATF_FAIL;
					}
					g_log.info("[%s]Recv msg %dB\n",__FUNCTION__,ret);
		
					StaticQuoteInterval_t *pInfo = (StaticQuoteInterval_t *)pRcvBuf;
					m_stQuoteIntervalStatic.avgQuoteInterval = pInfo->avgQuoteInterval;
					m_stQuoteIntervalStatic.maxQuoteInterval = pInfo->maxQuoteInterval;
					m_stQuoteIntervalStatic.minQuoteInterval = pInfo->minQuoteInterval;
					m_stQuoteIntervalStatic.quoteNum = pInfo->quoteNum;
		
					return ATF_SUCC;
				}
				else
				{
					g_log.error("[%s]Expect msg %d, actual %d!\n",__FUNCTION__,MSGID_C2S_RSP_QUOTE_INTERVAL_STATIC_TIME,pMsgHead->iMsgID);
							
					return ATF_FAIL;
				}
			}
			break;
		case MSGID_S2C_REDDY_FOR_PERFORM_TEST:
			{
				stHead.iMsgID = htons(MSGID_S2C_REDDY_FOR_PERFORM_TEST);
				stHead.iMsgBodyLen = htons(0);
						
				pSendBuf = sendBuf;
				memcpy(pSendBuf, &stHead, sizeof(stHead));
						
				int iSendLen = sizeof(stHead);
				ret = TcpSendData(m_agentSocket,(char*)pSendBuf,iSendLen);
				if(iSendLen != ret)
				{
					g_log.error("Send MSGID_S2C_REDDY_FOR_PERFORM_TEST error,expect %dB,actual %dB\n",iSendLen,ret);
					return ATF_FAIL;
				}
				g_log.info("Send MSGID_S2C_REDDY_FOR_PERFORM_TEST %dB\n",ret);

				return ATF_SUCC;
			}
			break;
		default:
			{
				
			}
			break;
	}

	close(m_agentSocket);

	return ATF_FAIL;
}


