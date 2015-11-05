#include "network.h"



int UdpCreateClient()
{
	int sock_fd;
	//struct sockaddr_in ser_addr;

	sock_fd = socket(AF_INET,SOCK_DGRAM,0);//udp
	if(sock_fd < 0)
	{
		printf("[%s] create socket error!\n",__FUNCTION__);
		return -1;	
	}
	return sock_fd;
	
}

int UdpCreateServer(int iPort)
{
	int sock_fd;
	struct sockaddr_in ser_addr;
	
	sock_fd = socket(AF_INET,SOCK_DGRAM,0);       //udp socket
	if(sock_fd < 0)
	{
		printf("[%s] create socket error\n",__FUNCTION__);
		return -1;
	}

	bzero(&ser_addr,sizeof(struct sockaddr_in));
	ser_addr.sin_family = AF_INET;                //internet domain
	ser_addr.sin_addr.s_addr = htonl(INADDR_ANY); //server side
	ser_addr.sin_port = htons(iPort) ;
	
	if(bind(sock_fd,(struct sockaddr*)&ser_addr,sizeof(struct sockaddr_in))<0)
	{
		printf("[%s] socket bind error,port=%d\n",__FUNCTION__,iPort);
		return -1;
	}
	return sock_fd;
}

int UdpSendData(int fd,char * ip,int port,char *pBuf,int iBufSize)
{
	int n = 0;
	struct sockaddr_in ser_addr;
	
	if(fd<0||ip==NULL||pBuf==NULL||iBufSize<1)
	{
		return -1;
	}

	bzero(&ser_addr,sizeof(struct sockaddr_in));
	ser_addr.sin_family = AF_INET;//internet-domain
	ser_addr.sin_addr.s_addr = inet_addr(ip);
	ser_addr.sin_port = htons(port);

	n = sendto(fd,pBuf,iBufSize,0,(struct sockaddr *)&ser_addr,sizeof(ser_addr));
	return n;
}

int UdpRecvData()
{
	return 0;
}

int TcpCreateServer(int iPort)
{
	int sock_fd;
	struct sockaddr_in ser_addr;
	
	sock_fd = socket(AF_INET,SOCK_STREAM,0);    //tcp socket
	if(sock_fd<0)
	{	
		printf("[%s] create socket error\n",__FUNCTION__);
		return -1;
	}	
	bzero(&ser_addr,sizeof(struct sockaddr_in));
	ser_addr.sin_family = AF_INET;                //internet domain
	ser_addr.sin_addr.s_addr = htonl(INADDR_ANY); //server side
	ser_addr.sin_port = htons(iPort) ;
	if(bind(sock_fd,(struct sockaddr*)&ser_addr,sizeof(struct sockaddr_in))<0)
	{	
		printf("[%s] socket bind error, iPort=%d\n",__FUNCTION__,iPort);
		return -1;
	}
	
	if (listen(sock_fd, BACKLOG) == -1) 	
	{
		printf("[%s]listen error!\n",__FUNCTION__);	
		close(sock_fd);		
		return -1;	
	}	

	return sock_fd;
}

int TcpCreateServer(char * ip,int iPort)
{
	int sock_fd;
	struct sockaddr_in ser_addr;

	if(ip == NULL || iPort < 0)
	{
		printf("[%s]Input param error,ip is NULL or port is less than zero!\n",__FUNCTION__);
		return -1;
	}
	
	sock_fd = socket(AF_INET,SOCK_STREAM,0);    //tcp socket
	if(sock_fd<0)
	{	
		printf("%s create socket error, %d(%s)\n",__FUNCTION__,errno,strerror(errno));
		return -1;
	}	

	bzero(&ser_addr,sizeof(struct sockaddr_in));
	ser_addr.sin_family = AF_INET;             //internet domain
	//ser_addr.sin_addr.s_addr=htonl(INADDR_ANY);  //server side
	ser_addr.sin_addr.s_addr = inet_addr(ip);
	ser_addr.sin_port = htons(iPort) ;
	
	if(bind(sock_fd,(struct sockaddr*)&ser_addr,sizeof(struct sockaddr_in))<0)
	{	
		printf("[%s] failed to bind socket %d for port %d, %d(%s)\n",__FUNCTION__,sock_fd,iPort,errno,strerror(errno));
		return -1;
	}
	
	if (listen(sock_fd, BACKLOG) == -1) 	
	{
		printf("[%s]failed to listen socket %d, %d(%s)\n",__FUNCTION__,sock_fd,errno,strerror(errno)); 
		close(sock_fd); 	
		return -1;	
	}	
	
	return sock_fd;
}

int TcpConnect(char * ip,int port)
{
	int sock_fd;
	struct sockaddr_in svr_addr;
	
	if(ip==NULL || port<0)
	{
		printf("[%s]Input param error,ip is NULL or port is less than zero!\n",__FUNCTION__);
		return -1;
	}

	sock_fd = socket(AF_INET,SOCK_STREAM,0);//tcp
	if(sock_fd<0)
	{
		printf("%s create socket error!\n",__FUNCTION__);
		return -1;	
	}
	
	bzero(&svr_addr,sizeof(struct sockaddr_in));
	svr_addr.sin_family = AF_INET;//internet-domain 
	svr_addr.sin_addr.s_addr = inet_addr(ip);
	svr_addr.sin_port = htons(port);

    if(TcpTimedConnect(sock_fd, (struct sockaddr*)&svr_addr, sizeof(struct sockaddr),3000) == 0) 
    {
        //printf("[%s]Connect to server (%s:%d) OK\n", __FUNCTION__,ip, port);
        return sock_fd;
    }
	
	return -1;
}

int TcpTimedConnect(int sock, const  struct sockaddr *addr,socklen_t addr_len, int timeout) /* timeout in millsec */
{
#ifdef LINUX 

    /* this may be GNU/Linux specific */
    int err;
    socklen_t errlen = sizeof(err);
    fd_set fds;
    struct timeval time_out ;
	
    int old_flags = fcntl(sock, F_GETFL);
    if (fcntl(sock, F_SETFL, O_NONBLOCK) == -1)
    {
        printf("[%s]%d(%s)\n",__FUNCTION__, errno,strerror(errno));
        return -1;
    }
    
    if (connect(sock, addr, addr_len) == -1 && errno != EINPROGRESS)
    {
        printf("[%s]Error in connect(), %d(%s)\n",__FUNCTION__, errno,strerror(errno));
        fcntl(sock, F_SETFL, old_flags);
        return -1;
    } 
    
    time_out.tv_sec = (timeout / 1000);
    time_out.tv_usec = (timeout % 1000) * 1000;
    
    FD_ZERO(&fds);
    FD_SET(sock, &fds);
    if (-1 == select(sock + 1, 0, &fds, 0, &time_out))
    {
        printf("[%s]connection timed out, %d(%s)\n",__FUNCTION__, errno,strerror(errno));
        fcntl(sock, F_SETFL, old_flags);
        return -1;
    } 
         
    err = ETIMEDOUT;  
    if (FD_ISSET(sock, &fds))
    {
        if (getsockopt(sock, SOL_SOCKET, SO_ERROR,  &err, &errlen) == -1)
        {
            printf("%d(%s)\n", errno,strerror(errno));
            return -1;
        }
    } 
	
    if (err == 0) /* succeeded */ 
        ;
    else
    {
        errno = err;
        printf("[%s]Connect failed, %d(%s)\n",__FUNCTION__,errno, strerror(errno));
        fcntl(sock, F_SETFL, old_flags);
        return -1;
    }       
    
    fcntl(sock, F_SETFL, old_flags);
    return 0;
#else
#warning not implemented on this platform!
    return connect(sock, addr, addr_len);
#endif    
}

/*
int TcpTimedAccept(int listen_fd,int & client_fd, int timeout)// timeout in millsec 
{
	struct timeval time_out;
	fd_set readfds;
	struct sockaddr_in cli_addr;
	char* pClientIp ;
	int ret;
	int client=-1;
	int clientPort = -1;

	
	socklen_t length = (socklen_t )sizeof(struct sockaddr_in);
    int old_flags = fcntl(listen_fd, F_GETFL);
    if (fcntl(listen_fd, F_SETFL, old_flags|O_NONBLOCK) == -1)
    {
        printf("%d(%s)\n", errno,strerror(errno));
        return -1;
    }

	do
	{
		time_out.tv_sec = (timeout / 1000);
		time_out.tv_usec = (timeout % 1000) * 1000;
	
		FD_ZERO(&readfds);
		FD_SET(listen_fd,&readfds);
		ret = select(listen_fd+1,&readfds,NULL,NULL,&time_out);
		if(ret<0)
		{
		   printf("[%s]select error !\n",__FUNCTION__);
		   close(listen_fd);
		   break;
		}
		else if (0 == ret)
		{
		   printf("[%s]select timeout !\n",__FUNCTION__);
		}
		else//can read
		{	
			if(FD_ISSET(listen_fd, &readfds))
			{
				if ((client = accept(listen_fd, (struct sockaddr *)&cli_addr, &length)) == -1)
				{
					printf("[%s]:accept error!\n",__FUNCTION__);
					continue;
				}
				
				client_fd = client;
				pClientIp = inet_ntoa(cli_addr.sin_addr);
				clientPort = ntohs(cli_addr.sin_port);
				//strcpy( &onlyone_client_ip[0], pClientIp);
				printf("[%s]:accept client:socket %d(ip:%s, port:%d)\n",__FUNCTION__,client_fd,pClientIp,clientPort);
			}
		}
	}while(0);

	fcntl(listen_fd, F_SETFL, old_flags);
	
	return 0;
}
*/


int TcpSendData(int fd, const char *buffer, int len)
{
    int send_len = 0;
    int total_len = 0;
    
    total_len = len;

    while(total_len > 0)
    {
        send_len = send(fd, buffer, total_len, 0);

        if(-1 == send_len)
        {
            if(errno == EINTR||errno == EAGAIN)
            {
                continue;
            }
            else
            {
                printf("[%s]sock:%d failed to send:%d(%s),len:%d\n",__FUNCTION__,fd,errno,strerror(errno),len);
                return -1;    
            }
        }

        buffer += send_len;
        total_len -= send_len;
    }
 
    return len;         
}

int TcpRecvData(int fd, char *buffer, int len)
{
    int n = 0;
    int ret = 0;
  
    while (n<len) 
    {
        ret = recv(fd, buffer, len-n, 0);
 
        if(ret < 0)
        {
            if(errno == EINTR)
            {
            	usleep(100);
                continue;
            }
			else if(errno == EAGAIN)
			{
				break;
			}
            else
		    {
                printf("[%s]recv error: %d(%s)\n",__FUNCTION__,errno,strerror(errno));
                return -1;
            }
        }
          
        n += ret;
        buffer += ret;
    }
    
    *buffer=0;
    return n;
}

int TcpTimedRecvData(int sock, char *buf, int ibufSize, int timeout)/* timeout in millsec */
{
    //int err;
    //socklen_t errlen = sizeof(err);
    fd_set readfds;
    struct timeval time_out ;
	int ret = 0;
	int iNotRcvSize = ibufSize;
	char * pBuf = &buf[0];
	int recvbytes = 0;
	int n = 0;
	
	//socklen_t length = (socklen_t )sizeof(struct sockaddr_in);
    int old_flags = fcntl(sock, F_GETFL);
    if (fcntl(sock, F_SETFL, O_NONBLOCK) == -1)
    {
        printf("%s %d(%s)\n",__FUNCTION__, errno,strerror(errno));
        return -1;
    }
	
    time_out.tv_sec = (timeout / 1000000);
    time_out.tv_usec = timeout;

	FD_ZERO(&readfds);
	FD_SET(sock,&readfds);
	
	ret = select(sock+1,&readfds,NULL,NULL,&time_out);
			
	if(ret<0)
	{
		printf("[%s]select error !\n",__FUNCTION__);
		fcntl(sock, F_SETFL, old_flags);
		close(sock);
		//break;
	}
	else if (ret==0)
	{
		//printf("[%s]select timeout !\n",__FUNCTION__);
	}
	else  //can read
	{	
		if(FD_ISSET(sock, &readfds))
		{
			memset(buf,0,ibufSize);
			
			while(recvbytes < ibufSize)
			{
				n = recv(sock, (void*)pBuf, iNotRcvSize, 0);

				if(n > 0)
				{
					recvbytes += n;

					if(recvbytes < ibufSize)
					{
						pBuf += n;
						iNotRcvSize -= n;
					}
					else if(recvbytes == ibufSize)
					{
						break;
					}
					else                    //recvbytes > iBufSize
					{
						recvbytes = -1;
						break;
					}
				}
				else
				{
					if(n<0)
					{
						/*if(errno == EAGAIN||errno == EWOULDBLOCK||errno == EINTR)
						{
							continue;//继续接收数据
							//EAGAIN: 套接字已标记为非阻塞，而接收操作被阻塞或者接收超时 
							//EINTR: 操作被信号中断
						}*/
						break;
					}
					else  //n=0代表关闭连接
					{
						close(sock);
						recvbytes = 0;
						break;           
					}
				}
				
			}
	
		}
	}

	fcntl(sock, F_SETFL, old_flags);

	return recvbytes;
}

TcpConnector::TcpConnector():m_connfd(-1),m_Begin(0),m_End(0)
{
	memset(m_acBuf,0,sizeof(m_acBuf));
}

TcpConnector::~TcpConnector()
{

}

void TcpConnector::Init(int fd)
{
	m_connfd =fd;
	m_Begin = 0;
	m_End = 0;
}

void TcpConnector::RecvData()
{
	if(m_connfd < 0)
	{
		printf("Error!TcpConnector::RecvData m_connfd<0\n");
		return;
	}
	
	if( m_Begin == m_End )
	{
		m_Begin = 0;
		m_End = 0;
	}
	
	//printf("TcpConnector::RecvData m_Begin=%d\n",m_Begin);
	
	int iRecvedBytes = 0;
	do
	{
		if( m_End == m_iUsrRecvBufSize )
		{
			if (m_Begin > 0) 
			{
				memmove(&m_acBuf[0], &m_acBuf[m_Begin], m_End - m_Begin);
				m_End -= m_Begin;
				m_Begin = 0;				
			}
			else//m_Begin equals 0 ,and m_End equals m_iUsrRecvBufSize, meaning buf is full!!
			{
				printf("TcpConnector::RecvData full!!! m_Begin=%d, m_End=%d\n",m_Begin,m_End);
				return ;
			}			
		}

		iRecvedBytes = recv(m_connfd, &m_acBuf[m_End],m_iUsrRecvBufSize-m_End, 0);
		if(iRecvedBytes > 0)
		{
			m_End += iRecvedBytes;
			//printf("=============TcpConnector::RecvData 1 iRecvedBytes=%d\n",iRecvedBytes);
		}
		/*else if( iRecvedBytes == 0 )
		{
			printf("=============TcpConnector::RecvData 2 iRecvedBytes=%d\n",iRecvedBytes);
			close(m_connfd);
			return ;
		}*/
		else if(errno == EINTR ||errno == EAGAIN)
		{
			continue;
		}
		else 
		{
			close(m_connfd);
			printf("error!TcpConnector::RecvData recv errno:%s!\n",strerror(errno));
			return ;
		}

	}while(iRecvedBytes>0);

	//printf("=============TcpConnector::RecvData 3 iRecvedBytes=%d\n",iRecvedBytes);

}

int TcpConnector::GetData(char * &rpcDataBeg, int len)
{
	
	rpcDataBeg = &m_acBuf[m_Begin];
	if(len <= m_End - m_Begin)
	{
		//printf("m_Begin=%5d,m_End=%5d, len=%d\n",m_Begin,m_End,len);
		return len;
	}

	return 0;
}

void TcpConnector::FlushData(int iFlushLength)
{
	if(m_acBuf == NULL )
	{
		return  ;
	}

	if(iFlushLength > m_End - m_Begin || iFlushLength == -1)
	{
		iFlushLength = m_End - m_Begin;
	}

	m_Begin += iFlushLength;

	if( m_Begin == m_End )
	{
		m_Begin = m_End = 0;
	}

	//printf("TcpConnector::FlushData   m_Begin=%d \n",m_Begin );
}

