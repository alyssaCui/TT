/////////////////////////////////////////////////////////////////////////
///@system ���ǰ��ϵͳ
///@company �Ϻ������ڻ���Ϣ�������޹�˾
///@file USTPFtdcMduserApi.h
///@brief �����˿ͻ��˽ӿ�
///@history 
///20130520	���һ�	�������ļ�
/////////////////////////////////////////////////////////////////////////

#if !defined(USTP_FTDCMDUSERAPI_H)
#define USTP_FTDCMDUSERAPI_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#if defined(ISLIB) && defined(WIN32)
#ifdef LIB_MDUSER_API_EXPORT
#define MDUSER_API_EXPORT __declspec(dllexport)
#else
#define MDUSER_API_EXPORT __declspec(dllimport)
#endif
#else
#define MDUSER_API_EXPORT 
#endif

#include "USTPFtdcUserApiStruct.h"
#include "thostsoDef.hpp"
#include "datafetcher.h"
#include "network.h"
//#include "MsgDef.hpp"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <pthread.h>
#include <set>
#include <signal.h>
#include <time.h>

extern "C"
{
	typedef void*(THREAD_PROC)(void*);
}

#define UDP_PORT_FOR_RCV_QUOTE   50003

enum FtdcMdApiState
{
	STATUS_QUOTE_DEFAULT=0,
	STATUS_QUOTE_INIT,
	STATUS_QUOTE_REQ_LOGIN,
	STATUS_QUOTE_REQ_SUBSCRIBE,
	STATUS_QUOTE_REQ_UNSUBSCRIBE,
	STATUS_QUOTE_RECVING
};



struct QuoteReqUserLoginToATF
{
	CUstpFtdcReqUserLoginField reqUserLogin;
	int nRequestID;
};

struct QuoteRspUserLoginFromATF
{
	CUstpFtdcRspUserLoginField rspUserLogin;
	CUstpFtdcRspInfoField rspInfo;
};


struct ReqSubMarketToATF
{
	TThostFtdcInstrumentIDType instr;
};

struct ReqUnsubMarketToATF
{
	TThostFtdcInstrumentIDType instr;
};


struct RspSubMarketFromATF
{
	CUstpFtdcSpecificInstrumentField specificInstrument;
	CUstpFtdcRspInfoField rspInfo;
	char QuoteSendPara[128];
};

struct RspUnsubMarketFromATF
{
	CUstpFtdcSpecificInstrumentField specificInstrument;
	CUstpFtdcRspInfoField rspInfo;
};

struct ResultOfSendQuote
{
	bool isSucc;
};


typedef struct
{
	MsgHead sthead;
	char aUserID[16];
}MdApiReqLoginPara;

struct SubscribeInstrument
{
	TThostFtdcInstrumentIDType instrument;
	//bool isValid;
	SubscribeInstrument *pNext;
};
class CUstpFtdcMduserSpi
{
public:
	CUstpFtdcMduserSpi();
	virtual ~CUstpFtdcMduserSpi();
	
	///���ͻ����뽻�׺�̨������ͨ������ʱ����δ��¼ǰ�����÷��������á�
	virtual void OnFrontConnected();
	
	///���ͻ����뽻�׺�̨ͨ�����ӶϿ�ʱ���÷��������á���������������API���Զ��������ӣ��ͻ��˿ɲ�������
	///@param nReason ����ԭ��
	///        0x1001 �����ʧ��
	///        0x1002 ����дʧ��
	///        0x2001 ����������ʱ
	///        0x2002 ��������ʧ��
	///        0x2003 �յ�������
	virtual void OnFrontDisconnected(int nReason);
		
	///������ʱ���档����ʱ��δ�յ�����ʱ���÷��������á�
	///@param nTimeLapse �����ϴν��ձ��ĵ�ʱ��
	virtual void OnHeartBeatWarning(int nTimeLapse);
	
	///���Ļص���ʼ֪ͨ����API�յ�һ�����ĺ����ȵ��ñ�������Ȼ���Ǹ�������Ļص�������Ǳ��Ļص�����֪ͨ��
	///@param nTopicID ������루��˽���������������������ȣ�
	///@param nSequenceNo �������
	virtual void OnPackageStart(int nTopicID, int nSequenceNo){};
	
	///���Ļص�����֪ͨ����API�յ�һ�����ĺ����ȵ��ñ��Ļص���ʼ֪ͨ��Ȼ���Ǹ�������Ļص��������ñ�������
	///@param nTopicID ������루��˽���������������������ȣ�
	///@param nSequenceNo �������
	virtual void OnPackageEnd(int nTopicID, int nSequenceNo){};

	///���ǰ��ϵͳ�û���¼Ӧ��
	virtual void OnRspUserLogin(CUstpFtdcRspUserLoginField *pRspUserLogin, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///�û��˳�Ӧ��
	virtual void OnRspUserLogout(CUstpFtdcRspUserLogoutField *pRspUserLogout, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///����Ӧ��
	virtual void OnRspError(CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;


	///��������Ӧ��
	virtual void OnRspSubscribeTopic(CUstpFtdcDisseminationField *pDissemination, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///�����ѯӦ��
	virtual void OnRspQryTopic(CUstpFtdcDisseminationField *pDissemination, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///�������֪ͨ
	virtual void OnRtnDepthMarketData(CUstpFtdcDepthMarketDataField *pDepthMarketData) ;

	///���ĺ�Լ�������Ϣ
	virtual void OnRspSubMarketData(CUstpFtdcSpecificInstrumentField *pSpecificInstrument, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///�˶���Լ�������Ϣ
	virtual void OnRspUnSubMarketData(CUstpFtdcSpecificInstrumentField *pSpecificInstrument, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;
};

class MDUSER_API_EXPORT CUstpFtdcMduserApi
{
public:	
	CUstpFtdcMduserApi();
	
	///����MduserApi
	///@param pszFlowPath ����������Ϣ�ļ���Ŀ¼��Ĭ��Ϊ��ǰĿ¼
	///@return ��������UserApi
	static CUstpFtdcMduserApi *CreateFtdcMduserApi(const char *pszFlowPath = "");
	
	///��ȡϵͳ�汾��
	///@param nMajorVersion ���汾��
	///@param nMinorVersion �Ӱ汾��
	///@return ϵͳ��ʶ�ַ���
	static const char *GetVersion(int &nMajorVersion, int &nMinorVersion);
	
	///ɾ���ӿڶ�����
	///@remark ����ʹ�ñ��ӿڶ���ʱ,���øú���ɾ���ӿڶ���
	void Release();
	
	///��ʼ��
	///@remark ��ʼ�����л���,ֻ�е��ú�,�ӿڲſ�ʼ����
	void Init();
	
	///�ȴ��ӿ��߳̽�������
	///@return �߳��˳�����
	int Join();
	
	///��ȡ��ǰ������
	///@retrun ��ȡ���Ľ�����
	///@remark ֻ�е�¼�ɹ���,���ܵõ���ȷ�Ľ�����
	const char *GetTradingDay();
	
	///ע��ǰ�û������ַ
	///@param pszFrontAddress��ǰ�û������ַ��
	///@remark �����ַ�ĸ�ʽΪ����protocol://ipaddress:port�����磺��tcp://127.0.0.1:17001���� 
	///@remark ��tcp��������Э�飬��127.0.0.1�������������ַ����17001������������˿ںš�
	void RegisterFront(char *pszFrontAddress);
	
	///ע�����ַ����������ַ
	///@param pszNsAddress�����ַ����������ַ��
	///@remark �����ַ�ĸ�ʽΪ����protocol://ipaddress:port�����磺��tcp://127.0.0.1:12001���� 
	///@remark ��tcp��������Э�飬��127.0.0.1�������������ַ����12001������������˿ںš�
	///@remark RegisterFront������RegisterNameServer
	virtual void RegisterNameServer(char *pszNsAddress){};
	
	///ע��ص��ӿ�
	///@param pSpi �����Իص��ӿ����ʵ��
	void RegisterSpi(CUstpFtdcMduserSpi *pSpi);
	
	///����֤��
	///@param pszCertFileName �û�֤���ļ���
	///@param pszKeyFileName �û�˽Կ�ļ���
	///@param pszCaFileName ������CA֤���ļ���
	///@param pszKeyFilePassword �û�˽Կ�ļ�����
	///@return 0 �����ɹ�
	///@return -1 ������CA֤������ʧ��
	///@return -2 �û�֤������ʧ��
	///@return -3 �û�˽Կ����ʧ��	
	///@return -4 �û�֤��У��ʧ��
	virtual int RegisterCertificateFile(const char *pszCertFileName, const char *pszKeyFileName, 
		const char *pszCaFileName, const char *pszKeyFilePassword);

	///�����г����顣
	///@param nTopicID �г���������  
	///@param nResumeType �г������ش���ʽ  
	///        USTP_TERT_RESTART:�ӱ������տ�ʼ�ش�
	///        USTP_TERT_RESUME:���ϴ��յ�������(�Ƕ���ȫ����Լʱ����֧������ģʽ)
	///        USTP_TERT_QUICK:�ȴ��͵�ǰ�������,�ٴ��͵�¼���г����������
	///@remark �÷���Ҫ��Init����ǰ���á����������򲻻��յ�˽���������ݡ�
	virtual void SubscribeMarketDataTopic(int nTopicID, USTP_TE_RESUME_TYPE nResumeType);

	///���ĺ�Լ���顣
	///@param ppInstrumentID ��ԼID  
	///@param nCount Ҫ����/�˶�����ĺ�Լ����
	///@remark 
	virtual int SubMarketData(char *ppInstrumentID[], int nCount);

	///�˶���Լ���顣
	///@param ppInstrumentID ��ԼID  
	///@param nCount Ҫ����/�˶�����ĺ�Լ����
	///@remark 
	virtual int UnSubMarketData(char *ppInstrumentID[], int nCount);		
	
	///����������ʱʱ�䡣
	///@param timeout ������ʱʱ��(��)  
	virtual void SetHeartbeatTimeout(unsigned int timeout);


	///���ǰ��ϵͳ�û���¼����
	virtual int ReqUserLogin(CUstpFtdcReqUserLoginField *pReqUserLogin, int nRequestID) ;

	///�û��˳�����
	virtual int ReqUserLogout(CUstpFtdcReqUserLogoutField *pReqUserLogout, int nRequestID);

	///������������
	virtual int ReqSubscribeTopic(CUstpFtdcDisseminationField *pDissemination, int nRequestID) ;

	///�����ѯ����
	virtual int ReqQryTopic(CUstpFtdcDisseminationField *pDissemination, int nRequestID);

	///���ĺ�Լ�������Ϣ
	virtual int ReqSubMarketData(CUstpFtdcSpecificInstrumentField *pSpecificInstrument, int nRequestID);

	///�˶���Լ�������Ϣ
	virtual int ReqUnSubMarketData(CUstpFtdcSpecificInstrumentField *pSpecificInstrument, int nRequestID);
	
	void SetState(FtdcMdApiState state);	
	int InsertNewInstr(CUstpFtdcSpecificInstrumentField *);	
	int RemoveInstr(CUstpFtdcSpecificInstrumentField *);	
	void PrintInstr();
	int ParseQuotePara(char *);
	bool IsQuoteReady();
	int HandleInit();
	int HandleLogin();
	int HandleUnsubmarket();
	int HandleSubmarket();
	int QuoteProc();	
	//static void *SendQuoteThreadFunc(void *);
	static void SendOneQuote(int sig, siginfo_t *si, void *uc);
	int CreateTimer();
	int HandleMsgFromATF();	
	void Reinit();
protected:
	~CUstpFtdcMduserApi();
private:
	friend void * ThostFtdcMdApiProc(void* para);
	//CUdpServer<CThostFtdcDepthMarketDataField>  m_UdpQuoteReceiver;
	LockFreeQueue<CUstpFtdcDepthMarketDataField> m_QuoteQueue;
		
private:
	const static int MAX_MDAPIS_NUM = 1;
	static CUstpFtdcMduserApi m_stMdApis[MAX_MDAPIS_NUM];
	static int m_iMdApis_used;

	int m_Inited;	
	bool m_bLogined;
	bool m_autoReconn;
	bool m_bQuoteReady;
	bool m_bQuoteOver;
	char m_iLocalFile[256];
	int m_iUseUdp;
	char m_quoteSvrIP[16];
	int m_shListenPort;
	int m_iRequestID;
	int m_fdConnQuoteSvr;
	
	CUstpFtdcMduserSpi * m_FtdcMdSpi_instance;

	FtdcMdApiState m_iState;
	QuoteReqUserLoginToATF  m_userLoginField;
	//SubscribeInfo m_subData;
	SubscribeInstrument *m_pInstrSubSucc;                 //�Ѿ��ɹ����ĵĺ�Լ
	TThostFtdcInstrumentIDType m_acInstrSubReq; 		  //��ǰ�����ĵĺ�Լ��δ�յ��ظ�
	TThostFtdcInstrumentIDType m_acInstrUnsubReq; 		  //��ǰ�����˶��ĺ�Լ����δ�յ��ظ�
	
	char m_acQuoteFile[64];
	int m_iQuoteInterval;
	//int m_iNumInstr;
	CDataFetcher m_quoteFetcher;
	timer_t m_timerid;
};

#endif
