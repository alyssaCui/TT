#include "../so_simu/ctp_so_simu/inc/USTPFtdcUserApiStruct.h"
#include "../so_simu/ctp_so_simu/inc/thostsoDef.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define SRC_DATA_TYPE CFfexFtdcDepthMarketData
#define DST_DATA_TYPE CUstpFtdcDepthMarketDataField
#define HeaderSize 8
#define TIMESTAMP_SIZE  8


#pragma pack(8)

struct dat_head
{
	unsigned short len;
	unsigned short type;
	unsigned int count;
};

/// �н�������г�����
struct CFfexFtdcDepthMarketData
{
    char szTradingDay[9];        ///< ������
    char szSettlementGroupID[9]; ///< ���������
    int nSettlementID;          ///< ������
    double dLastPrice;             ///< ���¼�
    double dPreSettlementPrice;    ///< �����
    double dPreClosePrice;         ///< ������
    double dPreOpenInterest;       ///< ��ֲ���
    double dOpenPrice;             ///< ����
    double dHighestPrice;          ///< ��߼�
    double dLowestPrice;           ///< ��ͼ�
    int nVolume;                ///< ����
    double dTurnover;              ///< �ɽ����
    double dOpenInterest;          ///< �ֲ���
    double dClosePrice;            ///< ������
    double dSettlementPrice;       ///< �����
    double dUpperLimitPrice;       ///< ��ͣ���
    double dLowerLimitPrice;       ///< ��ͣ���
    double dPreDelta;              ///< ����ʵ��
    double dCurrDelta;             ///< ����ʵ��
    char szUpdateTime[9];        ///< ����޸�ʱ��
    int nUpdateMillisec;        ///< ����޸ĺ���
    char szInstrumentID[31];     ///< ��Լ����
    double dBidPrice1;             ///< �����һ
    int nBidVolume1;            ///< ������һ
    double dAskPrice1;             ///< ������һ
    int nAskVolume1;            ///< ������һ
    double dBidPrice2;             ///< ����۶�
    int nBidVolume2;            ///< ��������
    double dAskPrice2;             ///< �����۶�
    int nAskVolume2;            ///< ��������
    double dBidPrice3;             ///< �������
    int nBidVolume3;            ///< ��������
    double dAskPrice3;             ///< ��������
    int nAskVolume3;            ///< ��������
    double dBidPrice4;             ///< �������
    int nBidVolume4;            ///< ��������
    double dAskPrice4;             ///< ��������
    int nAskVolume4;            ///< ��������
    double dBidPrice5;             ///< �������
    int nBidVolume5;            ///< ��������
    double dAskPrice5;             ///< ��������
    int nAskVolume5;            ///< ��������
};

#pragma pack()

void ConvertDataForm(DST_DATA_TYPE *pDst, SRC_DATA_TYPE *pSrc)
{
	strcpy(pDst->TradingDay,pSrc->szTradingDay);
	strcpy(pDst->InstrumentID,pSrc->szInstrumentID);

	pDst->LastPrice = pSrc->dLastPrice; ///< ���¼�
	pDst->PreSettlementPrice = pSrc->dPreSettlementPrice; ///< �����
	pDst->PreClosePrice = pSrc->dPreClosePrice;///< ������
	pDst->PreOpenInterest = pSrc->dPreOpenInterest; ///< ��ֲ���
	pDst->OpenPrice = pSrc->dOpenPrice; ///< ����
	pDst->HighestPrice = pSrc->dHighestPrice; ///< ��߼�
	pDst->LowestPrice = pSrc->dLowestPrice;  ///< ��ͼ�
	pDst->Volume = pSrc->nVolume; ///< ����

	pDst->Turnover = pSrc->dTurnover;  ///< �ɽ����
	pDst->OpenInterest = pSrc->dOpenInterest;  ///< �ֲ���
	pDst->ClosePrice = pSrc->dClosePrice;   ///< ������
	pDst->SettlementPrice = pSrc->dSettlementPrice;  ///< �����
	pDst->UpperLimitPrice = pSrc->dUpperLimitPrice;    ///< ��ͣ���
	pDst->LowerLimitPrice = pSrc->dLowerLimitPrice;  ///< ��ͣ���
	pDst->PreDelta = pSrc->dPreDelta;   ///< ����ʵ��
	pDst->CurrDelta = pSrc->dCurrDelta; ///< ����ʵ��

	strcpy(pDst->UpdateTime,pSrc->szUpdateTime);///< ����޸�ʱ��
	pDst->UpdateMillisec = pSrc->nUpdateMillisec;  ///< ����޸ĺ���
	
	pDst->BidPrice1 = pSrc->dBidPrice1;   ///< �����һ
	pDst->BidVolume1 = pSrc->nBidVolume1; ///< ������һ
	pDst->AskPrice1 = pSrc->dAskPrice1;   ///< ������һ
	pDst->AskVolume1 = pSrc->nAskVolume1; ///< ������һ
	pDst->BidPrice2 = pSrc->dBidPrice2;   ///< ����۶�
	pDst->BidVolume2 = pSrc->nBidVolume2; ///< ��������
	pDst->AskPrice2 = pSrc->dAskPrice2;   ///< �����۶�
	pDst->AskVolume2 = pSrc->nAskVolume2; ///< ��������
	pDst->BidPrice3 = pSrc->dBidPrice3;    ///< �������
	pDst->BidVolume3 = pSrc->nBidVolume3; ///< ��������
	pDst->AskPrice3 = pSrc->dAskPrice3; ///< ��������
	pDst->AskVolume3 = pSrc->nAskVolume3; ///< ��������
	pDst->BidPrice4 = pSrc->dBidPrice4; ///< �������
	pDst->BidVolume4 = pSrc->nBidVolume4; ///< ��������
	pDst->AskPrice4 = pSrc->dAskPrice4; ///< ��������
	pDst->AskVolume4 = pSrc->nAskVolume4; ///< ��������
	pDst->BidPrice5 = pSrc->dBidPrice5;  ///< �������
	pDst->BidVolume5 = pSrc->nBidVolume5; ///< ��������
	pDst->AskPrice5 = pSrc->dAskPrice5; ///< ��������
	pDst->AskVolume5 = pSrc->nAskVolume5; ///< ��������
}

int PrintSrcData(SRC_DATA_TYPE *pQuote)
{
	///������
	printf("szTradingDay = %s\n",pQuote->szTradingDay);
	
	/// ���������
	printf("szSettlementGroupID = %s\n",pQuote->szSettlementGroupID);

	/// ������
	printf("nSettlementID = %d\n",pQuote->nSettlementID);		
		
	///���¼�
	printf("dLastPrice = %f\n",pQuote->dLastPrice);
		
	///�ϴν����
	printf("dPreSettlementPrice = %f\n",pQuote->dPreSettlementPrice);
		
	///������
	printf("dPreClosePrice = %f\n",pQuote->dPreClosePrice);
		
	///��ֲ���
	printf("dPreOpenInterest = %f\n",pQuote->dPreOpenInterest);
		
	///����
	printf("dOpenPrice = %f\n",pQuote->dOpenPrice);
		
	///��߼�
	printf("dHighestPrice = %f\n",pQuote->dHighestPrice);
		
	///��ͼ�
	printf("dLowestPrice = %f\n",pQuote->dLowestPrice);
		
	///����
	printf("nVolume = %d\n",pQuote->nVolume);
		
	///�ɽ����
	printf("dTurnover = %f\n",pQuote->dTurnover);
		
	///�ֲ���
	printf("dOpenInterest = %f\n",pQuote->dOpenInterest);
		
	///������
	printf("dClosePrice = %f\n",pQuote->dClosePrice);
		
	///���ν����
	printf("dSettlementPrice = %f\n",pQuote->dSettlementPrice);
		
	///��ͣ���
	printf("dUpperLimitPrice = %f\n",pQuote->dUpperLimitPrice);
		
	///��ͣ���
	printf("dLowerLimitPrice = %d\n",pQuote->dLowerLimitPrice);
		
	///����ʵ��
	printf("dPreDelta = %f\n",pQuote->dPreDelta);
		
	///����ʵ��
	printf("dCurrDelta = %f\n",pQuote->dCurrDelta);
		
	///����޸�ʱ��
	printf("szUpdateTime = %s\n",pQuote->szUpdateTime);
		
	///����޸ĺ���
	printf("nUpdateMillisec = %d\n",pQuote->nUpdateMillisec);

	///��Լ����
	printf("szInstrumentID = %s\n",pQuote->szInstrumentID);
		
	///�����һ
	printf("dBidPrice1 = %f\n",pQuote->dBidPrice1);
		
	///������һ
	printf("nBidVolume1 = %d\n",pQuote->nBidVolume1);
		
	///������һ
	printf("dAskPrice1 = %f\n",pQuote->dAskPrice1);
		
	///������һ
	printf("nAskVolume1 = %d\n",pQuote->nAskVolume1);
		
	///����۶�
	printf("dBidPrice2 = %f\n",pQuote->dBidPrice2);
		
	///��������
	printf("nBidVolume2 = %d\n",pQuote->nBidVolume2);
		
	///�����۶�
	printf("dAskPrice2 = %f\n",pQuote->dAskPrice2);
		
	///��������
	printf("nAskVolume2 = %d\n",pQuote->nAskVolume2);
		
	///�������
	printf("dBidPrice3 = %f\n",pQuote->dBidPrice3);
		
	///��������
	printf("nBidVolume3 = %d\n",pQuote->nBidVolume3);
		
	///��������
	printf("dAskPrice3 = %d\n",pQuote->dAskPrice3);
		
	///��������
	printf("nAskVolume3 = %d\n",pQuote->nAskVolume3);
		
	///�������
	printf("dBidPrice4 = %f\n",pQuote->dBidPrice4);
	
	///��������
	printf("nBidVolume4 = %d\n",pQuote->nBidVolume4);
		
	///��������
	printf("dAskPrice4 = %f\n",pQuote->dAskPrice4);
		
	///��������
	printf("nAskVolume4 = %d\n",pQuote->nAskVolume4);
		
	///�������
	printf("dBidPrice5 = %f\n",pQuote->dBidPrice5);
		
	///��������
	printf("nBidVolume5 = %d\n",pQuote->nBidVolume5);
		
	///��������
	printf("dAskPrice5 = %f\n",pQuote->dAskPrice5);
		
	///��������
	printf("nAskVolume5 = %d\n",pQuote->nAskVolume5);
}

int PrintDstData(DST_DATA_TYPE *pQuote)
{
	///������
	printf("TradingDay = %s\n",pQuote->TradingDay);
	
	///��Լ����
	printf("InstrumentID = %s\n",pQuote->InstrumentID);
	
	
	///���¼�
	printf("LastPrice = %f\n",pQuote->LastPrice);
	
	///�ϴν����
	printf("PreSettlementPrice = %f\n",pQuote->PreSettlementPrice);
	
	///������
	printf("PreClosePrice = %f\n",pQuote->PreClosePrice);
	
	///��ֲ���
	printf("PreOpenInterest = %f\n",pQuote->PreOpenInterest);
	
	///����
	printf("OpenPrice = %f\n",pQuote->OpenPrice);
	
	///��߼�
	printf("HighestPrice = %f\n",pQuote->HighestPrice);
	
	///��ͼ�
	printf("LowestPrice = %f\n",pQuote->LowestPrice);
	
	///����
	printf("Volume = %d\n",pQuote->Volume);
	
	///�ɽ����
	printf("Turnover = %f\n",pQuote->Turnover);
	
	///�ֲ���
	printf("OpenInterest = %f\n",pQuote->OpenInterest);
	
	///������
	printf("ClosePrice = %f\n",pQuote->ClosePrice);
	
	///���ν����
	printf("SettlementPrice = %f\n",pQuote->SettlementPrice);
	
	///��ͣ���
	printf("UpperLimitPrice = %f\n",pQuote->UpperLimitPrice);
	
	///��ͣ���
	printf("LowerLimitPrice = %d\n",pQuote->LowerLimitPrice);
	
	///����ʵ��
	printf("PreDelta = %f\n",pQuote->PreDelta);
	
	///����ʵ��
	printf("CurrDelta = %f\n",pQuote->CurrDelta);
	
	///����޸�ʱ��
	printf("UpdateTime = %s\n",pQuote->UpdateTime);
	
	///����޸ĺ���
	printf("UpdateMillisec = %d\n",pQuote->UpdateMillisec);
	
	///�����һ
	printf("BidPrice1 = %f\n",pQuote->BidPrice1);
	
	///������һ
	printf("BidVolume1 = %d\n",pQuote->BidVolume1);
	
	///������һ
	printf("AskPrice1 = %f\n",pQuote->AskPrice1);
	
	///������һ
	printf("AskVolume1 = %d\n",pQuote->AskVolume1);
	
	///����۶�
	printf("BidPrice2 = %f\n",pQuote->BidPrice2);
	
	///��������
	printf("BidVolume2 = %d\n",pQuote->BidVolume2);
	
	///�����۶�
	printf("AskPrice2 = %f\n",pQuote->AskPrice2);
	
	///��������
	printf("AskVolume2 = %d\n",pQuote->AskVolume2);
	
	///�������
	printf("BidPrice3 = %f\n",pQuote->BidPrice3);
	
	///��������
	printf("BidVolume3 = %d\n",pQuote->BidVolume3);
	
	///��������
	printf("AskPrice3 = %d\n",pQuote->AskPrice3);
	
	///��������
	printf("AskVolume3 = %d\n",pQuote->AskVolume3);
	
	///�������
	printf("BidPrice4 = %f\n",pQuote->BidPrice4);
	
	///��������
	printf("BidVolume4 = %d\n",pQuote->BidVolume4);
	
	///��������
	printf("AskPrice4 = %f\n",pQuote->AskPrice4);
	
	///��������
	printf("AskVolume4 = %d\n",pQuote->AskVolume4);
	
	///�������
	printf("BidPrice5 = %f\n",pQuote->BidPrice5);
	
	///��������
	printf("BidVolume5 = %d\n",pQuote->BidVolume5);
	
	///��������
	printf("AskPrice5 = %f\n",pQuote->AskPrice5);
	
	///��������
	printf("AskVolume5 = %d\n",pQuote->AskVolume5);
}


int main(int argc, char ** argv)
{
	long long idr = 0;	
	long long idw = 0;	
	SRC_DATA_TYPE srcData;
	DST_DATA_TYPE dstData;
	SRC_DATA_TYPE *pSrc = &srcData; 
	DST_DATA_TYPE *pDst = &dstData;
	dat_head header[HeaderSize] = {0};
	char timestamp[TIMESTAMP_SIZE] = {0};
	char *pChar = (char *)pSrc;
	//char data[sizeof(SRC_DATA_TYPE) + TIMESTAMP_SIZE] = {0};
	
	char m_pSrcFilePath[256] = "/home/cuiju/datafeed/data/IF1508.dat";
	char m_pDstFilePath[256] = "/home/cuiju/datafeed/data/one_quote.dat";
	char instr[32] =  "IF1504";
	
	FILE* fp = fopen(m_pSrcFilePath, "rb");	
	if(NULL == fp)
	{
		printf("Failed to open file: %s\n",m_pSrcFilePath);
		return -1;
	}


	FILE* fpw = fopen(m_pDstFilePath, "wb");	
	if(NULL == fpw)
	{
		printf("Failed to open file: %s\n",m_pDstFilePath);
		return -1;
	}


	//read file head
	fread((char *)header,HeaderSize,1,fp);

	while(1)
	{
		if(TIMESTAMP_SIZE != fread(timestamp,1,TIMESTAMP_SIZE,fp))
		{
			if(0 == feof(fp))
			{
				printf("Datafetcher read error.\n");
			}
			else
			{
				printf("Datafetcher read over.\n");
			}
			
			fclose(fp);
			break;
		}

		if(sizeof(SRC_DATA_TYPE) != fread(pChar,1,sizeof(SRC_DATA_TYPE),fp))
		{
			printf("Datafetcher read error.\n");
		}
		
		pSrc = (SRC_DATA_TYPE *)pChar;
		ConvertDataForm(pDst,pSrc);
		
		//PrintSrcData(pSrc);
		//printf("********************************************************\n",idr);

		if(idr==0 || idr==2|| idr==3|| idr==4)
		{		
			idw++;
			if(1 != fwrite(pDst,sizeof(DST_DATA_TYPE),1,fpw))
			{
				printf("Failed to write data to file:\n",m_pDstFilePath);
				fclose(fpw);
			}
			printf("#######################Quote %lld#######################\n",idw);
			PrintDstData(pDst);
		}
		
		idr++;
		printf("\n");

		if(1 == idw)
		{
			fclose(fp);
			fclose(fpw);
			break;
		}
		//sleep(1);
	}

	return 0;

}

