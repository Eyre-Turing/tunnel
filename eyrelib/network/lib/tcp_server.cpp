/*
 * Class TcpServer can start a tcp server.
 * The call back function `NewConnecting` will catch tcp client connect event.
 *
 * Author: Eyre Turing.
 * Last edit: 2021-06-23 17:47.
 */

#include "tcp_server.h"
#include "debug_settings.h"

#ifndef _WIN32
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define READBUFFER_SIZE	1024

void *TcpServer::Thread::selectThread(void *s)
{
	TcpServer *tcpServer = (TcpServer *) s;
	
	fd_set &readfds = tcpServer->m_readfds;
	fd_set testfds;
	
	struct sockaddr_in clientAddr;
	int clientSockfd, result, nread;
#ifdef _WIN32
	int clientLen;
#else
	unsigned int clientLen;
#endif
	
	char readBuffer[READBUFFER_SIZE+1];
	
	tcpServer->m_runStatus = TCP_SERVER_RUNNING;

	if(tcpServer->m_onStartSucceed)
	{
		tcpServer->m_onStartSucceed(tcpServer);
	}
	
	while(tcpServer->m_runStatus == TCP_SERVER_RUNNING)
	{
#ifdef _WIN32
		//remove each tcpServer->m_waitForRemoveSockfds.
		pthread_mutex_lock(&(tcpServer->m_waitForRemoveSockfdsMutex));
		while(!(tcpServer->m_waitForRemoveSockfds.empty()))
		{
			clientSockfd = tcpServer->m_waitForRemoveSockfds.front();
			closesocket(clientSockfd);
			FD_CLR(clientSockfd, &readfds);
			tcpServer->m_waitForRemoveSockfds.pop();
		}
		pthread_mutex_unlock(&(tcpServer->m_waitForRemoveSockfdsMutex));
#endif
		testfds = readfds;
		
#if NETWORK_DETAIL
		fprintf(stdout, "server wait.\n");
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
			break;
		}
		
		if(result == 0)
		{
#if NETWORK_DETAIL
			fprintf(stdout, "select timeout.\n");
#endif
			continue;
		}
		
#if NETWORK_DETAIL
		fprintf(stdout, "server handle client event.\n");
#endif
		
		pthread_mutex_lock(&(tcpServer->m_readfdsMutex));
#ifdef _WIN32
		for(int fd=0; fd<readfds.fd_count; ++fd)
#else
		for(int fd=0; fd<FD_SETSIZE; ++fd)
#endif
		{
#ifdef _WIN32
			SOCKET &curSockfd = readfds.fd_array[fd];
#else
			int &curSockfd = fd;
#endif
			if(FD_ISSET(curSockfd, &testfds))
			{
				//client connect.
				if(curSockfd == tcpServer->m_sockfd)
				{
					clientLen = sizeof(clientAddr);
					clientSockfd = accept(tcpServer->m_sockfd,
										(struct sockaddr *) &clientAddr,
										&clientLen);
					
					
					TcpSocket *tcpSocket = tcpServer->appendClient(clientSockfd);
					
					if(tcpServer->m_onNewConnecting)
					{
						tcpServer->m_onNewConnecting(tcpServer, tcpSocket);
					}
				}
				else	//client message.
				{
#ifdef _WIN32
					std::map<SOCKET, TcpSocket *>::iterator it = tcpServer->m_clientMap.find(curSockfd);
#else
					std::map<int, TcpSocket *>::iterator it = tcpServer->m_clientMap.find(curSockfd);
#endif

					if(it != tcpServer->m_clientMap.end())
					{
						TcpSocket *tcpSocket = it->second;
#ifdef _WIN32
						char *recvBuffer = tcpSocket->recvBuffer;
						//pthread_mutex_lock(&(tcpSocket->m_readWriteMutex)); 
						nread = recv(it->first, recvBuffer, tcpSocket->recvBufferSize, 0);
						//pthread_mutex_unlock(&(tcpSocket->m_readWriteMutex));
						if(nread > 0)
						{
							recvBuffer[nread] = 0;
							if(tcpSocket->m_onRead)
							{
								tcpSocket->m_onRead(tcpSocket, ByteArray(recvBuffer, nread));
							}
						}
						else
						{
							tcpServer->removeClient(curSockfd);
							--fd;
						}
#else
						//pthread_mutex_lock(&(tcpSocket->m_readWriteMutex));
						ioctl(curSockfd, FIONREAD, &nread);
						if(nread > 0)
						{
							ByteArray recvData;
							while(nread > 0)
							{
								int recvSize = recv(curSockfd, readBuffer, READBUFFER_SIZE, 0);
								recvData.append(readBuffer, recvSize);
								nread -= recvSize;
							}
							//pthread_mutex_unlock(&(tcpSocket->m_readWriteMutex));
							if(tcpSocket->m_onRead)
							{
								tcpSocket->m_onRead(tcpSocket, recvData);
							}
						}
						else
						{
							//pthread_mutex_unlock(&(tcpSocket->m_readWriteMutex));
							tcpServer->removeClient(curSockfd);
						}
#endif
					}
					else
					{
						//nread = recv(curSockfd, readBuffer, READBUFFER_SIZE, 0);
						
						//if(nread <= 0)
						//{
#ifdef _WIN32
						closesocket(curSockfd);
#else
						close(curSockfd);
#endif
						FD_CLR(curSockfd, &readfds);
#ifdef _WIN32
						--fd;
#endif
						//}
					}
				}
			}
		}
		pthread_mutex_unlock(&(tcpServer->m_readfdsMutex));
	}
	
	tcpServer->abort();
	
	return NULL;
}

TcpServer::TcpServer()
{
	m_onNewConnecting = NULL;
	m_onStartSucceed = NULL;
	m_onClosed = NULL;
	m_runStatus = TCP_SERVER_CLOSED;
	
	FD_ZERO(&m_readfds);
	
#ifdef _WIN32
	if(WSAStartup(MAKEWORD(1, 1), &m_wsadata) == SOCKET_ERROR)
	{
		fprintf(stderr, "TcpServer(%p) WSAStartup() fail!\n", this);
	}
	pthread_mutex_init(&m_waitForRemoveSockfdsMutex, NULL);
#endif
	pthread_mutex_init(&m_readfdsMutex, NULL);
	pthread_mutex_init(&m_clientMapMutex, NULL);
	pthread_mutex_init(&m_readfdsMutexInAppend, NULL);
	pthread_mutex_init(&m_readfdsMutexInRemove, NULL);
	
#if NETWORK_DETAIL
	fprintf(stdout, "TcpServer(%p) created.\n", this);
#endif
}

TcpServer::~TcpServer()
{
	abort();
#ifdef _WIN32
	WSACleanup();
	pthread_mutex_destroy(&m_waitForRemoveSockfdsMutex);
#endif
	pthread_mutex_destroy(&m_readfdsMutex);
	pthread_mutex_destroy(&m_clientMapMutex);
	pthread_mutex_destroy(&m_readfdsMutexInAppend);
	pthread_mutex_destroy(&m_readfdsMutexInRemove);
	
#if NETWORK_DETAIL
	fprintf(stdout, "TcpServer(%p) destroyed.\n", this);
#endif
}

int TcpServer::start(unsigned short port, int family, unsigned long addr, int backlog)
{
	abort();
	if((m_sockfd=socket(family, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "TcpServer(%p) socket() fail!\n", this);
		return TCP_SERVER_SOCKETFD_ERROR;
	}
	
#if NETWORK_DETAIL
	fprintf(stdout, "TcpServer(%p) m_sockfd created.\n", this);
#endif 
	
	struct sockaddr_in serverAddr;
	
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = family;
	serverAddr.sin_addr.s_addr = htonl(addr);
	serverAddr.sin_port = htons(port);
	
	if(bind(m_sockfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) != 0)
	{
#ifdef _WIN32
		closesocket(m_sockfd);
#else
		close(m_sockfd);
#endif
		fprintf(stderr, "TcpServer(%p) bind() fail!\n", this);
		return TCP_SERVER_BIND_ERROR;
	}
	
#if NETWORK_DETAIL
	fprintf(stdout, "TcpServer(%p) bind.\n", this);
#endif
	
	if(listen(m_sockfd, backlog) != 0)
	{
#ifdef _WIN32
		closesocket(m_sockfd);
#else
		close(m_sockfd);
#endif
		fprintf(stderr, "TcpServer(%p) listen() fail!\n", this);
		return TCP_SERVER_LISTEN_ERROR;
	}
	
#if NETWORK_DETAIL
	fprintf(stdout, "TcpServer(%p) set listen.\n", this);
#endif
	
	FD_ZERO(&m_readfds);
	FD_SET(m_sockfd, &m_readfds);
	
#if NETWORK_DETAIL
	fprintf(stdout, "TcpServer(%p) set m_readfds.\n", this);
#endif
	
	if(pthread_create(&m_listenThread, NULL, TcpServer::Thread::selectThread, this) != 0)
	{
#ifdef _WIN32
		closesocket(m_sockfd);
#else
		close(m_sockfd);
#endif
		fprintf(stderr, "TcpServer(%p) can not create thread!\n", this);
		return TCP_SERVER_CREATETHREAD_ERROR;
	}
	
#if NETWORK_DETAIL
	fprintf(stdout, "TcpServer(%p) started.\n", this);
#endif
	return TCP_SERVER_READYTORUN;
}

void TcpServer::abort()
{
	if(m_runStatus == TCP_SERVER_CLOSED)
	{
		return ;
	}
	m_runStatus = TCP_SERVER_CLOSED;
	pthread_mutex_lock(&m_readfdsMutex);
	pthread_mutex_lock(&m_clientMapMutex);
#ifdef _WIN32
	for(std::map<SOCKET, TcpSocket *>::iterator it=m_clientMap.begin();
#else
	for(std::map<int, TcpSocket *>::iterator it=m_clientMap.begin();
#endif
		it!=m_clientMap.end(); ++it)
	{
#ifdef _WIN32
		closesocket(it->first);
#else
		close(it->first);
#endif
		delete it->second;
	}
	m_clientMap.erase(m_clientMap.begin(), m_clientMap.end());
	pthread_mutex_unlock(&m_clientMapMutex);
	FD_ZERO(&m_readfds);
#ifdef _WIN32
	closesocket(m_sockfd);
#else
	close(m_sockfd);
#endif
	pthread_mutex_unlock(&m_readfdsMutex);
	
	if(m_onClosed)
	{
		m_onClosed(this);
	}
	
#if NETWORK_DETAIL
	fprintf(stdout, "TcpServer(%p) abort.\n", this);
#endif
}

#ifdef _WIN32
TcpSocket *TcpServer::appendClient(SOCKET clientSockfd)
#else
TcpSocket *TcpServer::appendClient(int clientSockfd)
#endif
{
	pthread_mutex_lock(&m_readfdsMutexInAppend);
	FD_SET(clientSockfd, &m_readfds);
	pthread_mutex_unlock(&m_readfdsMutexInAppend);
	pthread_mutex_lock(&m_clientMapMutex);
#ifdef _WIN32
	std::map<SOCKET, TcpSocket *>::iterator it = m_clientMap.find(clientSockfd);
#else
	std::map<int, TcpSocket *>::iterator it = m_clientMap.find(clientSockfd);
#endif
	if(it != m_clientMap.end())
	{
		pthread_mutex_unlock(&m_clientMapMutex);
		return it->second;
	}
	TcpSocket *tcpSocket = new TcpSocket(this, clientSockfd);
	m_clientMap[clientSockfd] = tcpSocket;
	pthread_mutex_unlock(&m_clientMapMutex);
	return tcpSocket;
}

#ifdef _WIN32
bool TcpServer::removeClient(SOCKET clientSockfd)
#else
bool TcpServer::removeClient(int clientSockfd)
#endif
{
	pthread_mutex_lock(&m_clientMapMutex);
#ifdef _WIN32
	std::map<SOCKET, TcpSocket *>::iterator it = m_clientMap.find(clientSockfd);
#else
	std::map<int, TcpSocket *>::iterator it = m_clientMap.find(clientSockfd);
#endif
	if(it == m_clientMap.end())
	{
		pthread_mutex_unlock(&m_clientMapMutex);
		return false;
	}
	TcpSocket *tcpSocket = it->second;
#ifdef _WIN32
	//closesocket(clientSockfd);
	//FD_CLR(clientSockfd, &m_readfds); //this operation need selectThread to do.
	pthread_mutex_lock(&m_waitForRemoveSockfdsMutex);
	m_waitForRemoveSockfds.push(clientSockfd);
	pthread_mutex_unlock(&m_waitForRemoveSockfdsMutex);
#else
	close(clientSockfd);
	pthread_mutex_lock(&m_readfdsMutexInRemove);
	FD_CLR(clientSockfd, &m_readfds);
	pthread_mutex_unlock(&m_readfdsMutexInRemove);
#endif
	tcpSocket->m_connectStatus = TCP_SOCKET_DISCONNECTED;
	if(tcpSocket->m_onDisconnected)
	{
		tcpSocket->m_onDisconnected(tcpSocket);
	}
	m_clientMap.erase(it);
	pthread_mutex_unlock(&m_clientMapMutex);
	delete tcpSocket;
	return true;
}

void TcpServer::setNewConnectingCallBack(NewConnecting newConnecting)
{
	m_onNewConnecting = newConnecting;
#if NETWORK_DETAIL
	fprintf(stdout, "TcpServer(%p) set new connecting call back.\n", this);
#endif 
}

void TcpServer::setStartSucceedCallBack(StartSucceed startSucceed)
{
	m_onStartSucceed = startSucceed;
}

void TcpServer::setClosedCallBack(Closed closed)
{
	m_onClosed = closed;
}

int TcpServer::runStatus() const
{
	return m_runStatus;
}
