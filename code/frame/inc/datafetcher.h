#ifndef __DATA_FETCHER_H__
#define __DATA_FETCHER_H__
#include <stdio.h>
#include <stdlib.h>
#include "Thread.h"
#include "common.h"
#include "thostsoDef.hpp"
#include "ThostFtdcUserApiStruct.h"
#include "USTPFtdcUserApiStruct.h"


class CDataFetcher : public CThread
{
	public:
		CDataFetcher();
		~CDataFetcher();
	
		virtual void* Run(void *);

		int Init(LockFreeQueue<CUstpFtdcDepthMarketDataField> * pQueue );
		
		int SetFilePath(char *);

		int GetFlagQuoteReadOver(){return m_flag_quote_read_over;}
		long long GetStatistics(){return m_llDataStatistics;}

	private:
		LockFreeQueue<CUstpFtdcDepthMarketDataField> * m_DataQueue;
		char m_pFilePath[MAX_FILE_PATH_LEN];
		int m_flag_quote_read_over;
		long long m_llDataStatistics;
		

};



#endif
