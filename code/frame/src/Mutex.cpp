#include "Mutex.h"


CMutex::CMutex()
{
	pthread_mutex_init(&m_mutex,NULL);
}

CMutex::~CMutex()
{
	pthread_mutex_destroy(&m_mutex);
}

int CMutex::lock()
{
	return pthread_mutex_lock(&m_mutex);
}

int CMutex::unlock()
{
	return pthread_mutex_unlock(&m_mutex);
}

int CMutex::trylock()
{
	return pthread_mutex_trylock(&m_mutex);
}

