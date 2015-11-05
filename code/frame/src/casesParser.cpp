#include "casesParser.hpp"

CCasesParser::CCasesParser()
{
	m_uiTotalCases = 0;

}

CCasesParser::~CCasesParser()
{

}

int CCasesParser::GetCasesList(char *pTestcasesPath,TestCase * &pCasesList,unsigned int *num)
{
	unsigned int lineNo = 0; 
	unsigned int len = 0;
	char* ptr = NULL;	
	char *pRow = NULL;
	char srcLine[MAX_CMD_LINE_LEN] = {0};
	char refineLine[MAX_CMD_LINE_LEN] = {0};	
	char m_acTestcasesListFile[MAX_FILE_PATH_LEN] = {0};
	FILE *fp;	

	if(NULL == pTestcasesPath)
	{
		g_log.error("Input parameter pTestcasesPath is NULL !\n");
		return ATF_FAIL;
	}

	/*if(NULL == pCasesList)
	{
		g_log.error("Input parameter pCasesList is NULL !\n");
		return ATF_FAIL;
	}*/

	//open the test case list file
	sprintf(m_acTestcasesListFile,"%s/run_list",pTestcasesPath);
	fp = fopen(m_acTestcasesListFile, "r");	
	if (NULL == fp)
	{
		g_log.error("Fail to open test case list file: %s\n", m_acTestcasesListFile);
		return ATF_FAIL;
	}

	//loop to read each line of test case file
	while (1)
	{	
		lineNo++;
		ptr = fgets(srcLine, sizeof(srcLine)-1, fp);
		if (ptr == NULL)
		{
			break;
		}
				
		//drop the <cr> <lf> char		
		strcpy(refineLine, srcLine);
		len = strlen(refineLine);
		while ((len > 0) && (refineLine[len-1]== '\n' || refineLine[len-1]== '\r'))
		{
			refineLine[len-1] = '\0';
			len--;
		}
				
		//drop white char
		pRow = trim(refineLine);
		len = strlen(pRow);
		
		//if empty line, skip it	
		if (len <= 0)
		{
			continue;
		}
		
		//if rem line, skip it
		if (*pRow == '#')
		{
			continue;
		}

		TestCase *pItem = new TestCase;
		pItem->next = NULL;
		
		sprintf(pItem->absolutePath, "%s/%s",pTestcasesPath,pRow);
		
		if(ATF_FAIL == LoadTestCases(pItem->absolutePath,pItem))
		{
			g_log.error("Failed to load fileName: %s\n",pItem->fileName);
			delete pItem;
			pItem = NULL;
		}
		else
		{
			sprintf(pItem->fileName, "%s",pRow); 
			pItem->status = WORKITEM_NOT_DONE;
		
			if(NULL == pCasesList)
				pCasesList = pItem;
			else
			{ 
				TestCase *pTmp = pCasesList;
				while(NULL != pTmp->next)
					pTmp = pTmp->next;
		
				pTmp->next = pItem;
			}
			
			m_uiTotalCases++;
				
			g_log.debug("fileName: %s\n",pItem->fileName);
			g_log.debug("absolutePath: %s\n",pItem->absolutePath);
		}
	}	


 	*num = m_uiTotalCases;
	fclose(fp);
	return ATF_SUCC;

}

int CCasesParser::LoadTestCases(char *pPath,TestCase *pCase)
{
	char taskItemPath[MAX_FILE_PATH_LEN] = {0};	
	FILE *fp;	
	char srcLine[MAX_CMD_LINE_LEN] = {0};
	char refineLine[MAX_CMD_LINE_LEN] = {0};	
	char transLine[MAX_CMD_LINE_LEN] = {0}; 	
	char object[MAX_VAR_NAME_LEN] = {0};
	char subObject[MAX_VAR_NAME_LEN] = {0};
	unsigned int newLineNo = 0; 
	unsigned int len = 0;
	unsigned int ptr2_len = 0;
	char* ptr = NULL;	
	char *pObject = NULL;	
	char *pSubObject = NULL;
	char *pOperate = NULL;
	char *pValue = NULL; 
	char *pComment = NULL;
	//int timesOfRepeatOP = 0;
	int repeatNum = 0;
	bool isRepeatStart = false;
	bool isRepeatEnd = false;
	OPERATE_SET repeatOpSet;
	int lastOrder = -1;
	int lastID = -1;
	
	strncpy(taskItemPath, pPath,sizeof(taskItemPath));

	//open the test case file
	fp = fopen(taskItemPath, "r");	
	if (NULL == fp)
	{
		g_log.error("Fail to open test case file %s\n", taskItemPath);
		return ATF_FAIL;
	}
	g_log.info("start to load case %u: %s\n", m_uiTotalCases,taskItemPath);

	//tradeMode default value is LIMITED
	pCase->exchgOrder.tradeMode = LIMITED;
	
	//loop to read each line of test case file
	while (1)
	{	
		memset(srcLine,0,sizeof(srcLine));
		memset(refineLine,0,sizeof(refineLine));
		
		newLineNo++;
		ptr = fgets(srcLine, sizeof(srcLine)-1, fp);
		if (ptr == NULL)
		{
			break;
		}
		
		strcpy(refineLine, srcLine);
				
		//drop the <cr> <lf> char
		len = strlen(refineLine);
		while ((len > 0) && (refineLine[len-1]== '\n' || refineLine[len-1]== '\r'))
		{
			refineLine[len-1] = '\0';
			len--;
		}
				
		//drop white char
		pObject = trim(refineLine);
		len = strlen(pObject);
		
		//if empty line, skip it	
		if (len <= 0)
		{
			continue;
		}
		
		//if rem line, skip it
		if (*pObject == '#')
		{
			continue;
		}
		//printf_yellow("1: %s\n",pObject);

		
		pSubObject = strchr(pObject, '.');
		//handle over "REPEAT" control flow
		if (NULL == pSubObject)
		{
			pSubObject = strchr(pObject, '(');
			if (NULL == pSubObject)
			{
				pSubObject = strchr(pObject, '}');
				if (NULL == pSubObject)
				{
					g_log.error("[%s:%d]Invalid line: %s\n", __FILE__,__LINE__,refineLine);
					fclose(fp);
					return -1;
				}
				else
				{
					isRepeatEnd = true;
				}

				
				//handle over circular logic
				if((true == isRepeatStart)&&(true == isRepeatEnd))
				{
					for(int x=0;x<repeatNum-1;x++)
					{
						OPERATE_SET::iterator iterCur = repeatOpSet.begin();						
						OPERATE_SET::iterator iterEnd = repeatOpSet.end();
						for (; iterCur != iterEnd; ++iterCur)
						{
							if(1 == lastOrder)
								pCase->quoteOrder.quoteOp[lastID].push_back(*iterCur);
							else if(2 == lastOrder)
								pCase->exchgOrder.exchgOp[lastID].push_back(*iterCur);
							else
							{
								g_log.error("Undefined lastID(%D)\n",lastOrder);
							}
							g_log.debug("%d!\n",iterCur->msgID);
						}					
					}

					isRepeatStart = false;		
					isRepeatEnd = false;
					repeatOpSet.clear();
				}

				continue;
			}
			else
			{
				*pSubObject = '\0';
				pSubObject++;
				pObject = rtrim(pObject);
				
				if (strcasecmp(pObject, "REPEAT") == 0)
				{
					pValue = strchr(pSubObject, ')');
					if (pValue == NULL)
					{
						g_log.error("[%s:%d]Invalid line: %s\n", __FILE__,__LINE__,refineLine);
					}
					
					*pValue = '\0';					
					pSubObject = trimQuota(pSubObject);	
					isRepeatStart = true;
					repeatNum = atoi(pSubObject);
					g_log.info("Repeat %d time.\n", repeatNum);
				}
				else
					g_log.error("[%s:%d]Invalid line: %s\n", __FILE__,__LINE__,refineLine);

				continue;
			}
			
		}        
		else  //find the object
		{
			memset(object, 0, sizeof(object));
			strncpy(object, pObject, pSubObject-pObject);
	
			if (strlen(object) <= 0)
			{
				g_log.error("[%s:%d]Invalid line: %s\n", __FILE__,__LINE__,refineLine);
				fclose(fp);
				return -1;
			}	
			
			*pSubObject = '\0';
			pSubObject++;
		}
		//printf_yellow("2: %s(%d)\n",object,strlen(object));

		//find the subObject
		pOperate = strchr(pSubObject, '.');
		if (NULL == pOperate)
		{
			g_log.error("[%s:%d]Invalid line: %s\n", __FILE__,__LINE__,refineLine);
		}
		else
		{
			memset(subObject,0,sizeof(subObject));
			strncpy(subObject, pSubObject, pOperate-pSubObject);
	
			if (strlen(subObject) <= 0)
			{
				g_log.error("[%s:%d]Invalid line: %s\n", __FILE__,__LINE__,refineLine);
				fclose(fp);
				return -1;
			}	
			
			*pOperate = '\0';
			pOperate++;
		}

		//find the Operate
		pValue = strchr(pOperate, '(');
		if (pValue == NULL)
		{
			g_log.error("[%s:%d]Invalid line: %s\n", __FILE__,__LINE__,refineLine);
			g_log.error(">>object:%s\n", pObject);
			g_log.error(">>subObject:%s\n", pSubObject);
			g_log.error(">>operate:%s\n", pOperate);
					
			fclose(fp);
			return -1;
		}
		
		*pValue = '\0';
		pOperate = rtrim(pOperate);
		pValue = trim(pValue+1);
		//printf_yellow("3: %s(%d)\n",pOperate,strlen(pOperate));

		//drop comment and find the value
		pComment = strchr(pValue, ')');  //need add
		if (pComment != NULL)
		{
			*pComment = '\0';			
		}
		rtrim(pValue);

        //printf("pValue=%s\n",pValue);


		//drop quota char
		if (pValue[0] == '"')
		{
			memset(transLine,0,sizeof(transLine));
			transStr(pValue, transLine, strlen(pValue));
			pValue = transLine;
		}
			
		ptr2_len = strlen(pValue);
		if (pValue[0] == '"' && pValue[ptr2_len-1] == '"')
		{
			pValue[ptr2_len-1] = '\0';
			pValue = pValue + 1;			
		}	
		//printf("pValue=%s\n",pValue);

		//printf_yellow("4: %s(%d)\n",pValue,strlen(pValue));
		
		if (strcasecmp(object, "QuoteSvr") == 0)
		{
			lastOrder = 1;
			int id = 0;
			OP op;
			op.result = OPERATE_WAIT;
			
			if(NULL != strstr(subObject,"Provider"))
			{
				char *pID = &subObject[8];
				id = atoi(pID);
				*pID = '\0';

				lastID = id;
			}

			//g_log.debug("[%s:%d]subObject(%s),id(%d)\n", __FILE__,__LINE__,subObject,id);
			
    		if (strcasecmp(subObject, "Config") == 0)
    		{
				if (strcasecmp(pOperate, "SetProviderNum") == 0)
				{
	    			if (strlen(pValue) <= 0)
	    			{
	    				g_log.error("Invalid SetProviderNum !\n");
	    				fclose(fp);
	    				return ATF_FAIL;					
	    			}
					pCase->quoteOrder.uiProvidersTotal = atoi(pValue);
				}
				else
				{
					g_log.error("Undefined Operate:%s !\n",refineLine);
				}
			}
			else if(strcasecmp(subObject, "Provider") == 0)
			{
	    		if (strcasecmp(pOperate, "WaitConnectFromClient") == 0)
	    		{
					op.direction = RCV;
					op.msgID = MSGID_C2S_QUOTE_REQ_CONN;
					op.msgSize = sizeof(MsgHead);

					MsgHead *pHead = new MsgHead;
					memset(pHead,0,sizeof(*pHead));
					pHead->iMsgID = MSGID_C2S_QUOTE_REQ_CONN;
					pHead->iMsgBodyLen = 0;

					op.pMsg = (char *)pHead;

					pCase->quoteOrder.quoteOp[id].push_back(op);
	    		}
    			else if (strcasecmp(pOperate, "SendResultForConnect") == 0)
    			{
					op.direction = SEND;
					op.msgID = MSGID_S2C_QUOTE_RSP_CONN;
					op.msgSize = sizeof(MsgHead) + sizeof(RspInfoFromATF);				
					char *pMsg = new char[op.msgSize];
					op.pMsg = pMsg;
					memset(pMsg,0,op.msgSize);
					MsgHead *pHead = (MsgHead*)pMsg;
					pHead->iMsgID = htons(MSGID_S2C_QUOTE_RSP_CONN);
					pHead->iMsgBodyLen = htons(sizeof(RspInfoFromATF));
					RspInfoFromATF *pBody = (RspInfoFromATF*)(pMsg+sizeof(MsgHead));
				
        			if (strlen(pValue) <= 0)
        			{
        				g_log.debug("SendResultForConnect is not set,default SUCC !\n");
						
						pBody->ErrorID = 0;
						memcpy(pBody->ErrorMsg,"CTP：No Error",sizeof(pBody->ErrorMsg));
        			}
					else
					{
						if (strcasecmp(pValue, "SUCC") == 0)
						{
							pBody->ErrorID = 0;
							memcpy(pBody->ErrorMsg,"CTP：No Error",sizeof(pBody->ErrorMsg));
						}
						else if (strcasecmp(pValue, "FAIL") == 0)
						{
							pBody->ErrorID = htons(-1);
						}						
						else
						{
	    					g_log.error("Invalid SendResultForConnect !\n");
							return ATF_FAIL;
						}
					}

					pCase->quoteOrder.quoteOp[id].push_back(op);
    			}	
    			else if (strcasecmp(pOperate, "DisconnectClient") == 0)
    			{
					op.direction = SEND;
					op.msgID = MSGID_S2C_QUOTE_DISCONNECT;
					op.msgSize = sizeof(MsgHead);				
					char *pMsg = new char[op.msgSize];
					op.pMsg = pMsg;
					memset(pMsg,0,op.msgSize);
					MsgHead *pHead = (MsgHead*)pMsg;
					pHead->iMsgID = htons(MSGID_S2C_QUOTE_DISCONNECT);
					pHead->iMsgBodyLen = htons(0);

					pCase->quoteOrder.quoteOp[id].push_back(op);
    			}
    			else if (strcasecmp(pOperate, "WaitLoginFromClient") == 0)
    			{
					op.direction = RCV;
					op.msgID = MSGID_C2S_QUOTE_REQ_LOGIN;
					op.msgSize = sizeof(MsgHead) + sizeof(QuoteReqUserLoginToATF);			
					char *pReqLoginMsg = new char[op.msgSize];					
					memset(pReqLoginMsg,0,sizeof(op.msgSize));
					op.pMsg = pReqLoginMsg;
					MsgHead *pReqLoginHead = (MsgHead*)pReqLoginMsg;
					pReqLoginHead->iMsgID = MSGID_C2S_QUOTE_REQ_LOGIN;
					pReqLoginHead->iMsgBodyLen = sizeof(QuoteReqUserLoginToATF);
					
					QuoteReqUserLoginToATF *pReqLoginBody = (QuoteReqUserLoginToATF*)(pReqLoginMsg+sizeof(MsgHead));
					if (strlen(pValue) <= 0)
					{
						g_log.error("WaitLoginFromClient is null !\n");
						fclose(fp);
						return ATF_FAIL;					
					}

					char *pHead = NULL;
					char *pEnd = NULL;
					pHead = strstr(pValue,"account=");
					if(NULL != pHead)
					{
						pHead = pHead + strlen("account=");
						pEnd = strchr(pHead,'|');
						memset(pReqLoginBody->reqUserLogin.UserID,0,sizeof(pReqLoginBody->reqUserLogin.UserID));
						if(NULL == pEnd)
							strcpy(pReqLoginBody->reqUserLogin.UserID,pHead);
						else
							strncpy(pReqLoginBody->reqUserLogin.UserID,pHead,pEnd-pHead);
					}
					else
					{
						g_log.error("%s:no account !",pValue);
					}

					pHead = strstr(pValue,"password=");
					if(NULL != pHead)
					{
						pHead = pHead + strlen("password=");
						pEnd = strchr(pHead,'|');
						memset(pReqLoginBody->reqUserLogin.Password,0,sizeof(pReqLoginBody->reqUserLogin.Password));
						if(NULL == pEnd)
							strcpy(pReqLoginBody->reqUserLogin.Password,pHead);
						else
							strncpy(pReqLoginBody->reqUserLogin.Password,pHead,pEnd-pHead);
					}
					else
					{
						g_log.error("%s:no Password !",pValue);
					}

					pCase->quoteOrder.quoteOp[id].push_back(op);

    			}
    			else if (strcasecmp(pOperate, "SendResultForLogin") == 0)
    			{
					op.direction = SEND;
					op.msgID = MSGID_S2C_QUOTE_RSP_LOGIN;
					op.msgSize = sizeof(MsgHead) + sizeof(QuoteRspUserLoginFromATF);				
					char *pMsg = new char[op.msgSize];
					op.pMsg = pMsg;
					memset(pMsg,0,op.msgSize);
					MsgHead *pHead = (MsgHead*)pMsg;
					pHead->iMsgID = htons(MSGID_S2C_QUOTE_RSP_LOGIN);
					pHead->iMsgBodyLen = htons(sizeof(QuoteRspUserLoginFromATF));

					QuoteRspUserLoginFromATF *pBody = (QuoteRspUserLoginFromATF*)(pMsg+sizeof(MsgHead));
        			if (strlen(pValue) <= 0)
        			{
         				g_log.debug("SendResultForLogin is not set,default SUCC !\n");
					
						pBody->rspInfo.ErrorID = 0;
						memcpy(pBody->rspInfo.ErrorMsg,"CTP：No Error",sizeof(pBody->rspInfo.ErrorMsg));
        			}
					else if (strcasecmp(pValue, "FAIL") == 0)
					{
						pBody->rspInfo.ErrorID = -1;
						g_log.debug("SendResultForLogin is %d !\n",pBody->rspInfo.ErrorID);
					}
					else if (strcasecmp(pValue, "SUCC") == 0)
					{
						pBody->rspInfo.ErrorID = 0;
						memcpy(pBody->rspInfo.ErrorMsg,"CTP：No Error",sizeof(pBody->rspInfo.ErrorMsg));
					}
					else
					{
	    				g_log.error("Invalid SendResultForLogin !\n");
						return ATF_FAIL;
					}
					pCase->quoteOrder.quoteOp[id].push_back(op);
    			}
    			else if (strcasecmp(pOperate, "WaitSubscribeQuoteFromClient") == 0)
    			{
					op.direction = RCV;
					op.msgID = MSGID_C2S_QUOTE_REQ_SUBSCRIBE;
					op.msgSize = sizeof(MsgHead) + sizeof(ReqSubMarketToATF);	

					char *pMsg = new char[op.msgSize];
					op.pMsg = pMsg;
					memset(pMsg,0,op.msgSize);
					
					MsgHead *pHead = (MsgHead*)pMsg;			
					pHead->iMsgID = MSGID_C2S_QUOTE_REQ_SUBSCRIBE;
					pHead->iMsgBodyLen = sizeof(ReqSubMarketToATF);

					ReqSubMarketToATF *pBody = (ReqSubMarketToATF*)(pMsg+sizeof(MsgHead));
        			if (strlen(pValue) <= 0)
        			{
         				g_log.error("WaitSubscribeQuoteFromClient is NULL, must set a instrument !\n");
						return ATF_FAIL;
        			}
					strcpy(pBody->instr,pValue);
					
					pCase->quoteOrder.quoteOp[id].push_back(op);
    			}
    			else if (strcasecmp(pOperate, "SendRspForSubscribe") == 0)
    			{
					op.direction = SEND;
					op.msgID = MSGID_S2C_QUOTE_RSP_SUBSCRIBE;
					op.msgSize = sizeof(MsgHead) + sizeof(RspSubMarketFromATF);	
					
					char *pMsg = new char[op.msgSize];
					memset(pMsg,0,op.msgSize);
					op.pMsg = pMsg;
					
					MsgHead *pHead = (MsgHead*)pMsg;
					pHead->iMsgID = htons(MSGID_S2C_QUOTE_RSP_SUBSCRIBE);
					pHead->iMsgBodyLen = htons(sizeof(RspSubMarketFromATF));

					RspSubMarketFromATF *pBody = (RspSubMarketFromATF*)(pMsg+sizeof(MsgHead));				
    				if (strlen(pValue) <= 0)
    				{
	    				g_log.error("SendRspForSubscribe is NULL!\n");
						return ATF_FAIL;
    				}
					else if(strcasecmp(pValue, "FAIL") == 0)
					{
						pBody->rspInfo.ErrorID = -1;
					}
					else
					{
						pBody->rspInfo.ErrorID = 0;
						memcpy(pBody->rspInfo.ErrorMsg,"CTP：No Error",sizeof(pBody->rspInfo.ErrorMsg));

						strcpy(pBody->QuoteSendPara,pValue);
					}
				
					pCase->quoteOrder.quoteOp[id].push_back(op);
    			}
    			else if (strcasecmp(pOperate, "WaitUnsubscribeQuoteFromClient") == 0)
    			{
					op.direction = RCV;
					op.msgID = MSGID_C2S_QUOTE_REQ_UNSUBSCRIBE;
					op.msgSize = sizeof(MsgHead) + sizeof(ReqUnsubMarketToATF);	

					char *pMsg = new char[op.msgSize];
					op.pMsg = pMsg;
					memset(pMsg,0,op.msgSize);
					
					MsgHead *pHead = (MsgHead*)pMsg;			
					pHead->iMsgID = MSGID_C2S_QUOTE_REQ_UNSUBSCRIBE;
					pHead->iMsgBodyLen = sizeof(ReqUnsubMarketToATF);

					ReqUnsubMarketToATF *pBody = (ReqUnsubMarketToATF*)(pMsg+sizeof(MsgHead));
        			if (strlen(pValue) <= 0)
        			{
         				g_log.error("WaitUnsubscribeQuoteFromClient is NULL, must set a instrument !\n");
						return ATF_FAIL;
        			}
					strcpy(pBody->instr,pValue);
					
					pCase->quoteOrder.quoteOp[id].push_back(op);
    			}
    			else if (strcasecmp(pOperate, "SendRspForUnsubscribe") == 0)
    			{
					op.direction = SEND;
					op.msgID = MSGID_S2C_QUOTE_RSP_UNSUBSCRIBE;
					op.msgSize = sizeof(MsgHead) + sizeof(RspUnsubMarketFromATF);	
					
					char *pMsg = new char[op.msgSize];
					memset(pMsg,0,op.msgSize);
					op.pMsg = pMsg;
					
					MsgHead *pHead = (MsgHead*)pMsg;
					pHead->iMsgID = htons(MSGID_S2C_QUOTE_RSP_UNSUBSCRIBE);
					pHead->iMsgBodyLen = htons(sizeof(RspUnsubMarketFromATF));

					RspUnsubMarketFromATF *pBody = (RspUnsubMarketFromATF*)(pMsg+sizeof(MsgHead));				
    				if (strlen(pValue) <= 0)
    				{
	    				g_log.error("SendRspForUnsubscribe is NULL!\n");
						return ATF_FAIL;
    				}
					else if(strcasecmp(pValue, "FAIL") == 0)
					{
						pBody->rspInfo.ErrorID = -1;
					}
					else if(strcasecmp(pValue, "SUCC") == 0)
					{
						pBody->rspInfo.ErrorID = 0;
						memcpy(pBody->rspInfo.ErrorMsg,"CTP：No Error",sizeof(pBody->rspInfo.ErrorMsg));
					}
				
					pCase->quoteOrder.quoteOp[id].push_back(op);
    			}
				else if (strcasecmp(pOperate, "WaitResultOfSendQuote") == 0)
				{
					op.direction = RCV;
					op.msgID = MSGID_C2S_QUOTE_RESULT_SEND_QUOTE;
					op.msgSize = sizeof(MsgHead);				
					MsgHead *pMsg = new MsgHead;
					memset(pMsg,0,sizeof(*pMsg));
				
					pMsg->iMsgID = MSGID_C2S_QUOTE_RESULT_SEND_QUOTE;
					pMsg->iMsgBodyLen = 0;
					op.pMsg = (char *)pMsg;

					pCase->quoteOrder.quoteOp[id].push_back(op);
				}				
    			else if (strcasecmp(pOperate, "Sleep") == 0)
    			{
    				if (strlen(pValue) <= 0)
    				{
    					g_log.error("Invalid Sleep !\n");
    					fclose(fp);
    					return ATF_FAIL;					
    				}
				
					op.direction = LOCAL;
					op.msgID = MSGID_LOCAL_SLEEP;
					op.msgSize = sizeof(MsgHead) + sizeof(SleepTime);				
					char *pMsg = new char[op.msgSize];
					op.pMsg = pMsg;
					memset(pMsg,0,op.msgSize);
					MsgHead *pHead = (MsgHead*)pMsg;
					pHead->iMsgID = htons(MSGID_LOCAL_SLEEP);
					pHead->iMsgBodyLen = htons(sizeof(SleepTime));

					SleepTime *pBody = (SleepTime*)(pMsg+sizeof(MsgHead));
					pBody->time = atoi(pValue);
					//g_log.debug("Sleep:%d !\n",pBody->time);
					
					pCase->quoteOrder.quoteOp[id].push_back(op);
					
	    		}
				else
				{
					g_log.error("QuoteSvr:Undefined pOperate: %s!\n",pOperate);

					return ATF_FAIL;
				}

				g_log.debug("[%s:%d]object(%s),subObject(%s),id(%d),operate(%s),value(%s)\n", __FILE__,__LINE__,object,subObject,id,pOperate,pValue);
			}
		}
		else if (strcasecmp(object, "Exchange") == 0)
		{
			lastOrder = 2;
			int id = 0;
			OP op;
			op.result = OPERATE_WAIT;
			
			if((NULL != strstr(subObject,"Client"))||(NULL != strstr(subObject,"client")))
			{
				char *pID = &subObject[6];
				id = atoi(pID);
				*pID = '\0';
				
				lastID = id;
			}			

    		if (strcasecmp(subObject, "Config") == 0)
    		{
				if (strcasecmp(pOperate, "SetClientNum") == 0)
				{
	    			if (strlen(pValue) <= 0)
	    			{
	    				g_log.error("Invalid SetClientNum !\n");
	    				fclose(fp);
	    				return ATF_FAIL;					
	    			}
					pCase->exchgOrder.uiClientsTotal= atoi(pValue);
				}
				else
				{
					g_log.error("Undefined Operate:%s !\n",refineLine);
				}
			}
			else if(strcasecmp(subObject, "Client") == 0)
			{
				if (strcasecmp(pOperate, "WaitConnectFromClient") == 0)
				{
					op.direction = RCV;
					op.msgID = MSGID_C2S_TRADE_REQ_CONN;
					op.msgSize = sizeof(MsgHead);
					
					MsgHead *pMsg = new MsgHead;
					memset(pMsg,0,sizeof(*pMsg));
					pMsg->iMsgID = MSGID_C2S_TRADE_REQ_CONN;
					pMsg->iMsgBodyLen = 0;
					op.pMsg = (char *)pMsg;
					pCase->exchgOrder.exchgOp[id].push_back(op);

					if(true == isRepeatStart && false == isRepeatEnd)
						repeatOpSet.push_back(op);
								
				}
				else if (strcasecmp(pOperate, "SendResultForConnect") == 0)
				{
					op.direction = SEND;
					op.msgID = MSGID_S2C_TRADE_RSP_CONN;
					op.msgSize = sizeof(MsgHead) + sizeof(RspInfoFromATF);				
					char *pMsg = new char[op.msgSize];
					op.pMsg = pMsg;
					memset(pMsg,0,op.msgSize);
					MsgHead *pHead = (MsgHead*)pMsg;
					pHead->iMsgID = htons(MSGID_S2C_TRADE_RSP_CONN);
					pHead->iMsgBodyLen = htons(sizeof(RspInfoFromATF));
				
					RspInfoFromATF *pBody = (RspInfoFromATF*)(pMsg+sizeof(MsgHead));
								
					if (strlen(pValue) <= 0)
					{
						g_log.debug("SendResultForConnect is not set,default SUCC !\n");
									
						pBody->ErrorID = 0;
						memcpy(pBody->ErrorMsg,"CTP：No Error",sizeof(pBody->ErrorMsg));
					}
					else
					{
						if (strcasecmp(pValue, "SUCC") == 0)
						{
							pBody->ErrorID = 0;
							memcpy(pBody->ErrorMsg,"CTP：No Error",sizeof(pBody->ErrorMsg));
						}
						else if (strcasecmp(pValue, "FAIL") == 0)
						{
							pBody->ErrorID = -1;
						}
						else
						{
							g_log.error("Invalid SendResultForConnect !\n");
							return ATF_FAIL;
						}
					}
								
					pCase->exchgOrder.exchgOp[id].push_back(op);

					if(true == isRepeatStart && false == isRepeatEnd)
						repeatOpSet.push_back(op);
				}	
				else if (strcasecmp(pOperate, "DisconnectClient") == 0)
				{
					op.direction = SEND;
					op.msgID = MSGID_S2C_TRADE_DISCONNECT;
					op.msgSize = sizeof(MsgHead);				
					char *pMsg = new char[op.msgSize];
					op.pMsg = pMsg;
					memset(pMsg,0,op.msgSize);
					MsgHead *pHead = (MsgHead*)pMsg;
					pHead->iMsgID = htons(MSGID_S2C_TRADE_DISCONNECT);
					pHead->iMsgBodyLen = htons(0);
					
					pCase->exchgOrder.exchgOp[id].push_back(op);

					if(true == isRepeatStart && false == isRepeatEnd)
						repeatOpSet.push_back(op);
				}
				else if (strcasecmp(pOperate, "WaitLoginFromClient") == 0)
				{
					op.direction = RCV;
					op.msgID = MSGID_C2S_TRADE_REQ_LOGIN;
					op.msgSize = sizeof(MsgHead) + sizeof(TradeReqUserLoginToATF);			
					char *pReqLoginMsg = new char[op.msgSize];
					memset(pReqLoginMsg,0,sizeof(op.msgSize));
					op.pMsg = pReqLoginMsg;
					MsgHead *pReqLoginHead = (MsgHead*)pReqLoginMsg;
					pReqLoginHead->iMsgID = MSGID_C2S_TRADE_REQ_LOGIN;
					pReqLoginHead->iMsgBodyLen = sizeof(TradeReqUserLoginToATF);
										
					TradeReqUserLoginToATF *pReqLoginBody = (TradeReqUserLoginToATF*)(pReqLoginMsg+sizeof(MsgHead));
					if (strlen(pValue) <= 0)
					{
						g_log.error("WaitLoginFromClient is null !\n");
						fclose(fp);
						return ATF_FAIL;					
					}
					
					char *pHead = NULL;
					char *pEnd = NULL;
					pHead = strstr(pValue,"account=");
					if(NULL != pHead)
					{
						pHead = pHead + strlen("account=");
						pEnd = strchr(pHead,'|');						
						memset(pReqLoginBody->reqUserLogin.UserID,0,sizeof(pReqLoginBody->reqUserLogin.UserID));
						if(NULL == pEnd)
							strcpy(pReqLoginBody->reqUserLogin.UserID,pHead);
						else
							strncpy(pReqLoginBody->reqUserLogin.UserID,pHead,pEnd-pHead);
						
						//g_log.debug("ExchgSvr UserID:%s\n",pReqLoginBody->reqUserLogin.UserID);
					}
					
					pHead = strstr(pValue,"password=");
					if(NULL != pHead)
					{
						pHead = pHead + strlen("password=");
						pEnd = strchr(pHead,'|');
						if(NULL == pEnd)
							strcpy(pReqLoginBody->reqUserLogin.Password,pHead);
						else
							strncpy(pReqLoginBody->reqUserLogin.Password,pHead,pEnd-pHead);
						
						//g_log.debug("ExchgSvr Password:%s\n",pReqLoginBody->reqUserLogin.Password);
					}
				
					pCase->exchgOrder.exchgOp[id].push_back(op);
					
				}
				else if (strcasecmp(pOperate, "SendResultForLogin") == 0)
				{
					op.direction = SEND;
					op.msgID = MSGID_S2C_TRADE_RSP_LOGIN;
					op.msgSize = sizeof(MsgHead) + sizeof(TradeRspUserLoginFromATF);				
					char *pMsg = new char[op.msgSize];
					op.pMsg = pMsg;
					memset(pMsg,0,op.msgSize);
					MsgHead *pHead = (MsgHead*)pMsg;
					pHead->iMsgID = htons(MSGID_S2C_TRADE_RSP_LOGIN);
					pHead->iMsgBodyLen = htons(sizeof(TradeRspUserLoginFromATF));
			
					TradeRspUserLoginFromATF *pBody = (TradeRspUserLoginFromATF*)(pMsg+sizeof(MsgHead));
								
					if (strlen(pValue) <= 0)
					{
						g_log.debug("SendResultForLogin is not set,default SUCC !\n");
									
						pBody->rspInfo.ErrorID = 0;
						memcpy(pBody->rspInfo.ErrorMsg,"CTP：No Error",sizeof(pBody->rspInfo.ErrorMsg));
					}
					else if (strcasecmp(pValue, "FAIL") == 0)
					{
						pBody->rspInfo.ErrorID = -1;
						g_log.debug("SendResultForLogin is %d !\n",pBody->rspInfo.ErrorID);
					}
					else if (strcasecmp(pValue, "SUCC") == 0)
					{
						pBody->rspInfo.ErrorID = 0;
						memcpy(pBody->rspInfo.ErrorMsg,"CTP：No Error",sizeof(pBody->rspInfo.ErrorMsg));
					}
					else
					{
						g_log.error("Invalid SendResultForLogin !\n");
						return ATF_FAIL;
					}
					pCase->exchgOrder.exchgOp[id].push_back(op);

					if(true == isRepeatStart && false == isRepeatEnd)
						repeatOpSet.push_back(op);
				}
				else if (strcasecmp(pOperate, "SetTradeMode") == 0)
				{
					if (strlen(pValue) <= 0)
					{
						g_log.debug("SetTradeMode is not set,default LIMITED !\n");
									
						pCase->exchgOrder.tradeMode = LIMITED;
					}
					else if (strcasecmp(pValue, "unlimited") == 0)
					{
						pCase->exchgOrder.tradeMode = UNLIMITED;
						
						g_log.debug("SetTradeMode: UNLIMITED\n");
					}
					else if (strcasecmp(pValue, "limited") == 0)
					{
						pCase->exchgOrder.tradeMode = LIMITED;
						
						g_log.debug("SetTradeMode: LIMITED\n");
					}
					else
					{
						g_log.error("Invalid SetTradeMode !\n");
						return ATF_FAIL;
					}
				}
				else if (strcasecmp(pOperate, "WaitReqOrderInsert") == 0)
				{
					if(ATF_FAIL == ParseWaitReqOrderInsert(op,pValue))
					{
						fclose(fp);
						return ATF_FAIL;
					}

					pCase->exchgOrder.exchgOp[id].push_back(op);

					if(true == isRepeatStart && false == isRepeatEnd)
						repeatOpSet.push_back(op);
				}
				else if (strcasecmp(pOperate, "WaitReqOrderAction") == 0)
				{
					if(ATF_FAIL == ParseWaitReqOrderAction(op,pValue))
					{
						fclose(fp);
						return ATF_FAIL;
					}
					
					pCase->exchgOrder.exchgOp[id].push_back(op);
					
					if(true == isRepeatStart && false == isRepeatEnd)
						repeatOpSet.push_back(op);
				}
				else if (strcasecmp(pOperate, "SendOnRtnOrder") == 0)
				{
					if(ATF_FAIL == ParseSendOnRtnOrder(op,pValue))
					{
						fclose(fp);
						return ATF_FAIL;
					}
					
					pCase->exchgOrder.exchgOp[id].push_back(op);

					if(true == isRepeatStart && false == isRepeatEnd)
						repeatOpSet.push_back(op);
				}
				else if (strcasecmp(pOperate, "SendOnRtnTrade") == 0)
				{
					if(ATF_FAIL == ParseSendOnRtnTrade(op,pValue))
					{
						fclose(fp);
						return ATF_FAIL;
					}
					
					pCase->exchgOrder.exchgOp[id].push_back(op);

					if(true == isRepeatStart && false == isRepeatEnd)
						repeatOpSet.push_back(op);
				}
				else if (strcasecmp(pOperate, "SendOnRspOrderInsert") == 0)
				{
					if(ATF_FAIL == ParseSendOnRspOrderInsert(op,pValue))
					{
						fclose(fp);
						return ATF_FAIL;
					}
					
					pCase->exchgOrder.exchgOp[id].push_back(op);

					if(true == isRepeatStart && false == isRepeatEnd)
						repeatOpSet.push_back(op);
				}
				else if (strcasecmp(pOperate, "SendOnErrRtnOrderInsert") == 0)
				{
					if(ATF_FAIL == ParseSendOnErrRtnOrderInsert(op,pValue))
					{
						fclose(fp);
						return ATF_FAIL;
					}
					
					pCase->exchgOrder.exchgOp[id].push_back(op);

					if(true == isRepeatStart && false == isRepeatEnd)
						repeatOpSet.push_back(op);
				}
				else if (strcasecmp(pOperate, "SendOnRspOrderAction") == 0)
				{
					if(ATF_FAIL == ParseSendOnRspOrderAction(op,pValue))
					{
						fclose(fp);
						return ATF_FAIL;
					}
					
					pCase->exchgOrder.exchgOp[id].push_back(op);

					if(true == isRepeatStart && false == isRepeatEnd)
						repeatOpSet.push_back(op);
				}
				else if (strcasecmp(pOperate, "SendOnErrRtnOrderAction") == 0)
				{
					if(ATF_FAIL == ParseSendOnErrRtnOrderAction(op,pValue))
					{
						fclose(fp);
						return ATF_FAIL;
					}
					
					pCase->exchgOrder.exchgOp[id].push_back(op);

					if(true == isRepeatStart && false == isRepeatEnd)
						repeatOpSet.push_back(op);
				}
				else if (strcasecmp(pOperate, "Sleep") == 0)
				{
					if (strlen(pValue) <= 0)
					{
						g_log.error("Invalid Sleep !\n");
						fclose(fp);
						return ATF_FAIL;					
					}
					
					op.direction = LOCAL;
					op.msgID = MSGID_LOCAL_SLEEP;
					op.msgSize = sizeof(MsgHead) + sizeof(SleepTime);				
					char *pMsg = new char[op.msgSize];
					op.pMsg = pMsg;
					memset(pMsg,0,op.msgSize);
					MsgHead *pHead = (MsgHead*)pMsg;
					pHead->iMsgID = htons(MSGID_LOCAL_SLEEP);
					pHead->iMsgBodyLen = htons(sizeof(SleepTime));
				
					SleepTime *pBody = (SleepTime*)(pMsg+sizeof(MsgHead));
					pBody->time = atoi(pValue);
					g_log.debug("Sleep:%d !\n",pBody->time);
					
					pCase->exchgOrder.exchgOp[id].push_back(op);
					
					if(true == isRepeatStart && false == isRepeatEnd)
						repeatOpSet.push_back(op);
				}
				else
				{
					g_log.error("ExchgSvr:Undefined pOperate:%s!\n",pOperate);
				}

				
			}
			else
			{
				g_log.error("Undefined subObject !\n");

				return ATF_FAIL;
			}

			g_log.debug("[%s:%d]object(%s),subObject(%s),id(%d),operate(%s),value(%s)\n", __FILE__,__LINE__,object,subObject,id,pOperate,pValue);
		}
		else
		{
			g_log.error("Undefined object !\n");

			return ATF_FAIL;
		}
	}
	fclose(fp);
		
	return ATF_SUCC;
}

int CCasesParser::ParseWaitReqOrderInsert(OP &op,char *pValue)
{
	op.direction = RCV;
	op.msgID = MSGID_C2S_TRADE_ReqOrderInsert;
									
	op.msgSize = sizeof(MsgHead) + sizeof(ReqOrderInsertToATF);
	char *pMsg = new char[op.msgSize];
	memset(pMsg,0,sizeof(*pMsg));
	MsgHead *pHeadMsg = (MsgHead *)pMsg;
	pHeadMsg->iMsgID = MSGID_C2S_TRADE_ReqOrderInsert;
	pHeadMsg->iMsgBodyLen = sizeof(ReqOrderInsertToATF);
	op.pMsg = pMsg;
									
	ReqOrderInsertToATF *pBody = (ReqOrderInsertToATF*)(pMsg+sizeof(MsgHead));
	if (strlen(pValue) <= 0)
	{
		g_log.error("WaitOrder is null !\n");
		return ATF_FAIL;					
	}
	
	char *pHead = NULL;
	char *pEnd = NULL;
	//解析报单类型
	pHead = strstr(pValue,"instr=");
	if(NULL != pHead)
	{
		pHead = pHead + strlen("instr=");
		pEnd = strchr(pHead,'|');
	
		char instr[16] = {0};
		if(NULL == pEnd)
		{
			strcpy(instr,pHead);
		}
		else
		{
			strncpy(instr,pHead,pEnd-pHead);
		}
	
		if (strcasecmp(instr, "ROD") == 0)
		{
			pBody->order.OrderPriceType = THOST_FTDC_OPT_LimitPrice; 
			pBody->order.TimeCondition = THOST_FTDC_TC_GFD; 
			pBody->order.VolumeCondition = THOST_FTDC_VC_AV; 
			pBody->order.ContingentCondition = THOST_FTDC_CC_Immediately; 
		}
		else if (strcasecmp(instr, "FAK") == 0)
		{						
			pBody->order.OrderPriceType = THOST_FTDC_OPT_LimitPrice; 
			pBody->order.TimeCondition = THOST_FTDC_TC_IOC; 
			pBody->order.VolumeCondition = THOST_FTDC_VC_AV; //THOST_FTDC_VC_MV
			pBody->order.ContingentCondition = THOST_FTDC_CC_Immediately; 
		}						
		else if (strcasecmp(instr, "FOK") == 0)
		{
			pBody->order.OrderPriceType = THOST_FTDC_OPT_LimitPrice; 
			pBody->order.TimeCondition = THOST_FTDC_TC_IOC; 
			pBody->order.VolumeCondition = THOST_FTDC_VC_CV; 
			pBody->order.ContingentCondition = THOST_FTDC_CC_Immediately; 
		}
		else if (strcasecmp(instr, "MRK") == 0)
		{
			pBody->order.OrderPriceType = THOST_FTDC_OPT_AnyPrice; 
			pBody->order.TimeCondition = THOST_FTDC_TC_IOC; 
			pBody->order.VolumeCondition = THOST_FTDC_VC_AV; 
			pBody->order.ContingentCondition = THOST_FTDC_CC_Immediately; 
	
			pBody->order.LimitPrice = 0;
		}
		else
		{
			g_log.error("Undefined instr:%s\n",instr);
		}
	}
	
	//解析方向
	pHead = strstr(pValue,"direction=");
	if(NULL != pHead)
	{
		pHead = pHead + strlen("direction=");
		pEnd = strchr(pHead,'|');
							
		char direction[16] = {0};
							
		if(NULL == pEnd)
		{
			strcpy(direction,pHead);
		}
		else
		{
			strncpy(direction,pHead,pEnd-pHead);
		}
	
							
		if (strcasecmp(direction, "buy") == 0)
		{
			pBody->order.Direction= THOST_FTDC_D_Buy;
		}
		else if(strcasecmp(direction, "sell") == 0)
		{
			pBody->order.Direction= THOST_FTDC_D_Sell;
		}
		else
		{
			g_log.error("Undefined Direction:%s\n",direction);
		}
							
	}
	
	//解析数量
	pHead = strstr(pValue,"vol=");
	if(NULL != pHead)
	{
		pHead = pHead + strlen("vol=");
		pEnd = strchr(pHead,'|');
							
		char vol[16] = {0};
							
		if(NULL == pEnd)
		{
			strcpy(vol,pHead);
		}
		else
		{
			strncpy(vol,pHead,pEnd-pHead);
		}
	
		pBody->order.VolumeTotalOriginal = atoi(vol);
	
		//printf_red("#####################vol:%s,VolumeTotalOriginal:%d\n",vol,pBody->order.VolumeTotalOriginal);
	}
						
	//解析价格
	pHead = strstr(pValue,"price=");
	if(NULL != pHead)
	{
		pHead = pHead + strlen("price=");
		pEnd = strchr(pHead,'|');
							
		char price[16] = {0};
							
		if(NULL == pEnd)
		{
			strcpy(price,pHead);
		}
		else
		{
			strncpy(price,pHead,pEnd-pHead);
		}
	
		pBody->order.LimitPrice = atof(price);
							
		//printf_red("#####################price:%s,LimitPrice:%f\n",price,pBody->order.LimitPrice);
	}
	
	//解析开平标志
	pHead = strstr(pValue,"CombOffsetFlag=");
	if(NULL != pHead)
	{
		pHead = pHead + strlen("CombOffsetFlag=");
		pEnd = strchr(pHead,'|');
		memset(pBody->order.CombOffsetFlag,0,sizeof(pBody->order.CombOffsetFlag));
		char flag[16] = {0};
							
		if(NULL == pEnd)
		{
			strcpy(flag,pHead);
		}
		else
		{
			strncpy(flag,pHead,pEnd-pHead);
		}
							
		if (strcasecmp(flag, "Open") == 0)
		{
			pBody->order.CombOffsetFlag[0] = THOST_FTDC_OF_Open; 
		}
		else if (strcasecmp(flag, "Close") == 0)
		{
			pBody->order.CombOffsetFlag[0] = THOST_FTDC_OF_Close; 
		}
		else if (strcasecmp(flag, "CloseToday") == 0)
		{
	
			pBody->order.CombOffsetFlag[0] = THOST_FTDC_OF_CloseToday; 
		}
		else if (strcasecmp(flag, "CloseYesterday") == 0)
		{
	
			pBody->order.CombOffsetFlag[0] = THOST_FTDC_OF_CloseYesterday; 
		}
		else
		{
			g_log.error("Undefined CombOffsetFlag:%s\n",flag);
		}
	}
	else
	{
		g_log.error("%s:no CombOffsetFlag !\n",pValue);
	}

	return ATF_SUCC;					
}


int CCasesParser::ParseWaitReqOrderAction(OP &op,char *pValue)
{
	op.direction = RCV;
	op.msgID = MSGID_C2S_TRADE_ReqOrderAction;
		
	op.msgSize = sizeof(MsgHead);
	MsgHead *pMsg = new MsgHead;
	memset(pMsg,0,sizeof(*pMsg));
	pMsg->iMsgID = MSGID_C2S_TRADE_ReqOrderAction;
	pMsg->iMsgBodyLen = 0;
	op.pMsg = (char *)pMsg;

	return ATF_SUCC;					
}

int CCasesParser::ParseSendOnRtnOrder(OP &op,char *pValue)
{
	op.direction = SEND;
	op.msgID = MSGID_S2C_TRADE_OnRtnOrder;
									
	op.msgSize = sizeof(MsgHead) + sizeof(OnRtnOrderFromATF);
	char *pMsg = new char[op.msgSize];
	memset(pMsg,0,sizeof(*pMsg));	
	op.pMsg = pMsg;
	
	MsgHead *pHeadMsg = (MsgHead *)pMsg;
	pHeadMsg->iMsgID = MSGID_S2C_TRADE_OnRtnOrder;
	pHeadMsg->iMsgBodyLen = sizeof(OnRtnOrderFromATF);
									
	OnRtnOrderFromATF *pBody = (OnRtnOrderFromATF*)(pMsg+sizeof(MsgHead));
	if (strlen(pValue) <= 0)
	{
		g_log.error("SendOnRtnOrder is null !\n");
		return ATF_FAIL;					
	}

	char *pHead = NULL;
	char *pEnd = NULL;

	//解析orderID
	pHead = strstr(pValue,"orderID=");
	if(NULL != pHead)
	{
		pHead = pHead + strlen("orderID=");
		pEnd = strchr(pHead,'|');

		char orderID[16] = {0};
		if(NULL == pEnd)
		{
			strcpy(orderID,pHead);
		}
		else
		{
			strncpy(orderID,pHead,pEnd-pHead);
		}

		//利用RequestID存储对应的报单ID
		pBody->order.RequestID= atoi(orderID);
	}
	else
	{
		g_log.error("No orderID !\n");
		return ATF_FAIL;
	}
	
	//VolumeTraded
	pHead = strstr(pValue,"VolumeTraded=");
	if(NULL != pHead)
	{
		pHead = pHead + strlen("VolumeTraded=");
		pEnd = strchr(pHead,'|');

		char vol[16] = {0};
		if(NULL == pEnd)
		{
			strcpy(vol,pHead);
		}
		else
		{
			strncpy(vol,pHead,pEnd-pHead);
		}

		pBody->order.VolumeTraded= atoi(vol);
	}
	else
	{
		g_log.error("No VolumeTraded !\n");
		return ATF_FAIL;
	}

	//VolumeTotal
	pHead = strstr(pValue,"VolumeTotal=");
	if(NULL != pHead)
	{
		pHead = pHead + strlen("VolumeTotal=");
		pEnd = strchr(pHead,'|');

		char vol[16] = {0};
		if(NULL == pEnd)
		{
			strcpy(vol,pHead);
		}
		else
		{
			strncpy(vol,pHead,pEnd-pHead);
		}

		pBody->order.VolumeTotal= atoi(vol);
	}
	else
	{
		g_log.error("No VolumeTotal !\n");
		return ATF_FAIL;
	}
		
	//解析SubmitStatus
	pHead = strstr(pValue,"SubmitStatus=");
	if(NULL != pHead)
	{
		pHead = pHead + strlen("SubmitStatus=");
		pEnd = strchr(pHead,'|');
		
		char SubmitStatus[32] = {0};
		if(NULL == pEnd)
		{
			strcpy(SubmitStatus,pHead);
		}
		else
		{
			strncpy(SubmitStatus,pHead,pEnd-pHead);
		}

		if (strcasecmp(SubmitStatus, "InsertSubmitted") == 0)
		{
			pBody->order.OrderSubmitStatus = THOST_FTDC_OSS_InsertSubmitted; 
		}
		else if (strcasecmp(SubmitStatus, "CancelSubmitted") == 0)
		{
			pBody->order.OrderSubmitStatus = THOST_FTDC_OSS_CancelSubmitted; 
		}
		else if (strcasecmp(SubmitStatus, "Accepted") == 0)
		{
		
			pBody->order.OrderSubmitStatus = THOST_FTDC_OSS_Accepted; 
		}
		else if (strcasecmp(SubmitStatus, "InsertRejected") == 0)
		{
		
			pBody->order.OrderSubmitStatus = THOST_FTDC_OSS_InsertRejected; 
		}
		else if (strcasecmp(SubmitStatus, "CancelRejected") == 0)
		{
		
			pBody->order.OrderSubmitStatus = THOST_FTDC_OSS_CancelRejected; 
		}
		else
		{
			g_log.error("Undefined OrderSubmitStatus:%s\n",SubmitStatus);
		}		
	}	
	else
	{
		g_log.error("No SubmitStatus !\n");
		return ATF_FAIL;
	}
	
	//解析OrderStatus
	pHead = strstr(pValue,"OrderStatus=");
	if(NULL != pHead)
	{
		pHead = pHead + strlen("OrderStatus=");
		pEnd = strchr(pHead,'|');
		
		char OrderStatus[32] = {0};
		if(NULL == pEnd)
		{
			strcpy(OrderStatus,pHead);
		}
		else
		{
			strncpy(OrderStatus,pHead,pEnd-pHead);
		}

		if (strcasecmp(OrderStatus, "AllTraded") == 0)
		{
			pBody->order.OrderStatus = THOST_FTDC_OST_AllTraded; 
		}
		else if (strcasecmp(OrderStatus, "PartTradedQueueing") == 0)
		{
			pBody->order.OrderStatus = THOST_FTDC_OST_PartTradedQueueing; 
		}
		else if (strcasecmp(OrderStatus, "PartTradedNotQueueing") == 0)
		{
		
			pBody->order.OrderStatus = THOST_FTDC_OST_PartTradedNotQueueing; 
		}
		else if (strcasecmp(OrderStatus, "NoTradeQueueing") == 0)
		{
		
			pBody->order.OrderStatus = THOST_FTDC_OST_NoTradeQueueing; 
		}
		else if (strcasecmp(OrderStatus, "NoTradeNotQueueing") == 0)
		{
		
			pBody->order.OrderStatus = THOST_FTDC_OST_NoTradeNotQueueing; 
		}
		else if (strcasecmp(OrderStatus, "Canceled") == 0)
		{
		
			pBody->order.OrderStatus = THOST_FTDC_OST_Canceled; 
		}
		else if (strcasecmp(OrderStatus, "Unknown") == 0)
		{
		
			pBody->order.OrderStatus = THOST_FTDC_OST_Unknown; 
		}
		else if (strcasecmp(OrderStatus, "NotTouched") == 0)
		{
		
			pBody->order.OrderStatus = THOST_FTDC_OST_NotTouched; 
		}
		else if (strcasecmp(OrderStatus, "Touched") == 0)
		{
		
			pBody->order.OrderStatus = THOST_FTDC_OST_Touched; 
		}
		else
		{
			g_log.error("Undefined OrderStatus:%s\n",OrderStatus);
		}		
	}
	else
	{
		g_log.error("No OrderStatus !\n");
		return ATF_FAIL;
	}

	//g_log.debug("orderID=%d, SubmitStatus=%c, OrderStatus=%c, VolumeTraded=%d, VolumeTotal=%d\n",
	//	pBody->order.RequestID,pBody->order.OrderSubmitStatus,pBody->order.OrderStatus,pBody->order.VolumeTraded,pBody->order.VolumeTotal);

	return ATF_SUCC;
}

int CCasesParser::ParseSendOnRtnTrade(OP &op,char *pValue)
{
	op.direction = SEND;
	op.msgID = MSGID_S2C_TRADE_OnRtnTrade;
									
	op.msgSize = sizeof(MsgHead) + sizeof(OnRtnTradeFromATF);
	char *pMsg = new char[op.msgSize];
	memset(pMsg,0,sizeof(*pMsg));
	MsgHead *pHeadMsg = (MsgHead *)pMsg;
	pHeadMsg->iMsgID = MSGID_S2C_TRADE_OnRtnTrade;
	pHeadMsg->iMsgBodyLen = sizeof(OnRtnTradeFromATF);
	op.pMsg = pMsg;
									
	OnRtnTradeFromATF *pBody = (OnRtnTradeFromATF*)(pMsg+sizeof(MsgHead));
	if (strlen(pValue) <= 0)
	{
		g_log.error("SendOnRtnTrade is null !\n");
		return ATF_FAIL;					
	}

	char *pHead = NULL;
	char *pEnd = NULL;

	//解析orderID
	pHead = strstr(pValue,"orderID=");
	if(NULL != pHead)
	{
		pHead = pHead + strlen("orderID=");
		pEnd = strchr(pHead,'|');

		char orderID[16] = {0};
		if(NULL == pEnd)
		{
			strcpy(orderID,pHead);
		}
		else
		{
			strncpy(orderID,pHead,pEnd-pHead);
		}
		
		//利用SequenceNo存储对应的报单ID
		pBody->trade.SequenceNo = atoi(orderID);
	}
	else
	{
		g_log.error("No orderID !\n");
		return ATF_FAIL;
	}

	//解析数量
	pHead = strstr(pValue,"vol=");
	if(NULL != pHead)
	{
		pHead = pHead + strlen("vol=");
		pEnd = strchr(pHead,'|');
							
		char vol[16] = {0};
							
		if(NULL == pEnd)
		{
			strcpy(vol,pHead);
		}
		else
		{
			strncpy(vol,pHead,pEnd-pHead);
		}

		pBody->trade.Volume = atoi(vol);
	}
					
	//解析价格
	pHead = strstr(pValue,"price=");
	if(NULL != pHead)
	{
		pHead = pHead + strlen("price=");
		pEnd = strchr(pHead,'|');
							
		char price[16] = {0};
							
		if(NULL == pEnd)
		{
			strcpy(price,pHead);
		}
		else
		{
			strncpy(price,pHead,pEnd-pHead);
		}

		pBody->trade.Price = atof(price);
	}

	//g_log.debug("orderID=%d, vol=%d, price=%f\n",pBody->trade.SequenceNo,pBody->trade.Volume,pBody->trade.Price);
	return ATF_SUCC;
}

int CCasesParser::ParseSendOnRspOrderInsert(OP &op,char *pValue)
{
	op.direction = SEND;
	op.msgID = MSGID_S2C_TRADE_OnRspOrderInsert;	
	
	op.msgSize = sizeof(MsgHead) + sizeof(OnRspOrderInsertFromATF);	
	char *pMsg = new char[op.msgSize];
	memset(pMsg,0,sizeof(*pMsg));	
	op.pMsg = pMsg;
	
	MsgHead *pHeadMsg = (MsgHead *)pMsg;
	pHeadMsg->iMsgID = MSGID_S2C_TRADE_OnRspOrderInsert;
	pHeadMsg->iMsgBodyLen = sizeof(OnRspOrderInsertFromATF);

	OnRspOrderInsertFromATF *pBody = (OnRspOrderInsertFromATF*)(pMsg+sizeof(MsgHead));
	
	if (strlen(pValue) <= 0)
	{
		g_log.error("SendOnRspOrderInsert is null !\n");
		return ATF_FAIL;					
	}

	char *pHead = NULL;
	char *pEnd = NULL;

	//解析orderID
	pHead = strstr(pValue,"orderID=");
	if(NULL != pHead)
	{
		pHead = pHead + strlen("orderID=");
		pEnd = strchr(pHead,'|');

		char orderID[16] = {0};
		if(NULL == pEnd)
		{
			strcpy(orderID,pHead);
		}
		else
		{
			strncpy(orderID,pHead,pEnd-pHead);
		}

		pBody->inputOrder.RequestID = atoi(orderID);
	}
	else
	{
		g_log.error("[%s]No orderID !\n",__FUNCTION__);
		return ATF_FAIL;
	}

	
	//解析errorID
	pHead = strstr(pValue,"errorID=");
	if(NULL != pHead)
	{
		pHead = pHead + strlen("errorID=");
		pEnd = strchr(pHead,'|');

		char errorID[16] = {0};
		if(NULL == pEnd)
		{
			strcpy(errorID,pHead);
		}
		else
		{
			strncpy(errorID,pHead,pEnd-pHead);
		}

		pBody->rspInfo.ErrorID = atoi(errorID);
	}
	else
	{
		pBody->rspInfo.ErrorID = 15;  //综合交易平台：报单字段有误
	}
	
	return ATF_SUCC;
}

int CCasesParser::ParseSendOnErrRtnOrderInsert(OP &op,char *pValue)
{
	op.direction = SEND;
	op.msgID = MSGID_S2C_TRADE_OnErrRtnOrderInsert;	
	
	op.msgSize = sizeof(MsgHead) + sizeof(OnErrRtnOrderInsertFromATF);	
	char *pMsg = new char[op.msgSize];
	memset(pMsg,0,sizeof(*pMsg));	
	op.pMsg = pMsg;
	
	MsgHead *pHeadMsg = (MsgHead *)pMsg;
	pHeadMsg->iMsgID = MSGID_S2C_TRADE_OnErrRtnOrderInsert;
	pHeadMsg->iMsgBodyLen = sizeof(OnErrRtnOrderInsertFromATF);

	OnErrRtnOrderInsertFromATF *pBody = (OnErrRtnOrderInsertFromATF*)(pMsg+sizeof(MsgHead));

	if (strlen(pValue) <= 0)
	{
		g_log.error("SendOnErrRtnOrderInsert is null !\n");
		return ATF_FAIL;					
	}

	char *pHead = NULL;
	char *pEnd = NULL;

	//解析orderID
	pHead = strstr(pValue,"orderID=");
	if(NULL != pHead)
	{
		pHead = pHead + strlen("orderID=");
		pEnd = strchr(pHead,'|');

		char orderID[16] = {0};
		if(NULL == pEnd)
		{
			strcpy(orderID,pHead);
		}
		else
		{
			strncpy(orderID,pHead,pEnd-pHead);
		}

		pBody->inputOrder.RequestID = atoi(orderID);
	}
	else
	{
		g_log.error("[%s]No orderID !\n",__FUNCTION__);
		return ATF_FAIL;
	}

	//解析errorID
	pHead = strstr(pValue,"errorID=");
	if(NULL != pHead)
	{
		pHead = pHead + strlen("errorID=");
		pEnd = strchr(pHead,'|');

		char errorID[16] = {0};
		if(NULL == pEnd)
		{
			strcpy(errorID,pHead);
		}
		else
		{
			strncpy(errorID,pHead,pEnd-pHead);
		}

		pBody->rspInfo.ErrorID = atoi(errorID);
	}
	else
	{
		pBody->rspInfo.ErrorID = 17;  //综合交易平台：合约不能交易
	}
	
	return ATF_SUCC;
}

int CCasesParser::ParseSendOnRspOrderAction(OP &op,char *pValue)
{
	op.direction = SEND;
	op.msgID = MSGID_S2C_TRADE_OnRspOrderAction;	
	
	op.msgSize = sizeof(MsgHead) + sizeof(OnRspOrderActionFromATF);	
	char *pMsg = new char[op.msgSize];
	memset(pMsg,0,sizeof(*pMsg));	
	op.pMsg = pMsg;
	
	MsgHead *pHeadMsg = (MsgHead *)pMsg;
	pHeadMsg->iMsgID = MSGID_S2C_TRADE_OnRspOrderAction;
	pHeadMsg->iMsgBodyLen = sizeof(OnRspOrderActionFromATF);

	OnRspOrderActionFromATF *pBody = (OnRspOrderActionFromATF*)(pMsg+sizeof(MsgHead));	
	
	if (strlen(pValue) <= 0)
	{
		g_log.error("SendOnRspOrderAction is null !\n");
		return ATF_FAIL;					
	}

	char *pHead = NULL;
	char *pEnd = NULL;

	//解析cancelID
	pHead = strstr(pValue,"cancelID=");
	if(NULL != pHead)
	{
		pHead = pHead + strlen("cancelID=");
		pEnd = strchr(pHead,'|');

		char cancelID[16] = {0};
		if(NULL == pEnd)
		{
			strcpy(cancelID,pHead);
		}
		else
		{
			strncpy(cancelID,pHead,pEnd-pHead);
		}

		pBody->inputOrder.RequestID = atoi(cancelID);
	}
	else
	{
		g_log.error("[%s]No cancelID !\n",__FUNCTION__);
		return ATF_FAIL;
	}

	//解析errorID
	pHead = strstr(pValue,"errorID=");
	if(NULL != pHead)
	{
		pHead = pHead + strlen("errorID=");
		pEnd = strchr(pHead,'|');

		char errorID[16] = {0};
		if(NULL == pEnd)
		{
			strcpy(errorID,pHead);
		}
		else
		{
			strncpy(errorID,pHead,pEnd-pHead);
		}

		pBody->rspInfo.ErrorID = atoi(errorID);
	}
	else
	{
		pBody->rspInfo.ErrorID = 23;   //综合交易平台：错误的报单操作字段
	}
	
	return ATF_SUCC;
}

int CCasesParser::ParseSendOnErrRtnOrderAction(OP &op,char *pValue)
{
	op.direction = SEND;
	op.msgID = MSGID_S2C_TRADE_OnErrRtnOrderAction;	
	
	op.msgSize = sizeof(MsgHead) + sizeof(OnErrRtnOrderActionFromATF);	
	char *pMsg = new char[op.msgSize];
	memset(pMsg,0,sizeof(*pMsg));	
	op.pMsg = pMsg;
	
	MsgHead *pHeadMsg = (MsgHead *)pMsg;
	pHeadMsg->iMsgID = MSGID_S2C_TRADE_OnErrRtnOrderAction;
	pHeadMsg->iMsgBodyLen = sizeof(OnErrRtnOrderActionFromATF);

	OnErrRtnOrderActionFromATF *pBody = (OnErrRtnOrderActionFromATF*)(pMsg+sizeof(MsgHead));	
	
	if (strlen(pValue) <= 0)
	{
		g_log.error("SendOnErrRtnOrderAction is null !\n");
		return ATF_FAIL;					
	}

	char *pHead = NULL;
	char *pEnd = NULL;

	//解析cancelID
	pHead = strstr(pValue,"cancelID=");
	if(NULL != pHead)
	{
		pHead = pHead + strlen("cancelID=");
		pEnd = strchr(pHead,'|');

		char cancelID[16] = {0};
		if(NULL == pEnd)
		{
			strcpy(cancelID,pHead);
		}
		else
		{
			strncpy(cancelID,pHead,pEnd-pHead);
		}

		pBody->inputOrder.RequestID = atoi(cancelID);
	}
	else
	{
		g_log.error("[%s]No cancelID !\n",__FUNCTION__);
		return ATF_FAIL;
	}

	//解析errorID
	pHead = strstr(pValue,"errorID=");
	if(NULL != pHead)
	{
		pHead = pHead + strlen("errorID=");
		pEnd = strchr(pHead,'|');

		char errorID[16] = {0};
		if(NULL == pEnd)
		{
			strcpy(errorID,pHead);
		}
		else
		{
			strncpy(errorID,pHead,pEnd-pHead);
		}

		pBody->rspInfo.ErrorID = atoi(errorID);
	}
	else
	{
		pBody->rspInfo.ErrorID = 26;   //综合交易平台：报单已全成交或已撤销，不能再撤
	}

	return ATF_SUCC;
}

