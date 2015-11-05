#include "log.h"

#define MAX_LOG_LEN          8192
#define MIN_TIME_UNIT_MCS    1        //Î¢Ãë
#define ON                   1

//char g_progDir[MAX_FILE_PATH_LEN] = {0};

char* LOG_LEVEL_NAME[] = {"Null", "Debug", "Info", "Warn", "Error","Fatal"};

CLog::CLog()
{
	m_fpLog = NULL;
	m_filterLogLevel = LOG_LEVEL_INFO;
	m_filterPrintLevel = LOG_LEVEL_INFO;
}

CLog::~CLog()
{
	if (m_fpLog != NULL)
	{
		this->close();
	}
}

int CLog::init(LogConf conf)
{
	char logPath[MAX_FILE_PATH_LEN] = {0};
	//char logFile[MAX_FILE_PATH_LEN] = {0};
    int ret = 0;
	char *ptr1 = NULL;

	strcpy(logPath,conf.logFile);
	ptr1 = strrchr(logPath, '/');
	ptr1[0] = '\0';

	
    //get the directory for log files
	//sprintf(logPath, "%s/log", g_progDir);	
	ret = access(logPath, F_OK);
	if (ret != 0)
	{
		ret = mkdir(logPath, 0777);
		if (ret != 0)
		{
			printf("Failed to create log dir %s. Program exit.\n", logPath);
			return ATF_FAIL;
		}
	}
	else
	{
		ret = access(logPath, R_OK | W_OK | X_OK);
		if (ret != 0)
		{
			printf("Fail to access log dir %s. Program exit.\n", logPath);
			return ATF_FAIL;
		}
	}

    //sprintf(logFile, "%s/atf.log", conf.logDir);
	//printf("Log file: %s\n",logFile);
	
    open(conf.logFile);
    
    setFileLogFilter(conf.fileLogLevel);
    setPrintLogFilter(conf.stdoutLogLevel);	

	return ATF_SUCC;
}

int CLog::open(char *logFilePath)
{
	LOG_LEVEL_TYPE filterLogLevel;
	
	this->close();
	m_fpLog = fopen(logFilePath, "a");
	if (NULL == m_fpLog)
	{
		printf("Fail to create log file %s\n", logFilePath);
		return -1;
	}

	filterLogLevel = m_filterLogLevel;
	m_filterLogLevel = LOG_LEVEL_DEBUG;
	this->debug("Log file start. \n");
	m_filterLogLevel = filterLogLevel;	

	return 0;
	
}

int CLog::close()
{
	LOG_LEVEL_TYPE filterLogLevel;
	
	if (m_fpLog != NULL)
	{
		filterLogLevel = m_filterLogLevel;
		m_filterLogLevel = LOG_LEVEL_DEBUG;
		this->debug("Log file close. \n");
		m_filterLogLevel = filterLogLevel;	

		
		fclose(m_fpLog);
		m_fpLog = NULL;
	}
	
	return 0;
}

int CLog::setPrintLogFilter(LOG_LEVEL_TYPE filterLogLevel)
{
	m_filterPrintLevel = filterLogLevel;
	return 0;
}

int CLog::setFileLogFilter(LOG_LEVEL_TYPE filterLogLevel)
{
	m_filterLogLevel = filterLogLevel;
	return 0;
}

int CLog::writeLog(LOG_LEVEL_TYPE logLevel, const char* logContent, const char* logTail, int timeFormat, int logLevelFlag)
{
	if (logContent == NULL || logTail == NULL)
	{
		return -1;
	}

	//get time
	struct timeval t_cur;
	struct tm *t, tbuf;
	
	gettimeofday(&t_cur, (struct timezone *)0);
	t = localtime_r(&(t_cur.tv_sec), &tbuf);

	char timeStr[100] = {0};
	char userStr[100] = {0};
	char logLevelStr[100] = {0};

	if (MIN_TIME_UNIT_MCS == timeFormat)
	{
	    sprintf(timeStr, "%02d%02d_%02d:%02d:%02d.%06d",
			t->tm_mon+1,t->tm_mday,
			t->tm_hour, t->tm_min, t->tm_sec,
			(unsigned int)(t_cur.tv_usec));
	}
	else
	{
	    sprintf(timeStr, "%02d%02d_%02d:%02d:%02d",
			t->tm_mon+1,t->tm_mday,
			t->tm_hour, t->tm_min, t->tm_sec);
	}
	
	
	{
		strcpy(userStr, "");
	}

	
	if (ON == logLevelFlag)
	{
		sprintf(logLevelStr, "[%s] ", LOG_LEVEL_NAME[logLevel]);
	}
	else
	{
		strcpy(logLevelStr, "");
	}
	
	if (logLevel >= m_filterPrintLevel)
	{
		printf("%s%s %s%s%s", userStr, timeStr, logLevelStr, logContent, logTail);
	}

	if ((logLevel >= m_filterLogLevel) && (m_fpLog != NULL))
	{
		fprintf(m_fpLog, "%s%s %s %s, %s%s", userStr, timeStr, logLevelStr, __FILE__, logContent, logTail);
		fflush(m_fpLog);
	}

	return 0;
}


void CLog::debug(const char* format, ...)
{
	if (m_filterLogLevel > LOG_LEVEL_DEBUG 	&& m_filterPrintLevel > LOG_LEVEL_DEBUG)
	{
		return;
	}

	va_list arglist; 
	char logContent[MAX_LOG_LEN] = {0};

	va_start(arglist, format); 
	vsnprintf(logContent, sizeof(logContent)-1, format, arglist);
	va_end(arglist);

	writeLog(LOG_LEVEL_DEBUG, logContent, "");
}

void CLog::info(const char* format, ...)
{
	if (m_filterLogLevel > LOG_LEVEL_INFO && m_filterPrintLevel > LOG_LEVEL_INFO)
	{
		return;
	}

    va_list arglist; 
    char logContent[MAX_LOG_LEN] = {0};
    
    va_start(arglist, format); 
    vsprintf(logContent, format, arglist);
    va_end(arglist);

	writeLog(LOG_LEVEL_INFO, logContent, "");
}


void CLog::warn(const char* format, ...)
{
	if (m_filterLogLevel > LOG_LEVEL_WARN && m_filterPrintLevel > LOG_LEVEL_WARN)
	{
		return;
	}

    va_list arglist; 
    char logContent[MAX_LOG_LEN] = {0};
    
    va_start(arglist, format); 
    vsprintf(logContent, format, arglist);
    va_end(arglist);

	writeLog(LOG_LEVEL_WARN, logContent, "");
}

void CLog::error(const char* format, ...)
{
	if (m_filterLogLevel > LOG_LEVEL_ERROR && m_filterPrintLevel > LOG_LEVEL_ERROR)
	{
		return;
	}

    va_list arglist; 
    char logContent[MAX_LOG_LEN] = {0};
    
    va_start(arglist, format); 
    vsprintf(logContent, format, arglist);
    va_end(arglist);

	writeLog(LOG_LEVEL_ERROR, logContent, "");
}

void CLog::fatal(const char* format, ...)
{
	if (m_filterLogLevel > LOG_LEVEL_FATAL && m_filterPrintLevel > LOG_LEVEL_FATAL)
	{
		return;
	}
	
    va_list arglist; 
    char logContent[MAX_LOG_LEN] = {0};
    
    va_start(arglist, format); 
    vsprintf(logContent, format, arglist);
    va_end(arglist);

	writeLog(LOG_LEVEL_FATAL, logContent, "");
}


