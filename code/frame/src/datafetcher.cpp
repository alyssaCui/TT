#include "datafetcher.h"
#include "common.h"

#define DEFAULT_QUOTE_FILE "/data/ag_2014-12-09.dat"
extern char g_progDir[];

CDataFetcher::CDataFetcher():m_flag_quote_read_over(0)
{
	m_llDataStatistics = 0;
}

CDataFetcher::~CDataFetcher()
{
}

int CDataFetcher::Init(LockFreeQueue<CUstpFtdcDepthMarketDataField> * pQueue)
{
	m_DataQueue = pQueue;
	return ATF_SUCC;
}

int CDataFetcher::SetFilePath(char *pFile)
{
	memset(m_pFilePath,0,sizeof(m_pFilePath));
	strncpy(m_pFilePath,pFile,sizeof(m_pFilePath));
	m_flag_quote_read_over = 0;

	return ATF_SUCC;
}


void *CDataFetcher::Run(void *)
{
	int iReadSize = 0;
	//long long id = 0;	
	CUstpFtdcDepthMarketDataField data;
	char *pData = (char *)&data; 

	if(0 == strlen(m_pFilePath))
	{
		g_log.error("m_pFilePath is NULL\n");
		return NULL;
	}

	FILE* fp = fopen(m_pFilePath, "rb");	
	if(NULL == fp)
	{
		g_log.error("Failed to open file: %s\n",m_pFilePath);
		return NULL;
	}

	while(1)
	{
		iReadSize = fread(pData,sizeof(CUstpFtdcDepthMarketDataField),1,fp);
		if(1 != iReadSize)
		{
			if(0 == feof(fp))
			{
				g_log.error("Datafetcher read error, expect %d, actual %d\n",sizeof(CUstpFtdcDepthMarketDataField),iReadSize);
			}
			else
			{
				g_log.info("Datafetcher read over.\n");
				m_flag_quote_read_over = 1;
			}
			fclose(fp);
			break;
		}
        m_llDataStatistics++;
		
		while(1)
		{	
			if(ATF_FAIL == m_DataQueue->PushOneData(&data))
			{
				g_log.error("Datafetcher push data(no:%lld) error\n",m_llDataStatistics);
			}
			else
			{
				//g_log.debug("push a quote to queue %lld\n",m_llDataStatistics);
				break;
			}
			//usleep(1000);
		}
		
		//usleep(1000);
	}

	return NULL;

}



