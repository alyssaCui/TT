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

/// 中金所深度市场行情
struct CFfexFtdcDepthMarketData
{
    char szTradingDay[9];        ///< 交易日
    char szSettlementGroupID[9]; ///< 结算组代码
    int nSettlementID;          ///< 结算编号
    double dLastPrice;             ///< 最新价
    double dPreSettlementPrice;    ///< 昨结算
    double dPreClosePrice;         ///< 昨收盘
    double dPreOpenInterest;       ///< 昨持仓量
    double dOpenPrice;             ///< 今开盘
    double dHighestPrice;          ///< 最高价
    double dLowestPrice;           ///< 最低价
    int nVolume;                ///< 数量
    double dTurnover;              ///< 成交金额
    double dOpenInterest;          ///< 持仓量
    double dClosePrice;            ///< 今收盘
    double dSettlementPrice;       ///< 今结算
    double dUpperLimitPrice;       ///< 涨停板价
    double dLowerLimitPrice;       ///< 跌停板价
    double dPreDelta;              ///< 昨虚实度
    double dCurrDelta;             ///< 今虚实度
    char szUpdateTime[9];        ///< 最后修改时间
    int nUpdateMillisec;        ///< 最后修改毫秒
    char szInstrumentID[31];     ///< 合约代码
    double dBidPrice1;             ///< 申买价一
    int nBidVolume1;            ///< 申买量一
    double dAskPrice1;             ///< 申卖价一
    int nAskVolume1;            ///< 申卖量一
    double dBidPrice2;             ///< 申买价二
    int nBidVolume2;            ///< 申买量二
    double dAskPrice2;             ///< 申卖价二
    int nAskVolume2;            ///< 申卖量二
    double dBidPrice3;             ///< 申买价三
    int nBidVolume3;            ///< 申买量三
    double dAskPrice3;             ///< 申卖价三
    int nAskVolume3;            ///< 申卖量三
    double dBidPrice4;             ///< 申买价四
    int nBidVolume4;            ///< 申买量四
    double dAskPrice4;             ///< 申卖价四
    int nAskVolume4;            ///< 申卖量四
    double dBidPrice5;             ///< 申买价五
    int nBidVolume5;            ///< 申买量五
    double dAskPrice5;             ///< 申卖价五
    int nAskVolume5;            ///< 申卖量五
};

#pragma pack()

void ConvertDataForm(DST_DATA_TYPE *pDst, SRC_DATA_TYPE *pSrc)
{
	strcpy(pDst->TradingDay,pSrc->szTradingDay);
	strcpy(pDst->InstrumentID,pSrc->szInstrumentID);

	pDst->LastPrice = pSrc->dLastPrice; ///< 最新价
	pDst->PreSettlementPrice = pSrc->dPreSettlementPrice; ///< 昨结算
	pDst->PreClosePrice = pSrc->dPreClosePrice;///< 昨收盘
	pDst->PreOpenInterest = pSrc->dPreOpenInterest; ///< 昨持仓量
	pDst->OpenPrice = pSrc->dOpenPrice; ///< 今开盘
	pDst->HighestPrice = pSrc->dHighestPrice; ///< 最高价
	pDst->LowestPrice = pSrc->dLowestPrice;  ///< 最低价
	pDst->Volume = pSrc->nVolume; ///< 数量

	pDst->Turnover = pSrc->dTurnover;  ///< 成交金额
	pDst->OpenInterest = pSrc->dOpenInterest;  ///< 持仓量
	pDst->ClosePrice = pSrc->dClosePrice;   ///< 今收盘
	pDst->SettlementPrice = pSrc->dSettlementPrice;  ///< 今结算
	pDst->UpperLimitPrice = pSrc->dUpperLimitPrice;    ///< 涨停板价
	pDst->LowerLimitPrice = pSrc->dLowerLimitPrice;  ///< 跌停板价
	pDst->PreDelta = pSrc->dPreDelta;   ///< 昨虚实度
	pDst->CurrDelta = pSrc->dCurrDelta; ///< 今虚实度

	strcpy(pDst->UpdateTime,pSrc->szUpdateTime);///< 最后修改时间
	pDst->UpdateMillisec = pSrc->nUpdateMillisec;  ///< 最后修改毫秒
	
	pDst->BidPrice1 = pSrc->dBidPrice1;   ///< 申买价一
	pDst->BidVolume1 = pSrc->nBidVolume1; ///< 申买量一
	pDst->AskPrice1 = pSrc->dAskPrice1;   ///< 申卖价一
	pDst->AskVolume1 = pSrc->nAskVolume1; ///< 申卖量一
	pDst->BidPrice2 = pSrc->dBidPrice2;   ///< 申买价二
	pDst->BidVolume2 = pSrc->nBidVolume2; ///< 申买量二
	pDst->AskPrice2 = pSrc->dAskPrice2;   ///< 申卖价二
	pDst->AskVolume2 = pSrc->nAskVolume2; ///< 申卖量二
	pDst->BidPrice3 = pSrc->dBidPrice3;    ///< 申买价三
	pDst->BidVolume3 = pSrc->nBidVolume3; ///< 申买量三
	pDst->AskPrice3 = pSrc->dAskPrice3; ///< 申卖价三
	pDst->AskVolume3 = pSrc->nAskVolume3; ///< 申卖量三
	pDst->BidPrice4 = pSrc->dBidPrice4; ///< 申买价四
	pDst->BidVolume4 = pSrc->nBidVolume4; ///< 申买量四
	pDst->AskPrice4 = pSrc->dAskPrice4; ///< 申卖价四
	pDst->AskVolume4 = pSrc->nAskVolume4; ///< 申卖量四
	pDst->BidPrice5 = pSrc->dBidPrice5;  ///< 申买价五
	pDst->BidVolume5 = pSrc->nBidVolume5; ///< 申买量五
	pDst->AskPrice5 = pSrc->dAskPrice5; ///< 申卖价五
	pDst->AskVolume5 = pSrc->nAskVolume5; ///< 申卖量五
}

int PrintSrcData(SRC_DATA_TYPE *pQuote)
{
	///交易日
	printf("szTradingDay = %s\n",pQuote->szTradingDay);
	
	/// 结算组代码
	printf("szSettlementGroupID = %s\n",pQuote->szSettlementGroupID);

	/// 结算编号
	printf("nSettlementID = %d\n",pQuote->nSettlementID);		
		
	///最新价
	printf("dLastPrice = %f\n",pQuote->dLastPrice);
		
	///上次结算价
	printf("dPreSettlementPrice = %f\n",pQuote->dPreSettlementPrice);
		
	///昨收盘
	printf("dPreClosePrice = %f\n",pQuote->dPreClosePrice);
		
	///昨持仓量
	printf("dPreOpenInterest = %f\n",pQuote->dPreOpenInterest);
		
	///今开盘
	printf("dOpenPrice = %f\n",pQuote->dOpenPrice);
		
	///最高价
	printf("dHighestPrice = %f\n",pQuote->dHighestPrice);
		
	///最低价
	printf("dLowestPrice = %f\n",pQuote->dLowestPrice);
		
	///数量
	printf("nVolume = %d\n",pQuote->nVolume);
		
	///成交金额
	printf("dTurnover = %f\n",pQuote->dTurnover);
		
	///持仓量
	printf("dOpenInterest = %f\n",pQuote->dOpenInterest);
		
	///今收盘
	printf("dClosePrice = %f\n",pQuote->dClosePrice);
		
	///本次结算价
	printf("dSettlementPrice = %f\n",pQuote->dSettlementPrice);
		
	///涨停板价
	printf("dUpperLimitPrice = %f\n",pQuote->dUpperLimitPrice);
		
	///跌停板价
	printf("dLowerLimitPrice = %d\n",pQuote->dLowerLimitPrice);
		
	///昨虚实度
	printf("dPreDelta = %f\n",pQuote->dPreDelta);
		
	///今虚实度
	printf("dCurrDelta = %f\n",pQuote->dCurrDelta);
		
	///最后修改时间
	printf("szUpdateTime = %s\n",pQuote->szUpdateTime);
		
	///最后修改毫秒
	printf("nUpdateMillisec = %d\n",pQuote->nUpdateMillisec);

	///合约代码
	printf("szInstrumentID = %s\n",pQuote->szInstrumentID);
		
	///申买价一
	printf("dBidPrice1 = %f\n",pQuote->dBidPrice1);
		
	///申买量一
	printf("nBidVolume1 = %d\n",pQuote->nBidVolume1);
		
	///申卖价一
	printf("dAskPrice1 = %f\n",pQuote->dAskPrice1);
		
	///申卖量一
	printf("nAskVolume1 = %d\n",pQuote->nAskVolume1);
		
	///申买价二
	printf("dBidPrice2 = %f\n",pQuote->dBidPrice2);
		
	///申买量二
	printf("nBidVolume2 = %d\n",pQuote->nBidVolume2);
		
	///申卖价二
	printf("dAskPrice2 = %f\n",pQuote->dAskPrice2);
		
	///申卖量二
	printf("nAskVolume2 = %d\n",pQuote->nAskVolume2);
		
	///申买价三
	printf("dBidPrice3 = %f\n",pQuote->dBidPrice3);
		
	///申买量三
	printf("nBidVolume3 = %d\n",pQuote->nBidVolume3);
		
	///申卖价三
	printf("dAskPrice3 = %d\n",pQuote->dAskPrice3);
		
	///申卖量三
	printf("nAskVolume3 = %d\n",pQuote->nAskVolume3);
		
	///申买价四
	printf("dBidPrice4 = %f\n",pQuote->dBidPrice4);
	
	///申买量四
	printf("nBidVolume4 = %d\n",pQuote->nBidVolume4);
		
	///申卖价四
	printf("dAskPrice4 = %f\n",pQuote->dAskPrice4);
		
	///申卖量四
	printf("nAskVolume4 = %d\n",pQuote->nAskVolume4);
		
	///申买价五
	printf("dBidPrice5 = %f\n",pQuote->dBidPrice5);
		
	///申买量五
	printf("nBidVolume5 = %d\n",pQuote->nBidVolume5);
		
	///申卖价五
	printf("dAskPrice5 = %f\n",pQuote->dAskPrice5);
		
	///申卖量五
	printf("nAskVolume5 = %d\n",pQuote->nAskVolume5);
}

int PrintDstData(DST_DATA_TYPE *pQuote)
{
	///交易日
	printf("TradingDay = %s\n",pQuote->TradingDay);
	
	///合约代码
	printf("InstrumentID = %s\n",pQuote->InstrumentID);
	
	
	///最新价
	printf("LastPrice = %f\n",pQuote->LastPrice);
	
	///上次结算价
	printf("PreSettlementPrice = %f\n",pQuote->PreSettlementPrice);
	
	///昨收盘
	printf("PreClosePrice = %f\n",pQuote->PreClosePrice);
	
	///昨持仓量
	printf("PreOpenInterest = %f\n",pQuote->PreOpenInterest);
	
	///今开盘
	printf("OpenPrice = %f\n",pQuote->OpenPrice);
	
	///最高价
	printf("HighestPrice = %f\n",pQuote->HighestPrice);
	
	///最低价
	printf("LowestPrice = %f\n",pQuote->LowestPrice);
	
	///数量
	printf("Volume = %d\n",pQuote->Volume);
	
	///成交金额
	printf("Turnover = %f\n",pQuote->Turnover);
	
	///持仓量
	printf("OpenInterest = %f\n",pQuote->OpenInterest);
	
	///今收盘
	printf("ClosePrice = %f\n",pQuote->ClosePrice);
	
	///本次结算价
	printf("SettlementPrice = %f\n",pQuote->SettlementPrice);
	
	///涨停板价
	printf("UpperLimitPrice = %f\n",pQuote->UpperLimitPrice);
	
	///跌停板价
	printf("LowerLimitPrice = %d\n",pQuote->LowerLimitPrice);
	
	///昨虚实度
	printf("PreDelta = %f\n",pQuote->PreDelta);
	
	///今虚实度
	printf("CurrDelta = %f\n",pQuote->CurrDelta);
	
	///最后修改时间
	printf("UpdateTime = %s\n",pQuote->UpdateTime);
	
	///最后修改毫秒
	printf("UpdateMillisec = %d\n",pQuote->UpdateMillisec);
	
	///申买价一
	printf("BidPrice1 = %f\n",pQuote->BidPrice1);
	
	///申买量一
	printf("BidVolume1 = %d\n",pQuote->BidVolume1);
	
	///申卖价一
	printf("AskPrice1 = %f\n",pQuote->AskPrice1);
	
	///申卖量一
	printf("AskVolume1 = %d\n",pQuote->AskVolume1);
	
	///申买价二
	printf("BidPrice2 = %f\n",pQuote->BidPrice2);
	
	///申买量二
	printf("BidVolume2 = %d\n",pQuote->BidVolume2);
	
	///申卖价二
	printf("AskPrice2 = %f\n",pQuote->AskPrice2);
	
	///申卖量二
	printf("AskVolume2 = %d\n",pQuote->AskVolume2);
	
	///申买价三
	printf("BidPrice3 = %f\n",pQuote->BidPrice3);
	
	///申买量三
	printf("BidVolume3 = %d\n",pQuote->BidVolume3);
	
	///申卖价三
	printf("AskPrice3 = %d\n",pQuote->AskPrice3);
	
	///申卖量三
	printf("AskVolume3 = %d\n",pQuote->AskVolume3);
	
	///申买价四
	printf("BidPrice4 = %f\n",pQuote->BidPrice4);
	
	///申买量四
	printf("BidVolume4 = %d\n",pQuote->BidVolume4);
	
	///申卖价四
	printf("AskPrice4 = %f\n",pQuote->AskPrice4);
	
	///申卖量四
	printf("AskVolume4 = %d\n",pQuote->AskVolume4);
	
	///申买价五
	printf("BidPrice5 = %f\n",pQuote->BidPrice5);
	
	///申买量五
	printf("BidVolume5 = %d\n",pQuote->BidVolume5);
	
	///申卖价五
	printf("AskPrice5 = %f\n",pQuote->AskPrice5);
	
	///申卖量五
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

