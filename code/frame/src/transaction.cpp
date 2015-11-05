#include "transaction.hpp"


int CTransData::addRev(struct timeval transTime)
{
	m_num++;
	
	m_sumTime = time_add(m_sumTime, transTime.tv_sec, transTime.tv_usec);

	if (m_num == 1)
	{
		m_minTime = transTime;
		m_maxTime = transTime;
	}
	else
	{
		if (time_compare(transTime, m_maxTime) > 0) { m_maxTime = transTime; }
		if (time_compare(transTime, m_minTime) < 0) { m_minTime = transTime; }
	}

	return ATF_SUCC;
}

int CTransData::countSummary(struct timeval durationTime)
{
	float fDurationTime;
	
	if (durationTime.tv_sec > 0 || durationTime.tv_usec > 0)
	{
		fDurationTime = durationTime.tv_sec + durationTime.tv_usec*1.0 / (1000*1000);		
		if (fDurationTime < 1)
		{
			fDurationTime = 1; //if time less than 1 sec, count speed as time  is 1 sec
		}
		m_speed = m_num / fDurationTime;
	}
	else
	{
		m_speed = 0;
	}
	

	if (m_num > 0)
	{
		m_fAverTime = m_sumTime.tv_sec * 1.0 / m_num * 1000;
		m_fAverTime += m_sumTime.tv_usec * 1.0 / m_num / 1000;

		m_fMinTime = m_minTime.tv_sec * 1.0 * 1000;
		m_fMinTime += m_minTime.tv_usec * 1.0 / 1000;

		m_fMaxTime = m_maxTime.tv_sec * 1.0 * 1000;
		m_fMaxTime += m_maxTime.tv_usec * 1.0 / 1000;		
	}
	else
	{
		m_fAverTime = 0;
		m_fMinTime = 0;
		m_fMaxTime = 0;		
	}

	return 0;
}

int CTransInfo::countSummary(struct timeval countStartTime, struct timeval countEndTime)
{
	m_startTime = countStartTime;
	m_endTime = countEndTime;
	m_durationTime = time_diff(m_startTime, m_endTime);
	
	m_totalData.countSummary(m_durationTime);
	m_succData.countSummary(m_durationTime);
	m_failData.countSummary(m_durationTime);
	m_timeoutData.countSummary(m_durationTime);
	m_lostData.countSummary(m_durationTime);

	return 0;
}

int CTransaction::init(char* name)
{
	struct timeval curTime;
	
	lock();

	curTime = time_get_now();

	if (name != NULL)
	{
		if (strlen(name) > 0)
		{
			memset(m_name, 0, sizeof(m_name));
			strncpy(m_name, name, sizeof(m_name)-1);
		}
	}
	
	memset(&m_totalInfo, 0, sizeof(m_totalInfo));
	strncpy(m_totalInfo.name, m_name, sizeof(m_totalInfo.name)-1);

	memset(&m_curInfo, 0, sizeof(m_curInfo));
	strncpy(m_curInfo.name, m_name, sizeof(m_curInfo.name)-1);

	memset(&m_lastInfo, 0, sizeof(m_lastInfo));
	strncpy(m_lastInfo.name, m_name, sizeof(m_lastInfo.name)-1);


	m_initTime = curTime;
	m_lastSummaryTime = curTime;
	
	memset(&m_finishTime, 0, sizeof(m_finishTime));
	
	m_finishFlag = 0;
	memset(&m_transStartTime, 0, sizeof(m_transStartTime));

	unlock();

	return 0;
}

int CTransaction::finish()
{
	struct timeval curTime;
	CTransInfo totalInfo;
	CTransInfo curInfo;
	CTransInfo lastInfo;

	getTransSummary(totalInfo, curInfo, lastInfo, 0);
	
	lock();

	curTime = time_get_now();
		
	m_finishFlag = 1;
	m_finishTime = curTime;	

	unlock();

	return 0;
}

int CTransaction::startTrans()
{
	struct timeval curTime;

	if (m_finishFlag)
	{
		return 0;
	}
	
	lock();
	
	curTime = time_get_now();

	m_transStartTime = curTime;

	unlock();

	return 0;
}

int CTransaction::endTrans(TRANS_RESULT_TYPE transResult)
{
	int ret;
	
	long sec, usec;
	struct timeval curTime;
	struct timeval transTime;


	if (m_finishFlag)
	{
		return 0;
	}
		
	lock();

	//compute the transaction time
	if (m_transStartTime.tv_sec == 0)
	{
		g_log.error("Transaction %s end without start!\n", m_name);
		unlock();
		return -1;
	}

	curTime = time_get_now();
	time_diff(m_transStartTime, curTime, &sec, &usec);

	transTime.tv_sec = sec;
	transTime.tv_usec = usec;

	//add transaction rec
	ret = addTransRevBase(transResult, transTime);
	
	//clear start time
	m_transStartTime.tv_sec = 0;
	m_transStartTime.tv_usec = 0;

	unlock();
	
	return ret;
	
}

int CTransaction::addTransRev(TRANS_RESULT_TYPE transResult, struct timeval transTime)
{
	int ret;

	if (m_finishFlag)
	{
		return 0;
	}

	lock();

	ret = addTransRevBase(transResult, transTime);

	unlock();

	return ret;
}

int CTransaction::addTransRevBase(TRANS_RESULT_TYPE transResult, struct timeval transTime)
{
	//process the transaction data
	switch (transResult)
	{
		case TRANS_RESULT_SUCC:
			m_totalInfo.m_totalData.addRev(transTime);
			m_totalInfo.m_succData.addRev(transTime);

			m_curInfo.m_totalData.addRev(transTime);
			m_curInfo.m_succData.addRev(transTime);

			break;
			
		case TRANS_RESULT_FAIL:
			m_totalInfo.m_totalData.addRev(transTime);
			m_totalInfo.m_failData.addRev(transTime);

			m_curInfo.m_totalData.addRev(transTime);
			m_curInfo.m_failData.addRev(transTime);

			break;
			
		case TRANS_RESULT_TIMEOUT:
			m_totalInfo.m_totalData.addRev(transTime);
			m_totalInfo.m_timeoutData.addRev(transTime);

			m_curInfo.m_totalData.addRev(transTime);
			m_curInfo.m_timeoutData.addRev(transTime);

			break;
			
		case TRANS_RESULT_LOST:
			m_totalInfo.m_totalData.addRev(transTime);
			m_totalInfo.m_lostData.addRev(transTime);

			m_curInfo.m_totalData.addRev(transTime);
			m_curInfo.m_lostData.addRev(transTime);

			break;
			
		default:
			break;
			
	}	

	return 0;
}

int CTransaction::getTransSummary(CTransInfo &totalInfo, CTransInfo &curInfo, CTransInfo &lastInfo, int newSummaryFlag)
{
	struct timeval curTime;
	
	lock();

	if (m_finishFlag == 0)
	{
		curTime = time_get_now();

		m_totalInfo.countSummary(m_initTime, curTime);
		m_curInfo.countSummary(m_lastSummaryTime, curTime);

		totalInfo = m_totalInfo;
		curInfo = m_curInfo;
		lastInfo = m_lastInfo;
			
		if (newSummaryFlag == 1)
		{
			memcpy(&m_lastInfo, &m_curInfo, sizeof(m_lastInfo));
			memset(&m_curInfo, 0, sizeof(m_curInfo));
			strncpy(m_curInfo.name, m_name, sizeof(m_curInfo.name)-1);
			m_lastSummaryTime = curTime;
		}
	}
	else
	{
		totalInfo = m_totalInfo;
		curInfo = m_curInfo;
		lastInfo = m_lastInfo;
	}

	unlock();

	return 0;
}

int CTransaction::getTransInfo(CTransInfo &totalInfo)
{	
	lock();
	totalInfo = m_totalInfo;
	unlock();

	return 0;
}

CAllTrans::CAllTrans()
{

}


int CAllTrans::finish()
{
	transactConnect.finish();
	transactSendQuote.finish();
	transactRevOrderInsert.finish();	
	transactSendOrderRsp.finish();
	
	return ATF_SUCC;	
}

