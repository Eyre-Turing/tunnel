/*
 * Class TcpSocket can connect to server and send, recive data.
 * The call back function `Read` will exec in a subthread.
 *
 * Author: Eyre Turing.
 * Last edit: 2021-06-13 17:10.
 */

#include "tcp_socket.h"
#include "debug_settings.h"

#ifdef _WIN32
#include <ws2tcpip.h>
#else
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netdb.h>
#endif	//_WIN32

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "eyre_string.h"

#define ONCE_READ	1024

void *TcpSocket::Thread::connectThread(void *s)
{
	TcpSocket *tcpSocket = (TcpSocket *) s;
	
	if(connect(tcpSocket->m_sockfd, tcpSocket->m_res->ai_addr, tcpSocket->m_res->ai_addrlen) < 0)
	{
		int errorStatus = errno;
		fprintf(stderr, "connectThread error: %s\n", strerror(errorStatus));
		if(tcpSocket->m_onConnectError)
		{
			tcpSocket->m_onConnectError(tcpSocket, errorStatus);
		}
	}
	else
	{
		tcpSocket->m_connectStatus = TCP_SOCKET_CONNECTED;
		
		if(pthread_create(&(tcpSocket->m_readThread), NULL,
						TcpSocket::Thread::readThread, s) != 0)
		{
			tcpSocket->abort();
			if(tcpSocket->m_onConnectError)
			{
				tcpSocket->m_onConnectError(tcpSocket, TCP_SOCKET_CREATETHREAD_ERROR);
			}
		}
		else if(tcpSocket->m_onConnected)
		{
			tcpSocket->m_onConnected(tcpSocket);
		}
	}
	
	return NULL;
}

void *TcpSocket::Thread::readThread(void *s)
{
	TcpSocket *tcpSocket = (TcpSocket *) s;
	
	fd_set readfds, testfds;
	FD_ZERO(&readfds);
	FD_SET(tcpSocket->m_sockfd, &readfds);
	
	char buffer[ONCE_READ];
	int size, result;
	while(tcpSocket->m_connectStatus == TCP_SOCKET_CONNECTED)
	{
		testfds = readfds;
		
#if NETWORK_DETAIL
		fprintf(stdout, "tcpSocket(%p) wait.\n", tcpSocket);
#endif
		
#ifdef _WIN32
		TIMEVAL timeout;
		timeout.tv_sec = NETWORK_TIMEOUT;
		timeout.tv_usec = 0;
		result = select(0, &testfds, NULL, NULL, &timeout);
#else
		struct timeval timeout;
		timeout.tv_sec = NETWORK_TIMEOUT;
		timeout.tv_usec = 0;
		result = select(FD_SETSIZE, &testfds, (fd_set *) 0, (fd_set *) 0, &timeout);
#endif

		if(result < 0)
		{
			perror("select");
			tcpSocket->abort();
			break;
		}
		
		if(result == 0)
		{
#if NETWORK_DETAIL
			fprintf(stdout, "tcpSocket(%p) select timeout.\n", tcpSocket);
#endif
			continue;
		}
		
		if(!FD_ISSET(tcpSocket->m_sockfd, &testfds))
		{
			fprintf(stderr, "tcpSocket(%p) select unknow error!\n", tcpSocket);
			continue;
		}
		
#ifdef _WIN32
		pthread_mutex_lock(&(tcpSocket->m_readWriteMutex));
		size = recv(tcpSocket->m_sockfd, tcpSocket->recvBuffer, tcpSocket->recvBufferSize, 0);
		pthread_mutex_unlock(&(tcpSocket->m_readWriteMutex));
		if(size > 0)
		{
			if(tcpSocket->m_onRead)
			{
				tcpSocket->m_onRead(tcpSocket, ByteArray(tcpSocket->recvBuffer, size));
			}
		}
		else
		{
			tcpSocket->abort();
		}
#else
		pthread_mutex_lock(&(tcpSocket->m_readWriteMutex));
		ioctl(tcpSocket->m_sockfd, FIONREAD, &size);
		if(size > 0)
		{
			ByteArray recvData;
			while(size > 0)
			{
				int recvSize = recv(tcpSocket->m_sockfd, buffer, ONCE_READ, 0);
				recvData.append(buffer, recvSize);
				size -= recvSize;
			}
			pthread_mutex_unlock(&(tcpSocket->m_readWriteMutex));
			if(tcpSocket->m_onRead)
			{
				tcpSocket->m_onRead(tcpSocket, recvData);
			}
		}
		else
		{
			pthread_mutex_unlock(&(tcpSocket->m_readWriteMutex));
			tcpSocket->abort();
		} 
#endif
	}
	return NULL;
}

TcpSocket::TcpSocket()
{
	m_res = NULL;
	m_server = NULL;
	
	m_onDisconnected = NULL;
	m_onConnected = NULL;
	m_onRead = NULL;
	m_onConnectError = NULL;
	
	m_connectStatus = TCP_SOCKET_DISCONNECTED;
	
#ifdef _WIN32
	if(WSAStartup(MAKEWORD(1, 1), &m_wsadata) == SOCKET_ERROR)
	{
		fprintf(stderr, "TcpSocket(%p) WSAStartup() fail!\n", this);
	}
	recvBuffer = NULL;
#endif

#if NETWORK_DETAIL
	fprintf(stdout, "TcpSocket(%p) created as a client.\n", this);
#endif

	pthread_mutex_init(&m_readWriteMutex, NULL);
}

TcpSocket::TcpSocket(TcpServer *server, int sockfd) : m_server(server), m_sockfd(sockfd)
{
	m_res = NULL;
	
	m_onDisconnected = NULL;
	m_onConnected = NULL;
	m_onRead = NULL;
	m_onConnectError = NULL;
	
	m_connectStatus = TCP_SOCKET_CONNECTED;
	
#ifdef _WIN32
	int optLen = sizeof(recvBufferSize);
	getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char *) &recvBufferSize, &optLen);
	recvBuffer = (char *) malloc(recvBufferSize+1);
#endif

#if NETWORK_DETAIL
	fprintf(stdout, "TcpSocket(%p) created as a server(%p)\'s socket.\n", this, server);
#endif

	pthread_mutex_init(&m_readWriteMutex, NULL);
}

TcpSocket::~TcpSocket()
{
	if(!m_server)
	{
		abort();
	}
#ifdef _WIN32
	free(recvBuffer);
	if(!m_server)
	{
		WSACleanup();
	}
#endif

#if NETWORK_DETAIL
	fprintf(stdout, "TcpSocket(%p) destroyed.\n", this);
#endif

	pthread_mutex_destroy(&m_readWriteMutex);
}

int TcpSocket::connectToHost(const char *addr, unsigned short port, int family)
{
	if(m_server)
	{
		fprintf(stderr, "warning: this(%p) is a server socket, can not connect to other server!\n", this);
		return TCP_SOCKET_ISSERVER_ERROR;
	}
	abort();
	struct addrinfo hints = {0};
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = family;
	int getErr = getaddrinfo(addr, String::fromNumber(port), &hints, &m_res);
	if(getErr)
	{
		m_res = NULL;
		fprintf(stderr, "TcpSocket(%p)::connectToHost getaddrinfo() error: %s\n",
				this, gai_strerror(getErr));
		return TCP_SOCKET_GETADDRINFO_ERROR;
	}
	m_sockfd = socket(m_res->ai_family, m_res->ai_socktype, m_res->ai_protocol);
	if(m_sockfd < 0)
	{
		if(m_res)
		{
			freeaddrinfo(m_res);
			m_res = NULL;
		}
		fprintf(stderr, "TcpSocket(%p)::connectToHost socket() error: %s\n",
				this, strerror(errno));
		return TCP_SOCKET_SOCKETFD_ERROR;
	}

#ifdef _WIN32
	if(recvBuffer == NULL)
	{
		int optLen = sizeof(recvBufferSize);
		getsockopt(m_sockfd, SOL_SOCKET, SO_RCVBUF, (char *) &recvBufferSize, &optLen);
		recvBuffer = (char *) malloc(recvBufferSize+1);
	}
#endif
	
//#ifdef _WIN32
//	int timeout = NETWORK_TIMEOUT*1000;
//#else
//	struct timeval timeout;
//	timeout.tv_sec = NETWORK_TIMEOUT;
//	timeout.tv_usec = 0;
//#endif
	
//	if(setsockopt(m_sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout)) < 0)
//	{
//		if(m_res)
//		{
//			freeaddrinfo(m_res);
//			m_res = NULL;
//		}
//#ifdef _WIN32
//		closesocket(m_sockfd);
//#else
//		close(m_sockfd);
//#endif
//		fprintf(stderr, "TcpSocket(%p) set no block error!\n", this);
//		return TCP_SOCKET_SETSOCKOPT_ERROR;
//	}
	
	if(pthread_create(&m_connectThread, NULL, TcpSocket::Thread::connectThread, this) != 0)
	{
		if(m_res)
		{
			freeaddrinfo(m_res);
			m_res = NULL;
		}
#ifdef _WIN32
		closesocket(m_sockfd);
#else
		close(m_sockfd);
#endif
		fprintf(stderr, "TcpSocket(%p) can not create thread for connect to host!\n", this);
		return TCP_SOCKET_CREATETHREAD_ERROR;
	}
	
	return TCP_SOCKET_READYTOCONNECT;
}

void TcpSocket::abort()
{
	if(m_connectStatus == TCP_SOCKET_DISCONNECTED)
	{
		return ;
	}
	
	if(m_res)
	{
		freeaddrinfo(m_res);
		m_res = NULL;
	}
	
	if(m_server)
	{
		m_server->removeClient(m_sockfd);
	}
	else
	{
#ifdef _WIN32
		closesocket(m_sockfd);
#else
		close(m_sockfd);
#endif	//_WIN32

		m_connectStatus = TCP_SOCKET_DISCONNECTED;
		if(m_onDisconnected)
		{
			m_onDisconnected(this);
		}
		
#if NETWORK_DETAIL
		fprintf(stdout, "TcpSocket(%p) abort.\n", this);
#endif
	}
}

void TcpSocket::setDisconnectedCallBack(Disconnected disconnected)
{
	m_onDisconnected = disconnected;
}

void TcpSocket::setConnectedCallBack(Connected connected)
{
	m_onConnected = connected;
}

void TcpSocket::setReadCallBack(Read read)
{
	m_onRead = read;
}

void TcpSocket::setConnectErrorCallBack(ConnectError connectError)
{
	m_onConnectError = connectError;
}

bool TcpSocket::write(const ByteArray &data)
{
	return write(data, data.size());
}

bool TcpSocket::write(const char *data, unsigned int size)
{
	if(size == 0xffffffff)
	{
		size = strlen(data);
	}
	pthread_mutex_lock(&m_readWriteMutex);
	bool result = send(m_sockfd, data, size, 0)>0;
	pthread_mutex_unlock(&m_readWriteMutex);
	return result;
}

int TcpSocket::connectStatus() const
{
	return m_connectStatus;
}

String TcpSocket::getPeerIp() const
{
	struct sockaddr_in addr;
#ifdef _WIN32
	int len = sizeof(addr);
#else
	unsigned int len = sizeof(addr);
#endif
	if(getpeername(m_sockfd, (struct sockaddr *) &addr, &len) == 0)
	{
		return String(inet_ntoa(addr.sin_addr), CODEC_UTF8);
	}
	else
	{
		return "";
	}
}

unsigned short TcpSocket::getPeerPort() const
{
	struct sockaddr_in addr;
#ifdef _WIN32
	int len = sizeof(addr);
#else
	unsigned int len = sizeof(addr);
#endif
	if(getpeername(m_sockfd, (struct sockaddr *) &addr, &len) == 0)
	{
		return ntohs(addr.sin_port);
	}
	else
	{
		return 0;
	}
}

TcpServer *TcpSocket::server() const
{
	return m_server;
}
