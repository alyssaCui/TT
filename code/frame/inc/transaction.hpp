#include <time.h>
#include<sys/time.h>
#include <unistd.h>

#include "Mutex.h"
#include "Thread.h"
//#include "constants.h"
#include "common.h"

enum TRANS_RESULT_TYPE
{
	TRANS_RESULT_NULL = 0,
	TRANS_RESULT_SUCC = 1,
	TRANS_RESULT_FAIL = 2,
	TRANS_RESULT_TIMEOUT = 3,
	TRANS_RESULT_LOST = 4
};

class CTransData
{
public:
	
	long m_num;
	
	struct timeval m_sumTime;
	struct timeval m_minTime;
	struct timeval m_maxTime;
	
	float m_fAverTime;
	float m_fMinTime;
	float m_fMaxTime;

	float m_speed;

	int addRev(struct timeval);
	int countSummary(struct timeval);
	
};

class CTransInfo
{
public:

	char name[MAX_CMD_VAR_NAME_LEN];

	//trans occur first and last time
	struct timeval m_startTime;
	struct timeval m_endTime;
	struct timeval m_durationTime;

	CTransData m_totalData;
	CTransData m_succData;
	CTransData m_failData;
	CTransData m_timeoutData;
	CTransData m_lostData;

	int countSummary(struct timeval, struct timeval);
};


class CTransaction: public CMutex
{
public:
	int init(char* name);
	int finish();
	int startTrans();
	int endTrans(TRANS_RESULT_TYPE transResult);
	int addTransRev(TRANS_RESULT_TYPE transResult, struct timeval transTime);
	int addTransRevBase(TRANS_RESULT_TYPE transResult, struct timeval transTime);
	int getTransSummary(CTransInfo &totalInfo, CTransInfo &curInfo, CTransInfo &lastInfo, int newSummaryFlag);
	int getTransInfo(CTransInfo &totalInfo);	

	char m_name[MAX_CMD_VAR_NAME_LEN];
	
	CTransInfo m_totalInfo;
	CTransInfo m_curInfo;
	CTransInfo m_lastInfo;
	
	struct timeval m_initTime;
	struct timeval m_lastSummaryTime;
	struct timeval m_finishTime;
	struct timeval m_transStartTime;
	
	int m_finishFlag;
	//struct timeval m_transEndTime;
};

class CAllTrans
{
public:
	CTransaction transactConnect;
	CTransaction transactSendQuote;
	CTransaction transactRevOrderInsert;		
	CTransaction transactSendOrderRsp;	
	CTransaction processTime;

	CAllTrans();
	int finish();
};

