#ifndef __THOSTSO_DEF_H__
#define __THOSTSO_DEF_H__

#include <stdio.h>
#include <stdlib.h>
#include <set>
#include <string>

#define QUOTE_SERVER_IP "127.0.0.1"
#define QUOTE_SERVER_MSGCONN_PORT 12345      //quote_serverµÄ¶Ë¿ÚÒÔ70xx
#define MAX_NUM_QUOTE          77768

#define DEBUG_TRADERSO(x,...) printf("\e[31m");printf("[DEBUG_TRADER_SO][FUNC:%s] ",__FUNCTION__);printf(x,## __VA_ARGS__);printf("\033[0m");
#define ERROR_TRADERSO(x,...) printf("\e[31m");printf("[ERROR_TRADERSO][FUNC:%s] ",__FUNCTION__);printf(x,## __VA_ARGS__);printf("\033[0m");

#define DEBUG_QUOTESO(x,...) printf("\e[32m");printf("[DEBUG_QUOTESO][FUNC:%s] ",__FUNCTION__);printf(x,## __VA_ARGS__);printf("\033[0m");
#define ERROR_QUOTESO(x,...) printf("\e[32m");printf("[ERROR_QUOTESO][FUNC:%s] ",__FUNCTION__);printf(x,## __VA_ARGS__);printf("\033[0m");

#define DEBUG_ATFA(x,...) printf("\e[33m");printf("[DEBUG_ATFA][FUNC:%s] ",__FUNCTION__);printf(x,## __VA_ARGS__);printf("\033[0m");
#define ERROR_ATFA(x,...) printf("\e[33m");printf("[ERROR_ATFA][FUNC:%s] ",__FUNCTION__);printf(x,## __VA_ARGS__);printf("\033[0m");

#define DEBUG_RED(x,...) printf("\e[31m");printf(x,## __VA_ARGS__);printf("\033[0m");
#define DEBUG_GREEN(x,...) printf("\e[32m");printf(x,## __VA_ARGS__);printf("\033[0m");
#define DEBUG_YELLOW(x,...) printf("\e[33m");printf("[DEBUG_ATFA][FUNC:%s] ",__FUNCTION__);printf(x,## __VA_ARGS__);printf("\033[0m");
#define DEBUG_BLUE(x,...) printf("\e[34m");printf("[DEBUG_ATFA][FUNC:%s] ",__FUNCTION__);printf(x,## __VA_ARGS__);printf("\033[0m");

enum MSG_ID
{
	MSGID_C2S_QUOTE_REQ_CONN = 100,
	MSGID_S2C_QUOTE_RSP_CONN,            
	MSGID_C2S_QUOTE_REQ_LOGIN,          
	MSGID_S2C_QUOTE_RSP_LOGIN,          
	MSGID_C2S_QUOTE_REQ_SUBSCRIBE,       
	MSGID_S2C_QUOTE_RSP_SUBSCRIBE,  
	MSGID_C2S_QUOTE_REQ_UNSUBSCRIBE,		 
	MSGID_S2C_QUOTE_RSP_UNSUBSCRIBE,	
	MSGID_C2S_QUOTE_RESULT_SEND_QUOTE,
	MSGID_S2C_QUOTE_DISCONNECT,	
	
	MSGID_C2S_TRADE_REQ_CONN = 110,           
	MSGID_S2C_TRADE_RSP_CONN,          
	MSGID_C2S_TRADE_REQ_LOGIN,         
	MSGID_S2C_TRADE_RSP_LOGIN,  
	MSGID_C2S_TRADE_REQ_POSITION,		   
	MSGID_S2C_TRADE_RSP_POSITION,		  
	MSGID_C2S_TRADE_ReqOrderInsert,  
	MSGID_S2C_TRADE_OnRtnOrder,      
	MSGID_S2C_TRADE_OnRtnTrade,        
	MSGID_S2C_TRADE_OnRspOrderInsert,  
	MSGID_S2C_TRADE_OnErrRtnOrderInsert,
	MSGID_C2S_TRADE_ReqOrderAction,  
	MSGID_S2C_TRADE_OnRspOrderAction,	
	MSGID_S2C_TRADE_OnErrRtnOrderAction,
	MSGID_S2C_TRADE_DISCONNECT, 
		
	MSGID_LOCAL_TRADE_RESULT,
	MSGID_LOCAL_HANDLE_ORDER_ACTION,
	MSGID_LOCAL_SLEEP,

	MSGID_S2C_REQ_TRADE_PROCESS_STATIC_TIME,
	MSGID_C2S_RSP_TRADE_PROCESS_STATIC_TIME,

	MSGID_S2C_REQ_QUOTE_INTERVAL_STATIC_TIME,
	MSGID_C2S_RSP_QUOTE_INTERVAL_STATIC_TIME,

	MSGID_S2C_REDDY_FOR_PERFORM_TEST
};


struct tvTime_t
{
	struct timeval tv;
	struct tvTime_t *pNext;
};

#pragma pack(1)

typedef struct
{
	int iMsgID;
	int iMsgBodyLen;
}MsgHead;

struct RspInfoFromATF
{
	int ErrorID;
	char ErrorMsg[81];
};

struct StaticQuoteInterval_t
{
	long long quoteNum;	
	long long maxIdx;
	int  avgQuoteInterval;
	int  minQuoteInterval;
	int  maxQuoteInterval;
};

#pragma pack()







#endif
