/*
 * Author: Eyre Turing.
 * Last edit: 2021-01-11 16:55.
 */

#include "udp_socket.h"
#include "debug_settings.h"

#ifdef _WIN32
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <netdb.h>
#endif	//_WIN32

#include <stdio.h>
#include <string.h>

#include "eyre_string.h"

#define DGRAM_MAX_SIZE	 102400	//set max is 100k, although one udp dgram real max size is 64k.

void *UdpSocket::Thread::readThread(void *s)
{
	UdpSocket *udpSocket = (UdpSocket *) s;
	char buffer[DGRAM_MAX_SIZE];
	int size;
	struct sockaddr from;
#ifdef _WIN32
	int len = sizeof(from);
#else
	unsigned int len = sizeof(from);
#endif
	struct sockaddr_in sin;
	while(udpSocket->m_isBound)
	{
		size = recvfrom(udpSocket->m_sockfd, buffer, DGRAM_MAX_SIZE, 0, &from, &len);
		if(size < 0)
		{
#if NETWORK_DETAIL
			fprintf(stdout, "UdpSocket(%p) recvfrom timeout.\n", udpSocket);
#endif 
			continue ;
		}
		if(udpSocket->m_onRead)
		{
			memcpy(&sin, &from, sizeof(sin));
			udpSocket->m_onRead(udpSocket, String(inet_ntoa(sin.sin_addr), CODEC_UTF8), ntohs(sin.sin_port), ByteArray(buffer, size));
		}
	}
#if NETWORK_DETAIL
	fprintf(stdout, "UdpSocket(%p) read thread quit.\n", udpSocket);
#endif
	return NULL;
}

UdpSocket::UdpSocket()
{
	m_isBound = false;
	m_port = 0;
	
	m_onRead = NULL;
	
#ifdef _WIN32
	if(WSAStartup(MAKEWORD(1, 1), &m_wsadata) == SOCKET_ERROR)
	{
		fprintf(stderr, "TcpSocket(%p) WSAStartup() fail!\n", this);
	}
#endif
	
#if NETWORK_DETAIL
	fprintf(stdout, "UdpSocket(%p) created.\n", this);
#endif
}

UdpSocket::~UdpSocket()
{
	unbind();
	
#ifdef _WIN32
	WSACleanup();
#endif
	
#if NETWORK_DETAIL
	fprintf(stdout, "UdpSocket(%p) destroyed.\n", this);
#endif
}

bool UdpSocket::start(unsigned short port, int family, unsigned long addr)
{
	unbind();
	struct sockaddr_in socketAddr;
	memset(&socketAddr, 0, sizeof(socketAddr));
	socketAddr.sin_family = family;
	socketAddr.sin_addr.s_addr = htonl(addr);
	socketAddr.sin_port = htons(port);
	m_sockfd = socket(family, SOCK_DGRAM, 0);
	if(m_sockfd < 0)
	{
		return false;
	}
	if(bind(m_sockfd, (struct sockaddr *) &socketAddr, sizeof(socketAddr)) != 0)
	{
#ifdef _WIN32
		closesocket(m_sockfd);
#else
		close(m_sockfd);
#endif
		fprintf(stderr, "UdpSocket(%p) bind error!\n", this);
		return false;
	}
	
#ifdef _WIN32
	int timeout = NETWORK_TIMEOUT*1000;
#else
	struct timeval timeout;
	timeout.tv_sec = NETWORK_TIMEOUT;
	timeout.tv_usec = 0;
#endif
	
	if(setsockopt(m_sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout)) < 0)
	{
#ifdef _WIN32
		closesocket(m_sockfd);
#else
		close(m_sockfd);
#endif
		fprintf(stderr, "UdpSocket(%p) set no block error!\n", this);
		return false;
	}
	
	if(pthread_create(&m_readThread, NULL, UdpSocket::Thread::readThread, this) != 0)
	{
#ifdef _WIN32
		closesocket(m_sockfd);
#else
		close(m_sockfd);
#endif
		fprintf(stderr, "UdpSocket(%p) can not create thread!\n", this);
		return false;
	}
	
	m_isBound = true;
	m_port = port;
	
	return true;
}

void UdpSocket::unbind()
{
	if(!m_isBound)
	{
		return ;
	}
#ifdef _WIN32
	closesocket(m_sockfd);
#else
	close(m_sockfd);
#endif
	m_isBound = false;
	m_port = 0;
}

bool UdpSocket::send(const char *addr, unsigned short port, const ByteArray &data) const
{
	return send(addr, port, data, data.size());
}

bool UdpSocket::send(const char *addr, unsigned short port,
					const char *data, unsigned int size) const
{
	if(size == 0xffffffff)
	{
		size = strlen(data);
	}
	struct addrinfo hints = {0};
	struct addrinfo *res = NULL;
	hints.ai_socktype = SOCK_DGRAM;
	if(getaddrinfo(addr, String::fromNumber(port), &hints, &res) != 0)
	{
		fprintf(stderr, "UdpSocket(%p) send getaddrinfo error!\n", this);
		return false;
	}
	int sendsize = sendto(m_sockfd, data, size, 0, res->ai_addr, res->ai_addrlen);
	freeaddrinfo(res);
	return sendsize>=0;
}

void UdpSocket::setReadCallBack(Read read)
{
	m_onRead = read;
}

bool UdpSocket::isBound() const
{
	return m_isBound; 
}
