#ifndef __CASES_PARSER_H__
#define __CASES_PARSER_H__

#include "common.h"
#include "quoteSvrSimu.hpp"
#include "exchgSimu.hpp"


struct TestCase
{
    char fileName[MAX_FILE_PATH_LEN];
	char absolutePath[MAX_FILE_PATH_LEN];
	int  status;
	struct timeval startTime;
	struct timeval endTime;
	char errReason[128];
	QuoteOrder quoteOrder;
	ExchgOrder exchgOrder;
	TestCase  *next;
};


class CCasesParser
{
public:
	CCasesParser();
	~CCasesParser();
	int GetCasesList(char *pTestcasesPath,TestCase * & pCasesList,unsigned int *num);
	int LoadTestCases(char *pPath,TestCase *pCase);
	int ParseWaitReqOrderInsert(OP &op,char *pValue);
	int ParseWaitReqOrderAction(OP &op,char *pValue);
	int ParseSendOnRtnOrder(OP &op,char *pValue);
	int ParseSendOnRtnTrade(OP &op,char *pValue);
	int ParseSendOnRspOrderInsert(OP &op,char *pValue);
	int ParseSendOnErrRtnOrderInsert(OP &op,char *pValue);
	int ParseSendOnRspOrderAction(OP &op,char *pValue);
	int ParseSendOnErrRtnOrderAction(OP &op,char *pValue);
	
private:
	char m_arrTestcasesPath[MAX_FILE_PATH_LEN];
	
	unsigned int m_uiTotalCases;
};

#endif
