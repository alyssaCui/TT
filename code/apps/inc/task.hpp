#ifndef __TEST_LOGIC_H__
#define __TEST_LOGIC_H__

#include "casesParser.hpp"
#include "exchgSimu.hpp"
#include "quoteSvrSimu.hpp"
#include "transaction.hpp"

#include "log.h"
//#include "constants.h"
#include "common.h"

#include <vector>



struct FileNode
{
    char fileName[MAX_FILE_PATH_LEN];
	char absolutePath[MAX_FILE_PATH_LEN];
	int  status;
	struct FileNode *next;
};


struct GtestConf
{
	int argc;
	char argv[10][MAX_CMD_LINE_LEN];
};

struct TaskConf
{
	QuoteConf g_quoteConf;
	ExchgConf g_exchgConf;
	AgentConf g_agentConf;
	char m_arrTestcasesPath[MAX_FILE_PATH_LEN];
	char m_arrTestDataPath[MAX_FILE_PATH_LEN];
	char progDir[MAX_FILE_PATH_LEN];
	GtestConf gtestConf;   //to be deleted
};

class CTask
{
	public:
		CTask();
		virtual ~CTask();
		int Init(TaskConf);
		int LoadTestCases(char *pPath,TestCase *);		
		int GetCasesList();
		int InitWorkItem(TestCase &item);
		int ExecuteWorkItem();
		int CleanCase(TestCase *);		
		int BackupLastTask();
		int finishTransact();		
		int run();		
		int GetCaseResult(TestCase *);		
		int report();		
		int GotStaticInfo();		
		int CommuWithAgent(MSG_ID id);
		int PrintResultToXML();
		

	private:		
		struct FileNode *m_pTestcasesFileList;
		
		TestCase *m_casesList;
		unsigned int m_uiTotalCases;
		unsigned int m_uiSuccCases;
		unsigned int m_uiFailCases;

		struct timeval m_stTimeStartTask;
		struct timeval m_stTimeEndTask;
		unsigned long m_ulTimeTaskDone;
		
		QuoteConf m_quoteConf;
		ExchgConf m_exchgConf;
		AgentConf m_agentConf;
		GtestConf m_gtestConf;
		
		char m_arrTestcasesPath[MAX_FILE_PATH_LEN];
		char m_arrTestDataPath[MAX_FILE_PATH_LEN];
		char m_progPath[MAX_FILE_PATH_LEN];
		
		QuoteServer *m_pQuoteSvr;
		ExchgServer *m_pExchgSvr;

		InfoTradeProcessTime m_stTradeProcTimeStatic;
		StaticQuoteInterval_t m_stQuoteIntervalStatic;

		CAllTrans m_allTransact;

		int m_agentSocket;
};


#endif

