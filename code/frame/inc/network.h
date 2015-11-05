#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <netinet/in.h>//socket(),bind(),recvfrom(), struct sockaddr,
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>//close
#include <fcntl.h>
#include <poll.h>
#include <sys/epoll.h>
#include <sched.h>
#include <string.h>//bzero()

#include "Thread.h"
#include "common.h"

#define BUFSIZE     2048
#define TCP_BUFSIZE 10240   //32*1024
#define UDP_BUFSIZE 10240   //32*1024
#define BACKLOG     10


template<typename T>
class CTcpServer : public CThread
{
public:
	CTcpServer();
	~CTcpServer();
	virtual void* Run_epoll(void *);
	virtual void* Run(void *);
	int Init(int iPort, LockFreeQueue<T> * stQueue, int iDataItemSize);

private:
	int m_iListenFd;
	LockFreeQueue<T> * m_pstQueue;
	int m_iDataItemSize;
};

template<typename T>
CTcpServer<T>::CTcpServer()
{
	m_iListenFd = -1;
}

template<typename T>
CTcpServer<T>::~CTcpServer()
{
	if(m_iListenFd != -1)
	{
		close(m_iListenFd);	
	}
}

template<typename T>
int CTcpServer<T>::Init(int iPort, LockFreeQueue<T> * stQueue, int iDataItemSize)
{
	int sock_fd;
	struct sockaddr_in ser_addr;
	
	m_pstQueue = stQueue;
	m_iDataItemSize = iDataItemSize;
	
	sock_fd = socket(AF_INET,SOCK_STREAM,0);//tcp socket
	if(sock_fd<0)
	{	
		g_log.error("%s create socket error\n",__FUNCTION__);
		return -1;
	}
	m_iListenFd = sock_fd;
	
	bzero(&ser_addr,sizeof(struct sockaddr_in));
	ser_addr.sin_family = AF_INET;               //internet domain
	ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);//server side
	ser_addr.sin_port = htons(iPort) ;
	if(bind(sock_fd,(struct sockaddr*)&ser_addr,sizeof(struct sockaddr_in))<0)
	{	
		g_log.error("%s  socket bind error\n",__FUNCTION__);
		return -1;
	}
	
	if (listen(sock_fd, BACKLOG) == -1) 	
	{
		g_log.error("listen error!\n");	
		close(sock_fd);		
		return -1;	
	}	

	g_log.info("Init iPort=%d\n",iPort);
	return 0;
}


template<typename T>
void* CTcpServer<T>::Run(void *)
{
	struct sockaddr_in cli_addr;
	char buf[TCP_BUFSIZE] = {0};
	int client_fd;
	int recvbytes;
	fd_set readfds;
	int old_flags;
	struct timeval time_out;
	socklen_t length = (socklen_t )sizeof(struct sockaddr_in);
	int timeout = 3;//3s
	timeout *= 1000;
	int ret = 0;
	
	while(1)
	{
		if (-1 == (client_fd = accept(m_iListenFd, (struct sockaddr *)&cli_addr, &length)))
		{
			printf("accept error!\n");
			return NULL;
		}
		
		while(1)
		{
			FD_ZERO(&readfds);
			time_out.tv_sec = (timeout / 1000);
			time_out.tv_usec = (timeout % 1000) * 1000;
			
			old_flags = fcntl(client_fd, F_GETFL, 0);
			fcntl(client_fd, F_SETFL, old_flags | O_NONBLOCK);
			FD_SET(client_fd,&readfds);
			
			ret = select(client_fd+1,&readfds,NULL,NULL,&time_out);
			if(ret < 0)
			{
			  printf("select error\n");
			  close(client_fd);
			  exit(0);
			}
			else if (ret==0)
			{
			  printf("timeout\n");
			}
			else //can read
			{
				memset(buf,0,TCP_BUFSIZE);
				if (-1 == (recvbytes = recv(client_fd, (void*)buf, TCP_BUFSIZE, 0)))
				{
					perror("recv error !");
					return 0;
				}
				//printf("recvbytes=%d Received:%s",recvbytes,buf);	  
			}
			
			if(recvbytes <=0 || recvbytes != m_iDataItemSize)
			{
				//g_log.error("recv size=%d ,m_iDataItemSize=%d!\n",recvbytes, m_iDataItemSize);
			}
			else if(recvbytes == m_iDataItemSize)
			{
				if(ATF_FAIL == (m_pstQueue->PushOneData((T*)buf)))
				{
					g_log.error("Failed to push data to a queue !");
				}
			}
		}

		sleep(2);
	}
	
	return NULL;
}

template<typename T>
void* CTcpServer<T>::Run_epoll(void *)
{
	int recvbytes = 0;
	struct sockaddr_in cli_addr;
	char buf[TCP_BUFSIZE] = {0};
	int client_fd;
	int old_flags;
	int epfds;//epoll fd set
	int nfds;
	struct epoll_event events[20];
	struct epoll_event event;

	
	socklen_t length = (socklen_t )sizeof(struct sockaddr_in);
	while(1)
	{
		if (-1 == (client_fd = accept(m_iListenFd, (struct sockaddr *)&cli_addr, &length)))
		{
			g_log.error("accept error!\n");
			sleep(2);
			continue;
		}
		
	 	old_flags = fcntl(client_fd, F_GETFL, 0);
		fcntl(client_fd, F_SETFL, old_flags | O_NONBLOCK);
		g_log.debug("try epoll_wait() fd:%d\n",client_fd);
		
		while(1)
		{
			epfds = epoll_create(256);

			event.data.fd = client_fd;
	 		event.events = EPOLLIN;
	 		epoll_ctl(epfds,EPOLL_CTL_ADD,client_fd,&event);  //add fd into epfds
	 
			nfds = epoll_wait(epfds,events,20,500);
			if(-1 == nfds)
			{
				perror("epoll error !");
				g_log.error("epoll error !\n");
				return 0;
			}
			else if(nfds==0)
			{
			}
			else
			{
				 if(events[0].data.fd == client_fd)
				 {
		
				 }
				 else if(events[0].events&EPOLLIN)
				 {
					memset(buf,0,TCP_BUFSIZE);
					if (-1 == (recvbytes = recv(client_fd, (void*)buf, TCP_BUFSIZE, 0)))
					{
						perror("recv error !");
						g_log.error("recv error !\n");
						return 0;
					}
					printf("Received: %s",buf);
					if(recvbytes != m_iDataItemSize)
					{
						g_log.error("recv size=%d ,m_iDataItemSize=%d!\n",recvbytes, m_iDataItemSize);

					}
					else
					{
						while(1)
						{
							if(ATF_FAIL == m_pstQueue->PushOneData((T*)buf))
							{
								g_log.error("Failed to push one data to queue !");
							}
						}
					}
				}
				else if(events[0].events&EPOLLOUT)
				{
		
				}
			}
		}

		close(client_fd);
	}
}


template<typename T>
class CUdpServer : public CThread
{
public:
	CUdpServer();
	~CUdpServer();
	int Init(int iPort, LockFreeQueue<T> * stQueue, int iDataItemSize);
	int Init(int iPort);
	virtual void* Run(void *);

private:
	int m_iListenFd;
	LockFreeQueue<T> * m_pstQueue;
	int m_iDataItemSize;
};

template<typename T>
CUdpServer<T>::CUdpServer()
{
	m_iListenFd = -1;
}

template<typename T>
CUdpServer<T>::~CUdpServer()
{
	if(m_iListenFd != -1)
	{
		close(m_iListenFd);	
	}
}

template<typename T>
int CUdpServer<T>::Init(int iPort, LockFreeQueue<T> * stQueue, int iDataItemSize)
{
	m_pstQueue = stQueue;
	m_iDataItemSize = iDataItemSize;
	return Init(iPort);
}

template<typename T>
int CUdpServer<T>::Init(int iPort)
{
	int sock_fd;
	struct sockaddr_in ser_addr;
	
	sock_fd = socket(AF_INET,SOCK_DGRAM,0);//udp socket
	if(sock_fd<0)
	{
		g_log.error("%s create socket error\n",__FUNCTION__);
		return ATF_FAIL;
	}
	m_iListenFd = sock_fd;
	g_log.info("UDP server create socket %d\n",sock_fd);
	
	bzero(&ser_addr,sizeof(struct sockaddr_in));
	ser_addr.sin_family = AF_INET;                //internet domain
	ser_addr.sin_addr.s_addr = htonl(INADDR_ANY); //server side
	ser_addr.sin_port = htons(iPort) ;
	
	if(bind(sock_fd,(struct sockaddr*)&ser_addr,sizeof(struct sockaddr_in))<0)
	{
		g_log.error("%s socket bind error\n",__FUNCTION__);
		return ATF_FAIL;
	}

	return ATF_SUCC;
}


template<typename T>
void* CUdpServer<T>::Run(void *)
{
	struct sockaddr_in cli_addr;
	T buf;
	int n;
	int old_flags;	
	fd_set readfds;
	struct timeval time_out;
	int timeout = 5;//unit: us
	
	socklen_t length = (socklen_t)sizeof(struct sockaddr_in);
	old_flags = fcntl(m_iListenFd, F_GETFL, 0);
	fcntl(m_iListenFd, F_SETFL, old_flags|O_NONBLOCK);
	g_log.info("%s udpserver startup....\n",__FUNCTION__);

	/*cpu_set_t mask;
    //int cpuNum = sysconf(_SC_NPROCESSORS_CONF);
    //printf("system has %d processor(s)\n", cpuNum);
	CPU_ZERO(&mask);
	CPU_SET(3, &mask);
	//cpuId++;
	if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) 
	{
		fprintf(stderr, "set thread affinity failed\n");
	}*/


	while(1)
	{
	    FD_ZERO(&readfds);
		
		time_out.tv_sec = 0;
		time_out.tv_usec = timeout;
		
	    FD_SET(m_iListenFd,&readfds);
	
	    int ret = select(m_iListenFd+1,&readfds,NULL,NULL,&time_out);
	    if(ret < 0)
	    {
		    printf("select error\n");
		    close(m_iListenFd);
		    exit(0);
	    }
	    else if (0 == ret)
	    {
			//printf("CUdpServer ,timeout\n" );
	    }
		else
		{
			n = recvfrom(m_iListenFd,&buf,sizeof(T),0,(struct sockaddr*)&cli_addr,&length);
			if(n < 0)
			{
				g_log.error("udpserver recvfrom error\n" );
				return NULL;
			}
			else if(n == 0)
			{
			}
			else
			{
				if(n == sizeof(T))
				{
					while(ATF_FAIL == m_pstQueue->PushOneData((T*)&buf))
					{
						usleep(500);
						if(PerfTest)
						{
							static long long ifail = 0;
							g_log.fatal("udpserver push error %lld\n",ifail);
							ifail++;
						}
					}
					g_log.debug("udpserver recv %dB\n",n);
				}
				else
				{
					g_log.error("udpserver recvfrom return %d, but sizeof(T)=%d\n",n,sizeof(T));
				}
			}
		}
		//usleep(10);

	}

	return NULL;
}


int UdpCreateClient();
int UdpCreateServer(int iPort);
int UdpSendData(int fd,char * ip,int port,char *pBuf,int iBufSize);
int UdpRecvData(int fd, char *pBuf,int iBufSize);

int TcpCreateServer(int iPort);
int TcpCreateServer(char * ip,int iPort);
int TcpConnect(char * ip,int port);
int TcpTimedConnect(int sock, const  struct sockaddr *addr,socklen_t addr_len, int timeout) ;/* timeout in millsec */
int TcpSendData(int fd, const char *pBuf,int iBufSize);
int TcpRecvData(int fd, char *pBuf,int iBufSize);
int TcpTimedRecvData(int sock, char *buf, int ibufSize, int timeout);


class TcpConnector
{
public:
	TcpConnector();
	~TcpConnector();
	void Init(int fd);
	void RecvData();
	int GetData(char * &rpcDataBeg, int len);
	void FlushData(int len);
	const static int m_iUsrRecvBufSize = 30*1024;
	int m_connfd;
	char m_acBuf[m_iUsrRecvBufSize];
	int m_Begin;
	int m_End;
};

#endif

