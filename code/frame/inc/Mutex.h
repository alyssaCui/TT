#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <pthread.h>

class CMutex
{
public:
	CMutex();
	~CMutex();
	
	int lock();
	int unlock();
	int trylock();

protected:
	pthread_mutex_t m_mutex;
};

#endif

