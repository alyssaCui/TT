//Thread.cc
#include <stdio.h>
#include <cstring>
#include "Thread.h"

extern "C"
{
typedef void*(*THREAD_PROC)(void*);
}

////////////////////////////////////////////////////////////////////////////////
CThread::CThread() : m_bTerminated(0)
{
}

CThread::~CThread()
{
}

int CThread::Start()
{
	int ret;
	
	m_bStop=0;
	m_bForceStop = 0;
	m_bTerminated = 0;
	
	ret = Init();
    if(ret != 0)
    {
        m_bTerminated = 1;
        return -1;
    }

	ret = pthread_create(&m_tid,NULL,(THREAD_PROC)ThreadProc,this);
	if (ret != 0)
	{	
		perror("Fail to create thread");
		return -2;
	}

	return 0;
	
}

void CThread::Stop()
{	
	m_bStop=1;
}

void CThread::ForceStop()
{	
	m_bForceStop=1;
}

void CThread::CancelStop()
{	
	//pthread_cancel(m_tid);
	pthread_kill(m_tid, 2);//SIGINT
}

void CThread::Join()
{
	pthread_join(m_tid,NULL);
}

void CThread::Unblock()
{
}

void CThread::Terminate()
{
	m_bStop = 1;
	Unblock();
	Join();
}

void* CThread::ThreadProc(void* p)
{
	CThread* pThread = (CThread*)p;
	
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);   
	
	void* ret = pThread->Run(NULL);
	
	pThread->OnExit();
	
	pThread->m_bTerminated = 1;
	
	return ret;
}

int CThread::Init()
{
	return 0;
}

void* CThread::Run(void *)
{
	printf("error!!! CThread::Run ticking\n");

	while(!m_bStop && !m_bForceStop)
	{
		;
	}
		
	return NULL;
}

void CThread::OnExit()
{
}

