#ifndef __QUOTE_SVR_SIMU_H__
#define __QUOTE_SVR_SIMU_H__

#include <time.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <map>

//#include "constants.h"
#include "common.h"
#include "Mutex.h"
#include "Thread.h"
#include "network.h"

#include "USTPFtdcMduserApi.hpp"
#include "datafetcher.h"
#include "appsCommon.hpp"


//#define  FRONTID   0    //前置机的编号,后期改为从配置文件里读取
#define MAX_QUOTE_PROVIDERS  8


enum QUOTE_SEND_STATUS
{
	CAN_NOT_SEND_QUOTE = 0,
	CAN_SEND_QUOTE
};

struct QuoteConf
{
	int isValid;
	int quoteSvrFrontID;   //to be deleted
	int quoteSvrPort;
	char quoteSvrIP[128];
};

struct QuoteInfo_t
{
	char file[MAX_FILE_PATH_LEN];
	int  sendInterval;            //unit: ms
	
};

struct QuoteDataItem
{
	int iId;
};

struct QuoteOrder
{
	unsigned int uiProvidersTotal;
	OPERATE_SET quoteOp[MAX_QUOTE_PROVIDERS];	
};

struct ProviderInitData
{
	OPERATE_SET *pOpSet;
	int iFrontID;
};

class CProvider
{
public:	
	CProvider();
	~CProvider();
	void Init(ProviderInitData);
	void Destroy();	
	void SetFd(int);
	int GetFd();	
	int CloseFd();
	int IsFinished();
	void HandleMsgFromClient();	
	int GotMsgSize(MSG_ID id);
	void RcvMsgFromClient(char * pBuf, int iBufSize,int *msgSize, int clientSock);
	int CheckMsg(char * pExpectMsg, int expectMsgLen, char *actualMsg, int actualMsgLen);
	int CmpMsg(int iMsgID, char * pExpectMsg, char *pActualMsg);
	int CmpMsgReqUserLogin(QuoteReqUserLoginToATF * pExpectMsg,QuoteReqUserLoginToATF *pActualMsg);
	int CmpReqSubscribeMarketMsg(ReqSubMarketToATF * pExpectMsg,ReqSubMarketToATF *pActualMsg);
	int CmpReqUnsubscribeMarketMsg(ReqUnsubMarketToATF * pExpectMsg,ReqUnsubMarketToATF *pActualMsg);
	int FillMsgOnRspLogin(QuoteRspUserLoginFromATF *pRsp);
	int MsgIDToMsgName(MSG_ID id,char *pMsgName,int len);
	//int GetQuoteSendFlag(){ return m_flag_quote_send_over;}
	int GotAgentIp(char * ip);

private:	
	int m_clientSock;
	char *m_tcpbuf;
	char udpbuf[UDP_BUFSIZE];
	char m_agentIP[32];
		
		
	CONNECT_STATUS m_connect;
	LOGIN_STATUS m_login;
		
	struct timeval m_lastSendTime;
		
	UserInfoLogined m_userLogined;
	TThostFtdcInstrumentIDType m_acInstrSubReq; 
	TThostFtdcInstrumentIDType m_acInstrUnsubReq; 

	OPERATE_SET *m_vOperations;
	OPERATE_SET::iterator m_iterCur;
	OPERATE_SET::iterator m_iterEnd;
	int m_iProgress;
	char m_curOpErrReason[128];
	
	int m_frontID;
	int m_sessionID;
	long long m_maxOrderRef;
	long long m_llQuoteSentNum;
	
};

class QuoteServer : public CThread, public CMutex
{
	public:
		QuoteServer(struct QuoteConf);
		virtual ~QuoteServer();
		virtual void* Run(void *);
		void StartNewOrder(QuoteOrder *);		
		void GetOrderStatus();
		int InitProvider();
		void DestroyProvider();
	
	public:			
		int m_status;
		//long long m_llQuoteSentNum;
	
	private:
		char m_quoteSvrIP[128];
		int m_quoteSvrPort;
			
		int m_quoteSvrMsgConnPort;
		int m_quoteSvrMsgConnFd;
		
		QuoteOrder *m_vOrder;
		CProvider m_aProviders[MAX_QUOTE_PROVIDERS];
		unsigned int m_uiExpectProviderTotal;
		unsigned int m_uiActualProviderTotal;
		time_t m_tOrderStartTime;
};


#endif
