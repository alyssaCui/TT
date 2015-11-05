#ifndef __EXCHG_SIMU_H__
#define __EXCHG_SIMU_H__

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

#include "thosttraderapi.hpp"
#include "appsCommon.hpp"

//#define  FRONTID   0    //ǰ�û��ı��,���ڸ�Ϊ�������ļ����ȡ
#define MAX_CLIENTS        8


enum TRADE_MODE
{
	LIMITED = 0,
	UNLIMITED
};

enum Trade_RESULT_TYPE
{
	ALL_SUCC_ONE_TIME = 300,    //����һ��ȫ���ɽ�
	ALL_SUCC_TWO_TIME,		    //���׷����γɽ�ȫ���ɽ�
	ALL_FAIL_ONE_TIME,          //����ʧ��
	PART_SUCC_ONE_VOLUME,       //����ֻ�ɽ�һ��
	REJECT_BY_COUNTER,		   //���׺�����Ϊ��������
	REJECT_BY_EXCHANGE, 	  //��������Ϊ��������
	SUCC,
	MULTI_ALL_SUCC,             //��ʽ��ף�ȫ���ɹ�
	MULTI_ALL_FAIL,            //��ʽ��ף�ȫ��ʧ��
	MULTI_FAIL_SUCC,           //��ʽ��ף��Ȳ���ʧ�ܣ��󲿷ֳɹ�
	MULTI_SUCC_FAIL,           //��ʽ��ף��Ȳ��ֳɹ����󲿷�ʧ��	
	ORDER_NOT_TRIGGERED,       //���׹�̨����Ĵ��󣬱���δ����
	TWO_VOLUME_SUCC,
	THREE_VOLUME_SUCC,
    FOUR_VOLUME_SUCC,
	FIVE_VOLUME_SUCC,
    SIX_VOLUME_SUCC,
};

enum OrderType_t
{
	ROD = 0,
	FOK,
	FAK,
	MRK
};

struct ExchgConf
{
	int isValid;
	int exchgSvrFrontID; //to be deleted
	int exchgSvrPort;
	char exchgSvrIP[128];
	char exchangeID[LEN_ExchangeID];
};

struct ExchgOrder
{
	TRADE_MODE tradeMode;
	unsigned int uiClientsTotal;
	OPERATE_SET exchgOp[MAX_CLIENTS];
};

struct AccountInitData
{
	OPERATE_SET *pOpSet;
	int iFrontID;
	char exchangeID[LEN_ExchangeID];	
	char traderID[21];
};


struct TradeResult_t
{
	Trade_RESULT_TYPE result;		
};

class CAccountTrade
{
public:
	CAccountTrade();
	virtual ~CAccountTrade();	
	void Init(AccountInitData);
	void Destroy();	
	void SetFd(int sockfd);
	int GetFd();	
	int CloseFd();
	int IsFinished();	
	void HandleMsgFromClient();	
	void RcvMsgFromClient(char * buf, int iBufSize,int *msgSize, int clientSock);
	int RspUserAuthenticate();
	
	int OnFrontConnected(char *pMsg,int client_fd);
	int OnFrontDisconnected(char *pMsg,int client_fd);
	int OnRspUserLogin(char *pMsg,int client_fd);
	int OnRtnOrder(char *pMsg,int client_fd);	
	int OnRtnTrade(char *pMsg,int client_fd);
	int OnRspOrderInsert(char *pMsg,int client_fd);
	int OnErrRtnOrderInsert(char *pMsg,int client_fd);		
	int OnRspOrderAction(char *pMsg,int client_fd);
	int OnErrRtnOrderAction(char *pMsg,int client_fd);
	int CheckMsg(char * pExpectMsg, int expectMsgLen, char *actualMsg, int actualMsgLen);		
	int CmpMsg(int iMsgID, char * pExpectMsg, char *pActualMsg);
	int CmpMsgReqUserLogin(TradeReqUserLoginToATF * pExpectMsg,TradeReqUserLoginToATF *pActualMsg);
	int CmpMsgReqOrderInsert(ReqOrderInsertToATF * pExpectMsg,ReqOrderInsertToATF *pActualMsg);	
	int CmpMsgReqOrderAction(ReqOrderActionToATF * pExpectMsg,ReqOrderActionToATF *pActualMsg);
	
	int FillMsgOnRspLogin(TradeRspUserLoginFromATF *pRsp);
	int FillMsgOnRspOrderAction(CThostFtdcInputOrderActionField *pReq,CThostFtdcInputOrderActionField *pRsp);
	int FillMsgOnErrRtnOrderAction(CThostFtdcInputOrderActionField *pReq,CThostFtdcOrderActionField *pRsp);
	int FillMsgOnRtnOrder(CThostFtdcInputOrderField *pReq,CThostFtdcOrderField *pRsp);
	int FillMsgOnRtnTrade(CThostFtdcInputOrderField *pReq,CThostFtdcTradeField *pRsp);	
	int FillRtnOrderActionCommonPart(CThostFtdcInputOrderActionField *pReq,CThostFtdcOrderField *pRsp);
	int FillErrRtnOrderInsert(CThostFtdcInputOrderField *pReq,CThostFtdcOrderField *pRsp);
	
	int MsgIDToMsgName(MSG_ID id,char *pMsgName,int len);
		
	int GotMsgSize(MSG_ID id);
	void SetTradeMode(TRADE_MODE);	

		


private:
	int m_clientSock;
	
	char *m_tcpbuf;
	
	OPERATE_SET *m_vOperations;
	OPERATE_SET::iterator m_iterCur;
	OPERATE_SET::iterator m_iterEnd;
	char m_curOpErrReason[128];
	
	char m_account[32];
	char m_passwd[32];
	int m_resultForConnect;
	int m_resultForLogin;
	int m_resultForTrade;
	CONNECT_STATUS m_connect;
	LOGIN_STATUS m_login;
	UserInfoLogined m_userLogined;
	TRADE_MODE m_tradeMode;
	int m_iProgress;
	int m_frontID;   //ǰ�ñ�� 
	char m_exchangeID[LEN_ExchangeID];		// ExchangeID ����������
	long long m_iActiveUserID;
	int m_sessionID;
	long long m_maxOrderRef;	//��󱨵���� 
	char m_acTraderID[21];      //����Ա���룬���׺�������
	long long m_orderLocalID;   //���ر������,�ɽ��׺�������

	int m_iOrderNum;
	int m_iCancelOrderNum;
	std::map<int, ReqOrderInsertToATF>	  m_mOrder;
	std::map<std::string,std::string> m_mOrderRef2SysID;	
	std::map<int, ReqOrderActionToATF>	 m_mCancelOrder;
	
	std::map<std::string, ReqOrderInsertToATF>	 m_mPendingOrder;
	std::vector<ReqOrderActionToATF>		 m_vCancelOrder;
	
};



class ExchgServer : public CThread, public CMutex
{
	public:
		ExchgServer(struct ExchgConf);
		virtual ~ExchgServer();
		virtual void* Run(void *);
		void StartNewOrder(ExchgOrder *);		
		void GetOrderStatus();
		void Finish(){m_status=NO_TASK;}			
		int InitAccounts();
		void DestroyAccounts();
			
	public:				
		int m_status;
		
	private:
		char m_exchgSvrIP[128];
		unsigned int m_exchgSvrPort;
		char m_exchangeID[LEN_ExchangeID];		// ExchangeID ��Լ���ڽ������Ĵ���
		int m_exchgConnFd;	
		ExchgOrder *m_vOrder;
		time_t m_tOrderStartTime;

		CAccountTrade m_aClients[MAX_CLIENTS];
		unsigned int m_uiExpectClientTotal;
		unsigned int m_uiActualClientTotal;
};

#endif
