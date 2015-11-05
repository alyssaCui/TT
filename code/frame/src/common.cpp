
/**********************
*
* 
*
************************/
#include "common.h"

struct timeval utils_get_time()
{
    struct timeval t_cur;

    gettimeofday(&t_cur, (struct timezone *)0);
    if (t_cur.tv_usec < 0) 
	{
        t_cur.tv_sec--;
        t_cur.tv_usec += 1000000;
    } 
	else if (t_cur.tv_usec >= 1000000) 
	{
        t_cur.tv_sec++;
        t_cur.tv_usec -= 1000000;
    }	

	return t_cur;
}

char * getTimeStr_YYMMDD_HHMMSS(char *str, int len)
{
    struct tm *t, tbuf;
    time_t tsec;

	if (NULL == str)
	{
		return NULL;
	}
	
    time(&tsec);
    t = localtime_r(&tsec, &tbuf);
    snprintf(str, len, "%02d-%02d-%02d %02d:%02d:%02d", t->tm_year-100, t->tm_mon+1,
		t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

	return str;
}

char * getTimeStr_YYYYMMDD_HHMMSS(char *str, int len)
{
    struct tm *t, tbuf;
    time_t tsec;

	if (NULL == str)
	{
		return NULL;
	}
	
    time(&tsec);
    t = localtime_r(&tsec, &tbuf);
    snprintf(str, len, "%04d%02d%02d%02d%02d%02d", t->tm_year+1900, t->tm_mon+1,
		t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    
    return str;	
}

char * getTimeStr_YYMMDD_HHMMSS_MS(char *str, int len)
{
	struct timeval t_cur;
    struct tm *t, tbuf;

	if (NULL == str)
	{
		return NULL;
	}
	
	gettimeofday(&t_cur, (struct timezone *)0);
    t = localtime_r(&(t_cur.tv_sec), &tbuf);
    snprintf(str, len, "%02d-%02d-%02d %02d:%02d:%02d.%04d", t->tm_year-100, t->tm_mon+1,
		t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, (unsigned int)(t_cur.tv_usec)/ 100);

    return str;
}

int GotCurrentDate(char *pDate)
{
	time_t lNow;
	struct tm *stNow;

	if(NULL == pDate)
	{
		return ATF_FAIL;
	}

	time(&lNow);
	stNow = localtime(&lNow);

	sprintf(pDate, "%04d%02d%02d",stNow->tm_year + 1900, stNow->tm_mon + 1, stNow->tm_mday);
	return ATF_SUCC;
}


int GotCurrentTime(char *pTime)
{
	time_t lNow;
	struct tm *stNow;

	if(NULL == pTime)
	{
		return ATF_FAIL;
	}

	time(&lNow);
	stNow = localtime(&lNow);

	sprintf(pTime, "%02d:%02d:%02d",stNow->tm_hour, stNow->tm_min, stNow->tm_sec);
	return ATF_SUCC;
}

/*struct timeval get_time()
{

    struct timeval t_cur;

	gettimeofday(&t_cur,  (struct timezone *) 0);
	
	if (t_cur.tv_usec < 0)
	{
		t_cur.tv_sec--;
		t_cur.tv_usec += 1000000;
	}
	else if (t_cur.tv_usec >= 1000000) 
	{
		t_cur.tv_sec += t_cur.tv_usec / 1000000;
		t_cur.tv_usec = t_cur.tv_usec % 1000000;
	}

	return t_cur;
	
}*/

struct timeval time_get_now()
{
    struct timeval t_cur;

	gettimeofday(&t_cur,  (struct timezone *) 0);
	
	if (t_cur.tv_usec < 0)
	{
		t_cur.tv_sec--;
		t_cur.tv_usec += 1000000;
	}
	else if (t_cur.tv_usec >= 1000000) 
	{
		t_cur.tv_sec += t_cur.tv_usec / 1000000;
		t_cur.tv_usec = t_cur.tv_usec % 1000000;
	}

	return t_cur;
}


char* ltrim(const char* str)
{
	const char* ptr1;

	ptr1 = str;
	while (*ptr1 == ' ' || *ptr1=='\t')
	{
		ptr1++;
	}

	return (char*)ptr1;
}

char* rtrim(char* str)
{
	char* ptr1 = NULL;
	int len = 0;

	len = strlen(str);
	if (len == 0)
	{
		return str;		
	}
	
	ptr1 = str + len - 1;
	while ((ptr1>=str) && (*ptr1==' ' || *ptr1=='\t' || *ptr1=='\n'))
	{
		ptr1--;
	}

	*(ptr1+1) = '\0';

	return str;
}

char* trim(char* str)
{
	rtrim(str);
	return ltrim(str);
}

char* trimQuota(char* str)
{
	int len = 0;

	len = strlen(str);

	if (str[0] == '"' && str[len-1] == '"')
	{
		str[len-1] = '\0';
		return str+1;
	}
	else
	{
		return str;
	}
}

int trimLf(char* str)
{
	//char* ptr1 = NULL;
	int len = 0;
	int index = 0;

	len = strlen(str);
	if (len == 0)
	{
		return 0;
	}

	for (index=len-1; index>=0; index--)
	{
		if ((str[index] == '\n') || (str[index] == '\r'))
		{
			len--;			
		}
		else
		{
			break;
		}
	}
	str[len] = '\0';
	
	return len;
	
}

int transStr(char* fromStr, char* toStr, int fromStrLen)
{
	int fromStrIndex = 0;
	int toStrLen = 0;
	int quotaFlag = 0;
	char newChar;
	
	toStr[0] = '\0';
	while (fromStrIndex < fromStrLen)
	{
		//find Quota char, get the whole string in quota
		if (fromStr[fromStrIndex] == '"')
		{
			quotaFlag = 1 - quotaFlag;
			fromStrIndex++;
			continue;
		}
	
		if (quotaFlag == 0)
		{
			toStr[toStrLen++] = fromStr[fromStrIndex++];
			continue;
		}
	
		if (fromStr[fromStrIndex] == '\\')
		{
			switch (fromStr[fromStrIndex+1])
			{
				case '\\':
					newChar = '\\'; break;
				case 'r':
					newChar = '\r'; break;
				case 'n':
					newChar = '\n'; break;
				case '"':
					newChar = '"'; break;
				case 't':
					newChar = '\t'; break;
				default :
					newChar = fromStr[fromStrIndex+1]; break;
			}
	
			toStr[toStrLen++] = newChar;
			fromStrIndex += 2;
				
		}
		else
		{
			toStr[toStrLen++] = fromStr[fromStrIndex++];
			continue;
		}
				
	}
	
	toStr[toStrLen] = '\0';
		
	if (quotaFlag == 1)
	{
		printf("Invalid string. There is no end quota\n");
		return -1;
	}
	
		//printf(">>from=%s\n", fromStr);
		//printf(">>to=%s\n", toStr);
	return 0;
}

int time_diff(struct timeval startTime, struct timeval endTime, long* psec, long* pusec)
{
	long sec;
	long usec;

	if (endTime.tv_usec < startTime.tv_usec)
	{
		endTime.tv_sec--;
		endTime.tv_usec += 1000000;
	}
	
	usec = endTime.tv_usec - startTime.tv_usec;
	sec = endTime.tv_sec - startTime.tv_sec;

	*psec = sec;
	*pusec = usec;

	if ((sec > 0) || (sec == 0 && usec > 0))
	{
		return 1;
	}
	else if (sec == 0 && usec == 0)
	{
		return 0;
	}
	else
	{
		return -1;
	}

}


struct timeval time_diff(struct timeval startTime, struct timeval endTime)
{
	long sec;
	long usec;
	struct timeval returnTime;
	

	if (endTime.tv_usec < startTime.tv_usec)
	{
		endTime.tv_sec--;
		endTime.tv_usec += 1000000;
	}
	
	sec = endTime.tv_sec - startTime.tv_sec;
	usec = endTime.tv_usec - startTime.tv_usec;

	returnTime.tv_sec = sec;
	returnTime.tv_usec = usec;

	return returnTime;
	
}


unsigned long time_diff_ms(struct timeval startTime, struct timeval endTime)
{
	struct timeval diffTime;	
	unsigned long diffTime_ms;

	diffTime = time_diff(startTime, endTime);
		
	diffTime_ms = diffTime.tv_sec*1000 + diffTime.tv_usec/1000;

	return diffTime_ms;
}

unsigned long time_diff_ms(struct timeval startTime)
{
	return time_diff_ms(startTime, time_get_now());
}

unsigned long time_diff_us(struct timeval startTime, struct timeval endTime)
{
	struct timeval diffTime;	
	unsigned long diffTime_ms;

	diffTime = time_diff(startTime, endTime);
		
	diffTime_ms = diffTime.tv_sec*1000000 + diffTime.tv_usec;

	return diffTime_ms;
}


struct timeval time_add(struct timeval startTime, long sec, long usec)
{
	struct timeval endTime;

	endTime.tv_sec = startTime.tv_sec + sec;
	endTime.tv_usec = startTime.tv_usec + usec;

	if (endTime.tv_usec >= 1000*1000)
	{
		endTime.tv_sec += endTime.tv_usec / (1000*1000);
		endTime.tv_usec = endTime.tv_usec % (1000*1000);
	}

	return endTime;
}

struct timeval time_add(struct timeval startTime, struct timeval addTime)
{
	struct timeval endTime;

	endTime.tv_sec = startTime.tv_sec + addTime.tv_sec;
	endTime.tv_usec = startTime.tv_usec + addTime.tv_usec;

	if (endTime.tv_usec > 1000*1000)
	{
		endTime.tv_sec += endTime.tv_usec / (1000*1000);
		endTime.tv_usec = endTime.tv_usec % (1000*1000);
	}

	return endTime;
}


int time_compare(struct timeval time1, struct timeval time2)
{
	int ret;

	if ((time1.tv_sec == time2.tv_sec) && (time1.tv_usec == time2.tv_usec))
	{
		ret = 0;
	}
	else if ((time1.tv_sec > time2.tv_sec) || ((time1.tv_sec == time2.tv_sec) && (time1.tv_usec > time2.tv_usec)))
	{
		ret = 1;
	}
	else
	{
		ret = -1;
	}

	return ret;

}

/*
FILE* utils_file_open(char * pPath,char * pmode)
{
	if(pPath == NULL || pmode == NULL)
		return NULL;
	FILE* fp = fopen(pPath, pmode);	
	return fp;
}

int utils_file_read(FILE* iFd,char * pBuf,int iSize)
{
	return fread(pBuf,1,iSize,iFd);
}

void utils_file_close(FILE* iFd)
{
	fclose(iFd);
}
*/

/*
int my_printf(FILE* fp, const char* format, ...)
{

	va_list arglist; 
	char logContent[MAX_LOG_LEN];
    
	va_start(arglist, format); 
	vsprintf(logContent, format, arglist);
	va_end(arglist);

	printf("%s", logContent);

	if (fp != NULL)
	{
		fprintf(fp, "%s", logContent);
		fflush(fp);
	}
}
*/


ReadConfFile::ReadConfFile()
{
}

ReadConfFile::~ReadConfFile()
{
}

int ReadConfFile::GetDir(char *pDir)
{
	int  ret = 0;
	char *ptr1 = NULL;
	char dir[MAX_FILE_PATH_LEN] = {0};
	
    //get the absolute path of the process
	ret = readlink ("/proc/self/exe", dir, sizeof(dir)-1);
	if (ret < 0)
	{
		printf("Failed to get the absolute path .\n");
		return -1;
	}
	//printf("get the directory:%s\n",dir);
	
	//get the directory of the process
	ptr1 = strrchr(dir, '/');
	if (NULL == ptr1 || ptr1 == dir)
	{
		printf("Failed to get the directory .");
		return -1;
	}
	ptr1[0] = '\0';

	strcpy(pDir,dir);

	return 0;
}


FILE * ReadConfFile::OpenFile(char *pFile)
{
	FILE *fp = NULL;	
	
	//open the configure file
	fp = fopen(pFile, "r");	
	if (NULL == fp)
	{
		printf("[error] Fail to open file %s\n", pFile);
		return NULL;
	}
	
	return fp;
}

int ReadConfFile::CloseFile(FILE *fp)
{
	if(NULL == fp)
		return -1;	
	else
	{
		fclose(fp);
		return 0;
	}
}

int ReadConfFile::ReadStr(FILE *fp,char *pExpectTag,char *pExpectValue)
{

	char srcLine[MAX_CMD_LINE_LEN] = {0};
	char refineLine[MAX_CMD_LINE_LEN] = {0};	
	char transLine[MAX_CMD_LINE_LEN] = {0};		
	unsigned int newLineNo = 0;	
	unsigned int len = 0;
	unsigned int ptr2_len = 0;
	char* ptr = NULL;	
	char *pTag = NULL;
	char *pValue = NULL; 
	char *pComment = NULL;
	

	//loop to read each line of configure file
	while (1)
	{	
		newLineNo++;
		ptr = fgets(srcLine, sizeof(srcLine)-1, fp);
		if (ptr == NULL)
		{
			return -1;
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
		
		if (strcasecmp(pTag, pExpectTag) != 0)
		{
			continue;
		}
		else
		{
        	if (strlen(pValue) <= 0)
        	{
        		printf("[error] Invalid value !\n");
        		fclose(fp);
        		return -1;					
        	}

			strcpy(pExpectValue,pValue);
			return 0;
		}	
	}
}
