#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <sys/stat.h>
#include<signal.h>

#include "common.h"
#include "task.hpp"

//for test code
//#include "appObjects.hpp"


#define VERSION_ATEST "atest_beta001001"


enum DIR_TYPE
{
	RELATIVE_DIR = 0,
	ABSOLUTE_DIR = 1,
};


struct Conf
{
	LogConf g_logConf;
	TaskConf g_taskConf;
	char g_progDir[MAX_FILE_PATH_LEN];
};


struct timeval g_ProgramStartTime;
struct timeval g_ProgramStartHour;

Conf g_conf;
CLog g_log;
CTask g_task;
int g_timeout_case = 60;



/**
* This function is used to print application usage information
*/
void usage()
{
    printf("Description:\n\tatest.\n");
    printf("Version: %s\n", VERSION_ATEST);
    printf("Usage: \n"); 
}

int catch_sigint(int sig)
{
	exit(0);
}

int catch_hup(int sig)
{
	printf("\n.Recv hup sig %d.\n", sig);
	printf("->");
	fflush(stdout);
	return -1;
}

int GetProgRootDir()
{
	int  ret = 0;
	char *ptr1 = NULL;
	
    //get the absolute path of the process
	ret = readlink ("/proc/self/exe", g_conf.g_progDir, sizeof(g_conf.g_progDir)-1);
	if (ret < 0)
	{
		printf("Failed to get the absolute path of TT.\n");
		return ATF_FAIL;
	}
	
	//get the directory of the process
	ptr1 = strrchr(g_conf.g_progDir, '/');
	if (NULL == ptr1 || ptr1 == g_conf.g_progDir)
	{
		printf("Failed to get the directory of TT.");
		return ATF_FAIL;
	}
	ptr1[0] = '\0';
	ptr1 = strrchr(g_conf.g_progDir, '/');
	ptr1[0] = '\0';
	
	strcpy(g_conf.g_taskConf.progDir,g_conf.g_progDir);

	return ATF_SUCC;
}

void InitConfig()
{
	memset(g_conf.g_progDir,0,sizeof(g_conf.g_progDir));

	memset(g_conf.g_taskConf.m_arrTestcasesPath,0,sizeof(g_conf.g_taskConf.m_arrTestcasesPath));
	memset(g_conf.g_taskConf.m_arrTestDataPath,0,sizeof(g_conf.g_taskConf.m_arrTestDataPath));
	memset(g_conf.g_taskConf.progDir,0,sizeof(g_conf.g_taskConf.progDir));
	
	g_conf.g_taskConf.g_quoteConf.isValid = INVALID;
	g_conf.g_taskConf.g_quoteConf.quoteSvrFrontID = 0;
	g_conf.g_taskConf.g_quoteConf.quoteSvrPort = 0;
	memset(g_conf.g_taskConf.g_quoteConf.quoteSvrIP,0,sizeof(g_conf.g_taskConf.g_quoteConf.quoteSvrIP));

	g_conf.g_taskConf.g_exchgConf.isValid = INVALID;
	g_conf.g_taskConf.g_exchgConf.exchgSvrFrontID = 0;
	g_conf.g_taskConf.g_exchgConf.exchgSvrPort = 0;
	memset(g_conf.g_taskConf.g_exchgConf.exchgSvrIP,0,sizeof(g_conf.g_taskConf.g_exchgConf.exchgSvrIP));
	memset(g_conf.g_taskConf.g_exchgConf.exchangeID,0,sizeof(g_conf.g_taskConf.g_exchgConf.exchangeID));

	g_conf.g_logConf.fileLogLevel = LOG_LEVEL_NULL;
	g_conf.g_logConf.stdoutLogLevel = LOG_LEVEL_NULL;
	memset(g_conf.g_logConf.logFile,0,sizeof(g_conf.g_logConf.logFile));
}


int LoadConfig()
{
	char configFilePath[MAX_FILE_PATH_LEN] = {0};	
	FILE *fp;	
	char srcLine[MAX_CMD_LINE_LEN] = {0};
	char refineLine[MAX_CMD_LINE_LEN] = {0};	
	char transLine[MAX_CMD_LINE_LEN] = {0};		
	char curSectionName[MAX_CMD_VAR_NAME_LEN] = {0};
	unsigned int newLineNo = 0;	
	unsigned int len = 0;
	unsigned int ptr2_len = 0;
	char* ptr = NULL;	
	char *pTag = NULL;
	char *pValue = NULL; 
	char *pComment = NULL;
	int dirType = 0;
	
	sprintf(configFilePath, "%s/conf/tt.conf", g_conf.g_progDir);
	
	//open the configure file
	fp = fopen(configFilePath, "r");	
	if (NULL == fp)
	{
		printf("[error] Fail to open job file %s\n", configFilePath);
		return ATF_FAIL;
	}

	//loop to read each line of configure file
	while (1)
	{	
		newLineNo++;
		ptr = fgets(srcLine, sizeof(srcLine)-1, fp);
		if (ptr == NULL)
		{
			break;
		}
	
		strcpy(refineLine, srcLine);
			
		//drop the <cr> <lf> char
		len = strlen(refineLine);
		while ((len > 0) && (refineLine[len-1]== '\n' || refineLine[len-1]== '\r'))
		{
			refineLine[len-1] = '\0';
			len--;
		}
			
		//drop white char
		pTag = trim(refineLine);
		len = strlen(pTag);
	
		//if empty line, skip it	
		if (len <= 0)
		{
			continue;
		}
	
		//if rem line, skip it
		if (*pTag == '#')
		{
			continue;
		}
		
		//if new section start
		if (*pTag == '[')
		{
			//get the new section name
			pValue = strchr(pTag, ']');
			if (NULL == pValue)
			{
				printf("[error] Invalid configure line1. %s\n", refineLine);
				fclose(fp);
				return -1;
			}

			memset(curSectionName, 0, sizeof(curSectionName));
			strncpy(curSectionName, pTag+1, pValue-pTag-1);

			//printf("curSectionName=%s\n",curSectionName);
			if (strlen(curSectionName) <= 0)
			{
				printf("[error] Invalid configure line2. %s\n", refineLine);
				fclose(fp);
				return -1;
			}

			continue;			
		}
	
		pValue = strchr(pTag, '=');
		if (pValue == NULL)
		{
			printf("[error] Invalid configure line3. %s\n", refineLine);
			printf("[error] >>Tag:%s\n", pTag);
			printf("[error] >>Value:%s\n", pValue);
				
			fclose(fp);
			return -1;
		}
	
		*pValue = '\0';
		pTag = rtrim(pTag);
		pValue = trim(pValue+1);

		//printf("pTag = %s\n",pTag);
	
		//drop comment info
		pComment = strchr(pValue, '#');  //need add
		if (pComment != NULL)
		{
			*pComment = '\0';			
		}
		rtrim(pValue);
	
		//drop quota char
		if (pValue[0] == '"')
		{
		    memset(transLine,0,sizeof(transLine));
			transStr(pValue, transLine, strlen(pValue));
			pValue = transLine;
		}
	
		ptr2_len = strlen(pValue);
		if (pValue[0] == '"' && pValue[ptr2_len-1] == '"')
		{
			pValue[ptr2_len-1] = '\0';
			pValue = pValue + 1;			
		}	
		//printf("pValue = %s\n",pValue);
		
		if (strcasecmp(curSectionName, "TestCase") == 0)
		{
			if (strcasecmp(pTag, "dirType") == 0)
			{
        		if (strlen(pValue) <= 0)
        		{
        			printf("[error] Invalid dirType !\n");
        			fclose(fp);
        			return -1;					
        		}
				
        		if (strcasecmp(pValue, "relative") == 0)
					dirType = RELATIVE_DIR;
				else if(0 ==strcasecmp(pValue, "absolute"))
					dirType = ABSOLUTE_DIR;
			}	
        	else if (strcasecmp(pTag, "testCasesDir") == 0)
        	{
        		memset(g_conf.g_taskConf.m_arrTestcasesPath,0,sizeof(g_conf.g_taskConf.m_arrTestcasesPath));
        		if (strlen(pValue) <= 0)
        		{
        			printf("[error] Invalid testcasesDir !");
        			fclose(fp);
        			return -1;					
        		}

			    if(ABSOLUTE_DIR==dirType)
        			sprintf(g_conf.g_taskConf.m_arrTestcasesPath, "%s", pValue);
				else if(RELATIVE_DIR==dirType)
			    {
					if (pValue[0] == '/')
						sprintf(g_conf.g_taskConf.m_arrTestcasesPath, "%s%s", g_conf.g_progDir, pValue);
					else
						sprintf(g_conf.g_taskConf.m_arrTestcasesPath, "%s/%s", g_conf.g_progDir, pValue);
			    }
        		printf("testcases directory: %s\n", g_conf.g_taskConf.m_arrTestcasesPath);
        	}
			else if(strcasecmp(pTag, "timeout") == 0)
			{
        		if (strlen(pValue) <= 0)
        		{
        			printf("[error] Invalid timeout !");
        			fclose(fp);
        			return -1;					
        		}

				g_timeout_case = atoi(pValue);
			}
			else
			{
				g_log.error("Undefined tag:%s",pTag);
			}
		}
		else if (strcasecmp(curSectionName, "TestData") == 0)
		{
			if (strcasecmp(pTag, "dirType") == 0)
			{
				if (strlen(pValue) <= 0)
				{
					printf("[error] Invalid dirType !\n");
					fclose(fp);
					return -1;					
				}
					
				if (strcasecmp(pValue, "relative") == 0)
					dirType = RELATIVE_DIR;
				else if(0 ==strcasecmp(pValue, "absolute"))
					dirType = ABSOLUTE_DIR;
			}	
				
			if (strcasecmp(pTag, "testDataDir") == 0)
			{
				memset(g_conf.g_taskConf.m_arrTestDataPath,0,sizeof(g_conf.g_taskConf.m_arrTestDataPath));
				if (strlen(pValue) <= 0)
				{
					printf("[error] Invalid test data path !");
					fclose(fp);
					return -1;					
				}
	
				if(ABSOLUTE_DIR == dirType)
					sprintf(g_conf.g_taskConf.m_arrTestDataPath, "%s", pValue);
				else if(RELATIVE_DIR == dirType)
				{
					if (pValue[0] == '/')
						sprintf(g_conf.g_taskConf.m_arrTestDataPath, "%s%s", g_conf.g_progDir, pValue);
					else
						sprintf(g_conf.g_taskConf.m_arrTestDataPath, "%s/%s", g_conf.g_progDir, pValue);
				}
				//printf("testData directory: %s\n",g_conf.g_taskConf.m_arrTestDataPath);
			}
			
		}
		else if (strcasecmp(curSectionName, "QuoteSvr") == 0)
		{
			if (strcasecmp(pTag, "ip") == 0)
			{
				if (strlen(pValue) <= 0)
				{
					printf("[error] Invalid QuoteSvr ip !\n");
					fclose(fp);
					return -1;					
				}
	
				memset(g_conf.g_taskConf.g_quoteConf.quoteSvrIP,0,sizeof(g_conf.g_taskConf.g_quoteConf.quoteSvrIP));
				strcpy(g_conf.g_taskConf.g_quoteConf.quoteSvrIP,pValue);					
				//printf("=====Quote Svr IP: %s\n", g_conf.g_taskConf.g_quoteConf.quoteSvrIP);
			}	
			else if (strcasecmp(pTag, "port") == 0)
			{
				if (strlen(pValue) <= 0)
				{
					printf("[error] Invalid quote Svr port !\n");
					fclose(fp);
					return -1;					
				}
		
				g_conf.g_taskConf.g_quoteConf.quoteSvrPort = atoi(pValue);
				//printf("=====Quote Svr Port: %u\n", g_conf.g_taskConf.g_quoteConf.quoteSvrPort);
			
			}	
			else if (strcasecmp(pTag, "FrontID") == 0)
			{
				if (strlen(pValue) <= 0)
				{
					printf("[error] Invalid quote Svr FrontID !\n");
					fclose(fp);
					return -1;					
				}
			
				g_conf.g_taskConf.g_quoteConf.quoteSvrFrontID = atoi(pValue);
			
			}
			else
			{
				printf("[error]Undefined tag: %s!\n",pTag);
			}
			
			g_conf.g_taskConf.g_quoteConf.isValid = READABLE;
		}			
		else if (strcasecmp(curSectionName, "Exchange") == 0)
		{
			if (strcasecmp(pTag, "ip") == 0)
			{
				if (strlen(pValue) <= 0)
				{
					printf("[error] Invalid Exchange ip !\n");
					fclose(fp);
					return -1;					
				}
		
				memset(g_conf.g_taskConf.g_exchgConf.exchgSvrIP,0,sizeof(g_conf.g_taskConf.g_exchgConf.exchgSvrIP));
				strcpy(g_conf.g_taskConf.g_exchgConf.exchgSvrIP,pValue);					
				//printf("=====Exchange Svr IP: %s\n", g_conf.g_taskConf.g_exchgConf.exchgSvrIP);
			}		
			else if (strcasecmp(pTag, "port") == 0)
			{
				if (strlen(pValue) <= 0)
				{
					printf("[error] Invalid Exchange Svr port !\n");
					fclose(fp);
					return -1;					
				}
		
				g_conf.g_taskConf.g_exchgConf.exchgSvrPort = atoi(pValue);
				//printf("=====Exchange Svr Port: %u\n", g_conf.g_taskConf.g_exchgConf.exchgSvrPort);
			}			
			else if (strcasecmp(pTag, "FrontID") == 0)
			{
				if (strlen(pValue) <= 0)
				{
					printf("[error] Invalid Exchange Svr FrontID !\n");
					fclose(fp);
					return -1;					
				}
			
				g_conf.g_taskConf.g_exchgConf.exchgSvrFrontID = atoi(pValue);
			}
			else if (strcasecmp(pTag, "ExchangeID") == 0)
			{
				if (strlen(pValue) <= 0)
				{
					printf("[error] Invalid ExchangeID !\n");
					fclose(fp);
					return -1;					
				}
			
				memset(g_conf.g_taskConf.g_exchgConf.exchangeID,0,sizeof(g_conf.g_taskConf.g_exchgConf.exchangeID));
				strcpy(g_conf.g_taskConf.g_exchgConf.exchangeID,pValue);					
			}
			else
			{
				printf("[error]Undefined tag: %s!\n",pTag);
			}

			g_conf.g_taskConf.g_exchgConf.isValid = READABLE;
		}
		else if (strcasecmp(curSectionName, "Log") == 0)
		{
			if (strcasecmp(pTag, "FileLogLevel") == 0)
			{
				if (strlen(pValue) <= 0)
				{
					printf("[warn] FileLogLevel is not set,default info !\n");
					g_conf.g_logConf.fileLogLevel = LOG_LEVEL_INFO;
				}
				else
				{
					if (strcasecmp(pValue, "debug") == 0)
						g_conf.g_logConf.fileLogLevel = LOG_LEVEL_DEBUG;
					else if(strcasecmp(pValue, "info")== 0)
						g_conf.g_logConf.fileLogLevel = LOG_LEVEL_INFO;
					else if(strcasecmp(pValue, "warn")== 0)
						g_conf.g_logConf.fileLogLevel = LOG_LEVEL_WARN;
					else if(strcasecmp(pValue, "error")== 0)
						g_conf.g_logConf.fileLogLevel = LOG_LEVEL_ERROR;
					else if(strcasecmp(pValue, "fatal")== 0)
						g_conf.g_logConf.fileLogLevel = LOG_LEVEL_FATAL;
					else
						printf("[error] Invalid fileLogLevel:%s !\n",pValue);
				}
			}
				
			if (strcasecmp(pTag, "stdoutLogLevel") == 0)
			{
				if (strlen(pValue) <= 0)
				{
					printf("[warn] stdoutLogLevel is not set,default info !\n");
					g_conf.g_logConf.stdoutLogLevel = LOG_LEVEL_INFO;
				}
				else
				{
					if (strcasecmp(pValue, "debug") == 0)
						g_conf.g_logConf.stdoutLogLevel = LOG_LEVEL_DEBUG;
					else if(strcasecmp(pValue, "info")== 0)
						g_conf.g_logConf.stdoutLogLevel = LOG_LEVEL_INFO;
					else if(strcasecmp(pValue, "warn")== 0)
						g_conf.g_logConf.stdoutLogLevel = LOG_LEVEL_WARN;
					else if(strcasecmp(pValue, "error")== 0)
						g_conf.g_logConf.stdoutLogLevel = LOG_LEVEL_ERROR;
					else if(strcasecmp(pValue, "fatal")== 0)
						g_conf.g_logConf.stdoutLogLevel = LOG_LEVEL_FATAL;
					else
						printf("[error] Invalid stdoutLogLevel:%s !\n",pValue);
				}
			}

			if (strcasecmp(pTag, "logFileName") == 0)
			{
				if (strlen(pValue) <= 0)
				{
					printf("[warn] logFileName is not set,default atf.log!\n");
					
					sprintf(g_conf.g_logConf.logFile, "%s/log/atf.log", g_conf.g_progDir);	
				}
				
				sprintf(g_conf.g_logConf.logFile, "%s/log/%s", g_conf.g_progDir,pValue);	
			}
		}
		else if (strcasecmp(curSectionName, "Agent") == 0)
		{
			if (strcasecmp(pTag, "ip") == 0)
			{
				if (strlen(pValue) <= 0)
				{
					printf("[error]Invalid agent ip !\n");
					fclose(fp);
					return -1;					
				}
	
				memset(g_conf.g_taskConf.g_agentConf.agentIP,0,sizeof(g_conf.g_taskConf.g_agentConf.agentIP));
				strcpy(g_conf.g_taskConf.g_agentConf.agentIP,pValue);					
				//printf("=====Quote Svr IP: %s\n", g_conf.g_taskConf.g_quoteConf.quoteSvrIP);
			}	
			else if (strcasecmp(pTag, "port") == 0)
			{
				if (strlen(pValue) <= 0)
				{
					printf("[error] Invalid agent port !\n");
					fclose(fp);
					return -1;					
				}
		
				g_conf.g_taskConf.g_agentConf.agentPort = atoi(pValue);
				//printf("=====Quote Svr Port: %u\n", g_conf.g_taskConf.g_quoteConf.quoteSvrPort);
			
			}
			else
			{
				printf("[error]Undefined tag: %s!\n",pTag);
			}
		}
		else if (strcasecmp(curSectionName, "Gtest") == 0)
		{
			if (strcasecmp(pTag, "command_arg") == 0)
			{
				if (strlen(pValue) <= 0)
				{
					printf("[error]Invalid command_arg !\n");
					fclose(fp);
					return -1;					
				}
				
				int &argc = g_conf.g_taskConf.gtestConf.argc;
				argc++;
				
				strcpy(g_conf.g_taskConf.gtestConf.argv[argc],pValue);	
				//printf("argc=%d\n",argc);
				//printf("argv[%d]=%s\n",argc,g_conf.g_taskConf.gtestConf.argv[argc]);
			}
			else
			{
				printf("[error]Undefined tag: %s!\n",pTag);
			}
		}
		else
		{
			printf("[error]Undefined SectionName: %s!\n",curSectionName);
		}
	}


	fclose(fp);
	
	return ATF_SUCC;
}

/*
Function backupLastTask() backup log files,execute files,configure files,testcases
*/
int BackupLastTask()
{
	char curTime[32] = {0};
	char dateStr[32] = {0};
	char timeStr[32] = {0};
	char oldPath[MAX_FILE_PATH_LEN] = {0};
	char newPath[MAX_FILE_PATH_LEN] = {0};
	char tmpPath[MAX_FILE_PATH_LEN] = {0};
	char cmdStr[MAX_FILE_PATH_LEN] = {0};
	int  ret = 0;
    struct stat buf;
	bool logDirNoExist = false;


	sprintf(oldPath, "%s/log/last5",g_conf.g_progDir);
	remove(oldPath);	

	sprintf(oldPath, "%s/log/last4",g_conf.g_progDir);
	sprintf(newPath, "%s/log/last5", g_conf.g_progDir);
	rename(oldPath, newPath);
	
	sprintf(oldPath, "%s/log/last3", g_conf.g_progDir);
	sprintf(newPath, "%s/log/last4", g_conf.g_progDir);
	rename(oldPath, newPath);

	sprintf(oldPath, "%s/log/last2", g_conf.g_progDir);
	sprintf(newPath, "%s/log/last3", g_conf.g_progDir);
	rename(oldPath, newPath);
	
	memset(oldPath, 0, sizeof(oldPath));
	sprintf(oldPath, "%s/log/last", g_conf.g_progDir);
	sprintf(newPath, "%s/log/last2", g_conf.g_progDir);
	rename(oldPath, newPath);	
	unlink(oldPath);                                 // necessary ?

    getTimeStr_YYYYMMDD_HHMMSS(curTime, sizeof(curTime));
    strncpy(dateStr, curTime, 8);
    strncpy(timeStr, curTime+8, 6);
    
	memset(newPath, 0, sizeof(newPath));
    sprintf(newPath, "%s/log", g_conf.g_progDir);
    if(0 != stat(newPath, &buf))
    {
    	logDirNoExist = true;
	    ret = mkdir(newPath, 0777);
	    if (ret != 0)
	    {
	    	printf("Fail to create log dir %s\n", newPath);
	    	return ATF_FAIL;
	    }
    }
	
    sprintf(newPath, "%s/log/%s_%s", g_conf.g_progDir, dateStr, timeStr);
    ret = mkdir(newPath, 0777);
    if (ret != 0)
    {
    	printf("Fail to create log dir %s\n", newPath);
    	return ATF_FAIL;
    }
	symlink(newPath, oldPath);

	//backup execute file,configure files,testcases
	
	//create the atf.bak dir in log dir
	sprintf(tmpPath, "%s/atf.bk", newPath);
	ret = mkdir(tmpPath, 0777);
	if (ret != 0)
	{
		printf("Fail to create atf.bk dir %s\n", tmpPath);
		return -1;
	}

	sprintf(cmdStr, "/bin/cp -rf '%s/bin' '%s/atf.bk/'", g_conf.g_progDir, newPath);
	system(cmdStr);

	
	sprintf(cmdStr, "/bin/cp -rf '%s/cases' '%s/atf.bk/case'", g_conf.g_progDir, newPath);
	system(cmdStr);

	sprintf(cmdStr, "/bin/cp -rf '%s/data' '%s/atf.bk/data'", g_conf.g_progDir, newPath);
	system(cmdStr);

	if(false == logDirNoExist)
	{
		sprintf(cmdStr, "/bin/mv '%s/log/atf.log' '%s'", g_conf.g_progDir, newPath);
		system(cmdStr);
	}
	
	return ATF_SUCC;
}

int main(int argc, char ** argv)
{
	signal(SIGINT, (__sighandler_t)catch_sigint);
	signal(SIGHUP, (__sighandler_t)catch_hup);
	signal(SIGPIPE, SIG_IGN);
	
	g_ProgramStartTime = utils_get_time();
	g_ProgramStartHour.tv_sec = g_ProgramStartTime.tv_sec - (g_ProgramStartTime.tv_sec % 3600);

	InitConfig();

	if(ATF_FAIL == GetProgRootDir())
	{
		printf("[error] Failed to get ATF root Dir !");
		return ATF_FAIL;
	}
	
    //backup the data of last task
    if(ATF_FAIL == BackupLastTask())
	{
		printf("[error] Failed to backup the data of last task !\n");
		return ATF_FAIL;
	}
	
	if(ATF_FAIL == LoadConfig())
	{
		printf("[error] Failed to load configuration !");
		return ATF_FAIL;
	}

	if(ATF_FAIL == g_log.init(g_conf.g_logConf))
	{
		printf("[error] Failed to init log !");
	}

	g_task.Init(g_conf.g_taskConf);
	if(ATF_FAIL == g_task.run())
	{
		printf("[error] Failed to start task !");
	}

	while(1)
	{
		sleep(1);
	}

	return ATF_SUCC;

}
