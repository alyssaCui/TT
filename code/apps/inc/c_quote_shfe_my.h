#ifndef C_QUOTE_SHFE_MY_H
#define C_QUOTE_SHFE_MY_H

#include "BasicDataType.h"

// ��λ�����ݶ�Ϊ30��
#define  MY_SHFE_QUOTE_PRICE_POS_COUNT 30

///MY�Զ���������������ݽṹ
struct shfe_my_quote
{
	///������
	char	TradingDay[9];
	///���������
	char	SettlementGroupID[9];
	///������
	int	SettlementID;
	///���¼�
	double	LastPrice;
	///�����
	double	PreSettlementPrice;
	///������
	double	PreClosePrice;
	///��ֲ���
	double	PreOpenInterest;
	///����
	double	OpenPrice;
	///��߼�
	double	HighestPrice;
	///��ͼ�
	double	LowestPrice;
	///����
	int	Volume;
	///�ɽ����
	double	Turnover;
	///�ֲ���
	double	OpenInterest;
	///������
	double	ClosePrice;
	///�����
	double	SettlementPrice;
	///��ͣ���
	double	UpperLimitPrice;
	///��ͣ���
	double	LowerLimitPrice;
	///����ʵ��
	double	PreDelta;
	///����ʵ��
	double	CurrDelta;
	///����޸�ʱ��
	char	UpdateTime[9];
	///����޸ĺ���
	int	UpdateMillisec;
	///��Լ����
	char	InstrumentID[31];
	///�����һ
	double	BidPrice1;
	///������һ
	int	BidVolume1;
	///������һ
	double	AskPrice1;
	///������һ
	int	AskVolume1;
	///����۶�
	double	BidPrice2;
	///��������
	int	BidVolume2;
	///�����۶�
	double	AskPrice2;
	///��������
	int	AskVolume2;
	///�������
	double	BidPrice3;
	///��������
	int	BidVolume3;
	///��������
	double	AskPrice3;
	///��������
	int	AskVolume3;
	///�������
	double	BidPrice4;
	///��������
	int	BidVolume4;
	///��������
	double	AskPrice4;
	///��������
	int	AskVolume4;
	///�������
	double	BidPrice5;
	///��������
	int	BidVolume5;
	///��������
	double	AskPrice5;
	///��������
	int	AskVolume5;
	///ҵ��������
	char	ActionDay[9];

	// ����Ϊ���ڽ�������һ�����ݽṹ�����沿��Ϊ�������ֶ�
	int   data_flag; // ���ݱ�ǣ�1���г�������Ч��2��ί��������Ч��3���г������ί�����鶼��Ч

	// ������30��ί�����ݣ���������ʱ��price��volume ��ȡֵ 0��
    unsigned short  start_index;  // ���ݵ���ʼ�����ţ�0
    unsigned short  data_count;   // ί�����ݵ�λ����30
	double buy_price[MY_SHFE_QUOTE_PRICE_POS_COUNT];
	int    buy_volume[MY_SHFE_QUOTE_PRICE_POS_COUNT];
	double sell_price[MY_SHFE_QUOTE_PRICE_POS_COUNT];
	int    sell_volume[MY_SHFE_QUOTE_PRICE_POS_COUNT];

	// ��Ȩƽ���Լ�ί����������
    unsigned int   buy_total_volume;               //��ί������
    unsigned int   sell_total_volume;              //��ί������
    double         buy_weighted_avg_price;   //��Ȩƽ��ί��۸�
    double         sell_weighted_avg_price;  //��Ȩƽ��ί���۸�
};

#endif
