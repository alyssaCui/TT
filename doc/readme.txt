1.����һ��������Ҫ����ctp��login�ӿ��⣬������鲻��Ҫ������ص�login/logout������
  
   
 ��20150206��
	1.SessionID
		�����������������ڻ���˾ͨ����ϵͳ��������ز���ʵ�����Ƶġ�
		���������õ�����£�Ĭ����������Ϊ��
		1����һ�����ӻỰ��Session���У�ÿ���ͻ���ÿ�������ֻ�ܷ���6 �ʽ�����ص�ָ������������ȣ���
		2��ͬһ���˻�ͬʱ���ֻ�ܽ���6 ���Ự��Session������Ŀǰ��֧�ֵ��˻���Ӧ�����Ự���ɣ�
		ע�⣺����������������ʱ�������д��󷵻أ�ֻ�Ǳ����ᴦ���Ŷ�״̬���ȴ����������ư���������������������ѯ�����������������ʵ���ǳ��������д��󷵻أ�ֵ��2��3��
  
		���ʣ�	1��quote�������뽻������SessionID�Ƿֿ��ģ�
				�ֿ���
				2��ͬһ���˻�ͬʱ��ཨ������SessionID��
				��������
   
   ���������ƣ�1�����û�login���û���sessionID�Ķ�Ӧ����ʱֻ֧�ֵ��û���
                 2�����û����login�����µ�¼��SessionID ������
				     
	2.���µ�¼��SessionID �����ã����MaxOrderRef һ��Ҳ�����´�0 ����
		���ʣ�ÿ��Session��MaxOrderRef���������㣿	
		MaxOrderRef���ʺŶ�Ӧ���ڶϿ�һ��ʱ���ڲ��ᱻ����
  
	3.��������������ͽ��׵ĵ�¼������ͬ��CThostFtdcRspUserLoginField
		���ʣ��������Ϣ��ͬ��
		///ǰ�ñ��
		TThostFtdcFrontIDType	FrontID;
		///�Ự���
		TThostFtdcSessionIDType	SessionID;
		///��󱨵�����
		TThostFtdcOrderRefType	MaxOrderRef;
		
		����ͽ��׵���Ϣ�Ƕ�����
		MaxOrderRef������Щ����ϵͳ�ָʾ������յ�ʱ��������

		
��20150212��  
  ���������ƣ�
  1��֧�������������������֧��ÿ����6��������
  
  ��ʵ�ֵ�ҵ������
  1������������
  2�����µ�¼��SessionID �����ã����MaxOrderRef һ��Ҳ�����´�0 ������MaxOrderRef���ʺŶ�Ӧ���ڶϿ�һ��ʱ���ڲ��ᱻ���ã�
  
  
  
��20150213��  
  
	1.֧�������������������֧��ÿ����6��������  
		������
		1��ģ��so��sock����Ϊ������
		2��if(ATF_SUCC == pstTraderApi->m_OrderQueue.PopOneData(&pstTraderApi->m_inputOrder)) ����������while��1��ѭ����
		3���ѵȴ��µ��Ļظ����Ƿ���״̬���״̬��ΪSTATUS_TRADE_RSP_ORDER_INSERT��
			��STATUS_TRADE_REQ_SETTLEMENT_CONFIRM״̬�����󣬾Ͱ�״̬��ΪSTATUS_TRADE_RSP_ORDER_INSERT
		
	2.���Ʋ��Ա���

	3.��������ͳ��
		1������ķ�������
		2�������Ľ�������
		3�����鷢�������յ�������ʱ��
	
��20150216�� 	
	ctp �����������̣�ʵ��ϸ�ںͲ�ͬ����������ͬ�ڻ���˾�����й�
	
	����
		ReqOrderInsert
		OnRspOrderInsert - ��̨�ܾ� �����ֶ���д�����ʽ��㣬��λ���㣩
		OnRtnOrder - ��̨�ر� ��û��ί�кţ�
		OnErrRtnOrderInsert - �������ܾ� ��һ���ͬʱ�ر� OnRspOrderInsert �������ֹ����صĳ�����
		OnRtnOrder - �������ر� ����ί�кţ�
		OnRtnOrder  OnRtnTrade ������гɽ��������λ� OnRtnOrder��OnRtnTrade��
		
	����
		ReqOrderAction
		OnRspOrderAction - ��̨�ܾ� �����ѳ�����̨�Ҳ����ñ�����
		OnRtnOrder - �ر���ǰ������״̬�������� ActiveUserID ������Ϊ���ַ���
		OnErrRtnOrderAction - �������ܾ� ��һ���ͬʱ�ر� OnRspOrderAction �������ֹ����صĳ�����
		OnRtnOrder - �ѳ�״̬

	ʾ����־��
    \\192.168.1.6\share\alyssa\ctp_����������־������\mytrader_cffexl_20150216.log

    ʹ��M1ģ�ͣ�ctp��������˺ŵĲ�����־���������Ƚ϶࣬���Կ���

��20150226�� 
	1.	so����ϵͳ����ʱ���ͳ��----�ѽ��
	
��20150227�� 
	1.	�����������ȡ˳��----�ѽ��
	2.	֧��DisconnectClient----�ѽ��
	3.	ATF��so��ͨ��----UDP����Ҫ֪�����ؾ����IP
		1��ATF������
		2��so��ATF����TCP���ӣ���ȡso������IP
		3��ATF��so��Э��UDP server��port��ATF��so������UDP server��������
	4.	֧�ֶ��������----δ��ȫ���
	
��20150228--20150301�� 	
	1.	֧�ֶ��������
		1�����⣺�ڷ�������ǰ����DisconnectClient----���޸�
			��ǰ��ʵ�֣�Quote run()��sendQuote()��һ���߳����һ��ѭ���з����������������������һ��ѭ����ִ����DisconnectClient
			�޸�����������MSGID_S2C_QUOTE_SEND��Ϣ���ڶ�������Ϣʱ�������飬���������鷢�������Ŵ�����һ��op
		2��QuoteSvr��Exchge�����Ϳ�ʼ��ͬ��----���޸�
		
��20150302�� 
	1.	so��log�ļ�
	2.	ATF����ҵ���ϸ��
		1�����LOGIN�ṹ��
		2�������ṹ��
			StatusMsg=�������ύ        StatusMsg=δ�ɽ�            StatusMsg=δ�ɽ�            StatusMsg=�ѳ���            StatusMsg=δ�ɽ�            StatusMsg=ȫ���ɽ�
			OrderRef=10000019                                                                                               OrderRef=10000013
			VolumeTotalOriginal=1                
			VolumeTraded=0                                                                                                                               VolumeTraded=1                      
			VolumeTotal=1                                                                                                                                VolumeTotal=0                       
			RequestID=0       
			OrderLocalID=3058                                                                                               OrderLocalID=3058    
			OrderSubmitStatus=0         OrderSubmitStatus=3                                     OrderSubmitStatus=5                                      OrderSubmitStatus=3
			NotifySequence=0            
			SequenceNo=6034             SequenceNo=6034             SequenceNo=6035                                         SequenceNo=5994
			OrderStatus=a               OrderStatus=3                                           OrderStatus=5               OrderStatus=3                OrderStatus=0
			OrderSysID=                 OrderSysID=781259                                                                   OrderSysID=781031
	                                                            ActiveUserID=41003991                                   ActiveUserID=
	    
 ��20150303��  
	1.	����ҵ���ϸ��
		1��.	ATFĿǰ���ڱ����Ĵ����������ǰ�����������ܽ�����һ������
				���⣺�����Ⱥ�����������ĳ���û������------�ѽ��
				�������������һ����������vector��ű�����ÿ�δ�����ʱ��vector��ȡ����һ������
						  ����һ����������vector��ű�����ÿ�δ�����ʱ��vector��ȡ����һ������
		
		2��.	���ڳ�����ATFĿǰ������д���ķ�ʽ����------�ѽ��(20150304)
				���⣺���ܸ���ʵ�ʵ����������Ӧ�Ĵ���
				�������������һ��map����ѳɽ��ı��������յ���������ʱ��map�м���Ƿ��иõ�
				
				
 ��20150304��  
	1.	����ҵ���ϸ��				
		3��3�����к����漰�ı���������------�ѽ��
		
		
 ��20150305��  
	1.	����ҵ���ϸ��	
		4��so��ͳ��ϵͳ���͸�ATF
		
		
		
 ��20150308��  
	1.	so�Ĵ����makefile�ļ�����
	2.	ATF��so�����ܲ���
		���⣺1��ATF��core����core�ļ�ָʾfread()
			  2��CPU���Ȳ����⣬ҵ���̲߳��ܵõ���ʱ����
		��չ��SO�����������߳�����usleep(1000)�����ͷ�CPU
				Ŀǰ���鷢�ͼ��20���룬�������ܲ��Թ���û�г����κ�����

	
 ��20150309��
	���⣺ 
		1.�������ԣ����鷢�ͼ��С��5����ʱ�����ܲ��Գ�������
		2.�����Բ��ԣ����µ�6��ʱ��exchgSvr�յ��ı��������ԣ���ʱ2������ʱ3����
		3.�˿�����Ϊ9Wʱ����netstat�����Ķ˿ں�ʵ�ʵĲ�ͬ ---- ��Ϊ5W���
		4.��ȡ��������ʱ����ȷ ---- ��������д���ˣ���
		
 ��20150310��
		1.�������ԣ����鷢�ͼ��1����ʱ�����ܲ��Գ�������
			��λ��pop��error��push��error�����������������ȷ�������ڴ�Э��ջȡ����push̫����
			�޸ģ�	1.  udp server��timeout����ԭ����2���Ϊ100����
					2. QUEUE�Ĵ�С��1024��Ϊ10240


		
		2.�����Բ��ԣ����µ�6��ʱ��exchgSvr�յ��ı��������ԣ���ʱ2������ʱ3����
		  ---- �����ܲ��Բ��ֵĽ��պ�����RcvMsgFromClient()��Ϊ��TcpConnector����
		  ---- ���ܲ��Բ���Ҫ��ҪҲ�ģ�
		       �����Ժ�,��so����֡��ܵ��رա��Ĵ��󣬲�֪��ʲô����
		  
		  
		2.���鷢��100΢��ʱ���ַ�����������atf����core
		
 ��20150311��	
		1.	���鷢��100΢��ʱ���ַ�����������atf����core
			���⣺core
			��λ�������ܲ���ʱ��ͨ��gdb����m_traderMode���޸�ΪLIMITED��ͨ��watch������λ������û�з���ʲôʱ���޸ĵ�
			��������ExchgServer���m_tcpbuf��ջ�ĵ����ϣ�core������ͽ����
			
			���⣺���鲻�ܼ�ʱ�Ĵ���
			��λ��ͨ������log���֣�quote��order��queue����pop���󣬶���push������˶�λ����updserver��Э��ջȡ������ٶ���
			��������timeout��Ϊ5us��ȥ��ѭ����usleep
			��չ�������鷢�ͼ����Ϊ10us��Ҳ���Լ�ʱ����
				  ����ͳ�Ƴ��������鷢�ͼ����СֵΪ112us��quote��order��queue��pop��������17W���ϣ�˵������Ĵ����������кܴ����
			�����⣺����ʹͳ�Ƴ��������鷢�ͼ����ʵ�ʵ�һ�£�
			
		2.	ѧϰ�ڻ����׻���֪ʶ
		
		
 ��20150311��	
		1. ѧϰ�ڻ����׻���֪ʶ
		
		2.	���⣺���鷢�ͼ����Ϊ10us������ͳ�Ƴ��������鷢�ͼ��ƽ��ֵΪ122us��
			����1�����̰߳�CPU��CPUʹ����С��5%�����Ƕ�������Ľ��ռ��û��Ӱ�졣
			
 ��20150312 -- 20150315��	
		1.	һ����������ִ�����ϵͳ���ӡ 9(Bad file descriptor)
		2.	��������ִ�������ص�״̬δ����
	
 ��20150325��	
		��gtestʵ�ֲ���so������֤��������
		
 ��20150326��	
		��gtestʵ�ֲ���so:
		1.	���ļ����ȡҪִ�еĲ�������
		
 ��20150408��
 ///��ѯͶ���ֲ߳�
struct CThostFtdcQryInvestorPositionField
{
	///���͹�˾����
	TThostFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TThostFtdcInvestorIDType	InvestorID;
	///��Լ����
	TThostFtdcInstrumentIDType	InstrumentID;
};

	///�����ѯͶ���ֲ߳�
	virtual int ReqQryInvestorPosition(CThostFtdcQryInvestorPositionField *pQryInvestorPosition, int nRequestID) ;
 
	///�����ѯͶ���ֲ߳���Ӧ
	virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};
	
��20150608��
	TO DO:
	1������ִ�н����ȡ
	2��������sock
	3��CTP SO�ӱ����ļ���ȡ�������ݣ�����ͨ��UDP
	
��20150615��
 TO DO:
	1)ATT֧���˶���Լ
	2)���ݲ��Գ���ʵ��ģ�����so
	  a)�յ�����--���ͽ�����Ϣ--���ջر�
	  b)�յ�����--�µ�--����
	  c)�յ�����--��ӡ����

	
��20150617��
 TO DO:
	1)ATT֧���˶���Լ	
	
��20150619��
 TO DO:
	1)ģ��������Ӵ˲���������ÿ��ͬʱ������������������źţ�����8����10����trader������������������8��������2����ƽ��8��
	2)����������У�����ڴ�offset�Ƿ���ȷ
	3)budy/slab/kmalloc �鿴�ں��ڴ�
	4)�������в�����������Ӳ�������
	5)����������������ִ��
	6)�������ݵ�·��
	7)����Ͽ�������trader�Ƿ�����µ�¼�����ģ���
	8)ͨ���Ͽ�������trader�Ƿ�����µ�¼����

	
��20150619��
 TO DO:	
	1)FAK����CTPУ��ͽ�����У�鶼ͨ�������1�ֶ�û�гɽ����Ƿ��б����ر���֪δ�ɽ���
		FAK������ȫδ�ɽ����᷵��һ�����ѳ������Ļر�
				���ֳɽ�����δ�ɽ��Ĳ��ֻ᷵��һ�����ѳ������Ļر�
		FOK����δ�ɽ���Ҳ�᷵��һ�����ѳ������ر�
	ȷ��ATT�Ƿ���ȷʵ�ִ�ҵ��

	2��MARKET��
	һ��Market������10�֣��г�����һ4�ֺ�����6�֣�ȫ���ɽ����յ��������׻ر�
	һ��Market������10�֣���ǰ�г�����һ4�֣�4�����̳ɽ��յ�һ�����׻ر�����һ������յ�6�ֵĽ��׻ر����۸�Ϊ������ͣ�ۡ�
		
	SIG_STATUS_SUCCESS = 0, /* ����ȫ���ɽ� */
	SIG_STATUS_ENTRUSTED, /* ����ί�гɹ� */
	SIG_STATUS_PARTED, /* �������ֳɽ� */
	SIG_STATUS_CANCEL, /* ���������� */
	SIG_STATUS_REJECTED, /* �������ܾ� */

	