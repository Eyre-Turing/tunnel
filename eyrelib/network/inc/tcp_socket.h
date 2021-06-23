#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

/*
 * For send and recive serve's message easily.
 * All message is ByteArray, so need Eyre Turing lib framework.
 * Compile need -lpthread.
 * MinGW compile need -lws2_32.
 *
 * Author: Eyre Turing.
 * Last edit: 2021-06-23 17:45.
 */

#ifdef _WIN32
#include <winsock.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#endif	//_WIN32

#include <pthread.h>
#include "byte_array.h"

#define TCP_SOCKET_DISCONNECTED	0
#define TCP_SOCKET_CONNECTED		1

#define TCP_SOCKET_READYTOCONNECT	0	//aka succeed.
#define TCP_SOCKET_GETADDRINFO_ERROR	(-1)	//aka maybe server's ip or port your input is error.
#define TCP_SOCKET_SOCKETFD_ERROR	(-2)
//#define TCP_SOCKET_SETSOCKOPT_ERROR	(-3)
#define TCP_SOCKET_CREATETHREAD_ERROR	(-4)
#define TCP_SOCKET_ISSERVER_ERROR	(-5)	//aka this is serve's socket, peer is client, can't connect to other server.

class TcpServer;
#include "tcp_server.h"

class TcpSocket
{
public:
	typedef void (*Disconnected)(TcpSocket *s);
	typedef void (*Connected)(TcpSocket *s);
	typedef void (*Read)(TcpSocket *s, ByteArray data);
	typedef void (*ConnectError)(TcpSocket *s, int errorStatus);
	
	TcpSocket();
	virtual ~TcpSocket();
	
	//will return error type, if return TCP_SOCKET_READYTOCONNECT {aka 0} is succeed.
	int connectToHost(const char *addr, unsigned short port, int family=AF_INET);

	//if this is a server's socket connect from a client, the function will `delete this`.
	void abort();
	
	bool write(const ByteArray &data);
	bool write(const char *data, unsigned int size=0xffffffff);
	
	void setDisconnectedCallBack(Disconnected disconnected);
	void setConnectedCallBack(Connected connected);
	void setReadCallBack(Read read);
	void setConnectErrorCallBack(ConnectError connectError);
	
	int connectStatus() const;

	String getPeerIp() const;
	unsigned short getPeerPort() const;
	
	/*
	 * If this is a server's socket connect from a client,
	 * the function can get the manage server.
	 * Else return NULL.
	 */
	TcpServer *server() const;
	
	class Thread
	{
	public:
		static void *connectThread(void *s);
		static void *readThread(void *s);
	};
	
	friend class TcpServer;
	
private:
#ifdef _WIN32
	WSADATA m_wsadata;
	char *recvBuffer;
	int recvBufferSize;
	SOCKET m_sockfd;
#else
	int m_sockfd;
#endif
	struct addrinfo *m_res;
	int m_connectStatus;
	
	pthread_t m_connectThread;
	pthread_t m_readThread;
	
	Disconnected m_onDisconnected;
	Connected m_onConnected;
	Read m_onRead;
	ConnectError m_onConnectError;
	
	TcpServer *m_server;
	
	TcpSocket(TcpServer *server, int sockfd);
	
	//pthread_mutex_t m_readWriteMutex;
};

#endif	//TCP_SOCKET_H 
