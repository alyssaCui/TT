
/**********************
*
* 
*
************************/
#ifndef __COMMON_H__
#define __COMMON_H__
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "Thread.h"
#include "constants.h"
#include "log.h"

#define INVALID         -1
#define WRITEABLE        0
#define READABLE         1
#define QUEUE_EMPTY      0
#define QUEUE_FULL       1
#define QUEUE_READABLE   2
#define QUEUE_WRITEABLE  3
#define ATF_TRUE         0
#define ATF_FALSE        1
#define MAX_QUEUE_SIZE   10240

class CLog;
class CTask;
extern CLog g_log;
extern CTask g_task;

enum WORKITEM_STATUS_TYPE
{
	WORKITEM_NOT_DONE = 0,
	WORKITEM_DOING = 1,
	WORKITEM_DONE,
	WORKITEM_LOAD_FAIL,
	WORKITEM_DONE_SUCC,
	WORKITEM_DONE_FAIL
};

struct timeval utils_get_time();


char * getTimeStr_YYMMDD_HHMMSS(char *str, int len);

char * getTimeStr_YYYYMMDD_HHMMSS(char *str, int len);
char * getTimeStr_YYMMDD_HHMMSS_MS(char *str, int len);
//struct timeval get_time();
int GotCurrentDate(char *);
int GotCurrentTime(char *);

char* ltrim(const char* str);
char* rtrim(char* str);
char* trim(char* str);

char* trimQuota(char* str);
int trimLf(char* str);
int transStr(char* fromStr, char* toStr, int fromStrLen);

struct timeval time_get_now();
int time_diff(struct timeval startTime, struct timeval endTime, long* psec, long* pusec);
struct timeval time_diff(struct timeval startTime, struct timeval endTime);

unsigned long time_diff_ms(struct timeval startTime, struct timeval endTime);
unsigned long time_diff_ms(struct timeval startTime);
unsigned long time_diff_us(struct timeval startTime, struct timeval endTime);

struct timeval time_add(struct timeval startTime, long sec, long usec);
struct timeval time_add(struct timeval startTime, struct timeval addTime);
int time_compare(struct timeval time1, struct timeval time2);



template<class T>
class LockFreeQueue
{
public:
	LockFreeQueue();
	~LockFreeQueue();
	int PushOneData(T * );
	int PopOneData(T *);
	int IsEmpty();
	int IsFull();
			
private:
	T m_Items[MAX_QUEUE_SIZE];
	int m_ItemStatus[MAX_QUEUE_SIZE];
	int m_iHead;
	int m_iTail;
};


template<class T> 
LockFreeQueue<T>::LockFreeQueue()
{
	m_iHead = 0;
	m_iTail = 0;
	
	for(int i=0;i<MAX_QUEUE_SIZE;i++)
	{
		m_ItemStatus[i] = WRITEABLE;
	}
}

template<class T> 
LockFreeQueue<T>::~LockFreeQueue()
{

}

template<class T> 
int LockFreeQueue<T>::PushOneData(T * pData)
{
    //queue are full
	if(ATF_TRUE == IsFull())
		return ATF_FAIL;
	
	memcpy(&m_Items[m_iTail], pData,sizeof(T));
	
	m_ItemStatus[m_iTail] = READABLE;
	
	m_iTail = (m_iTail + 1)%MAX_QUEUE_SIZE;

	return ATF_SUCC;
}


template<class T> 
int LockFreeQueue<T>::PopOneData(T * pData)
{
	if(ATF_TRUE == IsEmpty())
		return ATF_FAIL;
	
	memcpy(pData, &m_Items[m_iHead], sizeof(T));
	
	m_ItemStatus[m_iHead] = WRITEABLE;
	
	m_iHead = (m_iHead + 1)%MAX_QUEUE_SIZE;

	return ATF_SUCC;
}


template<class T> 
inline int LockFreeQueue<T>::IsEmpty( )
{
	if(m_iTail == m_iHead)
		return ATF_TRUE;
	else
		return ATF_FALSE;
}

template<class T> 
inline int LockFreeQueue<T>::IsFull( )
{
	//判断循环链表是否满，留一个预留空间不用
	if((m_iTail+1)%MAX_QUEUE_SIZE == m_iHead)
		return ATF_TRUE;
	else
		return ATF_FALSE;
}

class ReadConfFile
{
public:
	ReadConfFile();
	~ReadConfFile();
	int GetDir(char *pDir);
	FILE * OpenFile(char *pFile);	
	int CloseFile(FILE *fp);
	int ReadStr(FILE *fp,char *pExpectTag,char *pExpectValue);
};

#endif
