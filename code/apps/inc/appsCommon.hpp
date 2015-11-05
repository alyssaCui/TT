#ifndef __APPS_COMMON_H__
#define __APPS_COMMON_H__

#define TIMEOUT_SEC 3 

enum OPERATE_DONE_TYPE
{
	OPERATE_DONE = 0,
	OPERATE_NOT_DONE
};

enum TASK_STATUS
{
	NO_TASK = 0,
	DOING_TASK
};

enum CONNECT_STATUS
{
	CONNECTED = 0,
	WAIT_CONNECTED,
	DISCONNECTED
};

enum LOGIN_STATUS
{
	LOGINED = 0,
	NOT_LOGIN
};

enum PROGRESS_STATUS
{
	TODO = 0,
	DONE
};

enum OPERATE_RESULT
{
	OPERATE_SUCC = 0,
	OPERATE_FAIL,
	OPERATE_WAIT,
	OPERATE_DOING
};

enum MSG_DIRECTION
{
	RCV = 0,
	SEND,
	LOCAL	
};

struct OP
{
	MSG_DIRECTION direction;
	MSG_ID msgID;
	OPERATE_RESULT result;
	int msgSize;
	char errReason[128];
	char * pMsg;
};

typedef std::vector<OP> OPERATE_SET;

struct UserInfoLogined
{
	 char UserID[16];
	 char BrokerID[11];  //经纪公司代码
	 int  ErrorID;
};

struct SleepTime
{
	int time;
};

struct AgentConf
{
	char agentIP[128];
	int agentPort;
};

#endif
