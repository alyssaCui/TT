1.除了一档行情需要调用ctp的login接口外，别的行情不需要进行相关的login/logout工作。
  
   
 【20150206】
	1.SessionID
		报单流量限制是由期货公司通过在系统中配置相关参数实现限制的。
		不进行配置的情况下，默认流量限制为：
		1）在一个连接会话（Session）中，每个客户端每秒钟最多只能发送6 笔交易相关的指令（报单，撤单等）。
		2）同一个账户同时最多只能建立6 个会话（Session）。（目前仅支持单账户对应单个会话即可）
		注意：报单操作超过限制时，不会有错误返回，只是报单会处于排队状态，等待处理。该限制包含包括报单，撤单，查询等所有请求操作。（实际是超过限制有错误返回，值是2或3）
  
		疑问：	1）quote服务器与交易所的SessionID是分开的？
				分开的
				2）同一个账户同时最多建立几个SessionID？
				单个即可
   
   待解决的设计：1）多用户login，用户与sessionID的对应（暂时只支持单用户）
                 2）单用户多次login，重新登录后SessionID 会重置
				     
	2.重新登录后SessionID 会重置，因此MaxOrderRef 一般也会重新从0 计数
		疑问：每个Session的MaxOrderRef都单独计算？	
		MaxOrderRef与帐号对应，在断开一段时间内不会被重置
  
	3.对于行情服务器和交易的登录，用相同的CThostFtdcRspUserLoginField
		疑问：下面的信息相同吗？
		///前置编号
		TThostFtdcFrontIDType	FrontID;
		///会话编号
		TThostFtdcSessionIDType	SessionID;
		///最大报单引用
		TThostFtdcOrderRefType	MaxOrderRef;
		
		行情和交易的信息是独立的
		MaxOrderRef，在有些行情系统里，指示行情接收的时间间隔限制

		
【20150212】  
  待解决的设计：
  1）支持连续多个报单（可以支持每秒钟6个报单）
  
  待实现的业务流程
  1）撤单的流程
  2）重新登录后SessionID 会重置，因此MaxOrderRef 一般也会重新从0 计数（MaxOrderRef与帐号对应，在断开一段时间内不会被重置）
  
  
  
【20150213】  
  
	1.支持连续多个报单（可以支持每秒钟6个报单）  
		方案：
		1）模拟so的sock设置为非阻塞
		2）if(ATF_SUCC == pstTraderApi->m_OrderQueue.PopOneData(&pstTraderApi->m_inputOrder)) 放在最外层的while（1）循环里
		3）把等待下单的回复还是放在状态机里，状态设为STATUS_TRADE_RSP_ORDER_INSERT。
			当STATUS_TRADE_REQ_SETTLEMENT_CONFIRM状态结束后，就把状态设为STATUS_TRADE_RSP_ORDER_INSERT
		
	2.完善测试报告

	3.完善事务统计
		1）行情的发送数量
		2）报单的接收数量
		3）行情发出到接收到报单的时间
	
【20150216】 	
	ctp 报撤单的流程，实际细节和不同交易所、不同期货公司部署有关
	
	报单
		ReqOrderInsert
		OnRspOrderInsert - 柜台拒绝 （如字段填写错误，资金不足，仓位不足）
		OnRtnOrder - 柜台回报 （没有委托号）
		OnErrRtnOrderInsert - 交易所拒绝 （一般会同时回报 OnRspOrderInsert ，但出现过不回的场景）
		OnRtnOrder - 交易所回报 （有委托号）
		OnRtnOrder  OnRtnTrade （如果有成交，会依次回 OnRtnOrder、OnRtnTrade）
		
	撤单
		ReqOrderAction
		OnRspOrderAction - 柜台拒绝 （如已撤，柜台找不到该报单）
		OnRtnOrder - 回报当前报单的状态，撤单后 ActiveUserID 将不再为空字符串
		OnErrRtnOrderAction - 交易所拒绝 （一般会同时回报 OnRspOrderAction ，但出现过不回的场景）
		OnRtnOrder - 已撤状态

	示例日志：
    \\192.168.1.6\share\alyssa\ctp_交互流程日志及快期\mytrader_cffexl_20150216.log

    使用M1模型，ctp仿真测试账号的测试日志，报撤单比较多，可以看看

【20150226】 
	1.	so对于系统处理时间的统计----已解决
	
【20150227】 
	1.	多测试用例读取顺序----已解决
	2.	支持DisconnectClient----已解决
	3.	ATF与so的通信----UDP不需要知道本地具体的IP
		1）ATF先启动
		2）so与ATF建立TCP连接，获取so的物理IP
		3）ATF与so已协商UDP server的port，ATF向so创建的UDP server发送行情
	4.	支持多测试用例----未完全解决
	
【20150228--20150301】 	
	1.	支持多测试用例
		1）问题：在发送行情前发送DisconnectClient----已修复
			当前的实现：Quote run()与sendQuote()在一个线程里，再一次循环中发送行情的条件不成立，下一次循环就执行了DisconnectClient
			修复方案：增加MSGID_S2C_QUOTE_SEND消息，在读到该消息时发送行情，阻塞到行情发生结束才处理下一个op
		2）QuoteSvr与Exchge结束和开始不同步----已修复
		
【20150302】 
	1.	so的log文件
	2.	ATF对于业务的细化
		1）填充LOGIN结构体
		2）报单结构体
			StatusMsg=报单已提交        StatusMsg=未成交            StatusMsg=未成交            StatusMsg=已撤单            StatusMsg=未成交            StatusMsg=全部成交
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
	    
 【20150303】  
	1.	对于业务的细化
		1）.	ATF目前对于报单的处理情况：当前报单处理后才能接收下一个报单
				问题：对于先后来多个报单的场景没法处理------已解决
				解决方案：采用一个待处理报单vector存放报单，每次处理报单时从vector中取出第一个处理
						  采用一个待处理撤单vector存放报单，每次处理撤单时从vector中取出第一个处理
		
		2）.	对于撤单，ATF目前都是用写死的方式处理------已解决(20150304)
				问题：不能根据实际的情况进行相应的处理
				解决方案：采用一个map存放已成交的报单，当收到撤单请求时从map中检查是否有该单
				
				
 【20150304】  
	1.	对于业务的细化				
		3）3组序列号所涉及的变量的类型------已解决
		
		
 【20150305】  
	1.	对于业务的细化	
		4）so将统计系统发送给ATF
		
		
		
 【20150308】  
	1.	so的代码和makefile文件整理
	2.	ATF到so的性能测试
		问题：1）ATF会core掉，core文件指示fread()
			  2）CPU调度不均衡，业务线程不能得到及时调度
		进展：SO侧接收行情的线程增加usleep(1000)主动释放CPU
				目前行情发送间隔20毫秒，整个性能测试过程没有出现任何问题

	
 【20150309】
	问题： 
		1.经过调试，行情发送间隔小于5毫秒时，性能测试出现问题
		2.功能性测试，当下单6个时，exchgSvr收到的报单数不对，有时2个、有时3个。
		3.端口设置为9W时，用netstat看到的端口和实际的不同 ---- 改为5W解决
		4.读取测试用例时不正确 ---- 测试用例写错了！！
		
 【20150310】
		1.经过调试，行情发送间隔1毫秒时，性能测试出现问题
			定位：pop的error比push的error大两个数量级，因此确认是由于从协议栈取行情push太慢了
			修改：	1.  udp server的timeout，由原来的2秒改为100毫秒
					2. QUEUE的大小由1024改为10240


		
		2.功能性测试，当下单6个时，exchgSvr收到的报单数不对，有时2个、有时3个。
		  ---- 将功能测试部分的接收函数由RcvMsgFromClient()改为用TcpConnector接收
		  ---- 性能测试部分要不要也改？
		       改了以后,在so侧出现“管道关闭”的错误，不知道什么导致
		  
		  
		2.行情发送100微秒时，又发生丢包，且atf出现core
		
 【20150311】	
		1.	行情发送100微秒时，又发生丢包，且atf出现core
			问题：core
			定位：在性能测试时，通过gdb发现m_traderMode被修改为LIMITED。通过watch继续定位，还是没有发现什么时候被修改的
			方案：将ExchgServer类的m_tcpbuf由栈改到堆上，core的问题就解决了
			
			问题：行情不能及时的处理
			定位：通过增加log发现，quote和order的queue都是pop错误，而无push错误，因此定位到是updserver从协议栈取行情的速度慢
			方案：将timeout改为5us，去掉循环的usleep
			进展：将行情发送间隔设为10us，也可以及时处理。
				  但是统计出来的行情发送间隔最小值为112us。quote和order的queue的pop错误仍有17W以上，说明行情的处理能力仍有很大空闲
			新问题：怎样使统计出来的行情发送间隔与实际的一致？
			
		2.	学习期货交易基本知识
		
		
 【20150311】	
		1. 学习期货交易基本知识
		
		2.	问题：行情发送间隔设为10us，但是统计出来的行情发送间隔平均值为122us。
			方案1：各线程绑定CPU，CPU使用率小于5%，但是对于行情的接收间隔没有影响。
			
 【20150312 -- 20150315】	
		1.	一个测试用例执行完后，系统会打印 9(Bad file descriptor)
		2.	测试用例执行完后，相关的状态未更新
	
 【20150325】	
		用gtest实现策略so，已验证方案可行
		
 【20150326】	
		用gtest实现策略so:
		1.	从文件里读取要执行的测试用例
		
 【20150408】
 ///查询投资者持仓
struct CThostFtdcQryInvestorPositionField
{
	///经纪公司代码
	TThostFtdcBrokerIDType	BrokerID;
	///投资者代码
	TThostFtdcInvestorIDType	InvestorID;
	///合约代码
	TThostFtdcInstrumentIDType	InstrumentID;
};

	///请求查询投资者持仓
	virtual int ReqQryInvestorPosition(CThostFtdcQryInvestorPositionField *pQryInvestorPosition, int nRequestID) ;
 
	///请求查询投资者持仓响应
	virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};
	
【20150608】
	TO DO:
	1）用例执行结果获取
	2）非阻塞sock
	3）CTP SO从本地文件读取行情数据，不再通过UDP
	
【20150615】
 TO DO:
	1)ATT支持退订合约
	2)根据测试场景实现模拟策略so
	  a)收到行情--发送交易信息--接收回报
	  b)收到行情--下单--撤单
	  c)收到行情--打印行情

	
【20150617】
 TO DO:
	1)ATT支持退订合约	
	
【20150619】
 TO DO:
	1)模拟策略增加此测试用例：每次同时产生买卖两个方向的信号（如买8、卖10），trader将发出三个请求：如买（8）／卖（2）／平（8）
	2)用例多次运行，检查内存offset是否正确
	3)budy/slab/kmalloc 查看内核内存
	4)整理已有测试用例，添加补充用例
	5)测试两个用例连续执行
	6)测试数据的路径
	7)行情断开重连，trader是否会重新登录、订阅？会
	8)通道断开重连，trader是否会重新登录？会

	
【20150619】
 TO DO:	
	1)FAK单，CTP校验和交易所校验都通过，最后1手都没有成交，是否有报单回报告知未成交？
		FAK单，完全未成交，会返回一个“已撤销”的回报
				部分成交，对未成交的部分会返回一个“已撤销”的回报
		FOK单，未成交，也会返回一个“已撤销”回报
	确认ATT是否正确实现此业务？

	2）MARKET单
	一个Market单，买10手；市场有卖一4手和卖二6手，全部成交，收到两个交易回报
	一个Market单，买10手；当前市场有卖一4手，4手立刻成交收到一个交易回报。过一会儿又收到6手的交易回报，价格为当日涨停价。
		
	SIG_STATUS_SUCCESS = 0, /* 报单全部成交 */
	SIG_STATUS_ENTRUSTED, /* 报单委托成功 */
	SIG_STATUS_PARTED, /* 报单部分成交 */
	SIG_STATUS_CANCEL, /* 报单被撤销 */
	SIG_STATUS_REJECTED, /* 报单被拒绝 */

	