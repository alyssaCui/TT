#ifndef __CLOG_H__
#define __CLOG_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "constants.h"

using namespace std;



struct LogConf
{
	LOG_LEVEL_TYPE fileLogLevel;		
	LOG_LEVEL_TYPE stdoutLogLevel;
	char logFile[MAX_FILE_PATH_LEN];
};


class CLog
{
public:
	CLog();
	virtual ~CLog();
	int open(char *logFilePath);		
	int init(LogConf conf);
	int close();
	int setPrintLogFilter(LOG_LEVEL_TYPE filterLogLevel);
	int setFileLogFilter(LOG_LEVEL_TYPE filterLogLevel);		
	void debug(const char* format, ...);
	void info(const char* format, ...);
	void warn(const char* format, ...);
	void error(const char* format, ...);
	void fatal(const char* format, ...);

private:	
	int writeLog(LOG_LEVEL_TYPE logLevel, const char* logContent, const char* logTail, int timeFormat=1, int logLevelFlag=1);
	
	LOG_LEVEL_TYPE m_filterLogLevel;
	LOG_LEVEL_TYPE m_filterPrintLevel;
		
	int logFlag;
	FILE *m_fpLog;
	int logFlag2;
};



#endif

