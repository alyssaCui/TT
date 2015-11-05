
/*************************************
*
* Global constants
*
**************************************/
#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__


#define ATF_SUCC     0
#define ATF_FAIL    -1


enum LOG_LEVEL_TYPE
{
	LOG_LEVEL_NULL = 0,
	LOG_LEVEL_DEBUG = 1,
	LOG_LEVEL_INFO = 2,
	LOG_LEVEL_WARN = 3,
	LOG_LEVEL_ERROR = 4,
	LOG_LEVEL_FATAL = 5,
};

#define MAX_FILE_PATH_LEN       256
#define MAX_CMD_LINE_LEN        256
#define MAX_CMD_VAR_NAME_LEN    64
#define SHARED_MEM_SIZE         2*1024*1024
#define MAX_QUOTE_SVR_NUM       5
#define MAX_TIME_STATISTICS_NUM 200000
#define LEN_ExchangeID          9
#define INITIAL_VALUE           1
#define MAX_VAR_NAME_LEN        32

#define ATFA_PORT               50004

#define  PerfTest  0

#define SEQTIME_COUNT 80*1024


#define IS_DEBUG        1
#define printf_red(x,...) printf("\e[31m");printf(x,## __VA_ARGS__);printf("\033[0m");
#define printf_green(x,...) printf("\e[32m");printf(x,## __VA_ARGS__);printf("\033[0m");
#define printf_yellow(x,...) printf("\e[33m");printf(x,## __VA_ARGS__);printf("\033[0m");
#define printf_blue(x,...) printf("\e[34m");printf(x,## __VA_ARGS__);printf("\033[0m");
#endif

