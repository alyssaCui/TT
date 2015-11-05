#ifndef __THREAD_H_
#define __THREAD_H_
#include <pthread.h>
#include<signal.h>

class CThread{
public:
	CThread();
	virtual ~CThread();
	int Start();
	void Stop();
	void ForceStop();
	void CancelStop();
	void Join();
	void Terminate();
	virtual void Unblock();
	static void* ThreadProc(void* p);
    int isTerminated() {return m_bTerminated;}
protected:
	virtual int Init();
	virtual void* Run(void *);
	virtual void OnExit();
	pthread_t m_tid;
	int m_bStop;
	int m_bForceStop; //add by ditto
	int m_bTerminated;
};
#endif

